/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/fast_task.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/ecs/entity_construction.hpp>
#include <src/api/ecs/entity_definition.hpp>
#include <src/api/entity.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/packets.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/base_objects/uuid.hpp>
#include <src/generated/entity/components.hpp>
#include <src/generated/entity/factory.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/calculations.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server {
    namespace api {
        struct entities_storage {
            std::unordered_map<int32_t, entity_data> _registry;
            std::unordered_map<std::string, int32_t> _name_to_id;
            std::unordered_map<std::string, std::shared_ptr<entity_data::world_processor>> entity_processors;
            int32_t id_adder = 0;
        };

        fast_task::protected_value<entities_storage> data_for_entities;

        const entity_data& entity_data::get_entity(int32_t id) {
            return data_for_entities.get([&](auto& data) -> const entity_data& {
                auto it = data._registry.find(id);
                if (it == data._registry.end())
                    throw std::runtime_error("Entity not found.");
                else
                    return it->second;
            });
        }

        const entity_data& entity_data::get_entity(const std::string& id) {
            return data_for_entities.get([&](auto& data) -> const entity_data& {
                auto it = data._registry.find(data._name_to_id.at(id));
                if (it == data._registry.end())
                    throw std::runtime_error("Entity not found.");
                else
                    return it->second;
            });
        }

        list_array<int32_t> entity_data::get_entity_ids() {
            return data_for_entities.get([&](auto& data) {
                list_array<int32_t> res;
                res.reserve(data._registry.size());
                for (auto& [id, ent] : data._registry)
                    res.push_back(id);
                return res;
            });
        }

        int32_t entity_data::register_entity(entity_data entity) {
            return data_for_entities.set([&](auto& data) {
                int32_t id = data.id_adder++;
                if (data.id_adder <= 0) {
                    --data.id_adder;
                    throw std::runtime_error("Too many entities.");
                }
                data._name_to_id[entity.id] = id;
                entity.entity_id = id;
                data._registry[id] = std::move(entity);
                return id;
            });
        }

        const entity_data& entity_data::view(ecs::entity entity) {
            return entity.get<ecs::com::entities::entity_type>().const_data();
        }

        entity_data& entity_data::initialization_get(int32_t id) {
            return data_for_entities.set([&](auto& data) -> entity_data& {
                auto it = data._registry.find(id);
                if (it == data._registry.end())
                    throw std::runtime_error("Entity not found.");
                else
                    return it->second;
            });
        }

        void entity_data::register_entity_world_processor(std::shared_ptr<world_processor> processor, const std::string& id) {
            data_for_entities.set([&](auto& data) {
                if (!data.entity_processors.emplace(id, processor).second)
                    throw std::runtime_error("Processor for this entity already registered.");
            });
        }

        void entity_data::reset_entities() {
            data_for_entities.set([&](auto& data) {
                data.id_adder = 0;
                data._registry.clear();
            });
        }

        void entity_data::initialize_entities() {
            data_for_entities.set([&](auto& data) {
                for (auto& [id, entity] : data._registry) {
                    if (auto it = data.entity_processors.find(entity.id); it != data.entity_processors.end())
                        entity.processor = it->second;
                }
                player_entity_id = data._name_to_id.at("minecraft:player");
            });
            generated::entity::register_entities();
        }

        int32_t entity_data::player_entity_id;

        storage::world_data* entity::current_world() const {
            return handle.get<ecs::com::entities::world_syncing>().world;
        }

        int32_t entity::get_protocol_id() const {
            return handle.get<api::ecs::com::entities::protocol_id>().value;
        }

        util::vector entity::get_position() const {
            return handle.get<api::ecs::com::entities::position>();
        }

        std::optional<ecs::entity> entity::copy() const {
            return handle.copy_and_wait();
        }

        void resolve_entity(std::variant<ecs::entity, base_objects::uuid>& it) {
            if (std::holds_alternative<base_objects::uuid>(it)) {
                auto entity = api::entity_id_map::get_entity(std::get<base_objects::uuid>(it));
                if (entity)
                    it = *entity;
            }
        }

        void reduce_effects(ecs::com::entities::effects& eff) { //TODO replace with system
            list_array<uint32_t> expired_effects;

            for (auto& [id, effect] : eff.active_effects()) {
                if (!effect.duration) {
                    expired_effects.push_back(id);
                    continue;
                }
                if (effect.duration != UINT32_MAX)
                    effect.duration--;
            }

            for (auto& [id, effects] : eff.hidden_effects()) {
                for (auto& effect : effects) {
                    if (!effect.duration)
                        continue;
                    if (effect.duration != UINT32_MAX)
                        effect.duration--;
                }

                effects.remove_if([](const ecs::com::entities::effects::effect& effect) {
                    return !effect.duration;
                });
                effects.sort([](const ecs::com::entities::effects::effect& effect0, const ecs::com::entities::effects::effect& effect1) {
                    return effect0.amplifier > effect1.amplifier;
                });
            }
            for (auto& id : expired_effects) {
                if (eff.hidden_effects().contains(id))
                    eff.active_effects().at(id) = eff.hidden_effects().at(id).take_front();
                else
                    eff.active_effects().erase(id);
            }
        }

        void entity::tick() {
            if (handle.has<ecs::com::entities::attached_to>())
                if (handle.modify<ecs::com::entities::attached_to>()->follow)
                    handle.modify<ecs::com::entities::attached_to>()->follow->try_resolve();

            if (handle.has<ecs::com::entities::attached>())
                for (auto& it : handle.modify<ecs::com::entities::attached>()->followers)
                    it.try_resolve();

            auto proc = const_data().processor;
            if (proc)
                if (proc->on_tick)
                    proc->on_tick(handle);
            reduce_effects(*handle.modify<ecs::com::entities::effects>());
        }

        base_objects::entity_metadata::entity_pose entity::get_pose() const {
            return handle.get<generated::com::pose>().value;
        }

        void entity::set_pose(base_objects::entity_metadata::entity_pose pose) {
            handle.modify<generated::com::pose>()->value = pose;
        }

        double entity::eye_height() const {
            return const_data().eye_height_in_each_pose.at(get_pose().value.value); //add scale and other modifiers
        }

        bool entity::kill() {
            if (!const_data().pre_death_callback(handle, false))
                return false;

            handle.add<ecs::com::dead_mark>();
            return true;
        }

        void entity::force_kill() {
            const_data().pre_death_callback(handle, true);
            handle.add<ecs::com::dead_mark>();
        }

        void entity::erase() {
            if (current_world())
                current_world()->entity_deinit(handle);
        }

        bool entity::is_died() const {
            return handle.has<ecs::com::dead_mark>();
        }

        const entity_data& entity::const_data() const {
            return entity_data::view(handle);
        }

        bool entity::hitboxes_touching_x(double min, double max) {
            auto& position = handle.get<api::ecs::com::entities::position>();
            auto& bounds = handle.get<api::ecs::com::entities::bounding_box>();
            return (position.x - bounds.xz) >= min && (position.x + bounds.xz) <= max;
        }

        bool entity::hitboxes_touching_y(double min, double max) {
            auto& position = handle.get<api::ecs::com::entities::position>();
            auto& bounds = handle.get<api::ecs::com::entities::bounding_box>();
            return (position.y) >= min && (position.y + bounds.y) <= max;
        }

        bool entity::hitboxes_touching_z(double min, double max) {
            auto& position = handle.get<api::ecs::com::entities::position>();
            auto& bounds = handle.get<api::ecs::com::entities::bounding_box>();
            return (position.z - bounds.xz) >= min && (position.z + bounds.xz) <= max;
        }

        void entity::moved(util::vector pos) {
            if (current_world())
                current_world()->entity_move(handle, pos);
            *handle.modify<api::ecs::com::entities::position>() = pos;
        }

        void entity::moved(util::vector pos, float yaw, float pitch) {
            if (current_world()) {
                current_world()->entity_move(handle, pos);
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            }
            *handle.modify<api::ecs::com::entities::position>() = pos;
            *handle.modify<api::ecs::com::entities::rotation>() = {yaw, pitch};
        }

        void entity::moved(util::vector pos, float yaw, float pitch, bool on_ground) {
            if (current_world()) {
                current_world()->entity_move(handle, pos);
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            }
            *handle.modify<api::ecs::com::entities::position>() = pos;
            *handle.modify<api::ecs::com::entities::rotation>() = {yaw, pitch};
            set_on_ground(on_ground);
        }

        void entity::rotated(float yaw, float pitch) {
            if (current_world())
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            *handle.modify<api::ecs::com::entities::rotation>() = {yaw, pitch};
        }

        void entity::rotated(float yaw, float pitch, bool on_ground) {
            if (current_world())
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            *handle.modify<api::ecs::com::entities::rotation>() = {yaw, pitch};
            set_on_ground(on_ground);
        }

        void entity::teleport(util::vector pos) {
            if (current_world())
                current_world()->entity_teleport(handle, pos);
            *handle.modify<api::ecs::com::entities::position>() = pos;
            auto assigned_player = handle.get<ecs::com::entities::assigned_player>().player;
            auto protocol_id = handle.get<ecs::com::entities::protocol_id>().value;
            auto mot = handle.get<ecs::com::entities::motion>();
            auto rot = handle.get<ecs::com::entities::rotation>();
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::entity_position_sync{
                    .id = protocol_id,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .velocity_x = mot.x,
                    .velocity_y = mot.y,
                    .velocity_z = mot.z,
                    .yaw = (float)rot.yaw,
                    .pitch = (float)rot.pitch,
                    .on_ground = is_on_ground()
                };
        }

        void entity::teleport(util::vector pos, float yaw, float pitch) {
            if (current_world()) {
                current_world()->entity_teleport(handle, pos);
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            }
            auto assigned_player = handle.get<ecs::com::entities::assigned_player>().player;
            auto protocol_id = handle.get<ecs::com::entities::protocol_id>().value;
            *handle.modify<api::ecs::com::entities::position>() = pos;
            auto mot = handle.get<ecs::com::entities::motion>();
            auto& rot = *handle.modify<api::ecs::com::entities::rotation>() = {yaw, pitch};
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::entity_position_sync{
                    .id = protocol_id,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .velocity_x = mot.x,
                    .velocity_y = mot.y,
                    .velocity_z = mot.z,
                    .yaw = (float)rot.yaw,
                    .pitch = (float)rot.pitch,
                    .on_ground = is_on_ground()
                };
        }

        void entity::teleport(util::vector pos, float yaw, float pitch, bool on_ground) {
            if (current_world()) {
                current_world()->entity_teleport(handle, pos);
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            }
            auto assigned_player = handle.get<ecs::com::entities::assigned_player>().player;
            auto protocol_id = handle.get<ecs::com::entities::protocol_id>().value;
            *handle.modify<api::ecs::com::entities::position>() = pos;
            auto mot = handle.get<ecs::com::entities::motion>();
            *handle.modify<api::ecs::com::entities::rotation>() = {yaw, pitch};
            set_on_ground(on_ground);
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::entity_position_sync{
                    .id = protocol_id,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .velocity_x = mot.x,
                    .velocity_y = mot.y,
                    .velocity_z = mot.z,
                    .yaw = (float)yaw,
                    .pitch = (float)pitch,
                    .on_ground = is_on_ground()
                };
        }

        void entity::set_ride_entity(ecs::entity entity) {
            if (current_world()) {
                if (api::entity(entity).current_world() == current_world()) {
                    current_world()->entity_rides(handle, entity.get<ecs::com::entities::world_syncing>().assigned_world_id);
                    auto& other = handle.modify<ecs::com::entities::ride_entity>()->other;
                    if (other)
                        current_world()->entity_leaves_ride(handle, other->get_entity().get<ecs::com::entities::world_syncing>().assigned_world_id);
                    other = {entity};
                    return;
                }
            }
        }

        void entity::remove_ride_entity() {
            if (handle.get<ecs::com::entities::ride_entity>().other) {
                if (current_world()) {
                    auto& ride_entity = handle.modify<ecs::com::entities::ride_entity>()->other;
                    if (ride_entity)
                        if (api::entity(ride_entity->get_entity()).current_world() == current_world())
                            current_world()->entity_leaves_ride(handle, ride_entity->get_entity().get<ecs::com::entities::world_syncing>().assigned_world_id);
                        
                }
            }
        }

        void entity::add_effect(uint32_t id_, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
            ecs::com::entities::effects::effect to_add_effect{
                .duration = duration,
                .id = id_,
                .amplifier = amplifier,
                .ambient = ambient,
                .particles = show_particles,
                .show_icon = show_icon,
                .use_blend = use_blend,
            };
            auto effects = handle.modify<api::ecs::com::entities::effects>();
            auto& active_effects = effects->active_effects();
            auto& hidden_effects = effects->hidden_effects();
            if (auto it = active_effects.find(id_); it != active_effects.end()) {
                auto& effect = it->second;
                if (effect.amplifier >= amplifier) {
                    if (effect.duration < duration)
                        hidden_effects[id_].push_back(to_add_effect);
                    if (current_world())
                        current_world()->entity_add_effect(handle, id_, duration, amplifier, ambient, show_particles, show_icon, use_blend);
                    return;
                } else
                    hidden_effects[id_].push_back(effect);
            }
            active_effects[id_] = to_add_effect;
            if (current_world())
                current_world()->entity_add_effect(handle, id_, duration, amplifier, ambient, show_particles, show_icon, use_blend);
        }

        void entity::remove_effect(uint32_t id_) {
            auto effects = handle.modify<api::ecs::com::entities::effects>();
            auto& active_effects = effects->active_effects();
            auto& hidden_effects = effects->hidden_effects();
            active_effects.erase(id_);
            hidden_effects.erase(id_);
            if (current_world())
                current_world()->entity_remove_effect(handle, id_);
        }

        void entity::remove_all_effects() {
            auto effects = handle.modify<api::ecs::com::entities::effects>();
            auto& active_effects = effects->active_effects();
            auto& hidden_effects = effects->hidden_effects();
            if (current_world())
                for (auto& [id_, effect] : active_effects)
                    current_world()->entity_remove_effect(handle, id_);
            active_effects.clear();
            hidden_effects.clear();
        }

        bool entity::is_sleeping() const {
            return handle.get<ecs::com::entities::world_syncing>().is_sleeping;
        }

        bool entity::is_on_ground() const {
            return handle.get<ecs::com::entities::world_syncing>().on_ground;
        }

        bool entity::is_sneaking() const {
            return handle.get<ecs::com::entities::world_syncing>().is_sneaking;
        }

        bool entity::is_sprinting() const {
            return handle.get<ecs::com::entities::world_syncing>().is_sprinting;
        }

        void entity::set_sleeping(bool sleeping) {
            if (current_world())
                handle.modify<ecs::com::entities::world_syncing>()->is_sleeping = sleeping;
        }

        void entity::set_on_ground(bool on_ground) {
            if (current_world())
                handle.modify<ecs::com::entities::world_syncing>()->on_ground = on_ground;
        }

        void entity::set_sneaking(bool sneaking) {
            if (current_world())
                handle.modify<ecs::com::entities::world_syncing>()->is_sneaking = sneaking;
        }

        void entity::set_sprinting(bool sprinting) {
            if (current_world())
                handle.modify<ecs::com::entities::world_syncing>()->is_sprinting = sprinting;
        }

        float entity::get_health() const {
            if (handle.has<generated::com::health>())
                return handle.get<generated::com::health>().value;
            else
                return 0;
        }

        void entity::set_health(float health) {
            if (handle.has<generated::com::health>()) {
                auto modify = handle.modify<generated::com::health>();
                modify->value = health;

                if (modify->value <= 0.0f)
                    kill();
            }
        }

        void entity::add_health(float health) {
            set_health(get_health() + health);
        }

        void entity::damage(float health, int32_t type_id, std::optional<util::vector> pos) {
            handle.modify<ecs::com::entities::world_syncing>()->inactivity_counter = 0;
            if (current_world())
                current_world()->entity_damage(handle, health, type_id, pos);
            reduce_health(health);
        }

        void entity::damage(float health, int32_t type_id, std::optional<ecs::entity> source, std::optional<util::vector> pos) {
            handle.modify<ecs::com::entities::world_syncing>()->inactivity_counter = 0;
            if (current_world())
                current_world()->entity_damage(handle, health, type_id, source, pos);
            reduce_health(health);
        }

        void entity::damage(float health, int32_t type_id, std::optional<ecs::entity> source, std::optional<ecs::entity> source_direct, std::optional<util::vector> pos) {
            handle.modify<ecs::com::entities::world_syncing>()->inactivity_counter = 0;
            if (current_world())
                current_world()->entity_damage(handle, health, type_id, source, source_direct, pos);
            reduce_health(health);
        }

        void entity::reduce_health(float health) {
            set_health(get_health() - health);
        }

        int32_t entity::get_food() const {
            if (handle.has<api::ecs::com::entities::food>()) {
                return handle.get<api::ecs::com::entities::food>().value;
            } else
                return 0;
        }

        void entity::set_food(int32_t food) {
            if (handle.has<api::ecs::com::entities::food>())
                handle.modify<api::ecs::com::entities::food>()->value = food;
        }

        void entity::add_food(int32_t food) {
            set_food(get_food() + food);
        }

        void entity::reduce_food(int32_t food) {
            set_food(get_food() - food);
        }

        float entity::get_saturation() const {
            if (handle.has<api::ecs::com::entities::saturation>()) {
                return handle.get<api::ecs::com::entities::saturation>().value;
            } else
                return 0;
        }

        void entity::set_saturation(float saturation) {
            if (handle.has<api::ecs::com::entities::saturation>())
                handle.modify<api::ecs::com::entities::saturation>()->value = saturation;
        }

        void entity::add_saturation(float saturation) {
            set_saturation(get_saturation() - saturation);
        }

        void entity::reduce_saturation(float saturation) {
            set_saturation(get_saturation() - saturation);
        }

        int32_t entity::get_breath() const {
            if (handle.has<generated::com::air>())
                return handle.get<generated::com::air>().value;
            else
                return 0;
        }

        void entity::set_breath(int32_t breath) {
            if (handle.has<generated::com::air>())
                handle.modify<generated::com::air>()->value = breath;
        }

        void entity::add_breath(int32_t breath) {
            set_breath(get_breath() + breath);
        }

        void entity::reduce_breath(int32_t breath) {
            set_breath(get_breath() - breath);
        }

        int32_t entity::get_level() const {
            if (handle.has<api::ecs::com::entities::experience>())
                return handle.get<api::ecs::com::entities::experience>().get_level();
            else
                return 0;
        }

        void entity::set_level(int32_t level) {
            if (handle.has<api::ecs::com::entities::experience>())
                handle.modify<api::ecs::com::entities::experience>()->set_level(level);
        }

        void entity::add_level(int32_t level) {
            set_level(get_level() + level);
        }

        void entity::reduce_level(int32_t level) {
            set_level(get_level() - level);
        }

        int32_t entity::get_experience() const {
            if (handle.has<api::ecs::com::entities::experience>())
                return handle.get<api::ecs::com::entities::experience>().get_experience();
            else
                return 0;
        }

        void entity::set_experience(int32_t experience) {
            if (handle.has<api::ecs::com::entities::experience>())
                handle.modify<api::ecs::com::entities::experience>()->set_experience(experience);
        }

        void entity::add_experience(int32_t experience) {
            if (handle.has<api::ecs::com::entities::experience>())
                handle.modify<api::ecs::com::entities::experience>()->add_experience(experience);
        }

        void entity::reduce_experience(int32_t experience) {
            if (handle.has<api::ecs::com::entities::experience>())
                handle.modify<api::ecs::com::entities::experience>()->reduce_experience(experience);
        }

        int32_t entity::get_fall_distance() const {
            //auto it = nbt.find("fall_distance");
            //if (it == nbt.end())
            //    return 0;
            //else
            //    return it->second;
            return 0; //TODO
        }

        void entity::set_fall_distance(int32_t fall_distance) {
            //nbt["fall_distance"] = fall_distance;//TODO
        }

        uint8_t entity::get_selected_item() const {
            if (handle.has<api::ecs::com::entities::held_slot>())
                return handle.get<api::ecs::com::entities::held_slot>().hotbar_slot;
            else
                return 0;
        }

        void entity::set_selected_item(uint8_t selected_item) {
            if (handle.has<api::ecs::com::entities::held_slot>())
                handle.modify<api::ecs::com::entities::held_slot>()->hotbar_slot = selected_item;
        }

        void entity::move([[maybe_unused]] float side, [[maybe_unused]] float forward, [[maybe_unused]] bool jump, [[maybe_unused]] bool sneaking) {
            //TODO
        }

        void entity::look(float yaw, float pitch) {
            set_head_rotation({yaw, pitch});
        }

        void entity::look_at(float x, float y, float z) {
            set_head_rotation(util::direction(handle.get<api::ecs::com::entities::position>(), util::vector{x, y, z}));
        }

        void entity::look_at(util::vector pos) {
            set_head_rotation(util::direction(handle.get<api::ecs::com::entities::position>(), pos));
        }

        void entity::look_at(ecs::entity entity) {
            if (api::entity(entity).current_world() == current_world())
                look_at(entity.get<api::ecs::com::entities::position>());
        }

        util::vector entity::get_motion() const {
            return handle.get<api::ecs::com::entities::motion>();
        }

        void entity::set_motion(util::vector mot) {
            if (current_world())
                current_world()->entity_motion_changes(handle, mot);
            *handle.modify<api::ecs::com::entities::motion>() = mot;
        }

        void entity::add_motion(util::vector mot) {
            set_motion(get_motion() += mot);
        }

        util::angle_deg entity::get_rotation() const {
            return handle.get<api::ecs::com::entities::rotation>();
        }

        void entity::set_rotation(util::angle_deg rot) {
            if (current_world())
                current_world()->entity_rotation_changes(handle, rot);
            *handle.modify<api::ecs::com::entities::rotation>() = rot;
        }

        void entity::add_rotation(util::angle_deg rot) {
            set_rotation(get_rotation() += rot);
        }

        util::angle_deg entity::get_head_rotation() const {
            return handle.get<api::ecs::com::entities::head_rotation>();
        }

        void entity::set_head_rotation(util::angle_deg rot) {
            if (current_world())
                current_world()->entity_look_changes(handle, rot);
            *handle.modify<api::ecs::com::entities::head_rotation>() = rot;
        }

        void entity::add_head_rotation(util::angle_deg rot) {
            set_head_rotation(get_head_rotation() += rot);
        }

        void entity::attack_from_this([[maybe_unused]] ecs::entity entity) {
        }

        void entity::breaking_block([[maybe_unused]] int32_t global_x, [[maybe_unused]] int32_t global_y, [[maybe_unused]] int32_t global_z, [[maybe_unused]] uint32_t time) {
        }

        void entity::place_block([[maybe_unused]] int32_t global_x, [[maybe_unused]] int32_t global_y, [[maybe_unused]] int32_t global_z, [[maybe_unused]] const base_objects::block&) {
        }

        void entity::place_block([[maybe_unused]] int32_t global_x, [[maybe_unused]] int32_t global_y, [[maybe_unused]] int32_t global_z, [[maybe_unused]] ecs::entity) {
        }


        ecs::entity entity::create(int32_t id) {
            auto it = entity_data::get_entity(id);
            ecs::entity res = ecs::global_registry::allocate_entity_and_wait(it.recipe);
            if (it.create_callback)
                it.create_callback(res);
            return res;
        }

        ecs::entity entity::create(int32_t id, const util::nbt_compound& nbt) {
            auto it = entity_data::get_entity(id);
            ecs::entity res = ecs::global_registry::allocate_entity_and_wait(it.recipe);
            res.modify<api::ecs::com::entities::entity_type>()->type = it.entity_id;
            res.modify<ecs::com::entities::nbt>()->get() = nbt;
            if (it.create_callback)
                it.create_callback(res);
            return res;
        }

        ecs::entity entity::create(const std::string& id) {
            auto it = entity_data::get_entity(id);
            ecs::entity res = ecs::global_registry::allocate_entity_and_wait(it.recipe);
            res.modify<api::ecs::com::entities::entity_type>()->type = it.entity_id;
            if (it.create_callback)
                it.create_callback(res);
            return res;
        }

        ecs::entity entity::create(const std::string& id, const util::nbt_compound& nbt) {
            auto it = entity_data::get_entity(id);
            ecs::entity res = ecs::global_registry::allocate_entity_and_wait(it.recipe);
            res.modify<ecs::com::entities::nbt>()->get() = nbt;
            if (it.create_callback)
                it.create_callback(res);
            return res;
        }

        std::optional<int32_t> entity::get_object_field() const {
            auto& obj_field_getter = entity_data::view(handle).get_object_field;
            if (!obj_field_getter)
                return std::nullopt;
            return obj_field_getter(handle);
        }

        bool entity::is_player() const {
            return handle.get<ecs::com::entities::entity_type>().type == entity_data::player_entity_id;
        }

        void entity::store_to_file(ecs::entity entity, util::nbt_write_stream& w) {
            entity.get<api::ecs::com::type_definition>().type->to_nbt(w, entity);
        }

        ecs::entity entity::load_from_file(util::nbt_read_stream& r) {
            std::string id;
            ecs::entity result;
            r.double_pass_read(
                [&](util::nbt_read_stream& id_find) {
                    id_find.read_compound().collect_into("id", id).force_all_collect();
                },
                [&](util::nbt_read_stream& load) {
                    result = api::ecs::get_entity_definition(id).from_nbt(load);
                }
            );

            try {
                auto load_callback = entity_data::view(result).load_callback;
                if (load_callback)
                    load_callback(result);
            } catch (...) {
                if (api::entity(result).current_world())
                    api::entity(result).current_world()->unregister_entity(result);
                throw;
            }
            return result;
        }
    }
}
