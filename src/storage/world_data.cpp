/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <library/fast_task/include/files.hpp>
#include <src/api/configuration.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/entity.hpp>
#include <src/api/log.hpp>
#include <src/api/registers.hpp>
#include <src/api/tags.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/mojang/api/hash256.hpp>
#include <src/util/readers.hpp>

namespace copper_server::storage {
    using sub_chunk_data = base_objects::world::sub_chunk_data;

    template <class T>
    T convert_chunk_global_pos(T pos) {
        if (pos == 0)
            return 0;
        if (pos < 0)
            return (pos + 1) / 16 - 1;
        return pos / 16;
    }

    template <class T>
    T convert_chunk_local_pos(T pos) {
        if (pos == 0)
            return 0;
        if (pos < 0)
            return 16 + (pos % 16);
        return pos % 16;
    }

#define TO_WORLD_POS_GLOBAL(new_value, raw_value)            \
    int32_t new_value = int32_t(raw_value) + world_y_offset; \
    assert(new_value <= 0 && "Invalid block position, y axis located outside world bound");

#define TO_WORLD_POS_CHUNK(new_value, raw_value)                   \
    int32_t new_value = int32_t(raw_value) + world_y_chunk_offset; \
    assert(new_value <= 0 && "Invalid block position, y axis located outside world bound");

    fast_task::protected_value<boost::unordered_flat_map<std::string, std::shared_ptr<chunk_generator>>> chunk_generators;

    void chunk_generator::register_it(const std::string& id, std::shared_ptr<chunk_generator> gen) {
        chunk_generators.set([&](auto& map) {
            map[id] = std::move(gen);
        });
    }

    void chunk_generator::unregister_it(const std::string& id) {
        chunk_generators.set([&](auto& map) {
            map.erase(id);
        });
    }

    std::shared_ptr<chunk_generator> chunk_generator::get_it(const std::string& id) {
        return chunk_generators.set([&](auto& map) {
            return map.at(id);
        });
    }

    void chunk_generator::process_complete(world_data& world, base_objects::world::chunk_data& chunk) {
        chunk.update_metadata(world.get_world_y_offset());
        world.reset_light_data(chunk.chunk_x, chunk.chunk_z);
        chunk.generator_stage = 0xFF;
    }

    fast_task::protected_value<boost::unordered_flat_map<std::string, std::shared_ptr<chunk_light_processor>>> light_processors;

    void chunk_light_processor::register_it(const std::string& id, std::shared_ptr<chunk_light_processor> processor) {
        light_processors.set([&](auto& map) {
            map[id] = std::move(processor);
        });
    }

    void chunk_light_processor::unregister_it(const std::string& id) {
        light_processors.set([&](auto& map) {
            map.erase(id);
        });
    }

    std::shared_ptr<chunk_light_processor> chunk_light_processor::get_it(const std::string& id) {
        return light_processors.set([&](auto& map) {
            return map.at(id);
        });
    }

    void world_data::make_save(int32_t chunk_x, int32_t chunk_z, bool also_unload) {
        int32_t rx = chunk_x >> 5;
        int32_t rz = chunk_z >> 5;

        std::unique_lock lock(mutex);
        auto it = regions.find(region_key(rx, rz));

        if (it != regions.end()) {
            auto region = it->second;
            lock.unlock();
            make_save(chunk_x, chunk_z, region, also_unload);
        }
    }

    void world_data::make_save(int32_t chunk_x, int32_t chunk_z, std::shared_ptr<base_objects::world::chunk_region> item, bool also_unload) {
        if (auto process = on_save_process.find({chunk_x, chunk_z}); process == on_save_process.end()) {
            auto& chunk = item->get(static_cast<uint8_t>(chunk_x), static_cast<uint8_t>(chunk_z));
            on_save_process[{chunk_x, chunk_z}] = fast_task::future<void>::start(
                [this, chunk, chunk_x, chunk_z, also_unload, item] {
                    region_manager.write_chunk(chunk_x, chunk_z, chunk, tick_counter)->wait();
                    std::unique_lock lock(mutex);
                    on_save_process.erase({chunk_x, chunk_z});
                    if (also_unload) {
                        item->unload(chunk_x, chunk_z);
                        if (profiling.enable_world_profiling) {
                            if (profiling.chunk_total_loaded)
                                --profiling.chunk_total_loaded;
                            if (profiling.chunk_unloaded)
                                profiling.chunk_unloaded(*this, chunk_x, chunk_z);
                        }
                    }

                    return true;
                }
            );
        }
    }

    fast_task::future_ptr<std::shared_ptr<base_objects::world::chunk_data>> world_data::create_chunk_generate_future(std::shared_ptr<base_objects::world::chunk_data>& chunk) {
        if (profiling.enable_world_profiling)
            ++profiling.chunk_generator_counter;

        return fast_task::future<std::shared_ptr<base_objects::world::chunk_data>>::start(
            [this, chunk = chunk]() {
                auto gen = get_generator();
                fast_task::mutex_unify unify(generator.mutex);
                std::unique_lock lock(unify);
                ++generator.count;
                generator.chunks_next_blocking_stage = generator.lowest_sync_stage;
                while (chunk->generator_stage != 0xFF) {
                    while (chunk->generator_stage >= generator.chunks_next_blocking_stage && !generator.sync_mode) { //sync all tasks to switch mode
                        ++generator.lock_count;
                        if (generator.lock_count < generator.count)
                            generator.notifier.wait(lock);
                        else {
                            generator.sync_mode = true;
                            generator.notifier.notify_all();
                        }
                        --generator.lock_count;
                    }

                    while (chunk->generator_stage > generator.chunks_next_blocking_stage) { //block all tasks that has too big mode
                        ++generator.stage_complete_count;
                        if (generator.stage_complete_count < generator.count)
                            generator.notifier.wait(lock);
                        else {
                            generator.next_stage_sync();
                            generator.notifier.notify_all();
                        }
                        --generator.stage_complete_count;
                    }

                    bool make_lock = generator.sync_mode && chunk->generator_stage == generator.chunks_next_blocking_stage;
                    lock.unlock();
                    std::unique_lock sync_guard(generator.limiter, std::defer_lock);
                    if (make_lock)
                        sync_guard.lock();
                    gen->process_chunk(*this, *chunk, chunk->generator_stage);
                    if (chunk->load_level > chunk->resume_gen_level)
                        break;
                }
                --generator.count;
                if (profiling.enable_world_profiling)
                    --profiling.chunk_generator_counter;
                return chunk;
            }
        );
    }

    std::shared_ptr<base_objects::world::chunk_data> world_data::load_chunk_sync(int32_t chunk_x, int32_t chunk_z) {
        try {

            auto chunk = region_manager.get_chunk(chunk_x, chunk_z)->take();
            if (!chunk) {
                chunk = std::make_shared<base_objects::world::chunk_data>(chunk_x, chunk_z);
                chunk->sub_chunks.resize(get_chunk_y_count());
                chunk->generator_stage = 0;
            }

            {
                int32_t rx = chunk_x >> 5;
                int32_t rz = chunk_z >> 5;
                std::unique_lock lock(mutex);
                regions[region_key(rx, rz)]->set(static_cast<uint8_t>(chunk_x), static_cast<uint8_t>(chunk_z), chunk);
            }

            if (chunk->generator_stage != 0xFF) {
                chunk->load_level = 31;
                std::unique_lock lock(mutex);
                if (auto process = on_generate_process.find({chunk_x, chunk_z}); process == on_generate_process.end()) {
                    auto it = on_generate_process[{chunk_x, chunk_z}] = create_chunk_generate_future(chunk);
                    it->wait_with(lock);
                    on_generate_process.erase({chunk_x, chunk_z});
                    return it->get();
                } else {
                    auto fut = process->second;
                    fut->wait_with(lock);
                    return fut->get();
                }
            } else {
                chunk->load_level = 34;
                get_light_processor(); //cache light proc
                int32_t y = (int32_t)std::min<size_t>(chunk->sub_chunks.size(), INT32_MAX);
                auto end = chunk->sub_chunks.rend();
                bool done_process = false;
                for (auto beg = chunk->sub_chunks.rbegin(); beg != end; beg++) {
                    --y;
                    if (beg->need_to_recalculate_light || done_process) {
                        light_processor->process_sub_chunk(*this, chunk_x, y, chunk_z);
                        done_process = true;
                    }
                }
                chunk->update_metadata(world_y_offset);
            }
            return chunk;
        } catch (...) {
            return nullptr;
        }
    }

    std::shared_ptr<chunk_generator>& world_data::get_generator() {
        if (!generator.process) {
            generator.process = chunk_generator::get_it(light_processor_id);
            generator.calculate();
        }
        return generator.process;
    }

    std::shared_ptr<chunk_light_processor>& world_data::get_light_processor() {
        if (!light_processor) {
            light_processor = chunk_light_processor::get_it(light_processor_id);
            enable_entity_light_source_updates = light_processor->enable_entity_light_source_updates;
            enable_entity_light_source_updates_include_rot = light_processor->enable_entity_light_source_updates_include_rot;
        }
        return light_processor;
    }

    template <auto fun, class... Args>
    inline void entity_notify_block(auto world, auto& entities, auto self, auto x, auto y, auto z, Args&&... args) {
        auto chunk_x = convert_chunk_global_pos(x);
        auto chunk_z = convert_chunk_global_pos(z);
        for (auto& [id, entity] : entities) {
            if (entity != self) {
                auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == world)
                    if ((*processor).*fun)
                        if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(chunk_x, chunk_z))
                            ((*processor).*fun)(entity, self, x, y, z, std::forward<Args>(args)...);
            }
        }
    }

    template <auto fun, class... Args>
    inline void entity_notify_change(auto world, auto& entities, auto self, Args&&... args) {
        auto pos = self.template get<api::ecs::com::entities::position>();
        auto chunk_x = convert_chunk_global_pos(pos.x);
        auto chunk_z = convert_chunk_global_pos(pos.z);
        for (auto& [id, entity] : entities) {
            if (entity != self) {
                auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == world)
                    if ((*processor).*fun)
                        if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds((int32_t)chunk_x, (int32_t)chunk_z))
                            ((*processor).*fun)(entity, self, std::forward<Args>(args)...);
            }
        }
    }

    template <auto fun, class... Args>
    inline void entity_notify_change_all(auto world, auto& entities, auto self, Args&&... args) {
        auto& pos = self.template get<api::ecs::com::entities::position>();
        auto chunk_x = convert_chunk_global_pos(pos.x);
        auto chunk_z = convert_chunk_global_pos(pos.z);
        for (auto& [id, entity] : entities) {
            auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
            if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == world)
                if ((*processor).*fun)
                    if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds((int32_t)chunk_x, (int32_t)chunk_z))
                        ((*processor).*fun)(entity, self, std::forward<Args>(args)...);
        }
    }

    template <auto fun, class... Args>
    void entity_notify_change_w_e(auto world, auto& entities, auto self, auto other_entity_id, Args&&... args) {
        auto other_entity_it = entities.find(other_entity_id);
        if (other_entity_it == entities.end())
            throw std::runtime_error("Entity not registered on world");
        auto& pos = self.template get<api::ecs::com::entities::position>();
        auto chunk_x = convert_chunk_global_pos(pos.x);
        auto chunk_z = convert_chunk_global_pos(pos.z);
        auto& other_entity = other_entity_it->second;
        for (auto& [id, entity] : entities) {
            auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
            if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == world)
                if ((*processor).*fun)
                    if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds((int32_t)chunk_x, (int32_t)chunk_z))
                        ((*processor).*fun)(entity, self, other_entity, std::forward<Args>(args)...);
        }
    }

    template <auto fun, class... Args>
    void world_notify(auto world, auto& entities, auto x, auto z, Args&&... args) {
        auto chunk_x = convert_chunk_global_pos(x);
        auto chunk_z = convert_chunk_global_pos(z);
        for (auto& [id, entity] : entities) {
            auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
            if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == world) {
                if ((*processor).*fun)
                    if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(chunk_x, chunk_z))
                        ((*processor).*fun)(entity, std::forward<Args>(args)...);
            }
        }
    }

#define WORLD_ASYNC_RUN(function, ...) \
    fast_task::task::run([=, this] { api::world::get(world_id, [&](auto& world) { world.function(__VA_ARGS__); }); })

    void world_data::entity_init(api::ecs::entity self) {
        std::unique_lock lock(mutex);
        auto& pos = self.template get<api::ecs::com::entities::position>();
        auto chunk_x = convert_chunk_global_pos(pos.x);
        auto chunk_z = convert_chunk_global_pos(pos.z);
        for (auto& [id, entity] : entities) {
            if (entity == self)
                continue;
            auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
            if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                if (processor->entity_init)
                    if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds((int32_t)chunk_x, (int32_t)chunk_z))
                        processor->entity_init(entity, self);
            }
        }
    }

    using ew_processor = api::entity_data::world_processor;

    void world_data::entity_teleport(api::ecs::entity self, util::vector new_pos) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_teleport>(this, entities, self, new_pos);
        if (enable_entity_light_source_updates)
            get_light_processor()->process_entity_light_source(*this, self, new_pos);
    }

    void world_data::entity_move(api::ecs::entity self, util::vector move) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_move>(this, entities, self, move);
        if (enable_entity_light_source_updates)
            get_light_processor()->process_entity_light_source(*this, self, move);
    }

    void world_data::entity_look_changes(api::ecs::entity self, util::angle_deg new_rotation) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_look_changes>(this, entities, self, new_rotation);
        if (enable_entity_light_source_updates_include_rot)
            get_light_processor()->process_entity_light_source_rot(*this, self, new_rotation);
    }

    void world_data::entity_rotation_changes(api::ecs::entity self, util::angle_deg new_rotation) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_rotation_changes>(this, entities, self, new_rotation);
    }

    void world_data::entity_motion_changes(api::ecs::entity self, util::vector new_motion) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_motion_changes>(this, entities, self, new_motion);
    }

    void world_data::entity_rides(api::ecs::entity self, size_t other_entity_id) {
        if (self.is_assigned_to_world(world_id)) {
            std::unique_lock lock(mutex);
            entities.at(other_entity_id).template modify<api::ecs::com::entities::ride_by_entity>()->ride_by.emplace_back(entities.at(self.template get<api::ecs::com::entities::world_syncing>().assigned_world_id));
            entity_notify_change_w_e<&ew_processor::entity_rides>(this, entities, self, other_entity_id);
        }
    }

    void world_data::entity_leaves_ride(api::ecs::entity self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entities.at(other_entity_id).template modify<api::ecs::com::entities::ride_by_entity>()->ride_by.remove_if([&self](auto it) {
            return it == self;
        });
        entity_notify_change_w_e<&ew_processor::entity_leaves_ride>(this, entities, self, other_entity_id);
    }

    void world_data::entity_attach(api::ecs::entity self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entity_notify_change_w_e<&ew_processor::entity_attach>(this, entities, self, other_entity_id);
    }

    void world_data::entity_detach(api::ecs::entity self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entity_notify_change_w_e<&ew_processor::entity_detach>(this, entities, self, other_entity_id);
    }

    void world_data::entity_damage(api::ecs::entity self, float health, int32_t type_id, std::optional<util::vector> pos) {
        entity_notify_change_all<&ew_processor::entity_damage>(this, entities, self, health, type_id, pos);
    }

    void world_data::entity_damage(api::ecs::entity self, float health, int32_t type_id, std::optional<api::ecs::entity> source, std::optional<util::vector> pos) {
        entity_notify_change_all<&ew_processor::entity_damage_with_source>(this, entities, self, health, type_id, source, pos);
    }

    void world_data::entity_damage(api::ecs::entity self, float health, int32_t type_id, std::optional<api::ecs::entity> source, std::optional<api::ecs::entity> source_direct, std::optional<util::vector> pos) {
        entity_notify_change_all<&ew_processor::entity_damage_with_sources>(this, entities, self, health, type_id, source, source_direct, pos);
    }

    void world_data::entity_attack(api::ecs::entity self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entity_notify_change_w_e<&ew_processor::entity_attack>(this, entities, self, other_entity_id);
    }

    void world_data::entity_iteract(api::ecs::entity self, size_t other_entity_id) {
        std::unique_lock lock(mutex);
        entity_notify_change_w_e<&ew_processor::entity_iteract>(this, entities, self, other_entity_id);
    }

    void world_data::entity_iteract(api::ecs::entity self, int32_t x, int32_t y, int32_t z) {
        std::unique_lock lock(mutex);
        entity_notify_block<&ew_processor::entity_iteract_block>(this, entities, self, x, y, z);
    }

    void world_data::entity_break(api::ecs::entity self, int32_t x, int32_t y, int32_t z, uint8_t state) {
        if (state > 9)
            return;
        std::unique_lock lock(mutex);
        entity_notify_block<&ew_processor::entity_break>(this, entities, self, x, y, z, state);
    }

    void world_data::entity_cancel_break(api::ecs::entity self, int32_t x, int32_t y, int32_t z) {
        std::unique_lock lock(mutex);
        entity_notify_block<&ew_processor::entity_cancel_break>(this, entities, self, x, y, z);
    }

    void world_data::entity_finish_break(api::ecs::entity self, int32_t x, int32_t y, int32_t z) {
        std::unique_lock lock(mutex);
        entity_notify_block<&ew_processor::entity_finish_break>(this, entities, self, x, y, z);
    }

    void world_data::entity_place(api::ecs::entity self, bool is_main_hand, int32_t x, int32_t y, int32_t z, base_objects::block block) {
        std::unique_lock lock(mutex);
        for (auto& [id, entity] : entities)
            if (entity != self) {
                auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                    if (processor->entity_place_block)
                        if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(convert_chunk_global_pos(x), convert_chunk_global_pos(z)))
                            processor->entity_place_block(entity, self, is_main_hand, x, y, z, block);
                }
            }
    }

    void world_data::entity_place(api::ecs::entity self, bool is_main_hand, int32_t x, int32_t y, int32_t z, api::ecs::entity block) {
        std::unique_lock lock(mutex);
        for (auto& [id, entity] : entities)
            if (entity != self) {
                auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                    if (processor->entity_place_block_entity)
                        if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(convert_chunk_global_pos(x), convert_chunk_global_pos(z)))
                            processor->entity_place_block_entity(self, entity, is_main_hand, x, y, z, block);
                }
            }
    }

    void world_data::entity_animation(api::ecs::entity self, base_objects::entity_animation animation) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_animation>(this, entities, self, animation);
    }

    void world_data::entity_event(api::ecs::entity self, base_objects::entity_event status) {
        entity_notify_change<&ew_processor::entity_event>(this, entities, self, status);
    }

    void world_data::entity_add_effect(api::ecs::entity self, uint32_t effect_id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
        entity_notify_change_all<&ew_processor::entity_add_effect>(this, entities, self, effect_id, duration, amplifier, ambient, show_particles, show_icon, use_blend);
    }

    void world_data::entity_remove_effect(api::ecs::entity self, uint32_t effect_id) {
        entity_notify_change_all<&ew_processor::entity_remove_effect>(this, entities, self, effect_id);
    }

    void world_data::entity_death(api::ecs::entity self) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_death>(this, entities, self);
    }

    void world_data::entity_deinit(api::ecs::entity self) {
        std::unique_lock lock(mutex);
        entity_notify_change<&ew_processor::entity_deinit>(this, entities, self);
    }

    void world_data::notify_block_event(const base_objects::world::block_action& action, int32_t x, int32_t y, int32_t z) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_event>(this, entities, x, z, action, x, y, z);
    }

    void world_data::notify_block_change(int32_t x, int32_t y, int32_t z, base_objects::block block) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_change>(this, entities, x, z, x, y, z, block);
    }

    void world_data::notify_block_change(int32_t x, int32_t y, int32_t z, api::ecs::entity block) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_entity_change>(this, entities, x, z, x, y, z, block);
    }

    void world_data::notify_block_destroy_change(int32_t x, int32_t y, int32_t z, base_objects::block block) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_destroy_change>(this, entities, x, z, x, y, z, block);
    }

    void world_data::notify_block_destroy_change(int32_t x, int32_t y, int32_t z, api::ecs::entity block) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_block_entity_destroy_change>(this, entities, x, z, x, y, z, block);
    }

    void world_data::notify_biome_change(int32_t x, int32_t y, int32_t z, uint32_t biome_id) {
        std::unique_lock lock(mutex);
        world_notify<&ew_processor::notify_biome_change>(this, entities, x, z, x, y, z, biome_id);
    }

    void world_data::notify_sub_chunk(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z) {
        get_sub_chunk(
            chunk_x,
            chunk_y,
            chunk_z,
            [this, chunk_x, chunk_y, chunk_z](auto& sub_chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                    if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                        if (processor->notify_sub_chunk)
                            if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(chunk_x, chunk_z))
                                processor->notify_sub_chunk(entity, chunk_x, chunk_y, chunk_z, sub_chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_chunk(int32_t chunk_x, int32_t chunk_z) {
        get_chunk(
            chunk_x,
            chunk_z,
            [this, chunk_x, chunk_z](auto& chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                    if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                        if (processor->notify_chunk)
                            if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(chunk_x, chunk_z))
                                processor->notify_chunk(entity, chunk_x, chunk_z, chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_sub_chunk_light(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z) {
        get_sub_chunk(
            chunk_x,
            chunk_y,
            chunk_z,
            [this, chunk_x, chunk_y, chunk_z](auto& sub_chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                    if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                        if (processor->notify_sub_chunk_light)
                            if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(chunk_x, chunk_z))
                                processor->notify_sub_chunk_light(entity, chunk_x, chunk_y, chunk_z, sub_chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_chunk_light(int32_t chunk_x, int32_t chunk_z) {
        get_chunk(
            chunk_x,
            chunk_z,
            [this, chunk_x, chunk_z](auto& chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                    if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                        if (processor->notify_chunk_light)
                            if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(chunk_x, chunk_z))
                                processor->notify_chunk_light(entity, chunk_x, chunk_z, chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_sub_chunk_blocks(int32_t chunk_x, int32_t chunk_y, int32_t chunk_z) {
        get_sub_chunk(
            chunk_x,
            chunk_y,
            chunk_z,
            [this, chunk_x, chunk_y, chunk_z](auto& sub_chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                    if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                        if (processor->notify_sub_chunk_blocks)
                            if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(chunk_x, chunk_z))
                                processor->notify_sub_chunk_blocks(entity, chunk_x, chunk_y, chunk_z, sub_chunk);
                    }
                }
            }
        );
    }

    void world_data::notify_chunk_blocks(int32_t chunk_x, int32_t chunk_z) {
        get_chunk(
            chunk_x,
            chunk_z,
            [this, chunk_x, chunk_z](auto& chunk) {
                for (auto& [id, entity] : entities) {
                    auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                    if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                        if (processor->notify_chunk_blocks)
                            if (entity.template get<api::ecs::com::entities::world_syncing>().processing_region.in_bounds(chunk_x, chunk_z))
                                processor->notify_chunk_blocks(entity, chunk_x, chunk_z, chunk);
                    }
                }
            }
        );
    }

    void world_data::__set_block_silent(base_objects::any_block block, int32_t global_x, int32_t global_y_raw, int32_t global_z, block_set_mode mode) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        base_objects::block updated_state;
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, std::move(block), current_world_reg);

            if (mode == block_set_mode::destroy){
                updated_state = sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15);
                if (updated_state.is_block_entity()) {
                    auto ee = sub_chunk.get_block_entity(global_x & 15, global_y & 15, global_z & 15);
                    WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y_raw, global_z, ee);
                } else 
                    WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y_raw, global_z, updated_state);
            }
        });

        __update_block(global_x, global_y_raw, global_z, mode, updated_state);
    }

    void world_data::__update_block(int32_t global_x, int32_t global_y_raw, int32_t global_z, block_set_mode mode, base_objects::block b) {
        if (mode != block_set_mode::keep) {
            if (b.is_liquid())
                if (auto general_id = b.general_block_id(); general_world_data.liquid.contains(general_id)){
                    query_for_liquid_tick(global_x, global_y_raw, global_z, tick_counter + general_world_data.liquid.at(general_id).spread_ticks);
                    return;
                }

            query_for_tick(global_x, global_y_raw, global_z, tick_counter);
        }
    }

    void world_data::tick_run_local_functions() { //TODO also synchronize across worlds and execute in id order to be vanilla complaint
        //TODO get custom function tag #copper_server:tick/0 or #copper_server:load/overworld
    }

    void world_data::tick_broadcast_time() {
        if (tick_counter % 20 == 0) {
            for (auto& [id, entity] : entities) {
                auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
                if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                    if (processor->sync_time)
                        processor->sync_time(entity, time, day_time);
                }
            }
        }
    }

    void world_data::tick_update_world_border() {
        //TODO
    }

    void world_data::tick_update_weather() {
        if (world_game_rules["doWeatherCycle"].as_byte()) {
            if (weather_time > 0) {
                --weather_time;
                if (weather_time == 0) {
                    current_weather = base_objects::weather::clear;
                    sync_weather();
                }
            } else if (clear_weather_time > 0) {
                --clear_weather_time;
            } else {
                std::mt19937_64 gen(std::random_device{}());
                std::uniform_int_distribution<uint16_t> dis_x(0, 1);
                current_weather = dis_x(gen) ? base_objects::weather::rain : base_objects::weather::thunder; //TODO get real chances
                sync_weather();
            }
        }
    }

    void world_data::tick_update_day_light() {
        if (world_game_rules["doDaylightCycle"].as_byte()) {
            if (time / 24'000 != 0) {
                day_time += time / 24'000;
                time = 0;
            } else
                ++time;
        }
    }

    void world_data::tick_run_local_scheduled_commands() {
        //TODO
    }

    void world_data::set_world_type(std::string_view type) {
        if (!world_type.empty())
            throw std::runtime_error("World type already been set.");
        world_type = std::string(type);
        auto& type_data = api::registers::dimension_types.at(world_type);
        chunk_y_count = type_data.height / 16;
        world_y_offset = type_data.min_y;
        world_y_chunk_offset = type_data.min_y ? type_data.min_y / 16 : 0;
    }

    void world_data::set_seed(int32_t seed) {
        world_seed = seed;
        util::mojang::api::hash256 hash;
        hash.update(&seed, sizeof(int32_t));
        hashed_seed_value = hash.to_part_hash();
    }

    void world_data::load(const util::nbt_compound& load_from_nbt) {
        general_world_data.other = load_from_nbt.at("general_world_data");
        if (general_world_data.other.contains("liquid"))
            for (auto& [block, settings] : general_world_data.other.at("liquid").get_compound())
                general_world_data.liquid[base_objects::block::get_block(block).general_block_id] = {.spread_size = settings.at("spread_size").as_short(), .spread_ticks = std::bit_cast<uint16_t>(settings.at("spread_ticks").as_short())};
        else
            general_world_data.liquid.clear();
        world_game_rules = load_from_nbt.at("world_game_rules");
        world_generator_data = load_from_nbt.at("world_generator_data");
        world_records = load_from_nbt.at("world_records");

        set_seed(load_from_nbt.at("world_seed").as_int());
        wandering_trader_id = load_from_nbt.at("wandering_trader_id").as_uuid();

        wandering_trader_spawn_chance = load_from_nbt.at("wandering_trader_spawn_chance").as_float();
        wandering_trader_spawn_delay = load_from_nbt.at("wandering_trader_spawn_delay").as_int();
        world_name = load_from_nbt.at("world_name").as_string();
        set_world_type(load_from_nbt.at("world_type").as_string());
        light_processor_id = load_from_nbt.at("light_processor_id").as_string();
        generator_id = load_from_nbt.at("generator_id").as_string();

        {
            auto& _spawn_data = load_from_nbt.at("spawn_data").get_compound();
            spawn_data.yaw = _spawn_data.at("yaw").as_float();
            spawn_data.pitch = _spawn_data.at("pitch").as_float();
            spawn_data.radius = _spawn_data.at("radius").as_int();
            spawn_data.x = _spawn_data.at("x").as_int();
            spawn_data.y = _spawn_data.at("y").as_int();
            spawn_data.z = _spawn_data.at("z").as_int();
        }

        border_center_x = load_from_nbt.at("border_center_x").as_double();
        border_center_z = load_from_nbt.at("border_center_z").as_double();
        border_size = load_from_nbt.at("border_size").as_double();
        border_safe_zone = load_from_nbt.at("border_safe_zone").as_double();
        border_damage_per_block = load_from_nbt.at("border_damage_per_block").as_double();
        border_lerp_target = load_from_nbt.at("border_lerp_target").as_double();
        border_lerp_time = load_from_nbt.at("border_lerp_time").as_long();
        border_warning_blocks = load_from_nbt.at("border_warning_blocks").as_double();
        border_warning_time = load_from_nbt.at("border_warning_time").as_double();
        day_time = load_from_nbt.at("day_time").as_long();
        time = load_from_nbt.at("time").as_int();
        ticks_per_second = load_from_nbt.at("ticks_per_second").as_long();
        portal_teleport_boundary = load_from_nbt.at("portal_teleport_boundary").as_int();
        ticking_frozen = load_from_nbt.at("ticking_frozen").as_byte();

        chunk_lifetime = std::chrono::milliseconds(load_from_nbt.at("chunk_lifetime").as_long());
        world_lifetime = std::chrono::milliseconds(load_from_nbt.at("world_lifetime").as_long());
        clear_weather_time = load_from_nbt.at("clear_weather_time").as_int();
        weather_time = load_from_nbt.at("weather_time").as_int();
        current_weather = base_objects::weather::from_string(load_from_nbt.at("current_weather").get_string());

        internal_version = load_from_nbt.at("internal_version").get_int();
        difficulty = load_from_nbt.at("difficulty").get_byte();
        default_gamemode = load_from_nbt.at("default_gamemode").get_byte();
        difficulty_locked = load_from_nbt.at("difficulty_locked").get_byte();
        is_hardcore = load_from_nbt.at("is_hardcore").get_byte();
        initialized = load_from_nbt.at("initialized").get_byte();
        has_skylight = load_from_nbt.at("has_skylight").get_byte();
        increase_time = load_from_nbt.at("increase_time").get_byte();
    }

    void world_data::load() {
        std::unique_lock lock(mutex);
        fast_task::files::async_iofstream file(
            path / "world.snbt",
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            throw std::runtime_error("Can't open world file");
        std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        load(util::nbt::from_snbt(res));
    }

    void world_data::save() {
        std::unique_lock lock(mutex);
        std::filesystem::create_directories(path);
        util::nbt_compound world_data_file;
        {
            util::nbt_compound res;
            res.reserve(general_world_data.liquid.size());
            for (auto& [it, data] : general_world_data.liquid)
                res[base_objects::block::get_block(it).name] = util::nbt_compound{{"spread_size", data.spread_size}, {"spread_ticks", data.spread_ticks}}.take_map();
            general_world_data.other["liquid"] = std::move(res).take_map();
        }

        world_data_file["general_world_data"] = general_world_data.other.get_map();
        world_data_file["world_game_rules"] = world_game_rules.get_map();
        world_data_file["world_generator_data"] = world_generator_data.get_map();
        world_data_file["world_records"] = world_records.get_map();

        world_data_file["world_seed"] = world_seed;
        world_data_file["wandering_trader_id"] = wandering_trader_id;


        world_data_file["wandering_trader_spawn_chance"] = wandering_trader_spawn_chance;
        world_data_file["wandering_trader_spawn_delay"] = wandering_trader_spawn_delay;
        world_data_file["world_name"] = world_name;
        world_data_file["world_type"] = world_type;
        world_data_file["light_processor_id"] = light_processor_id;
        world_data_file["generator_id"] = generator_id;

        world_data_file["spawn_data"] = util::nbt_compound{
            {"yaw", spawn_data.yaw},
            {"pitch", spawn_data.pitch},
            {"radius", spawn_data.radius},
            {"x", spawn_data.x},
            {"y", spawn_data.y},
            {"z", spawn_data.z}
        }.take_map();

        world_data_file["border_center_x"] = border_center_x;
        world_data_file["border_center_z"] = border_center_z;
        world_data_file["border_size"] = border_size;
        world_data_file["border_safe_zone"] = border_safe_zone;
        world_data_file["border_damage_per_block"] = border_damage_per_block;
        world_data_file["border_lerp_target"] = border_lerp_target;
        world_data_file["border_lerp_time"] = std::bit_cast<int64_t>(border_lerp_time);
        world_data_file["border_warning_blocks"] = border_warning_blocks;
        world_data_file["border_warning_time"] = border_warning_time;
        world_data_file["day_time"] = day_time;
        world_data_file["time"] = time;
        world_data_file["ticks_per_second"] = std::bit_cast<int64_t>(ticks_per_second);
        world_data_file["portal_teleport_boundary"] = portal_teleport_boundary;
        world_data_file["ticking_frozen"] = ticking_frozen;

        world_data_file["chunk_lifetime"] = chunk_lifetime.count();
        world_data_file["world_lifetime"] = world_lifetime.count();
        world_data_file["clear_weather_time"] = std::bit_cast<int32_t>(clear_weather_time);
        world_data_file["weather_time"] = std::bit_cast<int32_t>(weather_time);
        world_data_file["current_weather"] = current_weather.to_string();

        world_data_file["internal_version"] = internal_version;
        world_data_file["difficulty"] = difficulty;
        world_data_file["default_gamemode"] = default_gamemode;
        world_data_file["difficulty_locked"] = difficulty_locked;
        world_data_file["is_hardcore"] = is_hardcore;
        world_data_file["initialized"] = initialized;
        world_data_file["has_skylight"] = has_skylight;
        world_data_file["increase_time"] = increase_time;

        auto stringized = world_data_file.take_map().as_snbt();
        std::filesystem::create_directories(path);
        fast_task::files::atomic_async_ofstream file(path / "world.snbt");
        if (!file.is_open())
            throw std::runtime_error("Can't open world file");
        file.write(stringized.data(), stringized.size());
    }

    std::string world_data::preview_world_name() {
        std::unique_lock lock(mutex);
        fast_task::files::async_iofstream file(
            path / "world.snbt",
            fast_task::files::open_mode::read,
            fast_task::files::on_open_action::open,
            fast_task::files::_sync_flags{}
        );
        if (!file.is_open())
            throw std::runtime_error("Can't open world file");
        std::string res((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return util::nbt::from_snbt(res)["world_name"].as_string();
    }

    void world_data::sync_weather() {
        std::unique_lock lock(mutex);
        for (auto& [id, entity] : entities) {
            auto processor = entity.template get<api::ecs::com::entities::entity_type>().const_data().processor;
            if (processor && entity.template get<api::ecs::com::entities::world_syncing>().world == this) {
                if (processor->weather_change)
                    processor->weather_change(entity, weather_time, current_weather);
            }
        }
    }

    world_data::world_data(int32_t world_id, const std::filesystem::path& path)
        : path(path), world_id(world_id), limit_on_load(api::configuration::get().world.load_speed), current_world_reg(world_id), region_manager(path) {
        world_game_rules["reducedDebugInfo"] = api::configuration::get().game_play.reduced_debug_screen;
        if (!std::filesystem::exists(path))
            std::filesystem::create_directories(path);
        world_spawn_ticket_id = add_loading_ticket(
            base_objects::world::loading_point_ticket{
                [](auto&, auto, auto&) { return true; },
                {convert_chunk_global_pos(spawn_data.x),
                 convert_chunk_global_pos(spawn_data.z),
                 convert_chunk_global_pos(spawn_data.radius)},
                "Start ticket",
                22
            }
        );
        limit_on_load.enable();
    }

    void world_data::update_spawn_data(int32_t x, int32_t z, int32_t radius, float yaw, float pitch) {
        std::unique_lock lock(mutex);
        spawn_data = {x, z, radius, 0, yaw, pitch};
        loading_tickets.at(world_spawn_ticket_id).point = {
            convert_chunk_global_pos(x),
            convert_chunk_global_pos(z),
            convert_chunk_global_pos(radius)
        };
        get_height_maps_at(x, z, [&](base_objects::world::height_maps& height_maps) {
            auto mt = height_maps.motion_blocking.get(x % 16, z % 16);
            auto oc_flor = height_maps.ocean_floor.get(x % 16, z % 16);
            auto oc = height_maps.surface.get(x % 16, z % 16);
            spawn_data.y = std::max(mt, std::max(oc_flor, oc));
        });
    }

    size_t world_data::add_loading_ticket(base_objects::world::loading_point_ticket&& ticket) {
        std::unique_lock lock(mutex);
        size_t id = loading_tickets.size();
        while (loading_tickets.contains(id))
            ++id;
        loading_tickets.emplace(id, std::move(ticket));
        return id;
    }

    void world_data::remove_loading_ticket(size_t id) {
        std::unique_lock lock(mutex);
        loading_tickets.erase(id);
    }

    size_t world_data::loaded_chunks_count() {
        std::unique_lock lock(mutex);
        size_t count = 0;
        for (auto& [key, region] : regions)
            for (auto& chunk : region->chunks)
                count += (bool)chunk;
        return count;
    }

    bool world_data::exists(int32_t chunk_x, int32_t chunk_z) {
        int32_t rx = chunk_x >> 5;
        int32_t rz = chunk_z >> 5;
        std::unique_lock lock(mutex);
        auto it = regions.find(region_key(rx, rz));
        if (it != regions.end())
            if(it->second->get_unsafe(static_cast<uint8_t>(chunk_x), static_cast<uint8_t>(chunk_z)))
                return true;
        lock.unlock();
        return std::filesystem::exists(path / "chunks" / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"));
    }

    std::shared_ptr<base_objects::world::chunk_data> world_data::processed_load_chunk_sync(int32_t chunk_x, int32_t chunk_z, bool is_async_context) {
        auto chunk = load_chunk_sync(chunk_x, chunk_z);
        std::unique_lock lock(mutex);
        if (is_async_context) {
            on_load_process.erase({chunk_x, chunk_z});
            if (profiling.enable_world_profiling)
                --profiling.chunk_load_counter;
        }
        if (profiling.enable_world_profiling) {
            if (chunk) {
                ++profiling.chunk_total_loaded;
                if (profiling.chunk_loaded)
                    profiling.chunk_loaded(*this, *chunk);
                WORLD_ASYNC_RUN(notify_chunk, chunk_x, chunk_z);
            } else {
                if (profiling.chunk_load_failed)
                    profiling.chunk_load_failed(*this, chunk_x, chunk_z);
            }
        }
        return chunk;
    }

    fast_task::future_ptr<std::shared_ptr<base_objects::world::chunk_data>> world_data::create_chunk_load_future(int32_t chunk_x, int32_t chunk_z) {
        if (profiling.enable_world_profiling)
            ++profiling.chunk_load_counter;
        return fast_task::future<std::shared_ptr<base_objects::world::chunk_data>>::start(limit_on_load, [this, chunk_x, chunk_z]() -> std::shared_ptr<base_objects::world::chunk_data> {
            return processed_load_chunk_sync(chunk_x, chunk_z, true);
        });
    }

    fast_task::future_ptr<std::shared_ptr<base_objects::world::chunk_data>> world_data::create_chunk_load_future(int32_t chunk_x, int32_t chunk_z, const std::function<void(base_objects::world::chunk_data& chunk)>& callback, const std::function<void()>& fault) {
        if (profiling.enable_world_profiling)
            ++profiling.chunk_load_counter;
        return fast_task::future<std::shared_ptr<base_objects::world::chunk_data>>::start(limit_on_load, [this, chunk_x, chunk_z, callback, fault]() -> std::shared_ptr<base_objects::world::chunk_data> {
            auto chunk = processed_load_chunk_sync(chunk_x, chunk_z, true);
            if (chunk)
                callback(*chunk);
            else
                fault();
            return chunk;
        });
    }

    std::shared_ptr<base_objects::world::chunk_data> world_data::request_chunk_data_sync(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                if (chunk->generator_stage == 0xFF)
                    return chunk;

        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            auto it = on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
            it->wait_with(lock);
            return it->get();
        } else {
            auto fut = process->second;
            fut->wait_with(lock);
            return fut->get();
        }
    }

    fast_task::future_ptr<std::shared_ptr<base_objects::world::chunk_data>> world_data::request_chunk_data(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                if (chunk->generator_stage == 0xFF)
                    return fast_task::make_ready_future(chunk);

        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
            return on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
        else
            return process->second;
    }

    std::optional<std::shared_ptr<base_objects::world::chunk_data>> world_data::request_chunk_data_weak_gen(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);

        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                return std::make_optional(chunk);
                
        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
            on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
        
        return std::nullopt;
    }

    std::optional<std::shared_ptr<base_objects::world::chunk_data>> world_data::request_chunk_data_weak(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);

        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                return std::make_optional(chunk);

        return std::nullopt;
    }

    std::optional<std::shared_ptr<base_objects::world::chunk_data>> world_data::request_chunk_data_weak_sync(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);

        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                return std::make_optional(chunk);

        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            if (exists(chunk_x, chunk_z)) {
                auto it = on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
                it->wait_with(lock);
                return it->get();
            } else
                return std::nullopt;
        } else {
            auto fut = process->second;
            fut->wait_with(lock);
            return fut->get();
        }
    }

    void world_data::request_chunk_gen(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk) 
                return;
        
        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            bool make_gen = false;
            if (!exists(chunk_x, chunk_z))
                make_gen = true;
            else if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                    if (chunk->generator_stage != 0xFF)
                        if (chunk->load_level <= chunk->resume_gen_level)
                            if (!on_generate_process.contains({chunk_x, chunk_z})) {
                                make_gen = true;
                                chunk->resume_gen_level = 0xFF;
                            }

            if (make_gen)
                on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
        }
    }

    bool world_data::request_chunk_data_sync(int32_t chunk_x, int32_t chunk_z, const std::function<void(base_objects::world::chunk_data& chunk)>& callback) {
        std::unique_lock lock(mutex);

        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk) {
                callback(*chunk);
                return true;
            }
        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end()) {
            auto res = processed_load_chunk_sync(chunk_x, chunk_z, false);
            if (res)
                callback(*res);
            else
                return false;
            return true;
        } else {
            process->second->wait_with(lock);
            return request_chunk_data_sync(chunk_x, chunk_z, callback);
        }
    }

    void world_data::request_chunk_data(int32_t chunk_x, int32_t chunk_z, const std::function<void(base_objects::world::chunk_data& chunk)>& callback, const std::function<void()>& fault) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk) {
                callback(*chunk);
                return;
            }

        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
            on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z, callback, fault);
        else
            process->second->when_ready([callback, fault](std::shared_ptr<base_objects::world::chunk_data> chunk) {
                if (chunk)
                    callback(*chunk);
                else
                    fault();
            });
    }

    void world_data::await_save_chunks() {
        std::unique_lock lock(mutex);
        list_array<fast_task::future_ptr<void>> to_await;
        for (auto& [location, future] : on_save_process)
            to_await.push_back(future);
        lock.unlock();
        to_await.for_each([](fast_task::future_ptr<void>& i) { i->wait(); });
    }

    void world_data::save_chunks(bool unload, bool ignore_limits) {
        auto max_save = api::configuration::get().world.unload_speed;
        std::unique_lock lock(mutex);
        if (max_save && !ignore_limits) {
            for (auto& [key, region] : regions) {
                auto [x, z] = region_key_pos(key);
                make_save_region(x, z, region, unload);
                if (on_save_process.size() > max_save) {
                    lock.unlock();
                    await_save_chunks();
                }
            }
        } else 
            for (auto& [key, region] : regions){
                auto [x, z] = region_key_pos(key);
                make_save_region(x, z, region, unload);
            }
    }

    void world_data::save_and_unload_chunk(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        make_save(chunk_x, chunk_z, true);
    }

    void world_data::unload_chunk(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            region->second->unload(chunk_x, chunk_z);
    }

    void world_data::save_chunk(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        make_save(chunk_x, chunk_z, false);
    }

    void world_data::erase_chunk(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            region->second->unload(chunk_x, chunk_z);
        std::filesystem::remove(path / std::to_string(chunk_x) / (std::to_string(chunk_z) + ".dat"));
    }

    void world_data::regenerate_chunk(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        erase_chunk(chunk_x, chunk_z);
        if (auto process = on_load_process.find({chunk_x, chunk_z}); process == on_load_process.end())
            on_load_process[{chunk_x, chunk_z}] = create_chunk_load_future(chunk_x, chunk_z);
    }

    void world_data::reset_light_data(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        get_light_processor()->process_chunk(*this, chunk_x, chunk_z);
    }

    void world_data::save_and_unload_chunk_at(int32_t global_x, int32_t global_z) {
        save_and_unload_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::unload_chunk_at(int32_t global_x, int32_t global_z) {
        unload_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::save_chunk_at(int32_t global_x, int32_t global_z) {
        save_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::erase_chunk_at(int32_t global_x, int32_t global_z) {
        erase_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::regenerate_chunk_at(int32_t global_x, int32_t global_z) {
        regenerate_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::reset_light_data_at(int32_t global_x, int32_t global_z) {
        reset_light_data(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z));
    }

    void world_data::for_each_chunk(const std::function<void(base_objects::world::chunk_data& chunk)>& func) {
        std::unique_lock lock(mutex);
        for (auto& [key, region] : regions) 
            for (auto& chunk : region->chunks)
                if (chunk)
                    if (chunk->generator_stage == 0xFF)
                        func(*chunk);
    }

    void world_data::for_each_chunk(base_objects::cubic_bounds_chunk bounds, const std::function<void(base_objects::world::chunk_data& chunk)>& func) {
        std::unique_lock lock(mutex);
        for (int32_t x = bounds.x1; x <= bounds.x2; x++)
            for (int32_t z = bounds.z1; z <= bounds.z2; z++)
                if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                    if (auto chunk = region->second->get(x, z); chunk)
                        if (chunk->generator_stage == 0xFF)
                            func(*chunk);
    }

    void world_data::for_each_chunk(base_objects::spherical_bounds_chunk bounds, const std::function<void(base_objects::world::chunk_data& chunk)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    if (chunk->generator_stage == 0xFF)
                        func(*chunk);
        });
    }

    void world_data::for_each_sub_chunk(int32_t chunk_x, int32_t chunk_z, const std::function<void(sub_chunk_data& chunk)>& func) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                if (chunk->generator_stage == 0xFF)
                    chunk->for_each_sub_chunk(func);
    }

    void world_data::get_sub_chunk(int32_t chunk_x, int32_t chunk_y_raw, int32_t chunk_z, const std::function<void(sub_chunk_data& chunk)>& func) {
        std::unique_lock lock(mutex);
        TO_WORLD_POS_CHUNK(chunk_y, chunk_y_raw);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                if (chunk->generator_stage == 0xFF)
                    chunk->get_sub_chunk(chunk_y, func);
    }

    void world_data::get_chunk(int32_t chunk_x, int32_t chunk_z, const std::function<void(base_objects::world::chunk_data& chunk)>& func) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                if (chunk->generator_stage == 0xFF)
                    func(*chunk);
    }

    void world_data::for_each_chunk(base_objects::cubic_bounds_block bounds, const std::function<void(base_objects::world::chunk_data& chunk)>& func) {
        for_each_chunk((base_objects::cubic_bounds_chunk)bounds, func);
    }

    void world_data::for_each_chunk(base_objects::spherical_bounds_block bounds, const std::function<void(base_objects::world::chunk_data& chunk)>& func) {
        for_each_chunk((base_objects::spherical_bounds_chunk)bounds, func);
    }

    void world_data::for_each_sub_chunk_at(int32_t global_x, int32_t global_z, const std::function<void(sub_chunk_data& chunk)>& func) {
        for_each_sub_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), func);
    }

    void world_data::get_sub_chunk_at(int32_t global_x, int32_t global_y_raw, int32_t global_z, const std::function<void(sub_chunk_data& chunk)>& func) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_y), convert_chunk_global_pos(global_z), func);
    }

    void world_data::get_chunk_at(int32_t global_x, int32_t global_z, const std::function<void(base_objects::world::chunk_data& chunk)>& func) {
        get_chunk(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), func);
    }

    void world_data::for_each_entity(const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        for (auto& [id, entity] : entities) {
            if (entity.is_assigned_to_world(world_id))
                if (entity.template get<api::ecs::com::entities::world_syncing>().assigned_world_id == id)
                    func(entity);
        }
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_chunk bounds, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    for (auto& [id, entity] : chunk->stored_entities)
                        if (entity.is_assigned_to_world(world_id))
                            if (entity.template get<api::ecs::com::entities::world_syncing>().assigned_world_id == id)
                                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_chunk_radius bounds, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    for (auto& [id, entity] : chunk->stored_entities)
                        if (entity.is_assigned_to_world(world_id))
                            if (entity.template get<api::ecs::com::entities::world_syncing>().assigned_world_id == id)
                                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_chunk_radius_out bounds, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    for (auto& [id, entity] : chunk->stored_entities)
                        if (entity.is_assigned_to_world(world_id))
                            if (entity.template get<api::ecs::com::entities::world_syncing>().assigned_world_id == id)
                                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::spherical_bounds_chunk bounds, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    for (auto& [id, entity] : chunk->stored_entities)
                        if (entity.is_assigned_to_world(world_id))
                            if (entity.template get<api::ecs::com::entities::world_syncing>().assigned_world_id == id)
                                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::spherical_bounds_chunk_out bounds, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    for (auto& [id, entity] : chunk->stored_entities)
                        if (entity.is_assigned_to_world(world_id))
                            if (entity.template get<api::ecs::com::entities::world_syncing>().assigned_world_id == id)
                                func(entity);
        });
    }

    void world_data::for_each_entity(int32_t chunk_x, int32_t chunk_z, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                for (auto& [id, entity] : chunk->stored_entities)
                    if (entity.is_assigned_to_world(world_id))
                        if (entity.template get<api::ecs::com::entities::world_syncing>().assigned_world_id == id)
                            func(entity);
    }

    void world_data::for_each_block_entity(base_objects::cubic_bounds_chunk bounds, const std::function<void(api::ecs::entity block_entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    chunk->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(base_objects::cubic_bounds_chunk_radius bounds, const std::function<void(api::ecs::entity block_entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    chunk->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(base_objects::cubic_bounds_chunk_radius_out bounds, const std::function<void(api::ecs::entity block_entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    chunk->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(base_objects::spherical_bounds_chunk bounds, const std::function<void(api::ecs::entity block_entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                        chunk->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(base_objects::spherical_bounds_chunk_out bounds, const std::function<void(api::ecs::entity block_entity)>& func) {
        std::unique_lock lock(mutex);
        bounds.enum_points([&](int32_t x, int32_t z) {
            if (auto region = regions.find(region_key(x >> 5, z >> 5)); region != regions.end())
                if (auto chunk = region->second->get(x, z); chunk)
                    chunk->for_each_block_entity(func);
        });
    }

    void world_data::for_each_block_entity(int32_t chunk_x, int32_t chunk_z, const std::function<void(api::ecs::entity block_entity)>& func) {
        std::unique_lock lock(mutex);
        get_chunk(chunk_x, chunk_z, [&](auto& chunk) { chunk.for_each_block_entity(func); });
    }

    void world_data::for_each_block_entity(int32_t chunk_x, int32_t chunk_y_raw, int32_t chunk_z, const std::function<void(api::ecs::entity block_entity)>& func) {
        TO_WORLD_POS_CHUNK(chunk_y, chunk_y_raw);
        std::unique_lock lock(mutex);
        get_chunk(chunk_x, chunk_z, [&](auto& chunk) { chunk.for_each_block_entity(chunk_y, func); });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_block bounds, const std::function<void(api::ecs::entity entity)>& func) {
        for_each_entity((base_objects::cubic_bounds_chunk)bounds, [&](auto entity) {
            auto& pos = entity.template get<api::ecs::com::entities::position>();
            if (bounds.in_bounds((int32_t)pos.x, (int32_t)pos.y, (int32_t)pos.z))
                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_block_radius bounds, const std::function<void(api::ecs::entity entity)>& func) {
        for_each_entity((base_objects::cubic_bounds_chunk_radius)bounds, [&](auto entity) {
            auto& pos = entity.template get<api::ecs::com::entities::position>();
            if (bounds.in_bounds((int32_t)pos.x, (int32_t)pos.y, (int32_t)pos.z))
                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::cubic_bounds_block_radius_out bounds, const std::function<void(api::ecs::entity entity)>& func) {
        for_each_entity((base_objects::cubic_bounds_chunk_radius_out)bounds, [&](auto entity) {
            auto& pos = entity.template get<api::ecs::com::entities::position>();
            if (bounds.in_bounds((int32_t)pos.x, (int32_t)pos.y, (int32_t)pos.z))
                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::spherical_bounds_block bounds, const std::function<void(api::ecs::entity entity)>& func) {
        for_each_entity((base_objects::spherical_bounds_chunk)bounds, [&](auto entity) {
            auto& pos = entity.template get<api::ecs::com::entities::position>();
            if (bounds.in_bounds((int32_t)pos.x, (int32_t)pos.y, (int32_t)pos.z))
                func(entity);
        });
    }

    void world_data::for_each_entity(base_objects::spherical_bounds_block_out bounds, const std::function<void(api::ecs::entity entity)>& func) {
        for_each_entity((base_objects::spherical_bounds_chunk_out)bounds, [&](auto entity) {
            auto& pos = entity.template get<api::ecs::com::entities::position>();
            if (bounds.in_bounds((int32_t)pos.x, (int32_t)pos.y, (int32_t)pos.z))
                func(entity);
        });
    }

    void world_data::for_each_entity_at(int32_t global_x, int32_t global_z, const std::function<void(api::ecs::entity entity)>& func) {
        for_each_entity(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), func);
    }

    void world_data::for_each_block_entity_at(int32_t global_x, int32_t global_z, const std::function<void(api::ecs::entity block_entity)>& func) {
        for_each_block_entity(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), func);
    }

    void world_data::for_each_block_entity_at(int32_t global_x, int32_t global_y_raw, int32_t global_z, const std::function<void(api::ecs::entity block_entity)>& func) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        for_each_block_entity(convert_chunk_global_pos(global_x), convert_chunk_global_pos(global_z), convert_chunk_global_pos(global_y), func);
    }

    void world_data::query_for_tick(int32_t global_x, int32_t global_y_raw, int32_t global_z, uint64_t duration, int8_t priority) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        std::unique_lock lock(mutex);
        auto chunk_x = global_x >> 4;
        auto chunk_z = global_z >> 4;
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                chunk->query_for_tick(global_x & 15, global_y, global_z & 15, duration + tick_counter, priority);
    }

    void world_data::query_for_liquid_tick(int32_t global_x, int32_t global_y_raw, int32_t global_z, uint64_t duration) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        std::unique_lock lock(mutex);
        auto chunk_x = global_x >> 4;
        auto chunk_z = global_z >> 4;
        if (auto region = regions.find(region_key(chunk_x >> 5, chunk_z >> 5)); region != regions.end())
            if (auto chunk = region->second->get(chunk_x, chunk_z); chunk)
                chunk->query_for_liquid_tick(global_x & 15, global_y, global_z & 15, duration + tick_counter);
    }

    void world_data::set_block(const base_objects::any_block& block, int32_t global_x, int32_t global_y_raw, int32_t global_z, block_set_mode mode) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, block, current_world_reg);
            base_objects::block updated_state = sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15);
            if (mode == block_set_mode::destroy) {
                if (updated_state.is_block_entity()) {
                    auto ee = sub_chunk.get_block_entity(global_x & 15, global_y & 15, global_z & 15);
                    WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y_raw, global_z, ee);
                } else
                    WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y_raw, global_z, updated_state);
            } else {
                if (updated_state.is_block_entity()) {
                    auto ee = sub_chunk.get_block_entity(global_x & 15, global_y & 15, global_z & 15);
                    WORLD_ASYNC_RUN(notify_block_change, global_x, global_y_raw, global_z, ee);
                } else
                    WORLD_ASYNC_RUN(notify_block_change, global_x, global_y_raw, global_z, updated_state);
            }

            get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            __update_block(global_x, global_y_raw, global_z, mode, updated_state);
        });
    }

    void world_data::set_block(base_objects::any_block&& block, int32_t global_x, int32_t global_y_raw, int32_t global_z, block_set_mode mode) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, std::move(block), current_world_reg);
            base_objects::block updated_state = sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15);
            if (mode == block_set_mode::destroy) {
                if (updated_state.is_block_entity()) {
                    auto ee = sub_chunk.get_block_entity(global_x & 15, global_y & 15, global_z & 15);
                    WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y_raw, global_z, ee);
                } else
                    WORLD_ASYNC_RUN(notify_block_destroy_change, global_x, global_y_raw, global_z, updated_state);
            }else{
                if (updated_state.is_block_entity()) {
                    auto ee = sub_chunk.get_block_entity(global_x & 15, global_y & 15, global_z & 15);
                    WORLD_ASYNC_RUN(notify_block_change, global_x, global_y_raw, global_z, ee);
                } else
                    WORLD_ASYNC_RUN(notify_block_change, global_x, global_y_raw, global_z, updated_state);
            }

            get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            __update_block(global_x, global_y_raw, global_z, mode, updated_state);
        });
    }

    void world_data::remove_block(int32_t global_x, int32_t global_y_raw, int32_t global_z) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            base_objects::block air;
            WORLD_ASYNC_RUN(notify_block_change, global_x, global_y, global_z, air);
            sub_chunk.set_block(global_x & 15, global_y & 15, global_z & 15, air, current_world_reg);
            get_light_processor()->block_changed(*this, global_x, global_y, global_z);
            __update_block(global_x, global_y_raw, global_z, block_set_mode::replace, air);
        });
    }

    base_objects::block world_data::get_block(int32_t global_x, int32_t global_y_raw, int32_t global_z) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        base_objects::block res;
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            res = sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15);
        });
        return res;
    }

    void world_data::get_block(int32_t global_x, int32_t global_y_raw, int32_t global_z, const std::function<void(base_objects::block block)>& func, const std::function<void(api::ecs::entity)>& block_entity) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15, func, block_entity);
        });
    }

    void world_data::query_block(int32_t global_x, int32_t global_y_raw, int32_t global_z, const std::function<void(base_objects::block block)>& func, const std::function<void(api::ecs::entity)>& block_entity, const std::function<void()>& fault) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        request_chunk_data(
            global_x >> 4,
            global_z >> 4,
            [&](base_objects::world::chunk_data& chunk) {
                chunk.get_sub_chunk(
                    global_y >> 4,
                    [&, block_entity, func](sub_chunk_data& sub_chunk) {
                        sub_chunk.get_block(global_x & 15, global_y & 15, global_z & 15, func, block_entity);
                    }
                );
            },
            fault
        );
    }

    void world_data::block_updated(int32_t global_x, int32_t global_y, int32_t global_z) {
        std::unique_lock lock(mutex);
        get_block(
            global_x,
            global_y,
            global_z,
            [&](auto normal) { WORLD_ASYNC_RUN(notify_block_change, global_x, global_y, global_z, normal); },
            [&](auto block) { WORLD_ASYNC_RUN(notify_block_change, global_x, global_y, global_z, block); }
        );
        get_light_processor()->block_changed(*this, global_x, global_y, global_z);
    }

    void world_data::chunk_updated(int32_t chunk_x, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        get_light_processor()->process_chunk(*this, chunk_x, chunk_z);
    }

    void world_data::sub_chunk_updated(int32_t chunk_x, int32_t chunk_y_raw, int32_t chunk_z) {
        std::unique_lock lock(mutex);
        get_light_processor()->process_sub_chunk(*this, chunk_x, chunk_y_raw, chunk_z);
    }

    void world_data::locked(const std::function<void(world_data& self)>& func) {
        std::unique_lock lock(mutex);
        func(*this);
    }

    void world_data::set_block_range(base_objects::cubic_bounds_block bounds, const list_array<base_objects::any_block>& blocks, block_set_mode mode) {
        if (blocks.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
                });

                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
            });
        } else {
            size_t i = 0;
            size_t max = blocks.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
                    if (i == max)
                        i = 0;
                });
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
            });
        }

        if (mode != block_set_mode::destroy)
            ((base_objects::cubic_bounds_chunk)bounds).enum_points([&](int32_t x, int32_t z) {
                WORLD_ASYNC_RUN(notify_chunk_blocks, x, z);
            });
    }

    void world_data::set_block_range(base_objects::cubic_bounds_block bounds, list_array<base_objects::any_block>&& blocks, block_set_mode mode) {
        if (blocks.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.__set_block_silent(std::move(blocks[i++]), x, y, z, mode);
                });
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
            });
        } else {
            size_t i = 0;
            size_t max = blocks.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
                    if (i == max)
                        i = 0;
                });
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
            });
        }

        if (mode != block_set_mode::destroy)
            ((base_objects::cubic_bounds_chunk)bounds).enum_points([&](int32_t x, int32_t z) {
                WORLD_ASYNC_RUN(notify_chunk_blocks, x, z);
            });
    }

    void world_data::set_block_range(base_objects::spherical_bounds_block bounds, const list_array<base_objects::any_block>& blocks, block_set_mode mode) {
        if (blocks.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
                });
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
            });
        } else {
            size_t i = 0;
            size_t max = blocks.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.__set_block_silent(blocks[i++], x, y, z, mode);
                });
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    get_light_processor()->block_changed(*this, x, y, z);
                });
                if (i == max)
                    i = 0;
            });
        }

        if (mode != block_set_mode::destroy)
            ((base_objects::spherical_bounds_chunk)bounds).enum_points([&](int32_t x, int32_t z) {
                WORLD_ASYNC_RUN(notify_chunk_blocks, x, z);
            });
    }

    void world_data::set_block_range(base_objects::spherical_bounds_block bounds, list_array<base_objects::any_block>&& blocks, block_set_mode mode) {
        if (blocks.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.__set_block_silent(std::move(blocks[i++]), x, y, z, mode);
                });
                ((base_objects::spherical_bounds_chunk)bounds).enum_points([&](int32_t x, int32_t z) {
                    get_light_processor()->process_chunk(world, x, z);
                });
            });
        } else {
            size_t i = 0;
            size_t max = blocks.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    if (i == max)
                        return;
                    world.__set_block_silent(std::move(blocks[i++]), x, y, z, mode);
                    ++i;
                });
                ((base_objects::spherical_bounds_chunk)bounds).enum_points([&](int32_t x, int32_t z) {
                    get_light_processor()->process_chunk(world, x, z);
                });
            });
        }
        if (mode != block_set_mode::destroy)
            ((base_objects::spherical_bounds_chunk)bounds).enum_points([&](int32_t x, int32_t z) {
                WORLD_ASYNC_RUN(notify_chunk_blocks, x, z);
            });
    }

    int32_t world_data::get_biome(int32_t global_x, int32_t global_y_raw, int32_t global_z) {
        uint32_t res = 0;
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            res = sub_chunk.get_biome(global_x & 15, global_y & 15, global_z & 15);
        });
        return res;
    }

    void world_data::set_biome(int32_t global_x, int32_t global_y_raw, int32_t global_z, int32_t biome_id) {
        TO_WORLD_POS_GLOBAL(global_y, global_y_raw);
        get_sub_chunk(global_x >> 4, global_y >> 4, global_z >> 4, [&](sub_chunk_data& sub_chunk) {
            sub_chunk.set_biome(global_x & 15, global_y & 15, global_z & 15, biome_id);
            WORLD_ASYNC_RUN(notify_biome_change, global_x, global_y, global_z, biome_id);
        });
    }

    void world_data::set_biome_range(base_objects::cubic_bounds_block bounds, const list_array<int32_t>& biomes) {
        if (biomes.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                });
            });
        } else {
            size_t i = 0;
            size_t max = biomes.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                    if (i == max)
                        i = 0;
                });
            });
        }
    }

    void world_data::set_biome_range(base_objects::cubic_bounds_block bounds, list_array<int32_t>&& biomes) {
        if (biomes.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                });
            });
        } else {
            size_t i = 0;
            size_t max = biomes.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                    if (i == max)
                        i = 0;
                });
            });
        }
    }

    void world_data::set_biome_range(base_objects::spherical_bounds_block bounds, const list_array<int32_t>& biomes) {
        if (biomes.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                });
            });
        } else {
            size_t i = 0;
            size_t max = biomes.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                    if (i == max)
                        i = 0;
                });
            });
        }
    }

    void world_data::set_biome_range(base_objects::spherical_bounds_block bounds, list_array<int32_t>&& biomes) {
        if (biomes.size() == bounds.count()) {
            size_t i = 0;
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                });
            });
        } else {
            size_t i = 0;
            size_t max = biomes.size();
            locked([&](storage::world_data& world) {
                bounds.enum_points([&](int32_t x, int32_t y, int32_t z) {
                    world.set_biome(x, y, z, biomes[i++]);
                    if (i == max)
                        i = 0;
                });
            });
        }
    }

    void world_data::get_height_maps(int32_t chunk_x, int32_t chunk_z, const std::function<void(base_objects::world::height_maps& height_maps)>& func) {
        get_chunk(chunk_x, chunk_z, [&](base_objects::world::chunk_data& chunk) {
            func(chunk.height_maps);
        });
    }

    void world_data::get_height_maps_at(int32_t global_x, int32_t global_z, const std::function<void(base_objects::world::height_maps& height_maps)>& func) {
        get_chunk_at(global_x, global_z, [&](base_objects::world::chunk_data& chunk) {
            func(chunk.height_maps);
        });
    }

    void world_data::register_entity(api::ecs::entity entity) {
        if (entity.is_assigned_to_world(world_id))
            throw std::runtime_error("Entity already registered in another world");
        if (!current_world_reg.register_entity_and_block(entity))
            throw std::runtime_error("Failed to register entity");

        std::unique_lock lock(mutex);
        uint64_t id = local_entity_id_generator++;
        while (entities.contains(id))
            id = local_entity_id_generator++;
        auto& pos = entity.template get<api::ecs::com::entities::position>();
        auto& entity_data = entity.template get<api::ecs::com::entities::entity_type>().const_data();
        base_objects::cubic_bounds_chunk_radius processing_region((int32_t)pos.x, (int32_t)pos.z, entity_data.max_track_distance);

        auto world_sync = entity.template modify<api::ecs::com::entities::world_syncing>();
        *world_sync = api::ecs::com::entities::world_syncing(
            bit_list_array(),
            processing_region,
            id,
            this
        );
        world_sync->flush_processing();
        entities[id] = entity;
        to_load_entities[id] = entity;
        entity_init(entity);
        if (auto loading_level = entity_data.loading_ticket_level; loading_level <= 44)
            add_loading_ticket({base_objects::world::loading_point_ticket::entity_bound_ticket{id}, processing_region, "entity ticket", loading_level});
    }

    void world_data::unregister_entity(api::ecs::entity entity) {
        std::unique_lock lock(mutex);
        if (entity.is_assigned_to_world(world_id)) {
            auto assigned_world_id = entity.template get<api::ecs::com::entities::world_syncing>().assigned_world_id;
            entity_deinit(entity);
            entities.erase(assigned_world_id);
            to_load_entities.erase(assigned_world_id);
            entity.template modify<api::ecs::com::entities::world_syncing>()->world = nullptr;
            lock.unlock();
            (void)current_world_reg.unregister_entity_and_block(entity);
        }
    }

    void world_data::change_chunk_generator(const std::string& id) {
        std::unique_lock lock(mutex);
        light_processor = nullptr;
        light_processor_id = id;
    }

    void world_data::change_light_processor(const std::string& id) {
        std::unique_lock lock(mutex);
        light_processor = nullptr;
        light_processor_id = id;
    }

    void world_data::tick(std::mt19937& random_engine, std::chrono::high_resolution_clock::time_point current_time) {
        std::unique_lock lock(mutex);

        last_usage = current_time;
        list_array<std::shared_ptr<base_objects::world::chunk_data>> to_tick_chunks;
        list_array<size_t> expired_tickets;

        for (auto& [key, region] : regions) {
            for (auto& chunk : region->chunks) {
                if (chunk)
                    if (chunk->load_level <= 44)
                        chunk->load_level++;
            }
        }

        boost::unordered_flat_map<int32_t, boost::unordered_flat_set<int32_t>> loading_tickets_cc;
        size_t target_load_count = 0;
        for (auto& [id, ticket] : loading_tickets) {
            bool expired = false;
            std::visit(
                [&](auto& expr) {
                    using T = std::decay_t<decltype(expr)>;
                    if constexpr (std::is_same_v<T, uint16_t>) {
                        if (expr)
                            --expr;
                        else
                            expired = true;
                    } else if constexpr (std::is_same_v<T, base_objects::world::loading_point_ticket::callback>) {
                        if (!expr(*this, id, ticket))
                            expired = true;
                    } else if constexpr (std::is_same_v<T, base_objects::world::loading_point_ticket::entity_bound_ticket>) {
                        if (auto it = entities.find(expr.id); it != entities.end()) {
                            if (it->second.is_assigned_to_world(world_id))
                                ticket.point = it->second.template get<api::ecs::com::entities::world_syncing>().processing_region;
                            else
                                expired = true;
                        } else
                            expired = true;
                    }
                },
                ticket.expiration
            );
            if (expired)
                expired_tickets.push_back(id);
            else if (ticket.level < 44) {
                to_tick_chunks.reserve(ticket.point.count());
                ticket.point.enum_points_from_center([&](int32_t x, int32_t z) {
                    auto& local_x = loading_tickets_cc[x];
                    if (local_x.contains(z))
                        return;
                    local_x.insert(z);
                    ++target_load_count;
                    auto res = request_chunk_data_weak_gen(x, z);
                    if (res) {
                        res.value()->load_level = std::min<uint8_t>(res.value()->load_level, ticket.level);
                        if (res.value()->load_level < 33)
                            to_tick_chunks.push_back(res.value());
                    }
                });
                uint8_t propagation = 44 - ticket.level;
                if (propagation) {
                    base_objects::cubic_bounds_chunk_radius_out bounds(ticket.point.center_x, ticket.point.center_z, ticket.point.radius, ticket.point.radius + propagation);
                    to_tick_chunks.reserve(bounds.count());
                    bounds.enum_points_from_center_w_layer_no_center([&](int32_t x, int32_t z, int32_t layer) {
                        auto set_load_level = propagation + layer;
                        if (set_load_level <= 33) {
                            auto& local_x = loading_tickets_cc[x];
                            if (local_x.contains(z))
                                return;
                            local_x.insert(z);
                            ++target_load_count;
                            auto res = request_chunk_data_weak_gen(x, z);
                            if (res) {
                                res.value()->load_level = (uint8_t)std::min<int32_t>(res.value()->load_level, set_load_level);
                                if (res.value()->load_level < 32)
                                    to_tick_chunks.push_back(res.value());
                            }
                        } else if (set_load_level <= 44) {
                            auto res = request_chunk_data_weak_gen(x, z);
                            if (res)
                                res.value()->load_level = (uint8_t)std::min<int32_t>(res.value()->load_level, set_load_level);
                        }
                    });
                }
            }
        }
        expired_tickets.for_each([&](size_t id) {
            loading_tickets.erase(id);
        });

        {
            list_array<size_t> loaded_entities;
            for (auto& [id, entity] : to_load_entities) {
                auto& pos = entity.template get<api::ecs::com::entities::position>();
                auto chunk = request_chunk_data_weak_gen((int32_t)convert_chunk_global_pos(pos.x), (int32_t)convert_chunk_global_pos(pos.z));
                if (chunk) {
                    if ((*chunk)->generator_stage == 0xFF) {
                        if ((*chunk)->stored_entities.insert({id, entity}).second)
                            loaded_entities.push_back(id);
                    }
                }
            }
            for (auto id : loaded_entities)
                to_load_entities.erase(id);
        }
        tick_counter++;
        size_t random_tick_speed = (size_t)world_game_rules["randomTickSpeed"].as_long();
        chunk_tick_result rr;
        if (!profiling.enable_world_profiling) {
            tick_run_local_functions(); //tick/load
            tick_broadcast_time();
            tick_update_world_border();
            tick_update_weather();
            tick_update_day_light();
            tick_run_local_scheduled_commands();
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_players_sleep(rr, *this);
                for (auto id : rr.unrelated_entities)
                    entities.erase(id);
                rr.unrelated_entities.clear();
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_scheduled_blocks(rr, *this);
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_raid(rr, *this);
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_spawn_mobs(rr, *this);
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_ice_snow(rr, *this);
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_random_ticks(rr, *this, random_tick_speed, random_engine);
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_poi(rr, *this);
                for (auto id : rr.unrelated_entities)
                    entities.erase(id);
                rr.unrelated_entities.clear();
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_block_event(rr, *this);
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_dragon(rr, *this);
                for (auto id : rr.unrelated_entities)
                    entities.erase(id);
                rr.unrelated_entities.clear();
            });
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_entity(rr, *this, random_engine);
                for (auto id : rr.unrelated_entities)
                    entities.erase(id);
                rr.unrelated_entities.clear();
            });
            entity_tick_scheduler.execute_frame(current_world_reg, api::ecs::tick_phase::mobile_entity);
            entity_tick_scheduler.execute_frame(current_world_reg, api::ecs::tick_phase::block_entity);
            to_tick_chunks.for_each([&](auto&& chunk) {
                chunk->tick_game_event(rr, *this);
            });
            lock.unlock();
        } else {
            profiling.chunk_target_to_load = target_load_count;
            const auto tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::duration<double>(1) / ticks_per_second);

            tick_run_local_functions(); //tick/load
            tick_broadcast_time();
            tick_update_world_border();
            tick_update_weather();
            tick_update_day_light();
            tick_run_local_scheduled_commands();
            if (profiling.chunk_speedometer_callback || profiling.slow_chunk_tick_callback) {
                auto tick_local_time = std::chrono::high_resolution_clock::now();
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_players_sleep(rr, *this);
                    for (auto id : rr.unrelated_entities)
                        entities.erase(id);
                    rr.unrelated_entities.clear();
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_scheduled_blocks(rr, *this);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_raid(rr, *this);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_spawn_mobs(rr, *this);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_ice_snow(rr, *this);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_random_ticks(rr, *this, random_tick_speed, random_engine);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_poi(rr, *this);
                    for (auto id : rr.unrelated_entities)
                        entities.erase(id);
                    rr.unrelated_entities.clear();
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_block_event(rr, *this);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_dragon(rr, *this);
                    for (auto id : rr.unrelated_entities)
                        entities.erase(id);
                    rr.unrelated_entities.clear();
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_entity(rr, *this, random_engine);
                    for (auto id : rr.unrelated_entities)
                        entities.erase(id);
                    rr.unrelated_entities.clear();
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
                {
                    entity_tick_scheduler.execute_frame(current_world_reg, api::ecs::tick_phase::mobile_entity);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    profiling.entity_tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                }
                {
                    entity_tick_scheduler.execute_frame(current_world_reg, api::ecs::tick_phase::block_entity);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    profiling.block_entity_tick_speed = std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                }
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_game_event(rr, *this);
                    auto actual_time = std::chrono::high_resolution_clock::now();
                    chunk->tick_speed += std::chrono::duration_cast<std::chrono::milliseconds>(actual_time - tick_local_time);
                    tick_local_time = actual_time;
                });
            } else {
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_players_sleep(rr, *this);
                    for (auto id : rr.unrelated_entities)
                        entities.erase(id);
                    rr.unrelated_entities.clear();
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_scheduled_blocks(rr, *this);
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_raid(rr, *this);
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_spawn_mobs(rr, *this);
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_ice_snow(rr, *this);
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_random_ticks(rr, *this, random_tick_speed, random_engine);
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_poi(rr, *this);
                    for (auto id : rr.unrelated_entities)
                        entities.erase(id);
                    rr.unrelated_entities.clear();
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_block_event(rr, *this);
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_dragon(rr, *this);
                    for (auto id : rr.unrelated_entities)
                        entities.erase(id);
                    rr.unrelated_entities.clear();
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_entity(rr, *this, random_engine);
                    for (auto id : rr.unrelated_entities)
                        entities.erase(id);
                    rr.unrelated_entities.clear();
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_block_entity(rr, *this);
                });
                to_tick_chunks.for_each([&](auto&& chunk) {
                    chunk->tick_game_event(rr, *this);
                });
            }
            if (!profiling.sync_profiling)
                lock.unlock();
            if (profiling.chunk_speedometer_callback || profiling.slow_chunk_tick_callback)
                to_tick_chunks.for_each([&, slow_chunk_threshold = tick_speed * profiling.slow_chunk_tick_callback_threshold](auto&& chunk) {
                    if (profiling.slow_chunk_tick_callback)
                        if (slow_chunk_threshold < chunk->tick_speed)
                            profiling.slow_chunk_tick_callback(*this, chunk->chunk_x, chunk->chunk_z, chunk->tick_speed);
                    if (profiling.chunk_speedometer_callback)
                        profiling.chunk_speedometer_callback(*this, chunk->chunk_x, chunk->chunk_z, chunk->tick_speed);
                });
            if (profiling.chunk_speedometer_callback)
                profiling.chunk_speedometer_callback(*this, INT32_MAX, INT32_MAX, std::chrono::milliseconds(0));

            auto tick_local_time = std::chrono::high_resolution_clock::now();
            auto current_tick_speed = tick_local_time - current_time;
            if (profiling.got_tps_update) {
                ++profiling.got_ticks;
                if (tick_local_time - profiling.last_tick >= std::chrono::seconds(1)) {
                    auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(tick_local_time - profiling.last_tick).count();
                    profiling.tps_for_world = profiling.got_ticks / elapsed;
                    profiling.got_tps_update(*this);
                    profiling.last_tick = tick_local_time;
                    profiling.got_ticks = 0;
                }
            }

            if (profiling.slow_world_tick_callback) {
                auto slow_world_threshold = tick_speed * profiling.slow_world_tick_callback_threshold;
                if (slow_world_threshold < current_tick_speed)
                    profiling.slow_world_tick_callback(*this, std::chrono::duration_cast<std::chrono::milliseconds>(current_tick_speed));
            }
            if (profiling.sync_profiling)
                lock.unlock();
        }

        auto as = api::configuration::get().world.auto_save;
        if (as)
            if (tick_counter % as == 0)
                save_chunks();
    }

    bool world_data::collect_unused_data(std::chrono::high_resolution_clock::time_point current_time, size_t& unload_limit) {
        std::unique_lock lock(mutex);
        if (last_usage + world_lifetime < current_time)
            if (on_load_process.empty() && on_save_process.empty())
                return true;

        for (auto& [key, region] : regions) {
            auto [could_be_unloaded, has_unloadable_items] = region->could_be_unloaded();
            if (unload_limit == 0)
                return false;
            if (could_be_unloaded) {
                auto [x, y] = region_key_pos(key);
                make_save_region(x, y, region, true);
                --unload_limit;
            } else if (has_unloadable_items) {
                for (auto& chunk : region->chunks) {
                    if (unload_limit == 0)
                        return false;
                    if (chunk) {
                        if (chunk->could_be_unloaded()) {
                            make_save(chunk->chunk_x, chunk->chunk_z, region, true);
                            --unload_limit;
                        }
                    }
                }
            }
        }
        return false;
    }

#pragma region worlds_data

    std::shared_ptr<world_data> worlds_data::load(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (cached_worlds.find(world_id) == cached_worlds.end()) {
            auto path = base_path / std::to_string(world_id);
            if (!std::filesystem::exists(path))
                throw std::runtime_error("World not found");

            auto world = std::make_shared<world_data>(world_id, path.string());
            world->load();
            auto& res = cached_worlds[world_id] = world;
            on_world_loaded(world_id);
            return res;
        }
        return cached_worlds[world_id];
    }

    worlds_data::worlds_data(const std::filesystem::path& base_path)
        : base_path(base_path) {
        if (!std::filesystem::exists(base_path))
            std::filesystem::create_directories(base_path);
    }

    const list_array<int32_t>& worlds_data::get_ids() {
        std::unique_lock lock(mutex);
        if (!cached_ids.empty())
            return cached_ids;
        list_array<int32_t> result;
        for (auto& entry : std::filesystem::directory_iterator{base_path}) {
            if (entry.is_directory()) {
                auto& path = entry.path();
                try {
                    result.push_back(std::stoi(path.filename().string()));
                } catch (const std::exception&) {
                    api::log::warn("storage:worlds_data", "Got corrupted file path: " + path.string());
                }
            }
        }
        result.commit();
        return cached_ids = result;
    }

    size_t worlds_data::loaded_chunks_count() {
        std::unique_lock lock(mutex);
        size_t res = 0;
        for (auto& world : cached_worlds)
            res += world.second->loaded_chunks_count();
        return res;
    }

    size_t worlds_data::loaded_chunks_count(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto on_load = cached_worlds.find(world_id); on_load != cached_worlds.end())
            return on_load->second->loaded_chunks_count();
        return 0;
    }

    bool worlds_data::exists(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (cached_ids.empty()) {
            lock.unlock();
            get_list();
            lock.lock();
        }
        return cached_ids.contains(world_id);
    }

    bool worlds_data::exists(const std::string& name) {
        return get_id(name) != -1;
    }

    const list_array<int32_t>& worlds_data::get_list() {
        std::unique_lock lock(mutex);
        return get_ids();
    }

    std::string worlds_data::get_name(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto on_load = cached_worlds.find(world_id); on_load != cached_worlds.end())
            return on_load->second->world_name;

        auto world_path = base_path / std::to_string(world_id);
        if (std::filesystem::exists(world_path))
            return world_data(world_id, world_path).preview_world_name();
        else
            throw std::runtime_error("World with id " + std::to_string(world_id) + " not found.");
    }

    int32_t worlds_data::get_id(const std::string& name) {
        std::unique_lock lock(mutex);
        for (auto& world : cached_worlds)
            if (world.second->world_name == name)
                return world.first;

        if (cached_ids.empty())
            get_ids();
        size_t found = cached_ids.find_if([this, &name](int32_t id) {
            if (std::filesystem::exists(base_path / std::to_string(id) / "world.snbt"))
                return world_data(id, base_path / std::to_string(id)).preview_world_name() == name;
            else
                return false;
        });
        return found == list_array<int32_t>::npos ? -1 : cached_ids[found];
    }

    list_array<int32_t> worlds_data::get_all_ids() {
        return get_ids();
    }

    std::shared_ptr<world_data> worlds_data::get(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
            return load(world_id);
        else
            return world->second;
    }

    void worlds_data::save(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto item = cached_worlds.find(world_id); item == cached_worlds.end())
            throw std::runtime_error("World not found");
        else {
            auto w = item->second;
            lock.unlock();
            w->save();
            w->save_chunks();
        }
    }

    void worlds_data::save_all() {
        std::unique_lock lock(mutex);
        list_array<std::shared_ptr<world_data>> worlds;
        for (auto& [id, world] : cached_worlds)
            worlds.emplace_back(world);
        lock.unlock();
        for (auto& world : worlds) {
            world->save();
            world->save_chunks();
        }
    }

    void worlds_data::save_and_unload(int32_t world_id) {
        std::unique_lock lock(mutex);
        if (auto item = cached_worlds.find(world_id); item == cached_worlds.end())
            throw std::runtime_error("World not found");
        else {
            auto& world = item->second;
            on_world_unloaded(world_id);
            world->save();
            world->save_chunks(true, true);
            world->await_save_chunks();
            cached_worlds.erase(item);
        }
    }

    void worlds_data::save_and_unload_all() {
        std::unique_lock lock(mutex);
        for (auto& [id, world] : cached_worlds) {
            on_world_unloaded(id);
            world->save();
            world->save_chunks(true, true);
            world->await_save_chunks();
        }
        cached_worlds.clear();
    }

    //be sure that world is not used by anything, otherwise will throw exception
    void worlds_data::unload(int32_t world_id) {
        std::unique_lock lock(mutex);
        on_world_unloaded(world_id);
        cached_worlds.erase(world_id);
    }

    void worlds_data::unload_all() {
        std::unique_lock lock(mutex);
        for (auto&& [id, world] : cached_worlds)
            on_world_unloaded(id);
        cached_worlds.clear();
    }

    void worlds_data::erase(int32_t world_id) {
        std::unique_lock lock(mutex);
        std::filesystem::remove_all(std::filesystem::path(base_path) / std::to_string(world_id));
        cached_worlds.erase(world_id);
        cached_ids.remove(world_id);
        on_world_unloaded(world_id);
    }

    int32_t worlds_data::create(const std::string& name) {
        std::unique_lock lock(mutex);
        if (get_id(name) != -1)
            throw std::runtime_error("World with name " + name + " already exists.");
        int32_t id = 0;
        while (exists(id))
            id++;
        cached_ids.push_back(id);
        cached_worlds[id] = std::make_shared<world_data>(id, base_path / std::to_string(id));
        cached_worlds[id]->world_name = name;
        cached_worlds[id]->save();
        on_world_loaded(id);
        return id;
    }

    int32_t worlds_data::create(const std::string& name, const std::function<void(world_data& world)>& init) {
        std::unique_lock lock(mutex);
        if (get_id(name) != -1)
            throw std::runtime_error("World with name " + name + " already exists.");
        int32_t id = 0;
        while (exists(id))
            id++;
        std::shared_ptr<world_data> world = std::make_shared<world_data>(id, base_path / std::to_string(id));
        init(*world);
        cached_ids.push_back(id);
        cached_worlds[id] = world;
        cached_worlds[id]->world_name = name;
        cached_worlds[id]->save();
        on_world_loaded(id);
        return id;
    }

    void worlds_data::locked(const std::function<void()>& func) {
        std::unique_lock lock(mutex);
        func();
    }

    void worlds_data::locked(const std::function<void(worlds_data& self)>& func) {
        std::unique_lock lock(mutex);
        func(*this);
    }

    void worlds_data::for_each_entity(const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        for (auto& [id, world] : cached_worlds)
            world->for_each_entity(func);
    }

    void worlds_data::for_each_entity(int32_t chunk_x, int32_t chunk_z, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        for (auto& [id, world] : cached_worlds)
            world->for_each_entity(chunk_x, chunk_z, func);
    }

    void worlds_data::for_each_entity(int32_t world_id, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
            load(world_id)->for_each_entity(func);
        else
            world->second->for_each_entity(func);
    }

    void worlds_data::for_each_entity(int32_t world_id, int32_t chunk_x, int32_t chunk_z, const std::function<void(api::ecs::entity entity)>& func) {
        std::unique_lock lock(mutex);
        if (auto world = cached_worlds.find(world_id); world == cached_worlds.end())
            load(world_id)->for_each_entity(chunk_x, chunk_z, func);
        else
            world->second->for_each_entity(chunk_x, chunk_z, func);
    }

    void worlds_data::for_each_world(const std::function<void(int32_t id, world_data& world)>& func) {
        std::unique_lock lock(mutex);
        for (auto& [id, world] : cached_worlds)
            func(id, *world);
    }

    void worlds_data::apply_tick(std::chrono::high_resolution_clock::time_point current_time) {
        api::ecs::global_registry::global_tick();
        std::unique_lock lock(mutex);
        list_array<std::pair<int32_t, std::shared_ptr<world_data>>> worlds_to_tick;
        for (auto& [id, world] : cached_worlds) {
            if (!world->last_usage.time_since_epoch().count())
                world->last_usage = current_time;
            if (ticks_per_second > world->ticks_per_second) {
                auto tick_interval = std::chrono::nanoseconds(1'000'000'000 / world->ticks_per_second);
                world->accumulated_time += current_time - world->last_usage;
                world->last_usage = current_time;
                if (world->accumulated_time / tick_interval) {
                    world->accumulated_time -= tick_interval;
                    worlds_to_tick.push_back({id, world});
                }
            } else
                worlds_to_tick.push_back({id, world});
        }
        lock.unlock();
        size_t unload_speed = api::configuration::get().world.unload_speed;
        fast_task::future_tool::for_each_move(
            fast_task::future_tool::process<std::optional<int32_t>>(
                worlds_to_tick,
                [current_time, unload_speed](const auto& it) mutable -> std::optional<int32_t> {
                    auto id = it.first;
                    auto& world = it.second;
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    world->tick(gen, current_time);
                    if (world->collect_unused_data(current_time, unload_speed))
                        return id;
                    return std::nullopt;
                }
            ),
            [this](auto id) { if(id) save_and_unload(*id); }
        )->wait();
        lock.lock();
        got_ticks++;
        auto new_current_time = std::chrono::high_resolution_clock::now();
        if (current_time - last_tps_calculated >= std::chrono::seconds(1)) {
            auto elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(new_current_time - last_tps_calculated).count();
            tps = double(got_ticks) / elapsed;
            on_tps_changed.async_notify(tps);
            last_tps_calculated = new_current_time;
            got_ticks = 0;
        }
        on_tick.async_notify(got_ticks);
    }

#pragma endregion
}
