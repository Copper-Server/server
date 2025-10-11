/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/lockfree/queue.hpp>
#include <library/fast_task/include/future.hpp>
#include <library/list_array.hpp>
#include <src/api/ecs.hpp>

namespace copper_server::api::ecs {
    struct archetype_hash {
        size_t operator()(const std::vector<component_id>& ids) const {
            size_t seed = ids.size();
            for (auto id : ids)
                seed ^= id + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }

        size_t operator()(component_id* ids, size_t size) const {
            size_t seed = size;
            for (size_t i = 0; i < size; i++)
                seed ^= ids[i] + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    constexpr uint32_t CHUNK_CAPACITY = 256;

    struct chunk {
        char* memory_block = nullptr;
        int32_t world_bind = -1;
        uint32_t entity_count = 0;

        std::pair<int32_t*, size_t> entities() {
            return {reinterpret_cast<int32_t*>(memory_block), entity_count};
        }

        chunk() = default;

        ~chunk() {
            if (memory_block)
                delete[] memory_block;
        }

        bool has_free_slot() {
            return entity_count < CHUNK_CAPACITY;
        }
    };

    struct archetype {
        std::vector<component_id> component_ids;
        size_t hash;
        list_array<chunk*> chunks;

        std::unordered_map<component_id, archetype*> add_transition_cache;
        std::unordered_map<component_id, archetype*> remove_transition_cache;
        std::unordered_map<component_id, uint32_t> component_index_map;

        struct chunk_layout {
            size_t chunk_size_bytes = 0;
            std::vector<size_t> component_offsets;
        };

        chunk_layout layout;

        void calculate_layout() {
            if (layout.chunk_size_bytes == 0) {
                size_t current_offset = sizeof(int32_t) * CHUNK_CAPACITY;
                for (size_t i = 0; i < component_ids.size(); ++i) {
                    component_id id = component_ids[i];
                    const auto& info = detail::component_info_registry[id];

                    current_offset = (current_offset + info.alignment - 1) & ~(info.alignment - 1);

                    layout.component_offsets[i] = current_offset;
                    current_offset += info.size * CHUNK_CAPACITY;
                    component_index_map[id] = i;
                }
                layout.chunk_size_bytes = current_offset;
            }
        }

        bool matches_query(component_id* components, size_t components_size, component_id* without_components, size_t without_components_size) {
            for (size_t i = 0; i < components_size; i++)
                if (!component_index_map.contains(components[i]))
                    return false;
            for (size_t i = 0; i < without_components_size; i++)
                if (component_index_map.contains(without_components[i]))
                    return false;
            return true;
        }
    };

    chunk* allocate_chunk_for_archetype(archetype* arch, int32_t world_id = -1) {
        chunk* new_chunk = new chunk();
        new_chunk->memory_block = new char[arch->layout.chunk_size_bytes];
        new_chunk->entity_count = 0;
        new_chunk->world_bind = world_id;
        return new_chunk;
    }

    chunk* get_free_chunk(archetype* arch, int32_t world_id) {
        auto chunk_index = arch->chunks.find_if([world_id](auto chunk) {
            return chunk->world_bind == world_id && chunk->has_free_slot();
        });

        if (chunk_index == arch->chunks.npos) {
            arch->chunks.push_back(allocate_chunk_for_archetype(arch, world_id));
            chunk_index = arch->chunks.size() - 1;
        }
        return arch->chunks[chunk_index];
    }

    void free_chunk(chunk* chunk) {
        delete chunk;
    }

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

    struct mutation_queue_item {
        int32_t entity_id;
        uint32_t generation;
        component_id component;
        std::vector<char> data; //if nullptr the component will be removed

        ~mutation_queue_item() noexcept {
            if (data.size())
                detail::component_info_registry[component].destroy(data.data());
        }
    };

    struct entity_destroy_queue_item {
        int32_t id;
        uint32_t generation;
    };

    struct entity_allocation_request {
        entity_recipe& recipe;
        int32_t world_id = -1;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        entity result;
        bool ready = false;
    };

    struct entity_transfer_request {
        int32_t id;
        uint32_t generation;
        int32_t world_id = -1;
        fast_task::task_mutex mut;
        fast_task::task_condition_variable cv;
        bool ready = false;
        bool success = false;
    };

    list_array<entity_record> records;

    list_array<int32_t> free_entity_ids;
    boost::lockfree::queue<entity_destroy_queue_item> entity_destroy_queue(100);
    boost::lockfree::queue<mutation_queue_item> mutation_queue(100);
    boost::lockfree::queue<entity_allocation_request*> creation_queue(100);
    boost::lockfree::queue<entity_transfer_request*> transfer_queue(100);

    void compact_chunk(archetype* arch, chunk* chunk, int32_t old_pos) {
        if (chunk->entity_count == 0)
            return;

        int32_t last_entity_id = chunk->entities().first[chunk->entity_count];

        chunk->entities().first[old_pos] = last_entity_id;
        for (size_t i = 0; i < arch->component_ids.size(); ++i) {
            component_id id_to_process = arch->component_ids[i];
            const auto& type_info = detail::component_info_registry[id_to_process];
            size_t offset = arch->layout.component_offsets[i];

            void* dest_ptr = static_cast<char*>(chunk->memory_block) + offset + (old_pos * type_info.size);
            void* src_ptr = static_cast<char*>(chunk->memory_block) + offset + (chunk->entity_count * type_info.size);

            type_info.move(dest_ptr, src_ptr);
        }

        records.at(last_entity_id).chunk_index = old_pos;
    }

    void move_entity(int32_t id, archetype* to, int32_t world = -1) {
        auto& record = records.at(id);
        archetype* old_type = record.type;

        if (old_type == to && (record.chunk == nullptr || record.chunk->world_bind == world))
            return;

        chunk* old_chunk = record.chunk;
        uint32_t old_index = record.chunk_index;

        chunk* target_chunk = get_free_chunk(to, world);
        uint32_t new_index = target_chunk->entity_count;

        target_chunk->entities().first[new_index] = id;
        for (size_t i = 0; i < to->component_ids.size(); ++i) {
            component_id id_to_process = to->component_ids[i];
            const auto& type_info = detail::component_info_registry[id_to_process];

            size_t new_offset = to->layout.component_offsets[i];
            void* dest_ptr = static_cast<char*>(target_chunk->memory_block) + new_offset + (new_index * type_info.size);

            auto it = old_type->component_index_map.find(id_to_process);
            if (it != old_type->component_index_map.end()) {
                uint32_t old_component_index = it->second;
                size_t old_offset = old_type->layout.component_offsets[old_component_index];
                void* src_ptr = static_cast<char*>(old_chunk->memory_block) + old_offset + (old_index * type_info.size);

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

    void deallocate_entity(int32_t id) { //removes entity data, but keeps the record
        auto& record = records.at(id);
        --record.chunk->entity_count;
        ++record.generation;

        compact_chunk(record.type, record.chunk, record.chunk_index);


        if (record.chunk->entity_count == 0) {
            free_chunk(record.chunk);
            record.chunk = nullptr;
        }
        free_entity_ids.push_back(id);
    }

    entity allocate_entity(archetype* in, int32_t world) {
        if (free_entity_ids.empty()) {
            int32_t id = records.size();
            records.resize(records.size() + 1);
            auto& record = records.at(id);
            record.type = in;
            record.chunk = get_free_chunk(in, world);
            record.chunk_index = record.chunk->entity_count;
            record.chunk->entity_count++;
            record.generation = 0;
            return {id, record.generation};
        } else {
            int32_t id = free_entity_ids.take_front();
            auto& record = records.at(id);
            record.type = in;
            record.chunk = get_free_chunk(in, world);
            record.chunk_index = record.chunk->entity_count;
            record.chunk->entity_count++;
            ++record.generation;
            return {id, record.generation};
        }
    }

    fast_task::future_ptr<bool> world_local_registry::register_entity(entity& entity) {
        auto request = std::make_unique<entity_transfer_request>(entity.id, entity.generation, id);
        transfer_queue.push(request.get());
        return fast_task::future<bool>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return req->success;
        });
    }

    fast_task::future_ptr<bool> world_local_registry::unregister_entity(entity& entity) {
        auto request = std::make_unique<entity_transfer_request>(entity.id, entity.generation);
        transfer_queue.push(request.get());
        return fast_task::future<bool>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return req->success;
        });
    }

    fast_task::future_ptr<bool> world_local_registry::transfer_entity(entity& entity) {
        return register_entity(entity);
    }

    fast_task::future_ptr<entity> world_local_registry::create_entity(entity_recipe& recipe) {
        if (!recipe.is_frozen())
            throw std::runtime_error("The recipe schould be frozen before creating an entity!");
        auto request = std::make_unique<entity_allocation_request>(recipe, id);
        creation_queue.push(request.get());
        return fast_task::future<entity>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return std::move(req->result);
        });
    }

    fast_task::future_ptr<entity> global_registry::create_entity(entity_recipe& recipe) {
        if (!recipe.is_frozen())
            throw std::runtime_error("The recipe schould be frozen before creating an entity!");
        auto request = std::make_unique<entity_allocation_request>(recipe);
        creation_queue.push(request.get());
        return fast_task::future<entity>::start([req = std::move(request)]() {
            fast_task::mutex_unify unify(req->mut);
            fast_task::unique_lock lock(unify);
            while (!req->ready)
                req->cv.wait(lock);
            return std::move(req->result);
        });
    }

    struct system_node {
        std::unique_ptr<system_interface> instance;
        detail::system_info info;
        size_t in_degree = 0;
    };

    struct scheduler::data_t {
        std::vector<system_node> systems;
        std::unordered_map<size_t, std::vector<size_t>> dependency_graph;
        bool graph_is_dirty = false;

        void build_tree() {
            dependency_graph.clear();
            for (auto& node : systems) {
                node.in_degree = 0;
            }

            for (size_t i = 0; i < systems.size(); ++i) {
                for (size_t j = 0; j < systems.size(); ++j) {
                    if (i == j)
                        continue;

                    const auto& system_A = systems[i];
                    auto& system_B = systems[j];

                    bool creates_dependency = false;
                    for (const auto& component_id_A_writes : system_A.info.write_dependencies) {
                        for (const auto& component_id_B_reads : system_B.info.read_dependencies) {
                            if (component_id_A_writes == component_id_B_reads) {
                                creates_dependency = true;
                                break;
                            }
                        }
                        if (creates_dependency)
                            break;

                        for (const auto& component_id_B_writes : system_B.info.write_dependencies) {
                            if (component_id_A_writes == component_id_B_writes) {
                                creates_dependency = true;
                                break;
                            }
                        }
                        if (creates_dependency)
                            break;
                    }

                    if (creates_dependency) {
                        dependency_graph[i].push_back(j);
                        system_B.in_degree++;
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

                fast_task::future_tool::for_each(systems_in_current_stage, [&](size_t system_index) {
                    systems[system_index].instance->tick();
                })->wait();

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

    scheduler::scheduler() : data(new data_t{}) {}

    scheduler::~scheduler() {
        delete data;
    }

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

    void proceed_mutations() {
        entity_destroy_queue.consume_all([](entity_destroy_queue_item&& item) {
            if (records.size() > item.id)
                if (records[item.id].generation == item.generation)
                    deallocate_entity(item.id);
        });
        transfer_queue.consume_all([](entity_allocation_request&& item) {
            fast_task::mutex_unify unify(item.mut);
            fast_task::unique_lock lock(unify);
            auto arch = map_get_archtype(item.recipe.get_ids());
            item.result = allocate_entity(arch, item.world_id);
            item.ready = true;
            item.cv.notify_one();
        });
        creation_queue.consume_all([](entity_transfer_request&& item) {
            fast_task::mutex_unify unify(item.mut);
            fast_task::unique_lock lock(unify);
            item.success = false;
            if (records.size() > item.id)
                if (records[item.id].generation == item.generation) {
                    move_entity(item.id, records[item.id].type, item.world_id);
                    item.success = true;
                }
            item.ready = true;
            item.cv.notify_one();
        });
        mutation_queue.consume_all([](mutation_queue_item&& item) {
            auto& record = records.at(item.entity_id);
            archetype* current_archetype = record.type;
            archetype* next_archetype = nullptr;
            if (item.data.empty()) {
                if (auto it = current_archetype->remove_transition_cache.find(item.component);
                    it != current_archetype->remove_transition_cache.end()) {
                    next_archetype = it->second;
                } else
                    next_archetype = map_new_archtype(current_archetype, item.component);
            } else {
                if (auto it = current_archetype->add_transition_cache.find(item.component);
                    it != current_archetype->add_transition_cache.end()) {
                    next_archetype = it->second;
                } else
                    next_archetype = map_new_archtype(current_archetype, item.component);
            }
            move_entity(item.entity_id, next_archetype, record.chunk->world_bind);
        });

        for (auto& archetype : archetypes) {
            archetype->chunks.remove_if([](auto& chunk) { //collect empty chunks
                if (chunk->entity_count == 0) {
                    free_chunk(chunk);
                    return true;
                }
                return false;
            });
        }
    }

    void scheduler::execute_frame(world_local_registry& registry) {
        if (data->graph_is_dirty)
            data->build_tree();

        data->proceed_tree();
        proceed_mutations();
    }

    void scheduler::_add_system(std::unique_ptr<system_interface> system, detail::system_info& info) {
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
                return static_cast<char*>(record.chunk->memory_block) + base_offset + (record.chunk_index * type_info.size);
            }
        }

        void queue_set_entity_component(int32_t id, uint32_t generation, component_id component_id, void* component) {
            auto& info = component_info_registry.at(component_id);
            mutation_queue_item queue{id, generation, component_id};
            info.move_construct(queue.data.data(), component);
            mutation_queue.push(std::move(queue));
        }

        void queue_remove_entity_component(int32_t id, uint32_t generation, component_id component_id) {
            mutation_queue.push(mutation_queue_item{id, generation, component_id});
        }

        void queue_destroy_entity(int32_t id, uint32_t generation) {
            entity_destroy_queue.push(entity_destroy_queue_item{id, generation});
        }

        bool has_entity_component(int32_t id, component_id component_id) {
            return records.at(id).type->component_index_map.contains(component_id);
        }

        struct iteration_handle::iteration_data {
            size_t current_archetype_index = 0;
            size_t current_chunk_index = 0;
            std::vector<std::vector<size_t>> required_component_layout_offsets;
            std::optional<int32_t> world_id;
            std::vector<archetype*> matching_archetypes;
            std::vector<void*> component_arrays;

            void calculate_layouts(component_id* components, size_t components_size) {
                required_component_layout_offsets.clear();
                required_component_layout_offsets.reserve(matching_archetypes.size());
                for (auto& archetype : matching_archetypes) {
                    std::vector<size_t> component_layout_offsets;
                    component_layout_offsets.reserve(components_size);
                    auto& offsets = archetype->layout.component_offsets;
                    auto& index_map = archetype->component_index_map;
                    for (size_t i = 0; i < components_size; i++)
                        component_layout_offsets.push_back(offsets[index_map[components[i]]]);
                    required_component_layout_offsets.push_back(std::move(component_layout_offsets));
                }
            }
        };

        iteration_handle::~iteration_handle() {
            if (data != nullptr)
                delete data;
        }

        std::pair<size_t, void**> iteration_handle::next() {
            if (data == nullptr)
                return {0, nullptr};
            else {
                while (data->current_archetype_index < data->matching_archetypes.size()) {
                    auto& archetype = data->matching_archetypes[data->current_archetype_index];
                    auto& layout_offsets = data->required_component_layout_offsets[data->current_archetype_index];
                    while (data->current_chunk_index < archetype->chunks.size()) {
                        auto& chunk = archetype->chunks[data->current_chunk_index];
                        if (chunk->world_bind == data->world_id.value_or(-1)) {
                            for (size_t i = 0; i < layout_offsets.size(); i++)
                                data->component_arrays[i] = chunk->memory_block + layout_offsets[i];
                            return {data->component_arrays.size(), data->component_arrays.data()};
                        }
                        ++data->current_chunk_index;
                    }
                    ++data->current_archetype_index;
                    data->current_chunk_index = 0;
                }
                return {0, nullptr};
            }
        }

        iteration_handle iterate_components(int32_t world_id, component_id* components, size_t components_size, component_id* without_components, size_t without_components_size) {
            std::vector<archetype*> matching_archetypes;
            for (const auto& archetype_ptr : archetypes)
                if (archetype_ptr->matches_query(components, components_size, without_components, without_components_size))
                    matching_archetypes.push_back(archetype_ptr.get());

            iteration_handle handle;

            handle.data = new iteration_handle::iteration_data{
                .world_id = world_id,
                .matching_archetypes = matching_archetypes
            };
            handle.data->calculate_layouts(components, components_size);
            handle.data->component_arrays.resize(components_size);
            return handle;
        }

        iteration_handle iterate_components_global(component_id* components, size_t components_size, component_id* without_components, size_t without_components_size) {
            std::vector<archetype*> matching_archetypes;
            for (const auto& archetype_ptr : archetypes)
                if (archetype_ptr->matches_query(components, components_size, without_components, without_components_size))
                    matching_archetypes.push_back(archetype_ptr.get());

            iteration_handle handle;
            handle.data = new iteration_handle::iteration_data{.matching_archetypes = matching_archetypes};
            handle.data->calculate_layouts(components, components_size);
            handle.data->component_arrays.resize(components_size);
            return handle;
        }
    }

    void entity_recipe::freeze() {
        _is_frozen = true;
        component_ids = list_array<component_id>(component_ids).unify().to_container<std::vector>();
        std::sort(component_ids.begin(), component_ids.end());
    }

    const std::vector<component_id>& entity_recipe::get_ids() const {
        return component_ids;
    }
}
