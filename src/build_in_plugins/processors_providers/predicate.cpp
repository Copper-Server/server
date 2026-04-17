/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/ecs/base_components.hpp>
#include <src/api/entity.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/predicate.hpp>
#include <src/api/registers.hpp>
#include <src/api/tags.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/uuid.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::processors_providers {
    template <class T>
    bool diff_min_max(const util::nbt& val, T value) {
        if (!val.is_compound())
            return false;
        auto com = val.get_compound();
        if (!com.contains("min") || !com.contains("max"))
            return false;
        if constexpr (std::is_integral_v<T>)
            return value >= (T)com.at("min").as_long() && value <= (T)com.at("max").as_long();
        else
            return value >= (T)com.at("min").as_double() && value <= (T)com.at("max").as_double();
    }

    bool __item_check([[maybe_unused]] const std::unordered_map<std::string, util::nbt>& predicate, [[maybe_unused]] const std::unordered_map<std::string, util::nbt>& item) {
        return false; //TODO
    }

    bool __item_check([[maybe_unused]] const std::unordered_map<std::string, util::nbt>& predicate, [[maybe_unused]] const base_objects::slot_data& item) {
        return false; //TODO
    }

    bool __item_check([[maybe_unused]] const std::unordered_map<std::string, util::nbt>& predicate, [[maybe_unused]] const base_objects::slot& item) {
        return false; //TODO
    }

    bool __location_check([[maybe_unused]] const std::unordered_map<std::string, util::nbt>& predicate, [[maybe_unused]] util::vector pos, [[maybe_unused]] util::angle_deg rot, [[maybe_unused]] storage::world_data& assigned_world) {
        return false; //TODO
    }

    bool __entity_check([[maybe_unused]] const std::unordered_map<std::string, util::nbt>& predicate, [[maybe_unused]] base_objects::uuid entity_uuid) {
        auto entity_ = api::entity_id_map::get_entity(entity_uuid);
        if (!entity_)
            return false;
        api::entity entity(*entity_);
        auto entity_const_data = entity.const_data();
        if (predicate.contains("type"))
            if (entity_const_data.id != predicate.at("type").get_string())
                return false;

        if (predicate.contains("distance")) {
            auto& distance = predicate.at("distance").get_compound();
            auto pos = entity.get_position();
            if (distance.contains("x"))
                if (!diff_min_max(distance.at("x"), pos.x))
                    return false;
            if (distance.contains("y"))
                if (!diff_min_max(distance.at("y"), pos.y))
                    return false;
            if (distance.contains("z"))
                if (!diff_min_max(distance.at("z"), pos.z))
                    return false;
            if (distance.contains("absolute"))
                if (
                    !diff_min_max(distance.at("absolute"), pos.x)
                    || !diff_min_max(distance.at("absolute"), pos.y)
                    || !diff_min_max(distance.at("absolute"), pos.z)
                )
                    return false;

            if (distance.contains("horizontal"))
                if (
                    !diff_min_max(distance.at("horizontal"), pos.x)
                    || !diff_min_max(distance.at("horizontal"), pos.z)
                )
                    return false;
        }

        if (predicate.contains("effects")) {
            auto effects = predicate.at("effects").get_compound();
            auto& active_effects = entity_->get<api::ecs::com::entities::effects>().active_effects();
            for (auto& [key, value] : effects) {
                auto& conditions = value.get_compound();

                auto id = api::registers::effects.at(key).id;
                if (auto it = active_effects.find(id); it != active_effects.end()) {
                    auto& effect = it->second;
                    if (conditions.contains("amplifier"))
                        if (!diff_min_max(conditions.at("amplifier"), effect.amplifier))
                            return false;

                    if (conditions.contains("duration"))
                        if (!diff_min_max(conditions.at("duration"), effect.duration))
                            return false;

                    if (conditions.contains("ambient"))
                        if ((bool)conditions.at("ambient").as_byte() != effect.ambient)
                            return false;

                    if (conditions.contains("visible"))
                        if ((bool)conditions.at("visible").as_byte() != effect.particles)
                            return false;
                } else
                    return false;
            }
        }

        if (predicate.contains("equipment")) {
            auto& equipment = predicate.at("equipment").get_compound();

            for (auto& [key, value] : equipment) {
                auto& item = value.get_compound();
                uint32_t slot_ = 0;
                if (key == "mainhand") {
                    slot_ = std::bit_cast<uint32_t>(entity_const_data.data.at("slots")["mainhand"].get_int());
                } else if (key == "offhand") {
                    slot_ = std::bit_cast<uint32_t>(entity_const_data.data.at("slots")["offhand"].get_int());
                } else if (key == "head") {
                    slot_ = std::bit_cast<uint32_t>(entity_const_data.data.at("slots")["head"].get_int());
                } else if (key == "chest") {
                    slot_ = std::bit_cast<uint32_t>(entity_const_data.data.at("slots")["chest"].get_int());
                } else if (key == "legs") {
                    slot_ = std::bit_cast<uint32_t>(entity_const_data.data.at("slots")["legs"].get_int());
                } else if (key == "feet") {
                    slot_ = std::bit_cast<uint32_t>(entity_const_data.data.at("slots")["feet"].get_int());
                } else if (key == "body") {
                    for (util::nbt& body_slot__ : entity_const_data.data.at("slots")["body"].get_list()) {
                        auto& inventory = entity_->get<api::ecs::com::entities::inventory>().get();
                        auto body_slot_ = std::bit_cast<uint32_t>(body_slot__.get_int());
                        if (!inventory.contains(body_slot_))
                            return false;
                        if (__item_check(item, inventory.at(body_slot_)))
                            return false;
                    }
                    continue;
                } else if (key == "hand") {
                    for (util::nbt& hand_slot__ : entity_const_data.data.at("slots")["hand"].get_list()) {
                        auto& inventory = entity_->get<api::ecs::com::entities::inventory>().get();
                        auto hand_slot_ = std::bit_cast<uint32_t>(hand_slot__.get_int());
                        if (!inventory.contains(hand_slot_))
                            return false;
                        if (__item_check(item, inventory.at(hand_slot_)))
                            return false;
                    }
                    continue;
                } else {
                    return false;
                }
                auto& inventory = entity_->get<api::ecs::com::entities::inventory>().get();
                if (!inventory.contains(slot_))
                    return false;
                if (__item_check(item, inventory.at(slot_)))
                    return false;
            }
        }

        //if (predicate.contains("flags")) {
        //    if (!entity->nbt.contains("flags"))
        //        return false;
        //    auto flags = predicate["flags"].get_compound();
        //    auto entity_flags = entity->nbt.at("flags").get_compound();
        //    for (auto& [key, value] : flags) {
        //        if (!entity_flags.contains(key))
        //            return false;
        //        if (entity_flags.at(key) != value)
        //            return false;
        //    }
        //}
        //
        //if (predicate.contains("location")) {
        //    if (!entity->current_world())
        //        return false;
        //    auto location = predicate["location"].get_compound();
        //    if (!__location_check(location, entity->position, entity->rotation, *entity->current_world()))
        //        return false;
        //}
        //
        //if (predicate.contains("nbt")) {
        //    auto nbt = predicate["nbt"].get_compound();
        //    auto& entity_nbt = entity->nbt;
        //    for (auto& [key, value] : nbt) {
        //        if (!entity_nbt.contains(key))
        //            return false;
        //        if (entity_nbt.at(key) != value)
        //            return false;
        //    }
        //}
        //
        //if (predicate.contains("passenger")) {
        //    auto passenger = predicate["passenger"].get_compound();
        //    if (!entity->nbt.contains("passengers"))
        //        return false;
        //    auto passengers = entity->nbt.at("passengers").as_dyn_array();
        //    for (auto& passenger_ : passengers) {
        //        if (!passenger_.is_compound())
        //            return false;
        //        if (__entity_check(passenger, passenger_))
        //            return false;
        //    }
        //}
        //
        //if (predicate.contains("slots")) {
        //    //TODO
        //}
        //
        //if (predicate.contains("stepping_on")) {
        //    if (!entity->current_world())
        //        return false;
        //    auto stepping_on = predicate["stepping_on"].get_compound();
        //    if (!__location_check(stepping_on, entity->position, entity->rotation, *entity->current_world()))
        //        return false;
        //}
        //
        //if (predicate.contains("movement_affected_by")) {
        //    if (!entity->current_world())
        //        return false;
        //    auto movement_affected_by = predicate["movement_affected_by"].get_compound();
        //    if (!__location_check(
        //            movement_affected_by,
        //            {entity->position.x, entity->position.y - 0.5, entity->position.z},
        //            entity->rotation,
        //            *entity->current_world()
        //        ))
        //        return false;
        //}

        if (predicate.contains("team")) {
            //TODO
        }

        if (predicate.contains("targeted_entity")) {
            //TODO
        }

        //if (predicate.contains("vehicle")) {
        //    if (!__entity_check(predicate["vehicle"].get_compound(), entity->nbt.at("vehicle")))
        //        return false;
        //}
        //
        //if (predicate.contains("movement")) {
        //    auto movement = predicate["movement"].get_compound();
        //    if (movement.contains("x"))
        //        if (!diff_min_max(movement["x"], entity->motion.x))
        //            return false;
        //    if (movement.contains("y"))
        //        if (!diff_min_max(movement["y"], entity->motion.y))
        //            return false;
        //    if (movement.contains("z"))
        //        if (!diff_min_max(movement["z"], entity->motion.z))
        //            return false;
        //    if (movement.contains("horizontal_speed"))
        //        if (!diff_min_max(movement["horizontal_speed"], std::sqrt(entity->motion.x * entity->motion.x + entity->motion.z * entity->motion.z)))
        //            return false;
        //
        //    if (movement.contains("vertical_speed"))
        //        if (!diff_min_max(movement["vertical_speed"], entity->motion.y))
        //            return false;
        //
        //    if (movement.contains("fall_distance"))
        //        if (!diff_min_max(movement["fall_distance"], (double)entity->nbt.at("fall_distance")))
        //            return false;
        //}
        //if (predicate.contains("periodic_tick")) {
        //    if (entity->nbt.contains("age")) {
        //        int32_t age = entity->nbt.at("age");
        //        if (age % (int32_t)predicate.at("periodic_tick") != 0)
        //            return false;
        //    } else {
        //        return false;
        //    }
        //}
        //
        //if (predicate.contains("type_specific")) {
        //    auto type_specific = predicate["type_specific"].get_compound();
        //    std::string type = type_specific.at("type");
        //    if (type == "cat") {
        //        //TODO
        //    } else if (type == "fishing_hook") {
        //        if (!entity->nbt.contains("in_open_water"))
        //            return false;
        //        if (type_specific.contains("in_open_water"))
        //            if (entity->nbt.at("in_open_water") != type_specific.at("in_open_water"))
        //                return false;
        //    } else if (type == "frog") {
        //        //TODO
        //    } else if (type == "lightning") {
        //        //TODO
        //    } else if (type == "player") {
        //        //TODO
        //    } else if (type == "raider") {
        //        //TODO
        //    } else if (type == "slime") {
        //        //TODO
        //    } else if (type == "wolf") {
        //        //TODO
        //    } else
        //        return false;
        //}
        //
        //if (predicate.contains("source_entity")) { //TODO check
        //    auto source_entity = predicate["source_entity"].get_compound();
        //    if (!entity->nbt.contains("source_entity"))
        //        return false;
        //    if (__entity_check(source_entity, entity->nbt.at("source_entity")))
        //        return false;
        //}

        if (predicate.contains("is_direct")) {
            //TODO
        }

        if (predicate.contains("tags")) {
            //TODO
        }

        return true;
    }

    bool _server_helper__adventure_block_(const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        if (!predicate.contains("blocks"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();
        if (!loot_context.contains("block_state"))
            return false;
        auto block_id = std::bit_cast<base_objects::block_id_t>(loot_context.at("block_state").get_int());
        if (!loot_context.contains("origin"))
            return false;
        auto origin = loot_context.at("origin").get_compound();
        util::vector pos = {origin.at("x").as_double(), origin.at("y").as_double(), origin.at("z").as_double()};
        int32_t world_id;
        if (origin.contains("world_id"))
            world_id = origin.at("world_id").get_int();
        else
            world_id = api::world::resolve_id(context.executor.player_data.world_id);

        auto block = base_objects::block::getStaticData(block_id);
        auto& pred_block = predicate.at("blocks");
        if (pred_block.is_string()) {
            const std::string& block_or_id = pred_block.as_string();
            if (block_or_id.starts_with('#')) {
                if (!api::tags::unfold_tag(api::tags::builtin_entry::block, block_or_id).contains(block.general_block_id))
                    return false;
            } else {
                if (block.name != block_or_id)
                    return false;
            }
        } else {
            bool contains = false;
            for (auto& it : pred_block.get_list()) {
                if (it.get_string() == block.name) {
                    contains = true;
                    break;
                }
            }
            if (!contains)
                return false;
        }
        if (predicate.contains("state")) {
            auto properties = predicate["state"].get_compound();
            auto states_ = block.assigned_states_to_properties->left.at(block_id);
            for (auto& [key, value] : properties) {
                if (!states_.contains(key))
                    return false;
                if (!value.is_compound()) {
                    if (states_.at(key) != value.get_string())
                        return false;
                } else {
                    auto range = value.get_compound();
                    if (!range.contains("min") || !range.contains("max"))
                        return false;
                    auto key_ll = std::stoll(states_.at(key));
                    if (key_ll < range.at("min").as_long() || key_ll > range.at("max").as_long())
                        return false;
                }
            }
        }

        if (predicate.contains("nbt")) {
            bool pass = false;
            //TODO implement predicates for block entities
            //if (!predicate["nbt"].size())
            //    return true;
            //api::world::get(world_id, [&](storage::world_data& data) {
            //    data.get_block((int32_t)pos.x, (int32_t)pos.y, (int32_t)pos.z, [](auto) {}, [&](auto, const util::nbt& ex_data) { pass = ex_data != predicate["nbt"]; });
            //});
            return pass;
        }
        return true;
    }

    bool block_state_property(const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        if (!predicate.contains("block"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();
        if (!loot_context.contains("block_state"))
            return false;
        auto block_id = std::bit_cast<base_objects::block_id_t>(loot_context.at("block_state").get_int());
        auto block = base_objects::block::getStaticData(block_id);
        if (block.name != predicate.at("block").get_string())
            return false;
        if (!predicate.contains("properties"))
            return true;
        else {
            auto properties = predicate["properties"].get_compound();
            auto states_ = block.assigned_states_to_properties->left.at(block_id);
            for (auto& [key, value] : properties) {
                if (!states_.contains(key))
                    return false;
                if (!value.is_compound()) {
                    if (states_.at(key) != value.get_string())
                        return false;
                } else {
                    auto range = value.get_compound();
                    if (!range.contains("min") || !range.contains("max"))
                        return false;
                    auto key_ll = std::stoll(states_.at(key));
                    if (key_ll < range.at("min").get_long() || key_ll > range.at("max").get_long())
                        return false;
                }
            }
            return true;
        }
    }

    bool damage_source_properties(const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();

        if (!loot_context.contains("damage_source"))
            return false;
        if (!loot_context.contains("origin"))
            return false;
        auto& damage_source = loot_context.at("damage_source");
        //auto& origin = loot_context.at("origin");

        return __entity_check(predicate.at("predicate").get_compound(), damage_source.as_uuid());
    }

    bool enchantment_active_check(const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();

        if (!loot_context.contains("enchantment_active_status"))
            return false;

        return loot_context.at("enchantment_active_status") == predicate.at("active");
    }

    bool entity_properties(const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();
        if (!predicate.contains("entity"))
            return false;
        std::string entity = predicate.at("entity").get_string();

        if (!loot_context.contains(entity))
            return false;
        return __entity_check(predicate.at("predicate").get_compound(), loot_context.at(entity).as_uuid());
    }

    bool entity_scores([[maybe_unused]] const util::nbt_compound& predicate, [[maybe_unused]] const base_objects::command_context& context) {
        //TODO
        return false;
    }

    bool killed_by_player([[maybe_unused]] const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();
        if (!loot_context.contains("attacking_player"))
            return false;
        return true;
    }

    bool location_check(const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();
        if (!loot_context.contains("origin"))
            return false;
        auto origin = loot_context.at("origin").get_compound();
        util::vector pos = {origin.at("x").as_double(), origin.at("y").as_double(), origin.at("z").as_double()};
        int32_t world_id;
        if (origin.contains("world"))
            world_id = origin.at("world").get_int();
        else
            world_id = api::world::resolve_id(context.executor.player_data.world_id);


        int32_t offset_x = predicate.contains("offsetX") ? predicate.at("offsetX").as_int() : 0;
        int32_t offset_y = predicate.contains("offsetY") ? predicate.at("offsetY").as_int() : 0;
        int32_t offset_z = predicate.contains("offsetZ") ? predicate.at("offsetZ").as_int() : 0;

        bool res = false;
        api::world::get(world_id, [&](auto& world) {
            res = __location_check(
                predicate.at("predicate").get_compound(),
                {pos.x + offset_x,
                 pos.y + offset_y,
                 pos.z + offset_z},
                {0, 0},
                world
            );
        });
        return res;
    }

    bool match_tool(const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();
        if (!loot_context.contains("tool"))
            return false;
        return __item_check(predicate.at("predicate").get_compound(), loot_context.at("tool").get_compound());
    }

    bool random_chance([[maybe_unused]] const util::nbt_compound& predicate, [[maybe_unused]] const base_objects::command_context& context) {
        //TODO
        return false;
    }

    bool random_chance_with_enchanted_bonus([[maybe_unused]] const util::nbt_compound& predicate, const base_objects::command_context& context) {
        //int32_t enchantment_level = 0;
        if (context.other_data.contains("loot_context")) {
            auto loot_context = context.other_data.at("loot_context").get_compound();
            if (loot_context.contains("attacker")) {
                //TODO
            }
        }

        //TODO
        return false;
    }

    bool survives_explosion([[maybe_unused]] const util::nbt_compound& predicate, const base_objects::command_context& context) {
        if (!context.other_data.contains("loot_context"))
            return false;
        auto loot_context = context.other_data.at("loot_context").get_compound();
        if (!loot_context.contains("explosion_radius"))
            return false;
        double explosion_radius = loot_context.at("explosion_radius").as_double();
        double chance = 1 / explosion_radius;
        return rand() % 100 < chance * 100;
    }

    bool table_bonus([[maybe_unused]] const util::nbt_compound& predicate, [[maybe_unused]] const base_objects::command_context& context) {
        //TODO
        return false;
    }

    bool time_check([[maybe_unused]] const util::nbt_compound& predicate, [[maybe_unused]] const base_objects::command_context& context) {
        //TODO
        return false;
    }

    bool value_check([[maybe_unused]] const util::nbt_compound& predicate, [[maybe_unused]] const base_objects::command_context& context) {
        //TODO
        return false;
    }

    bool weather_check([[maybe_unused]] const util::nbt_compound& predicate, [[maybe_unused]] const base_objects::command_context& context) {
        //TODO
        return false;
    }

    struct predicate : public plugin_auto_register<"processors_provider/predicate", predicate> {
        void on_initialization(const plugin_registration_ptr&) override {
            api::predicate::register_handler("all_of", [&](const util::nbt_compound& predicate, const base_objects::command_context& context) {
                for (auto& value : predicate["terms"].get_list()) {
                    if (!api::predicate::process_predicate(
                            value.get_compound(),
                            context
                        ))
                        return false;
                }
                return true;
            });
            api::predicate::register_handler("any_of", [&](const util::nbt_compound& predicate, const base_objects::command_context& context) {
                for (auto& value : predicate["terms"].get_list()) {
                    if (api::predicate::process_predicate(
                            value.get_compound(),
                            context
                        ))
                        return true;
                }
                return false;
            });
            api::predicate::register_handler("inverted", [&](const util::nbt_compound& predicate, const base_objects::command_context& context) {
                return !api::predicate::process_predicate(
                    predicate.at("term").get_compound(),
                    context
                );
            });
            api::predicate::register_handler("reference", [&](const util::nbt_compound& predicate, const base_objects::command_context& context) {
                return api::predicate::process_predicate( //TODO
                    context.other_data.at(predicate.at("name").as_string()).get_compound(),
                    context
                );
            });


            api::predicate::register_handler("copper_server:__adventure_block_", _server_helper__adventure_block_);
            api::predicate::register_handler("block_state_property", block_state_property);
            api::predicate::register_handler("damage_source_properties", damage_source_properties);
            api::predicate::register_handler("enchantment_active_check", enchantment_active_check);
            api::predicate::register_handler("entity_properties", entity_properties);
            api::predicate::register_handler("entity_scores", entity_scores);
            api::predicate::register_handler("killed_by_player", killed_by_player);
            api::predicate::register_handler("location_check", location_check);
            api::predicate::register_handler("match_tool", match_tool);
            api::predicate::register_handler("random_chance", random_chance);
            api::predicate::register_handler("random_chance_with_enchanted_bonus", random_chance_with_enchanted_bonus);
            api::predicate::register_handler("survives_explosion", survives_explosion);
            api::predicate::register_handler("table_bonus", table_bonus);
            api::predicate::register_handler("time_check", time_check);
            api::predicate::register_handler("value_check", value_check);
            api::predicate::register_handler("weather_check", weather_check);
        }
    };
}
