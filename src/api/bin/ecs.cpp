/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <concurrentqueue/moodycamel/concurrentqueue.h>
#include <library/fast_task/include/future.hpp>
#include <library/list_array.hpp>
#include <new>
#include <span>
#include <src/api/ecs.hpp>
#include <unordered_set>

namespace copper_server::api::ecs {
    struct archetype_hash {
        static constexpr inline auto golden_ratio = 0x9e3779b9;

        size_t operator()(const std::vector<component_id>& ids) const {
            size_t seed = ids.size();
            for (auto id : ids)
                seed ^= id + golden_ratio + (seed << 6) + (seed >> 2);
            return seed;
        }

        size_t operator()(component_id* ids, size_t size) const {
            size_t seed = size;
            for (size_t i = 0; i < size; i++)
                seed ^= ids[i] + golden_ratio + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    constexpr uint32_t CHUNK_CAPACITY = 256;

    struct chunk {
        std::unique_ptr<char[]> memory_block;
        std::optional<int32_t> world_bind;
        uint32_t entity_count = 0;
        uint32_t last_free_list_index = 0;
        uint32_t global_index = 0;

        int32_t* entities() {
            return reinterpret_cast<int32_t*>(memory_block.get());
        }

        std::atomic_uint64_t* atomic_drity_flags(size_t offset) {
            return reinterpret_cast<std::atomic_uint64_t*>(memory_block.get() + offset);
        }

        chunk() = default;

        chunk(archetype* arch, std::optional<int32_t> world_id)
            : memory_block(std::make_unique_for_overwrite<char[]>(arch->layout.chunk_size_bytes)),
              world_bind(world_id) {}

        bool has_free_slot() {
            return entity_count < CHUNK_CAPACITY;
        }
    };

    struct archetype {
        std::vector<component_id> component_ids;
        size_t hash;
        std::vector<std::unique_ptr<chunk>> chunks;

        /**
         * @brief A non-owning free-list of indices pointing to chunks in the main 
         * `archetype::chunks` vector that have at least one empty slot.
         * @details We use a vector of indices instead of direct pointers to avoid issues
         * with vector reallocation. This allows for fast, O(1) retrieval of a free chunk
         * by popping from the back of this list.
         */
        std::vector<uint32_t> global_available_chunks;
        std::unordered_map<int32_t, std::vector<uint32_t>> world_available_chunks;

        std::unordered_map<component_id, archetype*> add_transition_cache;
        std::unordered_map<component_id, archetype*> remove_transition_cache;
        std::unordered_map<component_id, uint32_t> component_index_map;

        struct chunk_layout {
            std::vector<size_t> component_offsets;
            std::vector<size_t> dirty_flags_offsets;
            size_t chunk_size_bytes = 0;
        };

        chunk_layout layout;
        fast_task::task_mutex arch_mutex;

        std::span<std::atomic_uint64_t> dirty_flags(chunk* chunk, size_t component_index) {
            auto atomics = reinterpret_cast<std::atomic_uint64_t*>(chunk->memory_block.get() + layout.dirty_flags_offsets[component_index]);
            return {atomics, CHUNK_CAPACITY / 64};
        }

        std::vector<std::span<std::atomic_uint64_t>> dirty_flags(chunk* chunk) {
            std::vector<std::span<std::atomic_uint64_t>> res;
            for (size_t i = 0; i < component_ids.size(); ++i) {
                auto atomics = reinterpret_cast<std::atomic_uint64_t*>(chunk->memory_block.get() + layout.dirty_flags_offsets.at(i));
                res.push_back({atomics, CHUNK_CAPACITY / 64});
            }
            return res;
        }

        void mark_dirty(chunk* chunk, uint32_t component_index, size_t entity_pos) {
            size_t word_index = entity_pos / 64;
            uint64_t bit_mask = 1ULL << (entity_pos % 64);
            dirty_flags(chunk, component_index)[word_index].fetch_or(bit_mask, std::memory_order_relaxed);
        }

        void mark_dirty_entities(chunk* chunk, uint32_t component_index) {
            auto flags = dirty_flags(chunk, component_index);
            for (size_t i = 0; i < CHUNK_CAPACITY / 64; ++i)
                flags[i].store(UINT64_MAX, std::memory_order_relaxed);
        }

        void clear_mark_dirty_entities(chunk* chunk, uint32_t component_index) {
            auto flags = dirty_flags(chunk, component_index);
            for (size_t i = 0; i < CHUNK_CAPACITY / 64; ++i)
                flags[i].store(0, std::memory_order_relaxed);
        }

        bool is_dirty(chunk* chunk, uint32_t component_index, size_t entity_pos) {
            size_t word_index = entity_pos / 64;
            uint64_t bit_mask = 1ULL << (entity_pos % 64);
            return dirty_flags(chunk, component_index)[word_index].load() & bit_mask;
        }

        void calculate_layout() {
            if (layout.chunk_size_bytes == 0) {
                layout.component_offsets.resize(component_ids.size());
                layout.dirty_flags_offsets.resize(component_ids.size());
                size_t current_offset = sizeof(int32_t) * CHUNK_CAPACITY;
                for (size_t i = 0; i < component_ids.size(); ++i) {
                    component_id id = component_ids[i];
                    const auto& info = detail::component_info_registry[id];

                    current_offset = (current_offset + info.alignment - 1) & ~(info.alignment - 1);

                    layout.component_offsets[i] = current_offset;
                    current_offset += info.size * CHUNK_CAPACITY;
                    component_index_map[id] = i;
                }

                constexpr auto atomics_aligment = std::hardware_destructive_interference_size;
                constexpr auto atomics_map_size = (CHUNK_CAPACITY + (sizeof(std::atomic_uint64_t) * 8) - 1) / (sizeof(std::atomic_uint64_t) * 8);

                for (size_t i = 0; i < component_ids.size(); ++i) {
                    current_offset = (current_offset + atomics_aligment - 1) & ~(atomics_aligment - 1);
                    layout.dirty_flags_offsets[i] = current_offset;
                    current_offset += atomics_map_size;
                }

                layout.chunk_size_bytes = current_offset;
            }
        }

        bool matches_query(std::span<component_id> components, std::span<component_id> without_components) {
            for (auto component : components)
                if (!component_index_map.contains(component))
                    return false;
            for (auto component : without_components)
                if (component_index_map.contains(component))
                    return false;
            return true;
        }
    };

    //tries to get free chunk from allocated ones and allocates new one if there no free chunks for world or global space
    chunk* get_free_chunk(archetype* arch, std::optional<int32_t> world_id) {
        if (world_id) {
            auto& list = arch->world_available_chunks[*world_id];
            if (!list.empty())
                return arch->chunks[list.back()].get();
        } else {
            auto& list = arch->global_available_chunks;
            if (!list.empty())
                return arch->chunks[list.back()].get();
        }

        auto new_chunk = std::make_unique<chunk>(arch, world_id);
        chunk* chunk_ptr = new_chunk.get();

        arch->chunks.push_back(std::move(new_chunk));
        uint32_t new_chunk_index = arch->chunks.size() - 1;
        chunk_ptr->global_index = new_chunk_index;

        if (world_id) {
            auto& world_free_list = arch->world_available_chunks[*world_id];
            world_free_list.push_back(new_chunk_index);
            chunk_ptr->last_free_list_index = world_free_list.size() - 1;
        } else {
            arch->global_available_chunks.push_back(new_chunk_index);
            chunk_ptr->last_free_list_index = arch->global_available_chunks.size() - 1;
        }

        return chunk_ptr;
    }

    auto& select_free_list(archetype* arch, std::optional<int32_t> world_id) {
        if (world_id)
            return arch->world_available_chunks[*world_id];
        else
            return arch->global_available_chunks;
    }

    void update_freelists_after_swap(archetype* arch, chunk* moved_chunk, uint32_t new_idx) {
        auto& list = select_free_list(arch, moved_chunk->world_bind);

        list[moved_chunk->last_free_list_index] = new_idx;
    }

    void release_empty_chunk_swap_pop(archetype* arch, uint32_t index_to_remove) {
        uint32_t last_index = arch->chunks.size() - 1;
        chunk* moved_chunk = arch->chunks[last_index].get();

        if (index_to_remove != last_index) {
            std::swap(arch->chunks[index_to_remove], arch->chunks[last_index]);
            moved_chunk->global_index = index_to_remove;
            update_freelists_after_swap(arch, moved_chunk, index_to_remove);
        }

        arch->chunks.pop_back();
    }

    void remove_from_free_list(archetype* arch, chunk* chunk_to_remove) {
        auto& list = select_free_list(arch, chunk_to_remove->world_bind);

        list[chunk_to_remove->last_free_list_index] = list.back();
        arch->chunks[list.back()]->last_free_list_index = chunk_to_remove->last_free_list_index;
        list.pop_back();
    }

    void add_to_free_list(archetype* arch, chunk* chunk_to_add) {
        auto& list = select_free_list(arch, chunk_to_add->world_bind);
        list.push_back(chunk_to_add->global_index);
        chunk_to_add->last_free_list_index = list.size() - 1;
    }

    fast_task::task_mutex archetypes_mutex;
    std::unordered_map<
        std::vector<component_id>,
        archetype*,
        archetype_hash>
        archetype_lookup;

    std::vector<std::unique_ptr<archetype>> archetypes;

    struct entity_record {
        archetype* type;
        chunk* chunk;
        int32_t chunk_index = 0;
        uint32_t generation = 0;
    };

    struct entity_destroy_queue_item {
        int32_t id;
        uint32_t generation;
    };

    struct entity_allocation_request {
        const entity_recipe& recipe;
        std::optional<int32_t> world_id;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        entity result;
        bool ready = false;
    };

    struct entity_transfer_request {
        int32_t id;
        uint32_t generation;
        std::optional<int32_t> world_id;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        bool ready = false;
        bool success = false;
    };

    struct entity_dirty_mark_item {
        int32_t id;
        uint32_t generation;
        component_id component;
    };

    std::vector<entity_record> records;

    list_array<int32_t> free_entity_ids;
    moodycamel::ConcurrentQueue<detail::mutation_queue_item> mutation_queue;
    moodycamel::ConcurrentQueue<entity_destroy_queue_item> entity_destroy_queue;
    moodycamel::ConcurrentQueue<entity_dirty_mark_item> marking_queue;
    moodycamel::ConcurrentQueue<entity_allocation_request*> creation_queue;
    moodycamel::ConcurrentQueue<entity_transfer_request*> transfer_queue;

    //moves the entity from the back of the chunk to the position of the removed entity
    void compact_chunk(archetype* arch, chunk* chunk, int32_t old_pos) {
        if (chunk->entity_count == 0)
            return;

        int32_t last_entity_id = chunk->entities()[chunk->entity_count];

        chunk->entities()[old_pos] = last_entity_id;
        for (size_t i = 0; i < arch->component_ids.size(); ++i) {
            component_id id_to_process = arch->component_ids[i];
            const auto& type_info = detail::component_info_registry[id_to_process];
            size_t offset = arch->layout.component_offsets[i];

            void* dest_ptr = chunk->memory_block.get() + offset + (old_pos * type_info.size);
            void* src_ptr = chunk->memory_block.get() + offset + (chunk->entity_count * type_info.size);

            type_info.move(dest_ptr, src_ptr);
        }

        records.at(last_entity_id).chunk_index = old_pos;
    }

    //transfers the entity across archetypes and/or worlds
    void move_entity(int32_t id, archetype* to, std::optional<int32_t> world) {
        auto& record = records.at(id);
        archetype* old_type = record.type;

        if (old_type == to && (record.chunk == nullptr || record.chunk->world_bind == world))
            return;

        chunk* old_chunk = record.chunk;
        uint32_t old_index = record.chunk_index;

        chunk* target_chunk = get_free_chunk(to, world);
        uint32_t new_index = target_chunk->entity_count;

        target_chunk->entities()[new_index] = id;
        for (size_t i = 0; i < to->component_ids.size(); ++i) {
            component_id id_to_process = to->component_ids[i];
            const auto& type_info = detail::component_info_registry[id_to_process];

            size_t new_offset = to->layout.component_offsets[i];
            void* dest_ptr = target_chunk->memory_block.get() + new_offset + (new_index * type_info.size);

            auto it = old_type->component_index_map.find(id_to_process);
            if (it != old_type->component_index_map.end()) {
                uint32_t old_component_index = it->second;
                size_t old_offset = old_type->layout.component_offsets[old_component_index];
                void* src_ptr = old_chunk->memory_block.get() + old_offset + (old_index * type_info.size);

                type_info.move(dest_ptr, src_ptr);
            } else
                type_info.construct(dest_ptr);
        }

        record.type = to;
        record.chunk = target_chunk;
        record.chunk_index = new_index;
        target_chunk->entity_count++;

        old_chunk->entity_count--;
        compact_chunk(old_type, old_chunk, old_index);
    }

    //removes entity data, but keeps the record for reuse
    void deallocate_entity(int32_t id) {
        auto& record = records.at(id);
        bool was_full = (record.chunk->entity_count == CHUNK_CAPACITY);

        --record.chunk->entity_count;
        ++record.generation;

        compact_chunk(record.type, record.chunk, record.chunk_index);


        if (record.chunk->entity_count == 0) {
            release_empty_chunk_swap_pop(record.type, record.chunk->global_index);
            record.chunk = nullptr;
        } else if (was_full && record.chunk->entity_count == CHUNK_CAPACITY - 1)
            add_to_free_list(record.type, record.chunk);

        free_entity_ids.push_back(id);
    }

    entity allocate_entity(archetype* in, std::optional<int32_t> world) {
        if (free_entity_ids.empty()) {
            int32_t id = records.size();
            records.resize(records.size() + 1);
            auto& record = records.at(id);
            record.type = in;
            record.chunk = get_free_chunk(in, world);
            record.chunk_index = record.chunk->entity_count;
            record.chunk->entity_count++;
            if (record.chunk->entity_count == CHUNK_CAPACITY)
                remove_from_free_list(in, record.chunk);

            record.generation = 0;
            return {id, record.generation};
        } else {
            int32_t id = free_entity_ids.take_front();
            auto& record = records.at(id);
            record.type = in;
            record.chunk = get_free_chunk(in, world);
            record.chunk_index = record.chunk->entity_count;
            record.chunk->entity_count++;
            if (record.chunk->entity_count == CHUNK_CAPACITY)
                remove_from_free_list(in, record.chunk);

            ++record.generation;
            return {id, record.generation};
        }
    }

    fast_task::future_ptr<bool> world_local_registry::register_entity_async(entity& entity) {
        auto request = std::make_unique<entity_transfer_request>(entity.id, entity.generation, id);
        if (!transfer_queue.enqueue(request.get()))
            return fast_task::make_ready_future(false);
        return fast_task::future<bool>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return req->success;
        });
    }

    fast_task::future_ptr<bool> world_local_registry::unregister_entity_async(entity& entity) {
        auto request = std::make_unique<entity_transfer_request>(entity.id, entity.generation);
        if (!transfer_queue.enqueue(request.get()))
            return fast_task::make_ready_future(false);
        return fast_task::future<bool>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return req->success;
        });
    }

    fast_task::future_ptr<bool> world_local_registry::transfer_entity_async(entity& entity) {
        return register_entity_async(entity);
    }

    fast_task::future_ptr<entity> world_local_registry::create_entity_async(const entity_recipe& recipe) {
        if (!recipe.is_frozen())
            throw std::runtime_error("The recipe should be frozen before creating an entity!");
        auto request = std::make_unique<entity_allocation_request>(recipe, id);
        if (!creation_queue.enqueue(request.get()))
            throw std::bad_alloc();
        return fast_task::future<entity>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return std::move(req->result);
        });
    }

    bool world_local_registry::register_entity_and_block(entity& entity) {
        return register_entity_async(entity).get();
    }

    bool world_local_registry::unregister_entity_and_block(entity& entity) {
        return unregister_entity_async(entity).get();
    }

    bool world_local_registry::transfer_entity_and_block(entity& entity) {
        return transfer_entity_async(entity).get();
    }

    entity world_local_registry::create_entity_and_wait(const entity_recipe& recipe) {
        return create_entity_async(recipe)->take();
    }

    fast_task::future_ptr<entity> global_registry::create_entity_async(const entity_recipe& recipe) {
        if (!recipe.is_frozen())
            throw std::runtime_error("The recipe should be frozen before creating an entity!");
        auto request = std::make_unique<entity_allocation_request>(recipe);
        if (!creation_queue.enqueue(request.get()))
            throw std::bad_alloc();
        return fast_task::future<entity>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return std::move(req->result);
        });
    }

    entity global_registry::create_entity_and_wait(const entity_recipe& recipe) {
        return global_registry::create_entity_async(recipe)->take();
    }

    struct system_node {
        std::unique_ptr<system_interface> instance;
        detail::system_info info;
        size_t in_degree = 0;
    };

    struct scheduler::scheduler_data {
        std::vector<system_node> systems;
        std::unordered_map<size_t, std::vector<size_t>> dependency_graph;
        bool graph_is_dirty = false;

        void build_tree() {
            dependency_graph.clear();
            for (auto& node : systems)
                node.in_degree = 0;

            std::unordered_map<component_id, std::vector<size_t>> component_writers;
            std::unordered_map<component_id, std::vector<size_t>> component_readers;

            for (size_t i = 0; i < systems.size(); ++i) {
                const auto& info = systems[i].info;

                for (const auto& comp_id : info.write_dependencies)
                    component_writers[comp_id].push_back(i);
                for (const auto& comp_id : info.read_dependencies)
                    component_readers[comp_id].push_back(i);
            }


            bit_list_array<bool> dependency_added(systems.size());

            for (size_t predecessor_idx = 0; predecessor_idx < systems.size(); ++predecessor_idx) {
                const auto& predecessor_info = systems[predecessor_idx].info;

                dependency_added.zero_all();
                dependency_added.set(predecessor_idx, true);

                for (const auto& comp_id : predecessor_info.write_dependencies) {
                    if (component_readers.count(comp_id)) {
                        for (const auto successor_idx : component_readers.at(comp_id)) {
                            if (!dependency_added[successor_idx]) {
                                dependency_graph[predecessor_idx].push_back(successor_idx);
                                systems[successor_idx].in_degree++;
                                dependency_added[successor_idx] = true;
                            }
                        }
                    }

                    if (component_writers.count(comp_id)) {
                        for (const auto successor_idx : component_writers.at(comp_id)) {
                            if (!dependency_added.get_unchecked(successor_idx)) {
                                dependency_graph[predecessor_idx].push_back(successor_idx);
                                systems[successor_idx].in_degree++;
                                dependency_added.set(successor_idx, true);
                            }
                        }
                    }
                }
            }

            graph_is_dirty = false;
        }

        void proceed_tree() {
            std::vector<size_t> current_in_degrees;
            current_in_degrees.reserve(systems.size());
            for (const auto& node : systems) {
                current_in_degrees.push_back(node.in_degree);
            }

            size_t systems_executed_count = 0;
            list_array<size_t> ready_queue;
            for (size_t i = 0; i < systems.size(); ++i)
                if (current_in_degrees[i] == 0)
                    ready_queue.push_back(i);

            while (!ready_queue.empty()) {
                std::vector<size_t> systems_in_current_stage;
                while (!ready_queue.empty()) {
                    systems_in_current_stage.push_back(ready_queue.front());
                    ready_queue.pop_front();
                }

                fast_task::future_tool::for_each_wait(systems_in_current_stage, [&](size_t system_index) {
                    systems[system_index].instance->tick();
                });

                systems_executed_count += systems_in_current_stage.size();

                for (size_t completed_system_index : systems_in_current_stage) {
                    if (dependency_graph.count(completed_system_index)) {
                        for (size_t dependent_index : dependency_graph.at(completed_system_index)) {
                            current_in_degrees[dependent_index]--;
                            if (current_in_degrees[dependent_index] == 0)
                                ready_queue.push_back(dependent_index);
                        }
                    }
                }
            }

            if (systems_executed_count < systems.size()) {
                std::string error_message = "Circular dependency detected in systems! The following systems could not be executed:\n";
                for (size_t i = 0; i < systems.size(); ++i)
                    if (current_in_degrees[i] > 0)
                        error_message += "\tSystem " + std::string(systems[i].info.info.name()) + "\n";
                throw std::runtime_error(error_message);
            }
        }
    };

    scheduler::scheduler() : data(std::make_unique<scheduler_data>()) {}

    scheduler::~scheduler() {}

    archetype* map_new_archtype(archetype* old, component_id new_id) {
        std::vector<component_id> next_key = old->component_ids;
        next_key.push_back(new_id);
        std::sort(next_key.begin(), next_key.end());

        archetype* next_archetype = nullptr;
        if (auto it = archetype_lookup.find(next_key); it != archetype_lookup.end()) {
            next_archetype = it->second;
        } else {
            auto new_archetype_ptr = std::make_unique<archetype>();
            new_archetype_ptr->component_ids = next_key;
            new_archetype_ptr->hash = archetype_hash{}(next_key);
            new_archetype_ptr->component_index_map.reserve(next_key.size());
            new_archetype_ptr->calculate_layout();

            next_archetype = new_archetype_ptr.get();
            archetypes.push_back(std::move(new_archetype_ptr));
            archetype_lookup[next_key] = next_archetype;
        }

        old->add_transition_cache[new_id] = next_archetype;
        next_archetype->remove_transition_cache[new_id] = old;
        return next_archetype;
    }

    archetype* map_get_archtype(const std::vector<component_id>& decl) {
        auto& arch = archetype_lookup[decl];

        if (arch == nullptr) {
            auto new_archetype_ptr = std::make_unique<archetype>();
            new_archetype_ptr->component_ids = decl;
            new_archetype_ptr->hash = archetype_hash{}(decl);
            new_archetype_ptr->component_index_map.reserve(decl.size());
            new_archetype_ptr->calculate_layout();

            arch = new_archetype_ptr.get();
            archetypes.push_back(std::move(new_archetype_ptr));
        }
        return arch;
    }

    namespace mutation_processing {
        struct pre_allocated_mutation_op {
            int32_t entity_id;
            component_id component;
            archetype* to_archetype;
            std::vector<char> data; //if nullptr the component will be removed
        };

        struct prepared_move_op {
            int32_t entity_id;
            archetype* from_archetype;
            archetype* to_archetype;
            std::optional<int32_t> new_world_bind;
            component_id comp_id;
            std::vector<char> data; // Owns the new component data
        };

        struct prepared_in_place_update_op {
            int32_t entity_id;
            component_id comp_id;
            std::vector<char> data;
        };

        struct parallel_creation_op {
            entity_allocation_request* request;
            archetype* target_archetype;
        };

        struct grouped_mutation_ops {
            ::moodycamel::ConcurrentQueue<prepared_move_op> prepared_moves;
            ::moodycamel::ConcurrentQueue<prepared_in_place_update_op> prepared_updates;
        };

        template <class T, class FN>
        void consume_all_(::moodycamel::ConcurrentQueue<T>& queue, FN&& callback) {
            T item;
            while (queue.try_dequeue(item))
                callback(std::move(item));
        }

        template <class T, class FN>
        void parallel_drain(::moodycamel::ConcurrentQueue<T>& queue, FN&& process_func) {
            auto num_workers = std::thread::hardware_concurrency();
            std::vector<fast_task::future_ptr<void>> worker_futures;
            worker_futures.reserve(num_workers);

            for (decltype(num_workers) i = 0; i < num_workers; ++i) {
                worker_futures.push_back(fast_task::future<void>::start([&]() {
                    T item;
                    while (queue.try_dequeue(item))
                        process_func(std::move(item));
                }));
            }

            return fast_task::when_all(std::move(worker_futures));
        }

        template <class T>
        std::vector<T> collect_all_(::moodycamel::ConcurrentQueue<T>& queue) {
            std::vector<T> res;
            res.reserve(queue.size_approx());
            T item;
            while (queue.try_dequeue(item))
                res.emplace_back(std::move(item));
            return res;
        }

        void process_destruction_queue() {
            consume_all_(entity_destroy_queue, [](entity_destroy_queue_item&& item) {
                if (records.size() > item.id)
                    if (records[item.id].generation == item.generation)
                        deallocate_entity(item.id);
            });
        }

        std::vector<std::vector<prepared_move_op>> group_disjoint_moves(std::vector<prepared_move_op>&& all_moves) {
            std::vector<std::vector<prepared_move_op>> parallel_batches;

            while (!all_moves.empty()) {
                std::vector<prepared_move_op> current_batch;
                std::unordered_set<archetype*> archetypes_in_batch;

                auto new_end = std::remove_if(all_moves.begin(), all_moves.end(), [&](prepared_move_op& move_op) {
                    bool has_conflict = archetypes_in_batch.count(move_op.from_archetype) || archetypes_in_batch.count(move_op.to_archetype);

                    if (!has_conflict) {
                        archetypes_in_batch.insert(move_op.from_archetype);
                        archetypes_in_batch.insert(move_op.to_archetype);
                        current_batch.push_back(std::move(move_op));
                        return true;
                    }
                    return false;
                });

                all_moves.erase(new_end, all_moves.end());
                parallel_batches.push_back(std::move(current_batch));
            }
            return parallel_batches;
        }

        grouped_mutation_ops prepare_mutations() {
            grouped_mutation_ops res;
            consume_all_(mutation_queue, [&](detail::mutation_queue_item&& item) {
                if (records.size() <= item.entity_id || records[item.entity_id].generation != item.generation)
                    return;

                auto& record = records.at(item.entity_id);
                archetype* from_archetype = record.type;

                if (!item.data.empty() && from_archetype->component_index_map.count(item.component)) {
                    prepared_in_place_update_op update;
                    update.entity_id = item.entity_id;
                    update.comp_id = item.component;
                    update.data = std::move(item.data);

                    res.prepared_updates.enqueue(std::move(update));
                    return;
                }


                archetype* to_archetype = nullptr;
                if (item.data.empty()) {
                    if (auto it = from_archetype->remove_transition_cache.find(item.component); it == from_archetype->remove_transition_cache.end())
                        to_archetype = map_new_archtype(from_archetype, item.component);
                } else {
                    if (auto it = from_archetype->add_transition_cache.find(item.component); it == from_archetype->add_transition_cache.end())
                        to_archetype = map_new_archtype(from_archetype, item.component);
                }
                res.prepared_moves.enqueue(
                    prepared_move_op{
                        item.entity_id,
                        from_archetype,
                        to_archetype,
                        record.chunk->world_bind,
                        item.component,
                        std::move(item.data)
                    }
                );
            });
            return res;
        }

        std::vector<parallel_creation_op> prepare_creation_requests() {
            std::vector<parallel_creation_op> res;
            consume_all_(creation_queue, [&](entity_allocation_request* item) {
                res.emplace_back(item, map_get_archtype(item->recipe.get_ids()));
            });
            return res;
        }

        void execute_mutations(grouped_mutation_ops mutations) {
            consume_all_(mutations.prepared_updates, [](prepared_in_place_update_op& update) {
                auto& record = records.at(update.entity_id);
                auto& component_info = detail::component_info_registry.at(update.comp_id);
                auto component_index = record.type->component_index_map.at(update.comp_id);
                void* dest_ptr = record.chunk->memory_block.get() + record.type->layout.component_offsets[component_index] + (record.chunk_index * component_info.size);

                component_info.destroy(dest_ptr);
                component_info.move_construct(dest_ptr, (void*)update.data.data());
                record.type->mark_dirty(record.chunk, component_index, record.chunk_index);
            });

            for (auto& batch : group_disjoint_moves(collect_all_(mutations.prepared_moves))) {
                fast_task::future_tool::for_each_move(batch, [&](prepared_move_op&& move) {
                    archetype* arch1 = move.from_archetype;
                    archetype* arch2 = move.to_archetype;
                    if (arch1 == arch2)
                        return;

                    if (reinterpret_cast<uintptr_t>(arch1) > reinterpret_cast<uintptr_t>(arch2))
                        std::swap(arch1, arch2);

                    std::unique_lock lock1(arch1->arch_mutex);
                    std::unique_lock lock2(arch2->arch_mutex);

                    move_entity(move.entity_id, move.to_archetype, move.new_world_bind);
                    lock2.unlock();
                    lock1.unlock();

                    if (move.data.size()) {
                        auto& record = records.at(move.entity_id);
                        auto& component_info = detail::component_info_registry.at(move.comp_id);
                        auto component_index = record.type->component_index_map.at(move.comp_id);
                        void* dest_ptr = record.chunk->memory_block.get() + record.type->layout.component_offsets[component_index] + (record.chunk_index * component_info.size);

                        component_info.move(dest_ptr, (void*)move.data.data());
                        record.type->mark_dirty(record.chunk, component_index, record.chunk_index);
                    }
                })->wait();
            }
        }

        void process_transfers() {
            parallel_drain(transfer_queue, [](entity_transfer_request* transfer) {
                fast_task::unique_lock lock(transfer->mut);
                transfer->success = false;
                if (records.size() > transfer->id) {
                    auto& record = records[transfer->id];
                    if (record.generation == transfer->generation) {
                        std::lock_guard lock1(record.type->arch_mutex);
                        move_entity(transfer->id, record.type, transfer->world_id);
                        transfer->success = true;
                    }
                }
                transfer->ready = true;
                transfer->cv.notify_one();
            });
        }

        void process_dirty_marking() {
            parallel_drain(marking_queue, [](entity_dirty_mark_item&& mark) {
                auto& record = records.at(mark.id);
                if (record.generation != mark.generation)
                    if (auto component_index = record.type->component_index_map.find(mark.component); component_index != record.type->component_index_map.end())
                        record.type->mark_dirty(record.chunk, component_index->second, record.chunk_index);
            });
        }

        void process_creation(std::vector<parallel_creation_op>&& creation_requests) {
            fast_task::future_tool::for_each_move(std::move(creation_requests), [&](parallel_creation_op&& reg) {
                auto [item, arch] = reg;
                fast_task::lock_guard message_lock(item->mut);
                fast_task::lock_guard arch_lock(arch->arch_mutex);
                item->result = allocate_entity(arch, item->world_id);
                item->ready = true;
                item->cv.notify_one();
            })->wait();
        }

        void proceed_mutations() {
            process_destruction_queue();
            execute_mutations(prepare_mutations());
            process_transfers();
            process_dirty_marking();
            process_creation(prepare_creation_requests());
        }
    }

    void scheduler::execute_frame(world_local_registry& registry) {
        mutation_processing::proceed_mutations();

        if (data->graph_is_dirty)
            data->build_tree();
        data->proceed_tree();

        fast_task::future_tool::for_each_wait(archetypes, [](const std::unique_ptr<archetype>& archetype_ptr) {
            for (std::unique_ptr<chunk>& ch : archetype_ptr->chunks)
                for (auto& flags : archetype_ptr->dirty_flags(ch.get()))
                    for (auto& flag_word : flags)
                        flag_word.store(0, std::memory_order_relaxed);
        });
    }

    void scheduler::add_system_impl(std::unique_ptr<system_interface> system, detail::system_info& info) {
        data->systems.emplace_back(std::move(system), info);
        data->graph_is_dirty = true;
    }

    namespace detail {
        std::atomic<component_id> next_component_id = 0;
        std::vector<component_type_info> component_info_registry;
        fast_task::mutex registry_mutex;

        void* get_entity_component(int32_t id, uint32_t generation, component_id component_id) {
            auto& record = records.at(id);
            if (record.generation != generation)
                return nullptr;
            auto arch = record.type;
            auto it = arch->component_index_map.find(component_id);
            if (it == arch->component_index_map.end())
                return nullptr;
            else {
                size_t base_offset = record.type->layout.component_offsets[it->second];
                const auto& type_info = detail::component_info_registry[component_id];
                return record.chunk->memory_block.get() + base_offset + (record.chunk_index * type_info.size);
            }
        }

        void queue_command(detail::mutation_queue_item&& command) {
            if (!mutation_queue.enqueue(std::move(command)))
                throw std::bad_alloc();
        }

        void queue_remove_entity_component(int32_t id, uint32_t generation, component_id component_id) {
            if (!mutation_queue.enqueue(mutation_queue_item{id, generation, component_id}))
                throw std::bad_alloc();
        }

        void queue_destroy_entity(int32_t id, uint32_t generation) {
            if (!entity_destroy_queue.enqueue(entity_destroy_queue_item{id, generation}))
                throw std::bad_alloc();
        }

        void queue_mark_dirty(int32_t id, uint32_t generation, component_id component_id) {
            if (!marking_queue.enqueue(entity_dirty_mark_item{id, generation, component_id}))
                throw std::bad_alloc();
        }

        bool has_entity_component(int32_t id, uint32_t generation, component_id component_id) {
            auto& record = records.at(id);
            if (record.generation == generation)
                return record.type->component_index_map.contains(component_id);
            else
                return false;
        }

        struct iteration_handle::iteration_data {
            struct arch_data_t {
                archetype* type;
                std::vector<size_t> required_layout_offsets;
                std::vector<uint32_t> required_dirty_comp_indices; // Use correct type
                std::vector<uint32_t> make_dirty_comp_indices;     // Use correct type

                arch_data_t(archetype* type) : type(type) {}
            };

            size_t current_archetype_index = 0;
            size_t current_chunk_index = 0;
            std::vector<arch_data_t> arch_data;
            std::optional<int32_t> world_id;
            std::vector<void*> component_arrays;
            bool has_dirty_components_filter = false;

            void calculate_data(std::span<component_id> components, std::span<component_id> dirty_components, std::span<component_id> mark_dirty_components) {
                for (auto& res : arch_data) {
                    auto& index_map = res.type->component_index_map;
                    auto& offsets = res.type->layout.component_offsets;

                    res.required_layout_offsets.reserve(components.size());
                    for (auto component : components)
                        res.required_layout_offsets.push_back(offsets[index_map.at(component)]);

                    res.required_dirty_comp_indices.reserve(dirty_components.size());
                    for (auto component : dirty_components)
                        if (auto it = index_map.find(component); it != index_map.end())
                            res.required_dirty_comp_indices.push_back(it->second);

                    res.make_dirty_comp_indices.reserve(mark_dirty_components.size());
                    for (auto component : mark_dirty_components)
                        if (auto it = index_map.find(component); it != index_map.end())
                            res.make_dirty_comp_indices.push_back(it->second);
                }
                if (dirty_components.size())
                    has_dirty_components_filter = true;
            }

            // CRITICAL: after changing check mark_component_dirty for correctnes
            std::pair<size_t, void**> next() {
                while (current_archetype_index < arch_data.size()) {
                    arch_data_t& data = arch_data[current_archetype_index];
                    archetype* archetype = data.type;

                    while (current_chunk_index < archetype->chunks.size()) {
                        std::unique_ptr<chunk>& chunk = archetype->chunks[current_chunk_index];
                        const bool world_match = !world_id.has_value() || chunk->world_bind == world_id.value();

                        if (world_match) {
                            bool dirty_match = true;
                            if (!data.required_dirty_comp_indices.empty()) {
                                dirty_match = false;
                                for (uint32_t comp_index : data.required_dirty_comp_indices) {
                                    auto flags = archetype->dirty_flags(chunk.get(), comp_index);
                                    uint64_t combined_mask = 0;
                                    for (const auto& flag_word : flags) {
                                        combined_mask |= flag_word.load(std::memory_order_relaxed);
                                    }
                                    if (combined_mask != 0) {
                                        dirty_match = true;
                                        break;
                                    }
                                }
                            }

                            if (dirty_match) {
                                for (uint32_t comp_index : data.make_dirty_comp_indices)
                                    archetype->mark_dirty_entities(chunk.get(), comp_index);

                                for (size_t i = 0; i < data.required_layout_offsets.size(); i++)
                                    component_arrays[i] = chunk->memory_block.get() + data.required_layout_offsets[i];

                                ++current_chunk_index;
                                return {chunk->entity_count, component_arrays.data()};
                            }
                        }
                        ++current_chunk_index;
                    }
                    ++current_archetype_index;
                    current_chunk_index = 0;
                }

                return {0, nullptr};
            }

            // CRITICAL: this function depends on next() function, changes on using current_chunk_index
            //  would impact the current chunk retrivial
            //  this function used for explicit dirty marking
            void mark_component_dirty(component_id component, size_t entity_index) {
                if (current_archetype_index >= arch_data.size())
                    return;

                arch_data_t& data = arch_data[current_archetype_index];
                archetype* archetype = data.type;

                // CRITICAL: The next() function increments current_chunk_index *before* it returns.
                // This means the chunk the user is currently iterating over is at the *previous* index.
                if (current_chunk_index == 0)
                    return; // This should not happen if called from a valid iterator, but as a safeguard:

                std::unique_ptr<chunk>& chunk = archetype->chunks[current_chunk_index - 1];

                auto it = archetype->component_index_map.find(component);
                if (it != archetype->component_index_map.end()) {
                    uint32_t component_index_in_archetype = it->second;
                    archetype->mark_dirty(chunk.get(), component_index_in_archetype, entity_index);
                }
            }

            bool is_entity_match(size_t entity_index) const {
                if (current_chunk_index == 0)
                    return false;

                auto& active_arch_data = arch_data[current_archetype_index];
                auto& active_chunk = active_arch_data.type->chunks[current_chunk_index - 1];
                for (uint32_t comp_index : active_arch_data.required_dirty_comp_indices)
                    if (!active_arch_data.type->is_dirty(active_chunk.get(), comp_index, entity_index))
                        return false;
                return true;
            }

            std::pair<int32_t, uint32_t> get_current_entity(size_t entity_index) {
                if (current_chunk_index == 0)
                    return {0, UINT32_MAX};

                auto& active_arch_data = arch_data[current_archetype_index];
                auto& active_chunk = active_arch_data.type->chunks[current_chunk_index - 1];
                auto id = active_chunk.get()->entities()[entity_index];

                auto& record = records.at(id);
                return {id, record.generation};
            }
        };

        iteration_handle::~iteration_handle() {}

        std::pair<size_t, void**> iteration_handle::next() {
            if (data == nullptr)
                return {0, nullptr};
            else
                return data->next();
        }

        void iteration_handle::mark_component_dirty(component_id component, size_t index) {
            if (data != nullptr)
                data->mark_component_dirty(component, index);
        }

        bool iteration_handle::is_entity_match(size_t entity_index) const {
            if (data == nullptr)
                return false;
            return data->is_entity_match(entity_index);
        }

        std::pair<int32_t, uint32_t> iteration_handle::get_current_entity(size_t entity_index) {
            if (data == nullptr)
                return {0, -1};
            return data->get_current_entity(entity_index);
        }

        iteration_handle iterate_components(int32_t world_id, std::span<component_id> components, std::span<component_id> without_components, std::span<component_id> writes_components, std::span<component_id> with_dirty_components) {
            iteration_handle handle;
            handle.data = std::make_unique<iteration_handle::iteration_data>();
            handle.data->world_id = world_id;

            for (const auto& archetype_ptr : archetypes)
                if (archetype_ptr->matches_query(components, without_components))
                    handle.data->arch_data.push_back(archetype_ptr.get());

            handle.data->calculate_data(components, with_dirty_components, writes_components);
            handle.data->component_arrays.resize(components.size());
            return handle;
        }

        iteration_handle iterate_components_global(std::span<component_id> components, std::span<component_id> without_components, std::span<component_id> writes_components, std::span<component_id> with_dirty_components) {
            iteration_handle handle;
            handle.data = std::make_unique<iteration_handle::iteration_data>();

            for (const auto& archetype_ptr : archetypes)
                if (archetype_ptr->matches_query(components, without_components))
                    handle.data->arch_data.push_back(archetype_ptr.get());

            handle.data->calculate_data(components, with_dirty_components, writes_components);
            handle.data->component_arrays.resize(components.size());
            return handle;
        }
    }

    void entity_recipe::freeze() {
        is_frozen_ = true;
        std::sort(component_ids.begin(), component_ids.end());
        component_ids.erase(
            std::unique(component_ids.begin(), component_ids.end()),
            component_ids.end()
        );
    }

    const std::vector<component_id>& entity_recipe::get_ids() const {
        return component_ids;
    }
}
