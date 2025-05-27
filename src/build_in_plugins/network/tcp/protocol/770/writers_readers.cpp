
#include <src/base_objects/block.hpp>
#include <src/base_objects/recipe.hpp>
#include <src/build_in_plugins/network/tcp/protocol/770/writers_readers.hpp>
#include <src/registers.hpp>

namespace copper_server::build_in_plugins::network::tcp::protocol::play_770::reader {
    namespace slot_component_encoder {
        using namespace base_objects;

        void __firework_explosion_encoder(list_array<uint8_t>& data, const item_firework_explosion& value) {
            WriteVar<int32_t>((int32_t)value.shape, data);
            WriteVar<int32_t>(value.colors.size(), data);
            for (auto& it : value.colors)
                WriteValue(it, data);
            WriteVar<int32_t>(value.fade_colors.size(), data);
            for (auto& it : value.fade_colors)
                WriteValue(it, data);
            data.push_back(value.trail);
            data.push_back(value.twinkle);
        }

        void __block_predicate_encoder(list_array<uint8_t>& data, const slot_component::inner::block_predicate& predicate) {
            if (predicate.contains("block")) {
                //TODO  data.push_back(true); id set
                data.push_back(false); //temporary
            } else
                data.push_back(false);
            if (predicate.contains("properties")) {
                data.push_back(true);
                auto properties = predicate.at("properties").as_array();
                WriteVar<int32_t>(properties.size(), data);
                for (auto& property : properties) {
                    auto name = property.at("name").as_string();
                    WriteString(data, name);
                    if (property.contains("value")) {
                        data.push_back(true);
                        WriteString(data, property.at("value").as_string());
                    } else {
                        data.push_back(false);
                        WriteString(data, property.at("min").as_string());
                        WriteString(data, property.at("max").as_string());
                    }
                }
            } else
                data.push_back(false);
            if (predicate.contains("nbt")) {
                data.push_back(true);
                data.push_back(NBT::build(predicate.at("nbt")).get_as_network());
            } else
                data.push_back(false);
        }

        void __sound_event_encoder(list_array<uint8_t>& data, const slot_component::inner::sound_extended& sound) {
            WriteIdentifier(data, sound.sound_name);
            if (sound.fixed_range) {
                data.push_back(true);
                WriteValue<float>(*sound.fixed_range, data);
            } else
                data.push_back(false);
        }

        void __sound_event_encoder(list_array<uint8_t>& data, const std::variant<std::string, slot_component::inner::sound_extended>& sound) {
            std::visit(
                [&data](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                        WriteVar<int32_t>(1 + registers::view_reg_pro_id("minecraft:sound_event", it, 770), data);
                    } else {
                        WriteVar<int32_t>(0, data);
                        __sound_event_encoder(data, it);
                    }
                },
                sound
            );
        }

        void __effect_encoder(list_array<uint8_t>& data, const item_potion_effect::effect_data& effect) {
            WriteVar<int32_t>(effect.amplifier, data);
            WriteVar<int32_t>(effect.duration, data);
            data.push_back(effect.ambient);
            data.push_back(effect.show_particles);
            data.push_back(effect.show_icon);
            data.push_back((bool)effect.hidden_effect);
            if (effect.hidden_effect)
                __effect_encoder(data, *effect.hidden_effect);
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::apply_effects& effect) {
            WriteVar<int32_t>(0, data);
            WriteVar<int32_t>(effect.effects.size(), data);
            for (auto& it : effect.effects) {
                WriteVar<int32_t>(registers::effects_cache.at(it.potion_id)->second.id, data);
                __effect_encoder(data, it.data);
            }
            WriteValue<float>(effect.probability, data);
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::remove_effects& effect) {
            WriteVar<int32_t>(1, data);
            std::visit(
                [&data](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                        WriteVar<int32_t>(0, data); //custom
                        WriteIdentifier(data, it);
                    } else {
                        WriteVar<int32_t>(1 + it.size(), data); //type 1 + arr size 1
                        for (auto& c : it)
                            WriteVar<int32_t>(registers::effects.at(c).id, data);
                    }
                },
                effect.effects
            );
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::clear_all_effects& effect) {
            WriteVar<int32_t>(2, data);
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::teleport_randomly& effect) {
            WriteVar<int32_t>(3, data);
            WriteValue<float>(effect.diameter, data);
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::play_sound& effect) {
            WriteVar<int32_t>(4, data);
            __sound_event_encoder(data, effect.sound);
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::__custom& effect) {
            //unsupported, fill gap with empty effects
            __consume_effect_encoder(data, slot_component::inner::apply_effects{.probability = 0});
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::application_effect& effect) {
            std::visit([&data](auto& it) { __consume_effect_encoder(data, it); }, effect);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::custom_data& value) {
            WriteVar<int32_t>(0, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::max_stack_size& value) {
            WriteVar<int32_t>(1, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::max_damage& value) {
            WriteVar<int32_t>(2, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::damage& value) {
            WriteVar<int32_t>(3, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::unbreakable& value) {
            WriteVar<int32_t>(4, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::custom_name& value) {
            WriteVar<int32_t>(5, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::item_name& value) {
            WriteVar<int32_t>(6, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::item_model& value) {
            WriteVar<int32_t>(7, data);
            WriteIdentifier(data, value.value);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::lore& value) {
            WriteVar<int32_t>(8, data);
            WriteVar<int32_t>(value.value.size(), data);
            for (auto& lore : value.value)
                data.push_back(NBT::build(lore.ToENBT()).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::rarity& value) {
            WriteVar<int32_t>(9, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::enchantments& value) {
            WriteVar<int32_t>(10, data);
            WriteVar<int32_t>(value.value.size(), data);
            for (auto& enchantment : value.value) {
                WriteVar<int32_t>(enchantment.first, data);
                WriteVar<int32_t>(enchantment.second, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::can_place_on& value) {
            WriteVar<int32_t>(11, data);
            WriteVar<int32_t>(value.value.size(), data);
            for (auto& predicate : value.value)
                __block_predicate_encoder(data, predicate);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::can_break& value) {
            WriteVar<int32_t>(12, data);
            WriteVar<int32_t>(value.value.size(), data);
            for (auto& predicate : value.value)
                __block_predicate_encoder(data, predicate);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::attribute_modifiers& value) {
            WriteVar<int32_t>(13, data);
            WriteVar<int32_t>(value.value.size(), data);
            for (auto& attribute : value.value) {
                try {
                    WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:attribute", attribute.type, 770), data);
                } catch (...) {
                    continue; //ignore unsupported attributes for client and handle by server
                }
                WriteString(data, attribute.id);
                WriteValue(attribute.amount, data);
                WriteVar<int32_t>((int32_t)attribute.operation, data);
                WriteVar<int32_t>((int32_t)attribute.slot, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::custom_model_data& value) {
            WriteVar<int32_t>(14, data);
            WriteVar<int32_t>(value.value, data); //Check if this is correct
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tooltip_display& value) {
            WriteVar<int32_t>(15, data);
            data.push_back(value.hide_tooltip);
            list_array<int32_t> hidden_components;
            hidden_components.reserve(value.hidden_components.size());
            for (auto& component : value.hidden_components) {
                try {
                    hidden_components.push_back(registers::view_reg_pro_id("minecraft:data_component_type", component, 770));
                } catch (...) {
                    continue; //ignore unsupported components for client
                }
            }

            WriteVar<int32_t>(hidden_components.size(), data);
            for (auto& component : hidden_components)
                WriteVar<int32_t>(component, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::repair_cost& value) {
            WriteVar<int32_t>(16, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::creative_slot_lock& value) {
            WriteVar<int32_t>(17, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::enchantment_glint_override& value) {
            WriteVar<int32_t>(18, data);
            WriteVar<int32_t>(value.has_glint, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::intangible_projectile& value) {
            WriteVar<int32_t>(19, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::food& value) {
            WriteVar<int32_t>(20, data);
            WriteVar<int32_t>(value.nutrition, data);
            WriteValue<float>(value.saturation, data);
            data.push_back(value.can_always_eat);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::consumable& value) {
            static std::unordered_map<std::string, int32_t> animations_map{
                {"none", 0},
                {"eat", 1},
                {"drink", 2},
                {"block", 3},
                {"bow", 4},
                {"spear", 5},
                {"crossbow", 6},
                {"spyglass", 7},
                {"toot_horn", 8},
                {"brush", 9}
            };
            WriteVar<int32_t>(21, data);
            WriteValue<float>(value.consume_seconds, data);
            WriteVar<int32_t>(animations_map.at(value.animation), data);
            __sound_event_encoder(data, value.sound);
            data.push_back(value.has_consume_particles);
            WriteVar<int32_t>(value.on_consume_effects.size(), data);
            for (auto& effect : value.on_consume_effects) 
                __consume_effect_encoder(data, effect);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::use_remainder& value) {
            WriteVar<int32_t>(22, data);
            std::visit(
                [&](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, slot_data*>)
                        WriteSlot(data, *it);
                    else
                        WriteSlot(data, slot_data::create_item(it.id, it.count));
                },
                value.proxy_value
            );
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::use_cooldown value) {
            WriteVar<int32_t>(23, data);
            WriteValue<float>(value.seconds, data);
            data.push_back((bool)value.cooldown_group);
            if (value.cooldown_group)
                WriteIdentifier(data, *value.cooldown_group);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::damage_resistant& value) {
            WriteVar<int32_t>(24, data);
            if (value.types.starts_with("#"))
                WriteIdentifier(data, value.types.substr(1));
            else
                WriteIdentifier(data, value.types);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tool& value) {
            WriteVar<int32_t>(25, data);
            WriteVar<int32_t>(value.rules.size(), data);
            for (auto& rule : value.rules) {
                std::visit(
                    [&](auto& rul) {
                        using T = std::decay_t<decltype(rul)>;
                        if constexpr (std::is_same_v<T, std::string>) {
                            WriteVar<int32_t>(0, data);
                            WriteString(data, rul);
                        } else {
                            WriteVar<int32_t>(1 + rul.size(), data);
                            for (auto id : rul)
                                WriteVar<int32_t>(block::get_protocol_block_id(id, 770), data);
                        }
                    },
                    rule.value
                );
                data.push_back((bool)rule.speed);
                if (rule.speed)
                    WriteValue(*rule.speed, data);
                data.push_back((bool)rule.correct_for_drops);
                if (rule.correct_for_drops)
                    data.push_back((bool)rule.correct_for_drops);
            }
            WriteValue(value.default_mining_speed, data);
            WriteVar<int32_t>(value.damage_per_block, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::weapon& value) {
            WriteVar<int32_t>(26, data);
            WriteValue(value.item_damage_per_attack, data);
            WriteValue(value.disable_blocking_for_seconds, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::enchantable& value) {
            WriteVar<int32_t>(27, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::equippable& value) {
            int32_t slot_type = 0;
            if (value.slot == "mainhand")
                slot_type = 0;
            else if (value.slot == "feet")
                slot_type = 1;
            else if (value.slot == "legs")
                slot_type = 2;
            else if (value.slot == "chest")
                slot_type = 3;
            else if (value.slot == "head")
                slot_type = 4;
            else if (value.slot == "offhand")
                slot_type = 5;
            else if (value.slot == "body")
                slot_type = 6;
            else
                throw std::runtime_error("Unknown slot name");
            WriteVar<int32_t>(28, data);
            WriteVar<int32_t>(slot_type, data);
            __sound_event_encoder(data, value.equip_sound);
            data.push_back((bool)value.model);
            if (value.model)
                WriteIdentifier(data, *value.model);
            data.push_back((bool)value.camera_overlay);
            if (value.camera_overlay)
                WriteIdentifier(data, *value.camera_overlay);

            if (!std::holds_alternative<std::nullptr_t>(value.allowed_entities)) {
                data.push_back(true);
                std::visit(
                    [&](auto& allowed_entities) {
                        using T = std::decay_t<decltype(allowed_entities)>;
                        if constexpr (std::is_same_v<T, std::string>) {
                            WriteIdentifier(data, allowed_entities);
                        } else if constexpr (std::is_same_v<T, std::nullptr_t>) {
                        } else {
                            WriteVar<int32_t>(allowed_entities.size(), data);
                            for (auto& id : allowed_entities)
                                WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:entity_type", id, 770), data);
                        }
                    },
                    value.allowed_entities
                );
            } else
                data.push_back(false);
            data.push_back(value.dispensable);
            data.push_back(value.swappable);
            data.push_back(value.damage_on_hurt);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::repairable& value) {
            WriteVar<int32_t>(29, data);
            std::visit(
                [&](auto& items) {
                    using T = std::decay_t<decltype(items)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        data.push_back(0);
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:item", items, 770), data);
                    } else {
                        WriteVar<int32_t>(1 + items.size(), data);
                        for (auto& id : items)
                            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:item", id, 770), data);
                    }
                },
                value.items
            );
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::glider& value) {
            WriteVar<int32_t>(30, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tooltip_style value) {
            WriteVar<int32_t>(31, data);
            WriteIdentifier(data, value.value);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::death_protection& value) {
            WriteVar<int32_t>(32, data);
            WriteVar<int32_t>(value.death_effects.size(), data);
            for (auto& effect : value.death_effects)
                __consume_effect_encoder(data, effect);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::blocks_attacks& value) {
            WriteVar<int32_t>(33, data);
            WriteValue<float>(value.block_delay_seconds, data);
            WriteValue<float>(value.disable_cooldown_scale, data);
            WriteVar<int32_t>(value.damage_reductions.size(), data);
            for (auto& reduction : value.damage_reductions) {
                WriteValue<float>(reduction.horizontal_blocking_angle, data);
                data.push_back((bool)reduction.type.size());
                if (reduction.type.size()) {
                    if (reduction.type.size() == 1) {
                        WriteVar<int32_t>(0, data);
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:damage_kind", registers::damageTypes_cache.at(reduction.type[0])->first, 770), data);
                    } else {
                        WriteVar<int32_t>(1 + reduction.type.size(), data);
                        for (auto& id : reduction.type)
                            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:damage_kind", registers::damageTypes_cache.at(id)->first, 770), data);
                    }
                }
                WriteValue<float>(reduction.base, data);
                WriteValue<float>(reduction.factor, data);
            }
            WriteValue<float>(value.item_damage.threshold, data);
            WriteValue<float>(value.item_damage.base, data);
            WriteValue<float>(value.item_damage.factor, data);
            data.push_back((bool)value.bypassed_by);
            if (value.bypassed_by)
                WriteIdentifier(data, *value.bypassed_by);
            data.push_back((bool)value.block_sound);
            __sound_event_encoder(data, *value.block_sound);
            data.push_back((bool)value.disabled_sound);
            __sound_event_encoder(data, *value.disabled_sound);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::stored_enchantments& value) {
            WriteVar<int32_t>(34, data);
            WriteVar<int32_t>(value.enchants.size(), data);
            for (auto& enchantment : value.enchants) {
                WriteVar<int32_t>(enchantment.first, data);
                WriteVar<int32_t>(enchantment.second, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::dyed_color& value) {
            WriteVar<int32_t>(35, data);
            WriteVar<int32_t>(value.rgb, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_color& value) {
            WriteVar<int32_t>(36, data);
            WriteVar<int32_t>(value.rgb, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_id& value) {
            WriteVar<int32_t>(37, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_decorations& value) {
            WriteVar<int32_t>(38, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_post_processing& value) {
            WriteVar<int32_t>(39, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::charged_projectiles& value) {
            WriteVar<int32_t>(40, data);
            WriteVar<int32_t>(value.data.size(), data);
            for (auto& projectile : value.data)
                WriteSlot(data, *projectile, 770);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bundle_contents& value) {
            WriteVar<int32_t>(41, data);
            WriteVar<int32_t>(value.items.size(), data);
            for (auto& item : value.items)
                WriteSlot(data, *item, 770);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::potion_contents& value) {
            WriteVar<int32_t>(42, data);
            auto id = value.get_potion_id();
            data.push_back((bool)id);
            if (id)
                WriteVar<int32_t>(*id, data);
            auto custom_col = value.get_custom_color();
            data.push_back((bool)custom_col);
            if (custom_col)
                WriteVar<int32_t>(*custom_col, data);
            value.iterate_custom_effects(
                [&data](size_t size) { WriteVar<int32_t>(size, data); },
                [&](auto& effect) {
                    WriteVar<int32_t>(registers::effects_cache.at(effect.potion_id)->second.id, data);
                    __effect_encoder(data, effect.data);
                }
            );
            auto name = value.get_custom_name();
            if (name)
                WriteString(data, *name);
            else
                WriteString(data, "");
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::potion_duration_scale& value) {
            WriteVar<int32_t>(43, data);
            WriteValue<float>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::suspicious_stew_effects& value) {
            WriteVar<int32_t>(44, data);
            WriteVar<int32_t>(value.effects.size(), data);
            for (auto& effect : value.effects) {
                WriteVar<int32_t>(effect.first, data);
                WriteVar<int32_t>(effect.second, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::writable_book_content& value) {
            WriteVar<int32_t>(45, data);
            WriteVar<int32_t>(value.pages.size(), data);
            for (auto& page : value.pages) {
                WriteString(data, page.text, 1024);
                data.push_back((bool)page.filtered);
                if (page.filtered)
                    WriteString(data, *page.filtered, 1024);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::written_book_content& value) {
            WriteVar<int32_t>(46, data);
            WriteString(data, value.raw_title, 32);
            data.push_back((bool)value.filtered_title);
            if (value.filtered_title)
                WriteString(data, *value.filtered_title, 32);
            WriteString(data, value.author);
            WriteVar<int32_t>(value.generation, data);
            WriteVar<int32_t>(value.pages.size(), data);
            for (auto& page : value.pages) {
                data.push_back(NBT::build(page.text.ToENBT()).get_as_network());
                data.push_back((bool)page.filtered);
                if (page.filtered)
                    data.push_back(NBT::build(page.filtered->ToENBT()).get_as_network());
            }
            data.push_back((bool)value.resolved);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::trim& value) {
            WriteVar<int32_t>(47, data);
            std::visit(
                [&data](auto& it) {
                    using T = std::decay_t<decltype(it)>;
                    if constexpr (std ::is_same_v<uint32_t, T>) {
                        WriteVar<int32_t>(it + 1, data);
                    } else {
                        WriteString(data, it.asset_name);
                        WriteVar<int32_t>(it.ingredient, data);
                        WriteValue<float>(it.item_model_index, data);
                        WriteVar<int32_t>(it.overrides.size(), data);
                        for (auto& override_ : it.overrides) {
                            WriteVar<int32_t>(override_.first, data);
                            WriteString(data, override_.second);
                        }
                        data.push_back(NBT::build(it.description.ToENBT()).get_as_network());
                    }
                },
                value.material
            );
            std::visit(
                [&data](auto& it) {
                    using T = std::decay_t<decltype(it)>;
                    if constexpr (std ::is_same_v<uint32_t, T>) {
                        WriteVar<int32_t>(it + 1, data);
                    } else {
                        WriteString(data, it.asset_name);
                        WriteVar<int32_t>(it.template_item, data);
                        data.push_back(NBT::build(it.description.ToENBT()).get_as_network());
                        data.push_back(it.decal);
                    }
                },
                value.pattern
            );
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::debug_stick_state& value) {
            WriteVar<int32_t>(48, data);
            data.push_back(NBT::build((const enbt::value&)value.previous_state).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::entity_data& value) {
            WriteVar<int32_t>(49, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bucket_entity_data& value) {
            WriteVar<int32_t>(50, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::block_entity_data& value) {
            WriteVar<int32_t>(51, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::instrument& value) {
            WriteVar<int32_t>(52, data);
            std::visit(
                [&data](auto& it) {
                    using T = std::decay_t<decltype(it)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        WriteVar<int32_t>(0, data);
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:instrument", it, 770), data);
                    } else {
                        WriteVar<int32_t>(1, data);
                        __sound_event_encoder(data, it.sound);
                        WriteValue<float>(it.duration, data);
                        WriteValue<float>(it.range, data);
                        data.push_back(NBT::build(it.description.ToENBT()).get_as_network());
                    }
                },
                value.type
            );
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::provides_trim_material& value) {
            WriteVar<int32_t>(53, data);
            //TODO
            if (value.patterns.starts_with("#")) {
                //mode 1
            } else {
                //mode 0
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::ominous_bottle_amplifier& value) {
            WriteVar<int32_t>(54, data);
            WriteVar<int32_t>(value.amplifier, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::jukebox_playable& value) {
            WriteVar<int32_t>(55, data);
            std::visit(
                [&data](auto& it) {
                    using T = std::decay_t<decltype(it)>;
                    if constexpr (std::is_same_v<T, std::string>) {
                        data.push_back(false);
                        WriteString(data, it);
                    } else if constexpr (std::is_same_v<T, slot_component::jukebox_playable::jukebox_compact>) {
                        data.push_back(true);
                        WriteVar<int32_t>(1 + it.song_type, data);
                    } else if constexpr (std::is_same_v<T, slot_component::jukebox_playable::jukebox_extended>) {
                        std::visit([&data](auto& it) {
                            if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                                int32_t id = 0;
                                try {
                                    id = registers::view_reg_pro_id("minecraft:sound_event", it, 770);
                                } catch (...) {
                                    id = registers::view_reg_pro_id("minecraft:sound_event", "minecraft:entity.generic.eat", 770);
                                }
                                data.push_back(true);
                                WriteVar<int32_t>(1 + id, data);
                            } else {
                                data.push_back(true);
                                WriteVar<int32_t>(0, data); //custom
                                WriteIdentifier(data, it.sound_name);
                                data.push_back((bool)it.fixed_range);
                                if (it.fixed_range)
                                    WriteValue<float>(*it.fixed_range, data);
                            }
                        },
                                   it.sound_event);
                        data.push_back(it.description.ToTextComponent());
                        WriteValue<float>(it.length_in_seconds, data);
                        WriteVar<int32_t>(it.comparator_output, data);
                    }
                },
                value.song
            );
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::provides_banner_patterns& value) {
            WriteVar<int32_t>(56, data);
            WriteIdentifier(data, value.patterns);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::recipes& value) {
            WriteVar<int32_t>(57, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::lodestone_tracker& value) {
            WriteVar<int32_t>(58, data);
            data.push_back((bool)value.global_pos);
            if (value.global_pos) {
                WriteIdentifier(data, value.global_pos->dimension);
                WriteValue(value.global_pos->position.raw, data);
            }
            data.push_back(value.tracked);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::firework_explosion& value) {
            WriteVar<int32_t>(59, data);
            __firework_explosion_encoder(data, value.value);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::fireworks& value) {
            WriteVar<int32_t>(60, data);
            WriteVar<int32_t>(value.duration, data);
            WriteVar<int32_t>(value.explosions.size(), data);
            for (auto& it : value.explosions)
                __firework_explosion_encoder(data, it);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::profile& value) {
            WriteVar<int32_t>(61, data);
            data.push_back((bool)value.name);
            if (value.name)
                WriteString(data, *value.name, 16);
            data.push_back((bool)value.uid);
            if (value.uid)
                WriteUUID(*value.uid, data);
            WriteVar<int32_t>(value.properties.size(), data);
            for (auto& it : value.properties) {
                WriteString(data, it.name, 64);
                WriteString(data, it.value);
                data.push_back((bool)it.signature);
                if (it.signature)
                    WriteString(data, *it.signature, 1024);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::note_block_sound& value) {
            WriteVar<int32_t>(62, data);
            WriteIdentifier(data, value.sound);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::banner_patterns& value) {
            WriteVar<int32_t>(63, data);
            //TODO
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::base_color& value) {
            WriteVar<int32_t>(64, data);
            WriteVar<int32_t>((int32_t)value.color, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::pot_decorations& value) {
            WriteVar<int32_t>(65, data);
            WriteVar<int32_t>(4, data);
            WriteVar<int32_t>(block::get_protocol_block_id(value.decorations[0], 770), data);
            WriteVar<int32_t>(block::get_protocol_block_id(value.decorations[1], 770), data);
            WriteVar<int32_t>(block::get_protocol_block_id(value.decorations[2], 770), data);
            WriteVar<int32_t>(block::get_protocol_block_id(value.decorations[3], 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::container& value) {
            WriteVar<int32_t>(66, data);
            WriteVar<int32_t>(255, data);
            value.for_each_all([&data](auto& slot, size_t i) {
                if (slot)
                    WriteSlotItem(data, *slot, 770);
                else
                    data.push_back(0);
            });
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::block_state& value) {
            WriteVar<int32_t>(67, data);
            WriteVar<int32_t>(value.properties.size(), data);
            value.properties.for_each([&data](auto& property) {
                WriteString(data, property.name);
                WriteString(data, property.value);
            });
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bees& value) {
            WriteVar<int32_t>(68, data);
            WriteVar<int32_t>(value.values.size(), data);
            for (auto& it : value.values) {
                data.push_back(NBT::build(it.entity_data).get_as_network());
                WriteVar<int32_t>(it.ticks_in_hive, data);
                WriteVar<int32_t>(it.min_ticks_in_hive, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::lock& value) {
            WriteVar<int32_t>(69, data);
            data.push_back(NBT::build(enbt::value(value.key)).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::container_loot& value) {
            WriteVar<int32_t>(70, data);
            enbt::compound dat;
            dat["loot_table"] = value.loot_table;
            dat["seed"] = value.seed;
            data.push_back(NBT::build((enbt::value&)dat).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::break_sound& value) {
            WriteVar<int32_t>(71, data);
            __sound_event_encoder(data, value.value);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::villager_variant& value) {
            WriteVar<int32_t>(72, data);
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:villager_type", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::wolf_variant& value) {
            WriteVar<int32_t>(73, data);
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:wolf_variant", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::wolf_sound_variant& value) {
            WriteVar<int32_t>(74, data);
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:wolf_sound_variant", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::wolf_collar& value) {
            WriteVar<int32_t>(75, data);
            WriteVar<int32_t>(value.value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::fox_variant& value) {
            WriteVar<int32_t>(76, data);
            if (value.value == "snow")
                WriteVar<int32_t>(1, data);
            else
                WriteVar<int32_t>(0, data); //red and other
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::salmon_size& value) {
            WriteVar<int32_t>(77, data);
            if (value.value == "small")
                WriteVar<int32_t>(0, data);
            else if (value.value == "large")
                WriteVar<int32_t>(2, data);
            else
                WriteVar<int32_t>(1, data); //medium
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::parrot_variant& value) {
            WriteVar<int32_t>(78, data);
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:parrot_type", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tropical_fish_pattern& value) {
            WriteVar<int32_t>(79, data);
            static std::unordered_map<std::string, int32_t> patterns = {
                {"kob", 0},
                {"sunstreak", 1},
                {"snooper", 2},
                {"dasher", 3},
                {"brinely", 4},
                {"spotty", 5},
                {"flopper", 6},
                {"stripey", 7},
                {"glitter", 8},
                {"blockfish", 9},
                {"betty", 10},
                {"clayfish", 11},
            };
            WriteVar<int32_t>(patterns.at(value.pattern), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tropical_fish_base_color& value) {
            WriteVar<int32_t>(80, data);
            WriteVar<int32_t>(value.color.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tropical_fish_pattern_color& value) {
            WriteVar<int32_t>(81, data);
            WriteVar<int32_t>(value.color.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::mooshroom_variant& value) {
            WriteVar<int32_t>(82, data);
            WriteVar<int32_t>(value.value == "red" ? 0 : 1, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::rabbit_variant& value) {
            WriteVar<int32_t>(83, data);
            static std::unordered_map<std::string, int32_t> patterns = {
                {"brown", 0},
                {"white", 1},
                {"black", 2},
                {"white splotched", 3},
                {"white_splotched", 3},
                {"gold", 4},
                {"salt", 5},
                {"evil", 6},
            };
            WriteVar<int32_t>(patterns.at(value.value), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::pig_variant value) {
            WriteVar<int32_t>(84, data);
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:pig_variant", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::cow_variant value) {
            WriteVar<int32_t>(85, data);
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:cow_variant", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::chicken_variant value) {
            WriteVar<int32_t>(86, data);
            data.push_back(1); //mode 1
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:chicken_variant", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::frog_variant& value) {
            WriteVar<int32_t>(87, data);
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:frog_variant", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::horse_variant& value) {
            WriteVar<int32_t>(88, data);
            static std::unordered_map<std::string, int32_t> patterns = {
                {"white", 0},
                {"creamy", 1},
                {"chestnut", 2},
                {"brown", 3},
                {"black", 4},
                {"gray", 5},
                {"dark brown", 6},
                {"dark_brown", 6},
            };
            WriteVar<int32_t>(patterns.at(value.value), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::painting_variant& value) {
            WriteVar<int32_t>(89, data);
            //TODO
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::llama_variant& value) {
            WriteVar<int32_t>(90, data);
            static std::unordered_map<std::string, int32_t> patterns = {
                {"creamy", 0},
                {"white", 1},
                {"brown", 2},
                {"gray", 3},
            };
            WriteVar<int32_t>(patterns.at(value.value), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::axolotl_variant& value) {
            WriteVar<int32_t>(91, data);
            static std::unordered_map<std::string, int32_t> patterns = {
                {"lucy", 0},
                {"wild", 1},
                {"gold", 2},
                {"cyan", 3},
                {"blue", 4},
            };
            WriteVar<int32_t>(patterns.at(value.value), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::cat_variant value) {
            WriteVar<int32_t>(92, data);
            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:cat_variant", value.value, 770), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::cat_collar& value) {
            WriteVar<int32_t>(93, data);
            WriteVar<int32_t>(value.value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::sheep_color& value) {
            WriteVar<int32_t>(94, data);
            WriteVar<int32_t>(value.value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::shulker_color& value) {
            WriteVar<int32_t>(95, data);
            WriteVar<int32_t>(value.value.value, data);
        }
    }

    namespace slot_component_decoder {
        using namespace base_objects;

        list_array<std::string> variants(int32_t id) {
            return {
                registers::view_reg_pro_name("minecraft:data_component_type", id, 770)
            };
        }

        slot_component::unified decode(ArrayStream& data) {
            switch (data.read_var<int32_t>()) {
            case 0: {
                slot_component::custom_data res;
                res.value = ReadNBT(data).get_as_enbt();
                return res;
            }
            case 1:
                return slot_component::max_stack_size{
                    .value = (uint8_t)data.read_var<int32_t>()
                };
            case 2:
                return slot_component::max_damage{
                    .value = data.read_var<int32_t>()
                };
            case 3:
                return slot_component::damage{
                    .value = data.read_var<int32_t>()
                };
            case 4:
                return slot_component::unbreakable{};
            case 5:
                return slot_component::custom_name{.value = Chat::fromEnbt(ReadNBT(data).get_as_enbt())};
            case 6:
                return slot_component::item_name{.value = Chat::fromEnbt(ReadNBT(data).get_as_enbt())};
            case 7:
                return slot_component::item_model{.value = ReadNBT(data).get_as_enbt().as_string()};
            case 8: {
                slot_component::lore res;
                int32_t count = data.read_var<int32_t>();
                res.value.reserve(count);
                for (int32_t i = 0; i < count; i++)
                    res.value.push_back(Chat::fromEnbt(ReadNBT(data).get_as_enbt()));
                return res;
            }
            case 9:
                return slot_component::rarity{
                    (slot_component::rarity)data.read_var<int32_t>()
                };
            case 10: {
                slot_component::enchantments res;
                int32_t count = data.read_var<int32_t>();
                res.value.reserve(count);
                for (int32_t i = 0; i < count; i++) {
                    int32_t id = data.read_var<int32_t>();
                    int32_t lvl = data.read_var<int32_t>();
                    res.value.push_back(
                        {registers::enchantments.at(registers::view_reg_pro_name("minecraft:enchantment", id, 770)).id, lvl}
                    );
                }
                return res;
            }
            case 11: {
                slot_component::can_place_on res;
                int32_t count = data.read_var<int32_t>();
                res.value.reserve(count);
                for (int32_t i = 0; i < count; i++)
                    ;
                //res.predicates.push_back(ReadNBT(data).get_as_enbt()); TODO
                return res;
            }
            case 12: {
                slot_component::can_break res;
                int32_t count = data.read_var<int32_t>();
                res.value.reserve(count);
                for (int32_t i = 0; i < count; i++)
                    ;
                //res.predicates.push_back(ReadNBT(data).get_as_enbt()); TODO
                return res;
            }
            case 13: {
                slot_component::attribute_modifiers res;
                int32_t count = data.read_var<int32_t>();
                res.value.reserve(count);
                for (int32_t i = 0; i < count; i++) {
                    item_attribute attribute;
                    attribute.type = registers::view_reg_pro_name("minecraft:attribute", data.read_var<int32_t>());
                    data.read_uuid(); //ignore UUID
                    attribute.id = data.read_string();
                    attribute.amount = data.read_value<double>();
                    attribute.operation = (item_attribute::operation_e)data.read_var<int32_t>();
                    attribute.slot = (item_attribute::slot_filter)data.read_var<int32_t>();
                    res.value.push_back(attribute);
                }
                return res;
            }
            case 14:
                return slot_component::custom_model_data{
                    .value = data.read_var<int32_t>()
                };
            case 15:
                return slot_component::tooltip_display{};
            case 16: {
                slot_component::repair_cost res;
                res.value = data.read_var<int32_t>();
                return res;
            }
            case 17:
                return slot_component::creative_slot_lock{};
            case 18:
                return slot_component::enchantment_glint_override{
                    .has_glint = data.read_var<int32_t>()
                };
            case 19:
                return slot_component::intangible_projectile{};
            case 20: {
                slot_component::food res;
                res.nutrition = data.read_var<int32_t>();
                res.saturation = data.read_value<float>();
                res.can_always_eat = data.read_value<bool>();
                return res;
            }
            case 21: {
                slot_component::consumable res;
                //TODO
                return res;
            }
            case 22:
                return slot_component::use_remainder(*ReadSlotItem(data, 770));
            case 23: {
                slot_component::use_cooldown res;
                res.seconds = data.read_value<float>();
                if (data.read())
                    res.cooldown_group = data.read_identifier();
                return res;
            }
            case 24:
                return slot_component::damage_resistant(data.read_identifier());
            case 25: {
                slot_component::food res;
                //TODO
                return res;
            }
            default:
                throw std::runtime_error("Undefined component id");
            }
        }
    }

    void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot_data& slot, int16_t protocol) {
        auto sd = base_objects::slot_data::get_slot_data(slot.id);
        list_array<std::string> removed_components;
        for (auto& [name, ignored] : sd.default_components)
            if (!slot.has_component(name))
                removed_components.push_back(name);

        auto real_id = sd.internal_item_aliases.at(protocol).local_id;

        WriteVar<int32_t>(slot.count, data);
        WriteVar<int32_t>(real_id, data);
        WriteVar<int32_t>(slot.components.size(), data);
        WriteVar<int32_t>(removed_components.size(), data);
        for (auto& [ignored, it] : slot.components) {
            std::visit(
                [&data, &slot](auto& it) {
                    slot_component_encoder::encode(slot, data, it);
                },
                it
            );
        }
        for (auto& it : removed_components)
            WriteString(data, it);
    }

    void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol) {
        WriteSlotItem(data, *slot, protocol);
    }

    void WriteTradeItem(list_array<uint8_t>& data, const base_objects::slot_data& slot, int16_t protocol) {
        auto sd = base_objects::slot_data::get_slot_data(slot.id);
        list_array<std::string> removed_components;
        for (auto& [name, ignored] : sd.default_components)
            if (!slot.has_component(name))
                removed_components.push_back(name);

        auto real_id = sd.internal_item_aliases.at(protocol).local_id;

        WriteVar<int32_t>(real_id, data);
        WriteVar<int32_t>(slot.count, data);
        WriteVar<int32_t>(slot.components.size(), data);
        WriteVar<int32_t>(removed_components.size(), data);
        for (auto& [ignored, it] : slot.components) {
            std::visit(
                [&data, &slot](auto& it) {
                    slot_component_encoder::encode(slot, data, it);
                },
                it
            );
        }
        for (auto& it : removed_components)
            WriteString(data, it);
    }

    void WriteTradeItem(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol) {

        WriteTradeItem(data, *slot, protocol);
    }

    void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot, int16_t protocol) {
        data.push_back((bool)slot);
        if (slot)
            WriteSlotItem(data, slot, protocol);
    }

    void WriteIngredient(list_array<uint8_t>& data, const base_objects::slot_display& item) {
        std::visit(
            [&](auto&& arg) {
                using T = std::decay_t<decltype(arg)>;
                WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", T::name, 770), data);
                if constexpr (
                    std::is_same_v<T, base_objects::slot_displays::minecraft::empty>
                    || std::is_same_v<T, base_objects::slot_displays::minecraft::any_fuel>
                ) {
                } else if constexpr (std::is_same_v<T, base_objects::slot_displays::minecraft::item>) {
                    WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:item", arg.item, 770), data);
                } else if constexpr (std::is_same_v<T, base_objects::slot_displays::minecraft::item_stack>) {
                    WriteSlotItem(data, arg.item, 770);
                } else if constexpr (std::is_same_v<T, base_objects::slot_displays::minecraft::tag>) {
                    WriteIdentifier(data, arg.tag);
                } else if constexpr (std::is_same_v<T, base_objects::slot_displays::minecraft::smithing_trim>) {
                    WriteIngredient(data, *arg.base);
                    WriteIngredient(data, *arg.material);
                    WriteIngredient(data, *arg.pattern);
                } else if constexpr (std::is_same_v<T, base_objects::slot_displays::minecraft::with_remainder>) {
                    WriteIngredient(data, *arg.ingredient);
                    WriteIngredient(data, *arg.remainder);
                } else if constexpr (std::is_same_v<T, base_objects::slot_displays::minecraft::composite>) {
                    WriteVar<int32_t>(arg.options.size(), data);
                    for (auto& it : arg.options)
                        WriteIngredient(data, *it);
                }
            },
            item.value
        );
    }

    bool WriteRecipeDisplay(list_array<uint8_t>& data, const base_objects::recipe& display) {
        return std::visit(
            [&data](auto& it) {
                using T = std::decay_t<decltype(it)>;
                using T_info = base_objects::recipes::variant_data<T>;
                if constexpr (T_info::must_display) {
                    if constexpr (std::is_same_v<T, base_objects::recipes::minecraft::crafting_shapeless>) {
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", T_info::name, 770), data);
                        WriteVar<int32_t>(it.ingredients.size(), data);
                        for (auto& it : it.ingredients)
                            WriteIngredient(data, it);
                        WriteIngredient(data, it.result);
                        WriteIngredient(data, it.crafting_station);
                    } else if constexpr (std::is_same_v<T, base_objects::recipes::minecraft::crafting_shaped>) {
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", T_info::name, 770), data);
                        WriteVar<int32_t>(it.width, data);
                        WriteVar<int32_t>(it.height, data);
                        WriteVar<int32_t>(it.ingredients.size(), data);
                        for (auto& it : it.ingredients)
                            WriteIngredient(data, it);
                        WriteIngredient(data, it.result);
                        WriteIngredient(data, it.crafting_station);
                    } else if constexpr (
                        std::is_same_v<T, base_objects::recipes::minecraft::smelting>
                        || std::is_same_v<T, base_objects::recipes::minecraft::blasting>
                        || std::is_same_v<T, base_objects::recipes::minecraft::smoking>
                        || std::is_same_v<T, base_objects::recipes::minecraft::campfire_cooking>
                    ) {
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", "minecraft:furnace", 770), data);
                        WriteIngredient(data, it.ingredient);
                        WriteIngredient(data, it.fuel);
                        WriteIngredient(data, it.result);
                        WriteIngredient(data, it.crafting_station);
                        WriteVar<int32_t>(it.cooking_time, data);
                        WriteValue<float>(it.experience, data);
                    } else if constexpr (std::is_same_v<T, base_objects::recipes::minecraft::stonecutting>) {
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", "minecraft:stonecutter", 770), data);
                        WriteIngredient(data, it.ingredient);
                        WriteIngredient(data, it.result);
                        WriteIngredient(data, it.crafting_station);
                    } else if constexpr (std::is_same_v<T, base_objects::recipes::minecraft::smithing_transform>) {
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", "minecraft:smithing", 770), data);
                        WriteIngredient(data, it._template);
                        WriteIngredient(data, it.base);
                        WriteIngredient(data, it.addition);
                        WriteIngredient(data, it.result);
                        WriteIngredient(data, it.crafting_station);
                    } else
                        return false;
                    return true;
                } else
                    return false;
            },
            display.data
        );
    }

    base_objects::slot ReadSlotItem(ArrayStream& data, int16_t protocol) {
        auto res = ReadSlot(data, protocol);
        if (!res)
            throw std::runtime_error("Protocol error, excepted fully declared item");
        return res;
    }

    base_objects::slot ReadSlot(ArrayStream& data, int16_t protocol) {
        int32_t item_count = data.read_var<int32_t>();
        if (!item_count)
            return std::nullopt;
        int32_t item_id = data.read_var<int32_t>();
        base_objects::slot_data item = base_objects::slot_data::create_item(item_id);
        int32_t num_to_add = data.read_var<int32_t>();
        int32_t num_to_remove = data.read_var<int32_t>();
        for (int32_t i = 0; i < num_to_add; i++)
            item.add_component(slot_component_decoder::decode(data));
        for (int32_t i = 0; i < num_to_remove; i++)
            slot_component_decoder::variants(data.read_var<int32_t>()).for_each([&](auto& it) {
                item.remove_component(it);
            });
        return item;
    }
}