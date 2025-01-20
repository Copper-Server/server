
#include <src/base_objects/block.hpp>
#include <src/protocolHelper/packets/767/writers_readers.hpp>
#include <src/registers.hpp>

namespace copper_server::packets::release_767::reader {
    namespace slot_component_encoder {
        using namespace base_objects;

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
            std::visit(
                [&data](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                        WriteVar<int32_t>(4, data);
                        WriteIdentifier(data, it);
                        data.push_back(false);
                    } else {
                        WriteVar<int32_t>(4, data);
                        WriteIdentifier(data, it.sound_name);
                        data.push_back((bool)it.fixed_range);
                        if (it.fixed_range)
                            WriteValue<float>(*it.fixed_range, data);
                    }
                },
                effect.sound
            );
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::__custom& effect) {
            //unsupported, fill gap with empty effects
            __consume_effect_encoder(data, slot_component::inner::apply_effects{.probability = 0});
        }

        void __consume_effect_encoder(list_array<uint8_t>& data, const slot_component::inner::application_effect& effect) {
            std::visit([&data](auto& it) { __consume_effect_encoder(data, it); }, effect);
        }

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
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::custom_name& value) {
            WriteVar<int32_t>(5, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::item_name& value) {
            WriteVar<int32_t>(6, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::lore& value) {
            WriteVar<int32_t>(7, data);
            WriteVar<int32_t>(value.value.size(), data);
            for (auto& lore : value.value)
                data.push_back(NBT::build(lore.ToENBT()).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::rarity& value) {
            WriteVar<int32_t>(8, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::enchantments& value) {
            WriteVar<int32_t>(9, data);
            WriteVar<int32_t>(value.enchants.size(), data);
            for (auto& enchantment : value.enchants) {
                WriteVar<int32_t>(enchantment.first, data);
                WriteVar<int32_t>(enchantment.second, data);
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::can_place_on& value) {
            WriteVar<int32_t>(10, data);
            WriteVar<int32_t>(value.predicates.size(), data);
            for (auto& predicate : value.predicates) {
                //TODO create block predicate serializer
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::can_break& value) {
            WriteVar<int32_t>(11, data);
            WriteVar<int32_t>(value.predicates.size(), data);
            for (auto& predicate : value.predicates) {
                //TODO create block predicate serializer
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::attribute_modifiers& value) {
            WriteVar<int32_t>(12, data);
            WriteVar<int32_t>(value.attributes.size(), data);
            for (auto& attribute : value.attributes) {
                WriteVar<int32_t>((int32_t)registers::individual_registers[767].at("minecraft:attribute").at("entries").at(attribute.type), data);
                WriteUUID(enbt::raw_uuid::from_string(attribute.id), data);
                WriteString(data, attribute.id);
                WriteValue(attribute.amount, data);
                WriteVar<int32_t>((int32_t)attribute.operation, data);
                WriteVar<int32_t>((int32_t)attribute.slot, data);
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::custom_model_data& value) {
            WriteVar<int32_t>(13, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::hide_additional_tooltip& value) {
            WriteVar<int32_t>(14, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::hide_tooltip& value) {
            WriteVar<int32_t>(15, data);
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
            WriteValue(value.saturation, data);
            data.push_back(value.can_always_eat);
            float seconds_to_eat = 3;
            list_array<std::pair<float, item_potion_effect>> effects;
            if (slot.has_component<slot_component::consumable>()) {
                auto& consumable = slot.get_component<slot_component::consumable>();
                seconds_to_eat = consumable.consume_seconds;
                consumable.on_consume_effects.for_each(
                    [&](auto& vars) {
                        if (std::holds_alternative<slot_component::inner::apply_effects>(vars)) {
                            auto& it = std::get<slot_component::inner::apply_effects>(vars);
                            it.effects.for_each([&it, &effects](auto& effect) {
                                effects.push_back({it.probability, effect});
                            });
                        }
                    }
                );
            }
            WriteValue(seconds_to_eat, data);

            if (slot.has_component<slot_component::use_remainder>()) {
                auto& use_remainder = slot.get_component<slot_component::use_remainder>();
                std::visit(
                    [&](auto& value) {
                        using T = std::decay_t<decltype(value)>;
                        if constexpr (std::is_same_v<T, weak_slot_data>)
                            WriteSlotItem(data, slot_data::create_item(value.id, value.count), 767);
                        else
                            WriteSlotItem(data, *value, 767);
                    },
                    use_remainder.proxy_value
                );
            } else
                WriteSlotItem(data, slot_data::create_item("minecraft:air"), 767);

            WriteVar<int32_t>(effects.size(), data);
            for (auto& it : effects) {
                WriteVar<int32_t>(it.second.potion_id, data);
                __effect_encoder(data, it.second.data);
                WriteValue(it.first, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::damage_resistant& value) {
            if (value.types == "minecraft:fire")
                WriteVar<int32_t>(21, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tool& value) {
            WriteVar<int32_t>(22, data);
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
                                WriteVar<int32_t>(block::get_block_id(id, 767), data);
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

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::stored_enchantments& value) {
            WriteVar<int32_t>(23, data);
            WriteVar<int32_t>(value.enchants.size(), data);
            for (auto& enchantment : value.enchants) {
                WriteVar<int32_t>(enchantment.first, data);
                WriteVar<int32_t>(enchantment.second, data);
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::dyed_color& value) {
            WriteVar<int32_t>(24, data);
            WriteVar<int32_t>(value.rgb, data);
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_color& value) {
            WriteVar<int32_t>(25, data);
            WriteVar<int32_t>(value.rgb, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_id& value) {
            WriteVar<int32_t>(26, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_decorations& value) {
            WriteVar<int32_t>(27, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_post_processing& value) {
            WriteVar<int32_t>(28, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::charged_projectiles& value) {
            WriteVar<int32_t>(29, data);
            WriteVar<int32_t>(value.data.size(), data);
            for (auto& projectile : value.data)
                WriteSlot(data, *projectile, 767);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bundle_contents& value) {
            WriteVar<int32_t>(30, data);
            WriteVar<int32_t>(value.items.size(), data);
            for (auto& item : value.items)
                WriteSlot(data, *item, 767);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::potion_contents& value) {
            WriteVar<int32_t>(31, data);
            auto pot_id = value.get_potion_id();
            auto col = value.get_custom_color();

            data.push_back((bool)pot_id);
            if (pot_id)
                WriteVar<int32_t>(*pot_id, data);
            data.push_back((bool)col);
            if (col)
                WriteVar<int32_t>(*col, data);
            if (std::holds_alternative<int32_t>(value.value))
                data.push_back(0);
            else {
                auto& arr = std::get<slot_component::potion_contents::full>(value.value).custom_effects;
                WriteVar<int32_t>(arr.size(), data);
                for (auto& effect : arr) {
                    WriteVar<int32_t>(effect.potion_id, data);
                    __effect_encoder(data, effect.data);
                }
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::suspicious_stew_effects& value) {
            WriteVar<int32_t>(32, data);
            WriteVar<int32_t>(value.effects.size(), data);
            for (auto& effect : value.effects) {
                WriteVar<int32_t>(effect.first, data);
                WriteVar<int32_t>(effect.second, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::writable_book_content& value) {
            WriteVar<int32_t>(33, data);
            WriteVar<int32_t>(value.pages.size(), data);
            for (auto& page : value.pages) {
                WriteString(data, page.text, 1024);
                data.push_back((bool)page.filtered);
                if (page.filtered)
                    WriteString(data, *page.filtered, 1024);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::written_book_content& value) {
            WriteVar<int32_t>(34, data);
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
            WriteVar<int32_t>(35, data);
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
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::debug_stick_state& value) {
            WriteVar<int32_t>(36, data);
            data.push_back(NBT::build((const enbt::value&)value.previous_state).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::entity_data& value) {
            WriteVar<int32_t>(37, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bucket_entity_data& value) {
            WriteVar<int32_t>(38, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::block_entity_data& value) {
            WriteVar<int32_t>(39, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::instrument& value) {
            WriteVar<int32_t>(40, data);
            std::visit(
                [&data](auto& it) {
                    using T = std::decay_t<decltype(it)>;
                    //TODO
                },
                value.type
            );
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::ominous_bottle_amplifier& value) {
            WriteVar<int32_t>(41, data);
            WriteVar<int32_t>(value.amplifier, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::jukebox_playable& value) {
            WriteVar<int32_t>(42, data);
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
                                    id = registers::view_reg_pro_id("minecraft:sound_event", it, 768);
                                } catch (...) {
                                    id = registers::view_reg_pro_id("minecraft:sound_event", "minecraft:entity.generic.eat", 768);
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

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::recipes& value) {
            WriteVar<int32_t>(43, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::lodestone_tracker& value) {
            WriteVar<int32_t>(44, data);
            data.push_back((bool)value.global_pos);
            if (value.global_pos) {
                WriteIdentifier(data, value.global_pos->dimension);
                WriteValue(value.global_pos->position.raw, data);
            }
            data.push_back(value.tracked);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::firework_explosion& value) {
            WriteVar<int32_t>(45, data);
            __firework_explosion_encoder(data, value.value);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::fireworks& value) {
            WriteVar<int32_t>(46, data);
            WriteVar<int32_t>(value.duration, data);
            WriteVar<int32_t>(value.explosions.size(), data);
            for (auto& it : value.explosions)
                __firework_explosion_encoder(data, it);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::profile& value) {
            WriteVar<int32_t>(47, data);
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
            WriteVar<int32_t>(48, data);
            WriteIdentifier(data, value.sound);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::banner_patterns& value) {
            WriteVar<int32_t>(49, data);
            //TODO
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::base_color& value) {
            WriteVar<int32_t>(50, data);
            WriteVar<int32_t>((int32_t)value.color, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::pot_decorations& value) {
            WriteVar<int32_t>(51, data);
            WriteVar<int32_t>(4, data);
            WriteVar<int32_t>(block::get_block_id(value.decorations[0], 767), data);
            WriteVar<int32_t>(block::get_block_id(value.decorations[1], 767), data);
            WriteVar<int32_t>(block::get_block_id(value.decorations[2], 767), data);
            WriteVar<int32_t>(block::get_block_id(value.decorations[3], 767), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::container& value) {
            WriteVar<int32_t>(52, data);
            WriteVar<int32_t>(value.count(), data);
            value.for_each([&data](auto& slot, size_t i) {
                WriteSlotItem(data, slot, 767);
            });
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::block_state& value) {
            WriteVar<int32_t>(53, data);
            WriteVar<int32_t>(value.properties.size(), data);
            value.properties.for_each([&data](auto& property) {
                WriteString(data, property.name);
                WriteString(data, property.value);
            });
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bees& value) {
            WriteVar<int32_t>(54, data);
            WriteVar<int32_t>(value.values.size(), data);
            for (auto& it : value.values) {
                data.push_back(NBT::build(it.entity_data).get_as_network());
                WriteVar<int32_t>(it.ticks_in_hive, data);
                WriteVar<int32_t>(it.min_ticks_in_hive, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::lock& value) {
            WriteVar<int32_t>(55, data);
            data.push_back(NBT::build(enbt::value(value.key)).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::container_loot& value) {
            WriteVar<int32_t>(56, data);
            enbt::compound dat;
            dat["loot_table"] = value.loot_table;
            dat["seed"] = value.seed;
            data.push_back(NBT::build((enbt::value&)dat).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::consumable& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::death_protection& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::enchantable& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::equippable& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::glider& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::item_model& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::repairable& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tooltip_style& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::use_cooldown& value) {}

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::use_remainder& value) {}
    }

    namespace slot_component_decoder {
        using namespace base_objects;

        list_array<std::string> variants(int32_t id) {
            switch (id) {
            case 0:
                return {"minecraft:custom_data"};
            case 1:
                return {"minecraft:max_stack_size"};
            case 2:
                return {"minecraft:max_damage"};
            case 3:
                return {"minecraft:damage"};
            case 4:
                return {"minecraft:unbreakable"};
            case 5:
                return {"minecraft:custom_name"};
            case 6:
                return {"minecraft:item_name"};
            case 7:
                return {"minecraft:lore"};
            case 8:
                return {"minecraft:rarity"};
            case 9:
                return {"minecraft:enchantments"};
            case 10:
                return {"minecraft:can_place_on"};
            case 11:
                return {"minecraft:can_break"};
            case 12:
                return {"minecraft:attribute_modifiers"};
            case 13:
                return {"minecraft:custom_model_data"};
            case 14:
                return {"minecraft:hide_additional_tooltip"};
            case 15:
                return {"minecraft:hide_tooltip"};
            case 16:
                return {"minecraft:repair_cost"};
            case 17:
                return {"minecraft:creative_slot_lock"};
            case 18:
                return {"minecraft:enchantment_glint_override"};
            case 19:
                return {"minecraft:intangible_projectile"};
            case 20:
                return {"minecraft:food", "minecraft:consumable"};
            case 21:
                return {"minecraft:fire_resistant"};
            case 22:
                return {"minecraft:tool"};
            case 23:
                return {"minecraft:stored_enchantments"};
            case 24:
                return {"minecraft:dyed_color"};
            case 25:
                return {"minecraft:map_color"};
            case 26:
                return {"minecraft:map_id"};
            case 27:
                return {"minecraft:map_decorations"};
            case 28:
                return {"minecraft:map_post_processing"};
            case 29:
                return {"minecraft:charged_projectiles"};
            case 30:
                return {"minecraft:bundle_contents"};
            case 31:
                return {"minecraft:potion_contents"};
            case 32:
                return {"minecraft:suspicious_stew_effects"};
            case 33:
                return {"minecraft:writable_book_content"};
            case 34:
                return {"minecraft:written_book_content"};
            case 35:
                return {"minecraft:trim"};
            case 36:
                return {"minecraft:debug_stick_state"};
            case 37:
                return {"minecraft:entity_data"};
            case 38:
                return {"minecraft:bucket_entity_data"};
            case 39:
                return {"minecraft:block_entity_data"};
            case 40:
                return {"minecraft:instrument"};
            case 41:
                return {"minecraft:ominous_bottle_amplifier"};
            case 42:
                return {"minecraft:jukebox_playable"};
            case 43:
                return {"minecraft:recipes"};
            case 44:
                return {"minecraft:lodestone_tracker"};
            case 45:
                return {"minecraft:firework_explosion"};
            case 46:
                return {"minecraft:fireworks"};
            case 47:
                return {"minecraft:profile"};
            case 48:
                return {"minecraft:note_block_sound"};
            case 49:
                return {"minecraft:banner_patterns"};
            case 50:
                return {"minecraft:base_color"};
            case 51:
                return {"minecraft:pot_decorations"};
            case 52:
                return {"minecraft:container"};
            case 53:
                return {"minecraft:block_state"};
            case 54:
                return {"minecraft:bees"};
            case 55:
                return {"minecraft:lock"};
            case 56:
                return {"minecraft:container_loot"};
            default:
                return {};
            }
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
                return slot_component::unbreakable{.value = data.read_value<bool>()};
            case 5:
                return slot_component::custom_name{.value = Chat::fromEnbt(ReadNBT(data).get_as_enbt())};
            case 6:
                return slot_component::item_name{.value = Chat::fromEnbt(ReadNBT(data).get_as_enbt())};
            case 7: {
                slot_component::lore res;
                int32_t count = data.read_var<int32_t>();
                res.value.reserve(count);
                for (int32_t i = 0; i < count; i++)
                    res.value.push_back(Chat::fromEnbt(ReadNBT(data).get_as_enbt()));
                return res;
            }
            case 8:
                return slot_component::rarity{
                    (slot_component::rarity)data.read_var<int32_t>()
                };
                //TODO
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