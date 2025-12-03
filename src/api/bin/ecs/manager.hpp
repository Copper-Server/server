/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_BIN_ECS_MANAGER
#define SRC_API_BIN_ECS_MANAGER
#include <concurrentqueue/moodycamel/concurrentqueue.h>
#include <library/fast_task/include/future.hpp>
#include <library/list_array.hpp>
#include <new>
#include <span>
#include <src/api/ecs.hpp>
#include <unordered_set>

namespace copper_server::api::ecs {
    struct archetype;
    struct chunk;
    struct world;

    struct alignas(std::hardware_destructive_interference_size) entity_record {
        std::atomic<archetype*> archetype_before_mutation = nullptr;
        archetype* type;
        chunk* chunk;
        world* world_owner;
        int32_t chunk_index = 0;
        uint32_t generation = 0;


        entity_record() = default;

        entity_record(const entity_record& mov) {
            type = mov.type;
            chunk = mov.chunk;
            world_owner = mov.world_owner;
            archetype_before_mutation = mov.archetype_before_mutation.load();
            chunk_index = mov.chunk_index;
            generation = mov.generation;
        }

        entity_record& operator=(const entity_record& mov) {
            type = mov.type;
            chunk = mov.chunk;
            world_owner = mov.world_owner;
            archetype_before_mutation = mov.archetype_before_mutation.load();
            chunk_index = mov.chunk_index;
            generation = mov.generation;
            return *this;
        }
    };

    struct archetype_hash {
        static constexpr inline auto golden_ratio = 0x9e3779b9;

        size_t operator()(const std::vector<whole_component_id>& ids) const {
            size_t seed = ids.size();
            for (auto id : ids)
                seed ^= id + golden_ratio + (seed << 6) + (seed >> 2);
            return seed;
        }

        size_t operator()(whole_component_id* ids, size_t size) const {
            size_t seed = size;
            for (size_t i = 0; i < size; i++)
                seed ^= ids[i] + golden_ratio + (seed << 6) + (seed >> 2);
            return seed;
        }
    };

    constexpr uint32_t CHUNK_CAPACITY = 256;

    struct chunk {
        std::unique_ptr<char[]> memory_block;
        uint32_t entity_count = 0;
        uint32_t last_free_list_index = 0;
        uint32_t global_index = 0;

        int32_t* entities() {
            return reinterpret_cast<int32_t*>(memory_block.get());
        }

        bool has_free_slot() {
            return entity_count < CHUNK_CAPACITY;
        }
    };

    struct flat_relation_data {
        size_t chunk_offset;
        size_t comp_size;
        detail::component_type_info::get_relations_fn fn;
    };

    struct construction_plan {
        struct step {
            size_t offset;
            size_t size;
            detail::component_type_info::constructor_fn construct_fn;
        };

        std::vector<step> steps;

        void execute(char* dst_block, size_t dst_index) {
            for (const auto& op : steps) {
                void* d_ptr = dst_block + op.offset + (dst_index * op.size);
                op.construct_fn(d_ptr);
            }
        }
    };

    struct migration_plan {
        struct step {
            size_t src_offset;
            size_t dst_offset;
            size_t size;
            detail::component_type_info::move_constructor_fn move_fn;
        };

        std::vector<step> steps;

        struct construct_step {
            size_t dst_offset;
            size_t size;
            detail::component_type_info::constructor_fn construct_fn;
        };

        std::vector<construct_step> missing_steps;

        void execute(char* src_block, char* dst_block, size_t src_index, size_t dst_index) {
            for (const auto& op : steps) {
                void* s_ptr = src_block + op.src_offset + (src_index * op.size);
                void* d_ptr = dst_block + op.dst_offset + (dst_index * op.size);

                if (op.move_fn) [[unlikely]] {
                    op.move_fn(d_ptr, s_ptr);
                } else
                    std::memcpy(d_ptr, s_ptr, op.size);
            }
        }

        void execute_defaults(char* dst_block, size_t dst_index) {
            for (const auto& op : missing_steps) {
                void* d_ptr = dst_block + op.dst_offset + (dst_index * op.size);
                op.construct_fn(d_ptr);
            }
        }
    };

    struct destruction_plan {
        struct step {
            size_t offset;
            size_t size;
            detail::component_type_info::destructor_fn destruct_fn;
        };

        std::vector<step> steps;

        void execute(char* dst_block, size_t dst_index) {
            for (const auto& op : steps) {
                void* d_ptr = dst_block + op.offset + (dst_index * op.size);
                op.destruct_fn(d_ptr);
            }
        }
    };

    struct move_plan {
        struct step {
            size_t offset;
            size_t size;
            detail::component_type_info::move_constructor_fn move_fn;
        };

        std::vector<step> steps;

        void execute(char* src_block, char* dst_block, size_t src_index, size_t dst_index) {
            for (const auto& op : steps) {
                void* s_ptr = src_block + op.offset + (src_index * op.size);
                void* d_ptr = dst_block + op.offset + (dst_index * op.size);

                if (op.move_fn) [[unlikely]] {
                    op.move_fn(d_ptr, s_ptr);
                } else
                    std::memcpy(d_ptr, s_ptr, op.size);
            }
        }
    };

    struct archetype {
        std::vector<whole_component_id> whole_component_ids; //used for archetype indexing
        std::vector<component_id> component_ids;             //used for serializing
        size_t hash;
        std::vector<std::unique_ptr<chunk>> chunks;

        std::vector<uint32_t> available_chunks;

        std::unordered_map<component_id, archetype*> add_transition_cache;
        std::unordered_map<component_id, archetype*> remove_transition_cache;
        std::unordered_map<component_id, uint32_t> component_index_map;
        std::unordered_set<whole_component_id> whole_component_presence_helper;
        std::vector<flat_relation_data> component_calculate_relations;

        struct chunk_layout {
            std::vector<size_t> component_offsets;
            std::vector<size_t> dirty_flags_offsets;
            size_t chunk_size_bytes = 0;
        };

        chunk_layout layout;
        fast_task::task_rw_mutex arch_mutex;
        construction_plan construction_cache;
        destruction_plan destruction_cache;
        move_plan move_cache;
        std::unordered_map<archetype*, std::unique_ptr<migration_plan>> migration_cache;

        construction_plan& get_construction_plan() {
            if (construction_cache.steps.size())
                return construction_cache;

            for (size_t i = 0; i < component_ids.size(); ++i) {
                component_id cid = component_ids[i];
                const auto& info = detail::component_info_registry[cid];

                construction_cache.steps.push_back({.offset = layout.component_offsets[i], .size = info.size, .construct_fn = info.construct});
            }
            return construction_cache;
        }

        destruction_plan& get_destruction_plan() {
            if (destruction_cache.steps.size())
                return destruction_cache;

            for (auto& [component, pos] : component_index_map) {
                const auto& type_info = detail::component_info_registry[component];
                size_t offset = layout.component_offsets[pos];

                destruction_cache.steps.push_back({.offset = offset, .size = type_info.size, .destruct_fn = type_info.destroy});
            }

            return destruction_cache;
        }

        move_plan& get_move_plan() {
            if (move_cache.steps.size())
                return move_cache;

            for (auto& [component, pos] : component_index_map) {
                const auto& type_info = detail::component_info_registry[component];
                size_t offset = layout.component_offsets[pos];
                move_cache.steps.push_back({.offset = offset, .size = type_info.size, .move_fn = type_info.move_construct});
            }

            return move_cache;
        }

        migration_plan& get_migration_plan(archetype* to) {
            auto it = migration_cache.find(to);
            if (it != migration_cache.end())
                return *it->second.get();

            auto plan = std::make_unique<migration_plan>();

            for (size_t i = 0; i < to->component_ids.size(); ++i) {
                component_id cid = to->component_ids[i];
                const auto& info = detail::component_info_registry[cid];

                auto it_src = component_index_map.find(cid);

                if (it_src != component_index_map.end()) {
                    size_t src_comp_idx = it_src->second;

                    plan->steps.push_back({.src_offset = layout.component_offsets[src_comp_idx], .dst_offset = to->layout.component_offsets[i], .size = info.size, .move_fn = info.is_trivial ? nullptr : info.move_construct});
                } else
                    plan->missing_steps.push_back({.dst_offset = to->layout.component_offsets[i], .size = info.size, .construct_fn = info.construct});
            }

            migration_plan* ptr = plan.get();
            migration_cache[to] = std::move(plan);
            return *ptr;
        }

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
                for (uint32_t i = 0; i < component_ids.size(); ++i) {
                    whole_component_id id = component_ids[i];
                    const auto& info = detail::component_info_registry[id];

                    current_offset = (current_offset + info.alignment - 1) & ~(info.alignment - 1);

                    layout.component_offsets[i] = current_offset;
                    current_offset += info.size * CHUNK_CAPACITY;
                    component_index_map[id] = i;
                }

                constexpr auto atomics_alignment = std::hardware_destructive_interference_size;
                constexpr auto atomics_map_size = (CHUNK_CAPACITY + (sizeof(std::atomic_uint64_t) * 8) - 1) / (sizeof(std::atomic_uint64_t) * 8) * sizeof(std::atomic_uint64_t);

                for (size_t i = 0; i < component_ids.size(); ++i) {
                    current_offset = (current_offset + atomics_alignment - 1) & ~(atomics_alignment - 1);
                    layout.dirty_flags_offsets[i] = current_offset;
                    current_offset += atomics_map_size;
                }

                layout.chunk_size_bytes = current_offset;

                component_calculate_relations.reserve(component_ids.size());
                for (auto& id : component_ids) {
                    auto& com_reg = detail::component_info_registry[id];
                    if (com_reg.get_flat_relations)
                        component_calculate_relations.emplace_back(
                            layout.component_offsets[id],
                            com_reg.size,
                            com_reg.get_flat_relations
                        );
                }
                component_calculate_relations.shrink_to_fit();
            }
        }

        bool matches_query(
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<whole_component_id> with_tag_components,
            std::span<whole_component_id> without_tag_components
        ) {
            for (auto component : components)
                if (!component_index_map.contains(component))
                    return false;
            for (auto component : with_components)
                if (!component_index_map.contains(component))
                    return false;
            for (auto component : without_components)
                if (component_index_map.contains(component))
                    return false;
            for (auto component : with_tag_components)
                if (!whole_component_presence_helper.contains(component))
                    return false;
            for (auto component : without_tag_components)
                if (whole_component_presence_helper.contains(component))
                    return false;
            return true;
        }

        //tries to get free chunk from allocated ones and allocates new one if there no free chunks for world or global space
        chunk* get_free_chunk() {
            if (!available_chunks.empty())
                return chunks[available_chunks.back()].get();


            auto new_chunk = std::make_unique<chunk>(std::make_unique_for_overwrite<char[]>(layout.chunk_size_bytes));
            chunk* chunk_ptr = new_chunk.get();

            chunks.push_back(std::move(new_chunk));
            uint32_t new_chunk_index = uint32_t(chunks.size() - 1);
            chunk_ptr->global_index = new_chunk_index;

            available_chunks.push_back(new_chunk_index);
            chunk_ptr->last_free_list_index = uint32_t(available_chunks.size() - 1);

            return chunk_ptr;
        }

        void update_freelists_after_swap(chunk* moved_chunk, uint32_t new_idx) {
            available_chunks[moved_chunk->last_free_list_index] = new_idx;
        }

        void release_empty_chunk_swap_pop(uint32_t index_to_remove) {
            uint32_t last_index = uint32_t(chunks.size() - 1);
            chunk* moved_chunk = chunks[last_index].get();

            if (index_to_remove != last_index) {
                std::swap(chunks[index_to_remove], chunks[last_index]);
                moved_chunk->global_index = index_to_remove;
                update_freelists_after_swap(moved_chunk, index_to_remove);
            }

            chunks.pop_back();
        }

        void remove_from_free_list(chunk* chunk_to_remove) {
            available_chunks[chunk_to_remove->last_free_list_index] = available_chunks.back();
            chunks[available_chunks.back()]->last_free_list_index = chunk_to_remove->last_free_list_index;
            available_chunks.pop_back();
        }

        void add_to_free_list(chunk* chunk_to_add) {
            available_chunks.push_back(chunk_to_add->global_index);
            chunk_to_add->last_free_list_index = uint32_t(available_chunks.size() - 1);
        }

        //moves the entity from the back of the chunk to the position of the removed entity
        void compact_chunk(std::vector<entity_record>& records, chunk* chunk, int32_t old_pos) {
            if (chunk->entity_count == 0)
                return;

            auto index = chunk->entity_count - 1;
            int32_t last_entity_id = chunk->entities()[index];

            chunk->entities()[old_pos] = last_entity_id;
            get_move_plan().execute(chunk->memory_block.get(), chunk->memory_block.get(), index, old_pos);
        }

        void request_all_relations(entity_record& record, relation_visitor& visitor) {
            for (auto& [component_offset, size, fn] : component_calculate_relations)
                fn(record.chunk->memory_block.get() + component_offset + (record.chunk_index * size), visitor);
        }
    };

    struct entity_destroy_queue_item {
        uint32_t id;
        uint32_t generation;
    };

    struct entity_dirty_mark_item {
        uint32_t id;
        uint32_t generation;
        component_id component;
    };

    struct world {
        int32_t id = 0;
        size_t usages = 0;
        fast_task::task_rw_mutex world_mutex;
        std::unordered_map<size_t, std::vector<archetype*>> archetype_lookup;
        std::vector<std::unique_ptr<archetype>> archetypes;


        moodycamel::ConcurrentQueue<detail::mutation_queue_item> mutation_queue;
        moodycamel::ConcurrentQueue<entity_destroy_queue_item> entity_destroy_queue;
        moodycamel::ConcurrentQueue<entity_dirty_mark_item> marking_queue;
        moodycamel::ConcurrentQueue<uint32_t> arch_type_changes;

        archetype* map_get_archetype(const std::vector<whole_component_id>& sorted_ids) {
            auto hash = archetype_hash()(sorted_ids);
            auto bucket_it = archetype_lookup.find(hash);
            if (bucket_it != archetype_lookup.end()) {
                for (archetype* arch : bucket_it->second) {
                    if (arch->whole_component_ids == sorted_ids)
                        return arch;
                }
            }

            auto new_archetype_ptr = std::make_unique<archetype>();
            new_archetype_ptr->whole_component_ids = sorted_ids;
            {
                auto& component_ids = new_archetype_ptr->component_ids;
                component_ids.reserve(sorted_ids.size());
                for (auto component : sorted_ids)
                    if (component <= UINT32_MAX)
                        component_ids.push_back(component);
                component_ids.shrink_to_fit();
            }
            new_archetype_ptr->hash = hash;
            new_archetype_ptr->component_index_map.reserve(sorted_ids.size());
            new_archetype_ptr->calculate_layout();

            auto arch = new_archetype_ptr.get();
            archetypes.push_back(std::move(new_archetype_ptr));
            archetype_lookup[hash].push_back(arch);
            return arch;
        }

        archetype* map_new_archetype_with(archetype* old, whole_component_id new_id) {
            std::vector<whole_component_id> next_key = old->whole_component_ids;
            next_key.push_back(new_id);
            std::sort(next_key.begin(), next_key.end());

            archetype* next_archetype = map_get_archetype(next_key);

            old->add_transition_cache[new_id] = next_archetype;
            next_archetype->remove_transition_cache[new_id] = old;
            return next_archetype;
        }

        archetype* map_new_archetype_without(archetype* old, whole_component_id old_id) {
            std::vector<whole_component_id> next_key = old->whole_component_ids;
            if (auto it = std::find(next_key.begin(), next_key.end(), old_id); next_key.end() != it) {
                next_key.erase(it);
                archetype* next_archetype = map_get_archetype(next_key);
                old->remove_transition_cache[old_id] = next_archetype;
                next_archetype->add_transition_cache[old_id] = old;
                return next_archetype;
            } else
                return old;
        }
    };

    struct local_entity_id_cache {
        std::vector<uint32_t> ids;
        static constexpr inline size_t ID_CACHE_SIZE = 128;

        local_entity_id_cache() = default;
        ~local_entity_id_cache();

        static local_entity_id_cache& get_local();
        uint32_t get_next_id();
        void recycle_id(uint32_t id);
    };

    struct manager {
        fast_task::task_rw_mutex manager_mutex;

        world limbo;
        std::unordered_map<int32_t, world> worlds;

        std::vector<entity_record> records;
        moodycamel::ConcurrentQueue<uint32_t> free_entity_ids;

        static void swap_remove(std::vector<uint32_t>& arr, uint32_t value) {
            auto it = std::find(arr.begin(), arr.end(), value);
            if (it != arr.end()) {
                std::swap(*it, arr.back());
                arr.pop_back();
            }
        }

        //internal, should be used only by implementation
        //transfers the entity across archetypes and/or worlds
        //requires lock
        void move_entity(uint32_t id, archetype* to, world* w) {
            auto& record = records.at(id);
            archetype* old_type = record.type;

            if (old_type == to && (record.chunk == nullptr))
                return;

            chunk* old_chunk = record.chunk;
            uint32_t old_index = record.chunk_index;

            chunk* target_chunk = to->get_free_chunk();
            uint32_t new_index = target_chunk->entity_count;

            target_chunk->entities()[new_index] = id;
            migration_plan& plan = old_type->get_migration_plan(to);
            plan.execute(
                old_chunk->memory_block.get(),
                target_chunk->memory_block.get(),
                old_index,
                new_index
            );
            plan.execute_defaults(
                target_chunk->memory_block.get(),
                new_index
            );

            record.type = to;
            record.world_owner = w;
            record.chunk = target_chunk;
            record.chunk_index = new_index;
            target_chunk->entity_count++;

            old_chunk->entity_count--;
            old_type->compact_chunk(records, old_chunk, old_index);
        }

        //internal, should be used only by implementation
        //removes entity data, but keeps the record for reuse
        //requires lock
        void deallocate_entity(uint32_t id) {
            auto& record = records.at(id);
            bool was_full = (record.chunk->entity_count == CHUNK_CAPACITY);

            ++record.generation;
            --record.chunk->entity_count;

            record.type->compact_chunk(records, record.chunk, record.chunk_index);
            record.type->get_destruction_plan().execute(record.chunk->memory_block.get(), record.chunk_index);

            if (record.chunk->entity_count == 0) {
                record.type->release_empty_chunk_swap_pop(record.chunk->global_index);
                record.chunk = nullptr;
            } else if (was_full && record.chunk->entity_count == CHUNK_CAPACITY - 1)
                record.type->add_to_free_list(record.chunk);

            free_entity_ids.enqueue(id);
        }

        //internal, should be used only by implementation
        //locks on allocation, there's no locks if there free record
        entity allocate_entity(archetype* in, world* w) {
            uint32_t id = local_entity_id_cache::get_local().get_next_id();
            if (id == UINT32_MAX)
                return {UINT32_MAX, UINT32_MAX};
            auto& record = records.at(id);
            record.type = in;
            record.world_owner = w;
            record.chunk = in->get_free_chunk();
            record.chunk_index = record.chunk->entity_count;
            record.chunk->entities()[record.chunk_index] = id;
            record.chunk->entity_count++;
            if (record.chunk->entity_count == CHUNK_CAPACITY)
                in->remove_from_free_list(record.chunk);

            in->get_construction_plan().execute(record.chunk->memory_block.get(), record.chunk_index);
            record.generation++;
            return {id, record.generation};
        }

        bool has_entity(uint32_t id, uint32_t generation) {
            if (records.size() > id)
                if (records[id].generation == generation)
                    return true;
            return false;
        }

        world* get_world(int32_t id) {
            std::lock_guard guard(manager_mutex);
            return &worlds.at(id);
        }

        world* enable_world(int32_t id) {
            std::lock_guard guard(manager_mutex);
            auto& world = worlds[id];
            world.id = id;
            ++world.usages;
            return &world;
        }

        void disable_world(int32_t id) {
            std::lock_guard guard(manager_mutex);
            if (worlds[id].usages <= 1) {
                if (auto it = worlds.find(id); it != worlds.end()) {
                    for (auto& arch : it->second.archetypes) {
                        while (arch->chunks.size()) {
                            auto& chunk = arch->chunks.back();
                            auto entities = chunk->entities();
                            move_entity(entities[0], limbo.map_get_archetype(arch->whole_component_ids), &limbo);
                        }
                    }
                }
                worlds.erase(id);
            } else
                --worlds[id].usages;
        }

        static manager& instance();

        void bulk_release_ids(const std::vector<uint32_t>& ids) {
            if (ids.empty())
                return;
            free_entity_ids.enqueue_bulk(ids.data(), ids.size());
        }

        size_t bulk_acquire_ids(std::vector<uint32_t>& out_ids, size_t count) {
            size_t acquired = free_entity_ids.try_dequeue_bulk(std::back_inserter(out_ids), count);

            if (acquired >= count)
                return acquired;

            fast_task::unique_lock lock(manager_mutex);
            if (records.size() >= UINT32_MAX)
                return acquired;

            size_t old_size = records.size();
            size_t growth = std::max<size_t>(1000, count - acquired);
            size_t new_size = std::min<size_t>(static_cast<size_t>(records.size()) + growth, UINT32_MAX);

            if (new_size == old_size)
                return acquired;

            records.resize(new_size);

            size_t current_id = old_size;
            size_t needed = count - acquired;

            while (needed > 0 && current_id < new_size) {
                out_ids.push_back(static_cast<uint32_t>(current_id++));
                needed--;
                acquired++;
            }

            if (current_id < new_size) {
                std::vector<uint32_t> batch_creation;
                batch_creation.reserve(new_size - current_id);
                for (; current_id < new_size; ++current_id)
                    batch_creation.push_back(static_cast<uint32_t>(current_id));
                free_entity_ids.enqueue_bulk(batch_creation.data(), batch_creation.size());
            }

            return acquired;
        }
    };
}

#endif /* SRC_API_BIN_ECS_MANAGER */
