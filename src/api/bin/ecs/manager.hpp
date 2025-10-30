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

    struct entity_record {
        archetype* type;
        chunk* chunk;
        world* world_owner;
        std::atomic<archetype*> archetype_before_mutation = nullptr;
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

    struct archetype {
        std::vector<component_id> component_ids;
        size_t hash;
        std::vector<std::unique_ptr<chunk>> chunks;

        std::vector<uint32_t> available_chunks;

        std::unordered_map<component_id, archetype*> add_transition_cache;
        std::unordered_map<component_id, archetype*> remove_transition_cache;
        std::unordered_map<component_id, uint32_t> component_index_map;

        struct chunk_layout {
            std::vector<size_t> component_offsets;
            std::vector<size_t> dirty_flags_offsets;
            size_t chunk_size_bytes = 0;
        };

        chunk_layout layout;
        fast_task::task_rw_mutex arch_mutex;

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
                    component_id id = component_ids[i];
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
            }
        }

        bool matches_query(std::span<component_id> components, std::span<component_id> with_components, std::span<component_id> without_components) {
            for (auto component : components)
                if (!component_index_map.contains(component))
                    return false;
            for (auto component : with_components)
                if (!component_index_map.contains(component))
                    return false;
            for (auto component : without_components)
                if (component_index_map.contains(component))
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
            for (size_t i = 0; i < component_ids.size(); ++i) {
                component_id id_to_process = component_ids[i];
                const auto& type_info = detail::component_info_registry[id_to_process];
                size_t offset = layout.component_offsets[i];

                void* dest_ptr = chunk->memory_block.get() + offset + (old_pos * type_info.size);
                void* src_ptr = chunk->memory_block.get() + offset + (index * type_info.size);

                type_info.move(dest_ptr, src_ptr);
            }

            records.at(last_entity_id).chunk_index = old_pos;
        }
    };

    struct entity_destroy_queue_item {
        int32_t id;
        uint32_t generation;
    };

    struct entity_dirty_mark_item {
        int32_t id;
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
        moodycamel::ConcurrentQueue<int32_t> arch_type_changes;

        archetype* map_get_archetype(const std::vector<component_id>& sorted_ids) {
            auto hash = archetype_hash()(sorted_ids);
            auto bucket_it = archetype_lookup.find(hash);
            if (bucket_it != archetype_lookup.end()) {
                for (archetype* arch : bucket_it->second) {
                    if (arch->component_ids == sorted_ids)
                        return arch;
                }
            }

            auto new_archetype_ptr = std::make_unique<archetype>();
            new_archetype_ptr->component_ids = sorted_ids;
            new_archetype_ptr->hash = hash;
            new_archetype_ptr->component_index_map.reserve(sorted_ids.size());
            new_archetype_ptr->calculate_layout();

            auto arch = new_archetype_ptr.get();
            archetypes.push_back(std::move(new_archetype_ptr));
            archetype_lookup[hash].push_back(arch);
            return arch;
        }

        archetype* map_new_archetype(archetype* old, component_id new_id) {
            std::vector<component_id> next_key = old->component_ids;
            next_key.push_back(new_id);
            std::sort(next_key.begin(), next_key.end());

            archetype* next_archetype = map_get_archetype(next_key);

            old->add_transition_cache[new_id] = next_archetype;
            next_archetype->remove_transition_cache[new_id] = old;
            return next_archetype;
        }
    };

    struct manager {
        fast_task::task_rw_mutex manager_mutex;

        world limbo;
        std::unordered_map<int32_t, world> worlds;

        std::vector<entity_record> records;
        list_array<int32_t> free_entity_ids;


        fast_task::task_rw_mutex relation_mutex;
        std::unordered_map<component_id, std::unordered_map<int32_t, std::vector<int32_t>>> relation_index;         //component -> parent -> child
        std::unordered_map<int32_t, std::unordered_map<component_id, std::vector<int32_t>>> reverse_relation_index; //child -> component -> parent
        std::unordered_map<int32_t, std::unordered_set<component_id>> defined_relations;                            //entity -> component

        static void swap_remove(std::vector<int32_t>& arr, int32_t value) {
            auto it = std::find(arr.begin(), arr.end(), value);
            if (it != arr.end()) {
                std::swap(*it, arr.back());
                arr.pop_back();
            }
        }

        //internal, should be used only by implementation
        //transfers the entity across archetypes and/or worlds
        void move_entity(int32_t id, archetype* to, world* w) {
            auto& record = records.at(id);
            archetype* old_type = record.type;

            if (old_type == to && (record.chunk == nullptr))
                return;

            chunk* old_chunk = record.chunk;
            uint32_t old_index = record.chunk_index;

            chunk* target_chunk = to->get_free_chunk();
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

                    type_info.move_construct(dest_ptr, src_ptr);
                } else
                    type_info.construct(dest_ptr);
            }

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
        void deallocate_entity(int32_t id) {
            auto& record = records.at(id);
            bool was_full = (record.chunk->entity_count == CHUNK_CAPACITY);

            --record.chunk->entity_count;
            ++record.generation;

            record.type->compact_chunk(records, record.chunk, record.chunk_index);


            if (record.chunk->entity_count == 0) {
                record.type->release_empty_chunk_swap_pop(record.chunk->global_index);
                record.chunk = nullptr;
            } else if (was_full && record.chunk->entity_count == CHUNK_CAPACITY - 1)
                record.type->add_to_free_list(record.chunk);

            free_entity_ids.push_back(id);

            fast_task::shared_lock guard(relation_mutex);
            for (auto& relations : defined_relations[id]) {
                relation_index[relations].erase(id);
                if (relation_index[relations].empty())
                    relation_index.erase(relations);
            }
            defined_relations.erase(id);
            for (auto& relations : reverse_relation_index[id]) {
                relation_index[relations.first].erase(id);
                if (relation_index[relations.first].empty())
                    relation_index.erase(relations.first);
            }
            reverse_relation_index.erase(id);
            guard.unlock();

            for (auto& [component, pos] : record.type->component_index_map) {
                const auto& type_info = detail::component_info_registry[component];
                size_t offset = record.type->layout.component_offsets[pos];
                void* dest_ptr = record.chunk->memory_block.get() + offset + (record.chunk_index * type_info.size);
                type_info.destroy(dest_ptr);
            }
        }

        //internal, should be used only by implementation
        entity allocate_entity(archetype* in, world* w) {
            int32_t id;
            if (free_entity_ids.empty()) {
                id = int32_t(records.size());
                records.resize(records.size() + 1);
            } else
                id = free_entity_ids.take_front();
            auto& record = records.at(id);
            record.type = in;
            record.world_owner = w;
            record.chunk = in->get_free_chunk();
            record.chunk_index = record.chunk->entity_count;
            record.chunk->entities()[record.chunk_index] = id;
            record.chunk->entity_count++;
            if (record.chunk->entity_count == CHUNK_CAPACITY)
                in->remove_from_free_list(record.chunk);

            for (auto& [component, pos] : in->component_index_map) {
                const auto& type_info = detail::component_info_registry[component];
                size_t offset = in->layout.component_offsets[pos];
                void* dest_ptr = record.chunk->memory_block.get() + offset + (record.chunk_index * type_info.size);
                type_info.construct(dest_ptr);
            }

            record.generation = 0;
            return {id, record.generation};
        }

        //internal, should be used only by implementation
        void add_entity_relation(int32_t parent, int32_t child, component_id component) {
            relation_index[component][parent].push_back(child);
            reverse_relation_index[child][component].push_back(parent);
            defined_relations[parent].insert(component);
        }

        //internal, should be used only by implementation
        void remove_entity_relation(int32_t parent, int32_t child, component_id component) {
            auto& relation = relation_index[component][parent];
            swap_remove(relation, child);
            auto& reverse_relation = reverse_relation_index[child][component];
            swap_remove(reverse_relation, parent);

            if (relation.empty())
                relation_index.erase(component);
            if (reverse_relation.empty())
                reverse_relation_index.erase(child);

            defined_relations[parent].erase(component);
            if (defined_relations[parent].empty())
                defined_relations.erase(parent);
        }

        //internal, should be used only by implementation
        std::vector<int32_t> get_relation_query(std::span<std::pair<component_id, entity>> relations) {
            if (relations.empty()) 
                return {};

            auto& first_rel = relations[0];
            auto it_comp = relation_index.find(first_rel.first);
            if (it_comp == relation_index.end())
                return {};
            auto it_parent = it_comp->second.find(first_rel.second.id);
            if (it_parent == it_comp->second.end())
                return {};

            std::vector<int32_t> candidates = it_parent->second;

            for (size_t i = 1; i < relations.size(); ++i) {
                if (candidates.empty())
                    break;

                auto& rel = relations[i];
                std::unordered_set<int32_t> children_of_current_relation;
                if (auto it_c = relation_index.find(rel.first); it_c != relation_index.end()) {
                    if (auto it_p = it_c->second.find(rel.second.id); it_p != it_c->second.end()) {
                        children_of_current_relation.insert(it_p->second.begin(), it_p->second.end());
                    }
                }

                std::erase_if(candidates, [&](int32_t candidate_id) {
                    return children_of_current_relation.find(candidate_id) == children_of_current_relation.end();
                });
            }

            return candidates;
        }

        bool has_entity(int32_t id, uint32_t generation) {
            if (records.size() > id)
                if (records[id].generation == generation)
                    return true;
            return false;
        }

        void enable_world(int32_t id) {
            std::lock_guard guard(manager_mutex);
            auto& world = worlds[id];
            world.id = id;
            ++world.usages;
        }

        void disable_world(int32_t id) {
            std::lock_guard guard(manager_mutex);
            if (worlds[id].usages <= 1) {
                if (auto it = worlds.find(id); it != worlds.end()) {
                    for (auto& arch : it->second.archetypes) {
                        while (arch->chunks.size()) {
                            auto& chunk = arch->chunks.back();
                            auto entities = chunk->entities();
                            move_entity(entities[0], limbo.map_get_archetype(arch->component_ids), &limbo);
                        }
                    }
                }
                worlds.erase(id);
            } else
                --worlds[id].usages;
        }
    };
}

#endif /* SRC_API_BIN_ECS_MANAGER */
