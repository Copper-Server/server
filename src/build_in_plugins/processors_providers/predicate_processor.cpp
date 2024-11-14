#include <src/api/internal/predicate.hpp>
#include <src/api/predicate.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/player.hpp>
#include <src/build_in_plugins/processors_providers/predicate_processor.hpp>
#include <src/registers.hpp>

namespace copper_server {
    namespace build_in_plugins {
        template <class T>
        bool diff_min_max(const enbt::value& val, T value) {
            if (!val.is_compound())
                return false;
            auto com = val.as_compound();
            if (!com.contains("min") || !com.contains("max"))
                return false;
            return value >= (T)com.at("min") && value <= (T)com.at("max");
        }

        bool __item_check(const enbt::compound_const_ref& predicate, const enbt::compound_const_ref& item) {
            return false; //TODO
        }

        bool __location_check(const enbt::compound_const_ref& predicate, calc::VECTOR pos, calc::VECTOR rot, storage::world_data* assigned_world) {
            return false; //TODO
        }

        bool __entity_check(const enbt::compound_const_ref& predicate, enbt::raw_uuid entity_uuid) {
            base_objects::entity_ref entity; //TODO resolve form entity_uuid
            if (!entity)
                return false;
            auto entity_const_data = entity->const_data();
            if (predicate.contains("type"))
                if (entity_const_data.id != (std::string)predicate.at("type"))
                    return false;

            if (predicate.contains("distance")) {
                auto distance = predicate["distance"].as_compound();
                if (distance.contains("x"))
                    if (!diff_min_max(distance["x"], entity->position.x))
                        return false;
                if (distance.contains("y"))
                    if (!diff_min_max(distance["y"], entity->position.y))
                        return false;
                if (distance.contains("z"))
                    if (!diff_min_max(distance["z"], entity->position.z))
                        return false;
                if (distance.contains("absolute"))
                    if (
                        !diff_min_max(distance["absolute"], entity->position.x)
                        || !diff_min_max(distance["absolute"], entity->position.y)
                        || !diff_min_max(distance["absolute"], entity->position.z)
                    )
                        return false;

                if (distance.contains("horizontal"))
                    if (
                        !diff_min_max(distance["horizontal"], entity->position.x)
                        || !diff_min_max(distance["horizontal"], entity->position.z)
                    )
                        return false;
            }

            if (predicate.contains("effects")) {
                auto effects = predicate["effects"].as_compound();
                for (auto& [key, value] : effects) {
                    auto conditions = value.as_compound();
                    if (!entity->nbt.contains("effects"))
                        return false;

                    auto entity_effects = entity->nbt.at("effects").as_compound();
                    if (!entity_effects.contains(key))
                        return false;
                    auto effect = entity_effects.at(key).as_compound();
                    if (conditions.contains("amplifier"))
                        if (!diff_min_max(conditions.at("amplifier"), (int32_t)effect.at("amplifier")))
                            return false;

                    if (conditions.contains("duration"))
                        if (!diff_min_max(conditions.at("duration"), (int32_t)effect.at("duration")))
                            return false;

                    if (conditions.contains("ambient"))
                        if (conditions.at("ambient") != effect.at("ambient"))
                            return false;

                    if (conditions.contains("visible"))
                        if (conditions.at("visible") != effect.at("visible"))
                            return false;
                }
            }

            if (predicate.contains("equipment")) {
                auto equipment = predicate["equipment"].as_compound();
                if (!entity->nbt.contains("inventory"))
                    return false;
                for (auto& [key, value] : equipment) {
                    auto item = value.as_compound();
                    uint32_t slot_;
                    if (key == "mainhand") {
                        slot_ = entity_const_data.data.at("slots")["mainhand"];
                    } else if (key == "offhand") {
                        slot_ = entity_const_data.data.at("slots")["offhand"];
                    } else if (key == "head") {
                        slot_ = entity_const_data.data.at("slots")["head"];
                    } else if (key == "chest") {
                        slot_ = entity_const_data.data.at("slots")["chest"];
                    } else if (key == "legs") {
                        slot_ = entity_const_data.data.at("slots")["legs"];
                    } else if (key == "feet") {
                        slot_ = entity_const_data.data.at("slots")["feet"];
                    } else if (key == "body") {
                        for (uint32_t slot_ : entity_const_data.data.at("slots")["body"].as_ui32_array()) {
                            auto inventory = entity->nbt.at("inventory").as_dyn_array();
                            if (!inventory.at(slot_).is_compound())
                                return false;
                            auto item_ = inventory.at(slot_).as_compound();
                            if (__item_check(item, item_))
                                return false;
                        }
                    } else if (key == "hand") {
                        for (uint32_t slot_ : entity_const_data.data.at("slots")["hand"].as_ui32_array()) {
                            auto inventory = entity->nbt.at("inventory").as_dyn_array();
                            if (!inventory.at(slot_).is_compound())
                                return false;
                            auto item_ = inventory.at(slot_).as_compound();
                            if (__item_check(item, item_))
                                return false;
                        }
                    } else {
                        return false;
                    }
                    auto inventory = entity->nbt.at("inventory").as_dyn_array();
                    if (!inventory.at(slot_).is_compound())
                        return false;
                    auto item_ = inventory.at(slot_).as_compound();
                    if (__item_check(item, item_))
                        return false;
                }
            }

            if (predicate.contains("flags")) {
                if (!entity->nbt.contains("flags"))
                    return false;
                auto flags = predicate["flags"].as_compound();
                auto entity_flags = entity->nbt.at("flags").as_compound();
                for (auto& [key, value] : flags) {
                    if (!entity_flags.contains(key))
                        return false;
                    if (entity_flags.at(key) != value)
                        return false;
                }
            }

            if (predicate.contains("location")) {
                auto location = predicate["location"].as_compound();
                if (!__location_check(location, entity->position, entity->rotation, entity->world))
                    return false;
            }

            if (predicate.contains("nbt")) {
                auto nbt = predicate["nbt"].as_compound();
                auto& entity_nbt = entity->nbt;
                for (auto& [key, value] : nbt) {
                    if (!entity_nbt.contains(key))
                        return false;
                    if (entity_nbt.at(key) != value)
                        return false;
                }
            }

            if (predicate.contains("passenger")) {
                auto passenger = predicate["passenger"].as_compound();
                if (!entity->nbt.contains("passengers"))
                    return false;
                auto passengers = entity->nbt.at("passengers").as_dyn_array();
                for (auto& passenger_ : passengers) {
                    if (!passenger_.is_compound())
                        return false;
                    if (__entity_check(passenger, passenger_))
                        return false;
                }
            }

            if (predicate.contains("slots")) {
                //TODO
            }

            if (predicate.contains("stepping_on")) {
                auto stepping_on = predicate["stepping_on"].as_compound();
                if (!__location_check(stepping_on, entity->position, entity->rotation, entity->world))
                    return false;
            }

            if (predicate.contains("movement_affected_by")) {
                auto movement_affected_by = predicate["movement_affected_by"].as_compound();
                if (!__location_check(
                        movement_affected_by,
                        {entity->position.x, entity->position.y - 0.5, entity->position.z},
                        entity->rotation,
                        entity->world
                    ))
                    return false;
            }

            if (predicate.contains("team")) {
                //TODO
            }

            if (predicate.contains("targeted_entity")) {
                //TODO
            }

            if (predicate.contains("vehicle")) {
                if (!__entity_check(predicate["vehicle"].as_compound(), entity->nbt.at("vehicle")))
                    return false;
            }

            if (predicate.contains("movement")) {
                auto movement = predicate["movement"].as_compound();
                if (movement.contains("x"))
                    if (!diff_min_max(movement["x"], entity->motion.x))
                        return false;
                if (movement.contains("y"))
                    if (!diff_min_max(movement["y"], entity->motion.y))
                        return false;
                if (movement.contains("z"))
                    if (!diff_min_max(movement["z"], entity->motion.z))
                        return false;
                if (movement.contains("horizontal_speed"))
                    if (!diff_min_max(movement["horizontal_speed"], std::sqrt(entity->motion.x * entity->motion.x + entity->motion.z * entity->motion.z)))
                        return false;

                if (movement.contains("horizontal_speed"))
                    if (!diff_min_max(movement["vertical_speed"], entity->motion.y))
                        return false;

                if (movement.contains("fall_distance"))
                    if (!diff_min_max(movement["fall_distance"], (double)entity->nbt.at("fall_distance")))
                        return false;
            }
            if (predicate.contains("periodic_tick")) {
                if (entity->nbt.contains("age")) {
                    int32_t age = entity->nbt.at("age");
                    if (age % (int32_t)predicate.at("periodic_tick") != 0)
                        return false;
                } else {
                    return false;
                }
            }

            if (predicate.contains("type_specific")) {
                auto type_specific = predicate["type_specific"].as_compound();
                std::string type = type_specific.at("type");
                if (type == "cat") {
                    //TODO
                } else if (type == "fishing_hook") {
                    if (!entity->nbt.contains("in_open_water"))
                        return false;
                    if (type_specific.contains("in_open_water"))
                        if (entity->nbt.at("in_open_water") != type_specific.at("in_open_water"))
                            return false;
                } else if (type == "frog") {
                    //TODO
                } else if (type == "lightning") {
                    //TODO
                } else if (type == "player") {
                    //TODO
                } else if (type == "raider") {
                    //TODO
                } else if (type == "slime") {
                    //TODO
                } else if (type == "wolf") {
                    //TODO
                } else
                    return false;
            }

            if (predicate.contains("source_entity")) { //TODO check
                auto source_entity = predicate["source_entity"].as_compound();
                if (!entity->nbt.contains("source_entity"))
                    return false;
                if (__entity_check(source_entity, entity->nbt.at("source_entity")))
                    return false;
            }

            if (predicate.contains("is_direct")) {
                //TODO
            }

            if (predicate.contains("tags")) {
                //TODO
            }

            return true;
        }

        bool _server_helper__adventure_block_(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            if (!predicate.contains("blocks"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();
            if (!loot_context.contains("block_state"))
                return false;
            base_objects::block_id_t block_id = loot_context.at("block_state");
            if (!loot_context.contains("origin"))
                return false;
            auto origin = loot_context.at("origin").as_compound();
            calc::VECTOR pos = {origin.at("x"), origin.at("y"), origin.at("z")};
            uint64_t world_id;
            if (origin.contains("world"))
                world_id = origin.at("world");
            else if (context.executor)
                world_id = api::world::resolve_id(context.executor->player_data.world_id);
            else
                world_id = api::world::get_default_world_id();

            auto block = base_objects::block::getStaticData(block_id);
            auto& pred_block = predicate.at("blocks");
            if (pred_block.is_string()) {
                const std::string& block_or_id = pred_block.as_string();
                if (block_or_id.starts_with('#')) {
                    if (!registers::unfold_tag("block", block_or_id).contains(block.name))
                        return false;
                } else {
                    if (block.name != block_or_id)
                        return false;
                }
            } else {
                bool contains = false;
                for (auto& it : pred_block.as_dyn_array()) {
                    if (it == block.name) {
                        contains = true;
                        break;
                    }
                }
                if (!contains)
                    return false;
            }
            if (predicate.contains("state")) {
                auto properties = predicate["state"].as_compound();
                auto states_ = block.assigned_states.left.at(block_id);
                for (auto& [key, value] : properties) {
                    if (!block.defintion.contains(key))
                        return false;
                    if (!value.is_compound()) {
                        if (states_.at(key) != (std::string)value)
                            return false;
                    } else {
                        auto range = value.as_compound();
                        if (!range.contains("min") || !range.contains("max"))
                            return false;
                        auto value = std::stoll(states_.at(key));
                        if (value < (int64_t)range.at("min") || value > (int64_t)range.at("max"))
                            return false;
                    }
                }
            }

            if (predicate.contains("nbt")) {
                bool pass = false;
                if (!predicate["nbt"].size())
                    return true;
                api::world::get(world_id, [&](storage::world_data& data) {
                    data.get_block(pos.x, pos.y, pos.z, [](auto) {}, [&](auto, const enbt::value& ex_data) { pass = ex_data != predicate["nbt"]; });
                });
                return pass;
            }
            return true;
        }

        bool block_state_property(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            if (!predicate.contains("block"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();
            if (!loot_context.contains("block_state"))
                return false;
            base_objects::block_id_t block_id = loot_context.at("block_state");
            auto block = base_objects::block::getStaticData(block_id);
            if (block.name != (std::string)predicate.at("block"))
                return false;
            if (!predicate.contains("properties"))
                return true;
            else {
                auto properties = predicate["properties"].as_compound();
                auto states_ = block.assigned_states.left.at(block_id);
                for (auto& [key, value] : properties) {
                    if (!block.defintion.contains(key))
                        return false;
                    if (!value.is_compound()) {
                        if (states_.at(key) != (std::string)value)
                            return false;
                    } else {
                        auto range = value.as_compound();
                        if (!range.contains("min") || !range.contains("max"))
                            return false;
                        auto value = std::stoll(states_.at(key));
                        if (value < (int64_t)range.at("min") || value > (int64_t)range.at("max"))
                            return false;
                    }
                }
                return true;
            }
        }

        bool damage_source_properties(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();

            if (!loot_context.contains("damage_source"))
                return false;
            if (!loot_context.contains("origin"))
                return false;
            auto& damage_source = loot_context.at("damage_source");
            auto& origin = loot_context.at("origin");

            return __entity_check(predicate.at("predicate").as_compound(), damage_source);
        }

        bool enchantment_active_check(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();

            if (!loot_context.contains("enchantment_active_status"))
                return false;

            return loot_context.at("enchantment_active_status") == predicate.at("active");
        }

        bool entity_properties(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();
            if (!predicate.contains("entity"))
                return false;
            std::string entity = predicate.at("entity");

            if (!loot_context.contains(entity))
                return false;
            return __entity_check(predicate.at("predicate").as_compound(), loot_context.at(entity));
        }

        bool entity_scores(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            //TODO
            return false;
        }

        bool killed_by_player(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();
            if (!loot_context.contains("attacking_player"))
                return false;
            return true;
        }

        bool location_check(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();
            if (!loot_context.contains("origin"))
                return false;
            auto origin = loot_context.at("origin").as_compound();
            calc::VECTOR pos = {origin.at("x"), origin.at("y"), origin.at("z")};
            uint64_t world_id;
            if (origin.contains("world"))
                world_id = origin.at("world");
            else if (context.executor)
                world_id = api::world::resolve_id(context.executor->player_data.world_id);
            else
                world_id = api::world::get_default_world_id();


            int32_t offset_x = predicate.contains("offsetX") ? (int32_t)predicate.at("offsetX") : 0;
            int32_t offset_y = predicate.contains("offsetY") ? (int32_t)predicate.at("offsetY") : 0;
            int32_t offset_z = predicate.contains("offsetZ") ? (int32_t)predicate.at("offsetZ") : 0;

            bool res = false;
            api::world::get(world_id, [&](auto& world) {
                res = __location_check(
                    predicate.at("predicate").as_compound(),
                    {pos.x + offset_x,
                     pos.y + offset_y,
                     pos.z + offset_z
                    },
                    {0, 0},
                    &world
                );
            });
            return res;
        }

        bool match_tool(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();
            if (!loot_context.contains("tool"))
                return false;
            return __item_check(predicate.at("predicate").as_compound(), loot_context.at("tool").as_compound());
        }

        bool random_chance(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            //TODO
            return false;
        }

        bool random_chance_with_enchanted_bonus(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            int32_t enchantmant_level = 0;
            if (context.other_data.contains("loot_context")) {
                auto loot_context = context.other_data.at("loot_context").as_compound();
                if (loot_context.contains("attacker")) {
                    //TODO
                }
            }

            //TODO
            return false;
        }

        bool survives_explosion(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            if (!context.other_data.contains("loot_context"))
                return false;
            auto loot_context = context.other_data.at("loot_context").as_compound();
            if (!loot_context.contains("explosion_radius"))
                return false;
            double explosion_radius = loot_context.at("explosion_radius");
            double chance = 1 / explosion_radius;
            return rand() % 100 < chance * 100;
        }

        bool table_bonus(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            //TODO
            return false;
        }

        bool time_check(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            //TODO
            return false;
        }

        bool value_check(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            //TODO
            return false;
        }

        bool weather_check(const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
            //TODO
            return false;
        }

        PredicateProcessor::PredicateProcessor() {}

        void PredicateProcessor::OnInitialization(const PluginRegistrationPtr& self) {
            processor.register_handler("all_of", [&](const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
                for (auto& value : predicate["terms"].as_array()) {
                    if (!processor.process_predicate(
                            value.as_compound(),
                            context
                        ))
                        return false;
                }
                return true;
            });
            processor.register_handler("any_of", [&](const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
                for (auto& value : predicate["terms"].as_array()) {
                    if (processor.process_predicate(
                            value.as_compound(),
                            context
                        ))
                        return true;
                }
                return false;
            });
            processor.register_handler("inverted", [&](const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
                return !processor.process_predicate(
                    predicate.at("term").as_compound(),
                    context
                );
            });
            processor.register_handler("reference", [&](const enbt::compound_const_ref& predicate, const base_objects::command_context& context) {
                return processor.process_predicate( //TODO
                    context.other_data.at(predicate.at("name")).as_compound(),
                    context
                );
            });


            processor.register_handler("copper_server:__adventure_block_", _server_helper__adventure_block_);
            processor.register_handler("block_state_property", block_state_property);
            processor.register_handler("damage_source_properties", damage_source_properties);
            processor.register_handler("enchantment_active_check", enchantment_active_check);
            processor.register_handler("entity_properties", entity_properties);
            processor.register_handler("entity_scores", entity_scores);
            processor.register_handler("killed_by_player", killed_by_player);
            processor.register_handler("location_check", location_check);
            processor.register_handler("match_tool", match_tool);
            processor.register_handler("random_chance", random_chance);
            processor.register_handler("random_chance_with_enchanted_bonus", random_chance_with_enchanted_bonus);
            processor.register_handler("survives_explosion", survives_explosion);
            processor.register_handler("table_bonus", table_bonus);
            processor.register_handler("time_check", time_check);
            processor.register_handler("value_check", value_check);
            processor.register_handler("weather_check", weather_check);
            api::predicate::register_processor(processor);
        }
    }
}
