
#include <src/base_objects/block.hpp>
#include <src/base_objects/recipe.hpp>
#include <src/protocolHelper/packets/769/writers_readers.hpp>
#include <src/registers.hpp>

namespace copper_server::packets::release_769::reader {
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
            WriteVar<int32_t>(value.enchants.size(), data);
            for (auto& enchantment : value.enchants) {
                WriteVar<int32_t>(enchantment.first, data);
                WriteVar<int32_t>(enchantment.second, data);
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::can_place_on& value) {
            WriteVar<int32_t>(11, data);
            WriteVar<int32_t>(value.predicates.size(), data);
            for (auto& predicate : value.predicates) {
                //TODO create block predicate serializer
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::can_break& value) {
            WriteVar<int32_t>(12, data);
            WriteVar<int32_t>(value.predicates.size(), data);
            for (auto& predicate : value.predicates) {
                //TODO create block predicate serializer
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::attribute_modifiers& value) {
            WriteVar<int32_t>(13, data);
            WriteVar<int32_t>(value.attributes.size(), data);
            for (auto& attribute : value.attributes) {
                try {
                    WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:attribute", attribute.type, 769), data);
                } catch (...) {
                    continue; //ignore unsupported attributes for client and handle by server
                }
                WriteUUID(enbt::raw_uuid::from_string(attribute.id), data);
                WriteString(data, attribute.id);
                WriteValue(attribute.amount, data);
                WriteVar<int32_t>((int32_t)attribute.operation, data);
                WriteVar<int32_t>((int32_t)attribute.slot, data);
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::custom_model_data& value) {
            WriteVar<int32_t>(14, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::hide_additional_tooltip& value) {
            WriteVar<int32_t>(15, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::hide_tooltip& value) {
            WriteVar<int32_t>(16, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::repair_cost& value) {
            WriteVar<int32_t>(17, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::creative_slot_lock& value) {
            WriteVar<int32_t>(18, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::enchantment_glint_override& value) {
            WriteVar<int32_t>(19, data);
            WriteVar<int32_t>(value.has_glint, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::intangible_projectile& value) {
            WriteVar<int32_t>(20, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::food& value) {
            WriteVar<int32_t>(21, data);
            WriteVar<int32_t>(value.nutrition, data);
            WriteValue<float>(value.saturation, data);
            data.push_back(value.can_always_eat);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::damage_resistant& value) {
            WriteVar<int32_t>(25, data);
            WriteIdentifier(data, value.types);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tool& value) {
            WriteVar<int32_t>(26, data);
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
            WriteVar<int32_t>(33, data);
            WriteVar<int32_t>(value.enchants.size(), data);
            for (auto& enchantment : value.enchants) {
                WriteVar<int32_t>(enchantment.first, data);
                WriteVar<int32_t>(enchantment.second, data);
            }
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::dyed_color& value) {
            WriteVar<int32_t>(34, data);
            WriteVar<int32_t>(value.rgb, data);
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_color& value) {
            WriteVar<int32_t>(35, data);
            WriteVar<int32_t>(value.rgb, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_id& value) {
            WriteVar<int32_t>(36, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_decorations& value) {
            WriteVar<int32_t>(37, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::map_post_processing& value) {
            WriteVar<int32_t>(38, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::charged_projectiles& value) {
            WriteVar<int32_t>(39, data);
            WriteVar<int32_t>(value.data.size(), data);
            for (auto& projectile : value.data)
                WriteSlot(data, *projectile, 767);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bundle_contents& value) {
            WriteVar<int32_t>(40, data);
            WriteVar<int32_t>(value.items.size(), data);
            for (auto& item : value.items)
                WriteSlot(data, *item, 767);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::potion_contents& value) {
            WriteVar<int32_t>(41, data);
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

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::suspicious_stew_effects& value) {
            WriteVar<int32_t>(42, data);
            WriteVar<int32_t>(value.effects.size(), data);
            for (auto& effect : value.effects) {
                WriteVar<int32_t>(effect.first, data);
                WriteVar<int32_t>(effect.second, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::writable_book_content& value) {
            WriteVar<int32_t>(43, data);
            WriteVar<int32_t>(value.pages.size(), data);
            for (auto& page : value.pages) {
                WriteString(data, page.text, 1024);
                data.push_back((bool)page.filtered);
                if (page.filtered)
                    WriteString(data, *page.filtered, 1024);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::written_book_content& value) {
            WriteVar<int32_t>(44, data);
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
            WriteVar<int32_t>(45, data);
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
            WriteVar<int32_t>(46, data);
            data.push_back(NBT::build((const enbt::value&)value.previous_state).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::entity_data& value) {
            WriteVar<int32_t>(47, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bucket_entity_data& value) {
            WriteVar<int32_t>(48, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::block_entity_data& value) {
            WriteVar<int32_t>(49, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::instrument& value) {
            WriteVar<int32_t>(50, data);
            std::visit(
                [&data](auto& it) {
                    using T = std::decay_t<decltype(it)>;
                    //TODO
                },
                value.type
            );
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::ominous_bottle_amplifier& value) {
            WriteVar<int32_t>(51, data);
            WriteVar<int32_t>(value.amplifier, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::jukebox_playable& value) {
            WriteVar<int32_t>(52, data);
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
                                    id = registers::view_reg_pro_id("minecraft:sound_event", it, 769);
                                } catch (...) {
                                    id = registers::view_reg_pro_id("minecraft:sound_event", "minecraft:entity.generic.eat", 769);
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
            data.push_back(value.show_in_tooltip);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::recipes& value) {
            WriteVar<int32_t>(53, data);
            data.push_back(NBT::build((const enbt::value&)value.value).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::lodestone_tracker& value) {
            WriteVar<int32_t>(54, data);
            data.push_back((bool)value.global_pos);
            if (value.global_pos) {
                WriteIdentifier(data, value.global_pos->dimension);
                WriteValue(value.global_pos->position.raw, data);
            }
            data.push_back(value.tracked);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::firework_explosion& value) {
            WriteVar<int32_t>(55, data);
            __firework_explosion_encoder(data, value.value);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::fireworks& value) {
            WriteVar<int32_t>(56, data);
            WriteVar<int32_t>(value.duration, data);
            WriteVar<int32_t>(value.explosions.size(), data);
            for (auto& it : value.explosions)
                __firework_explosion_encoder(data, it);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::profile& value) {
            WriteVar<int32_t>(57, data);
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
            WriteVar<int32_t>(58, data);
            WriteIdentifier(data, value.sound);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::banner_patterns& value) {
            WriteVar<int32_t>(59, data);
            //TODO
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::base_color& value) {
            WriteVar<int32_t>(60, data);
            WriteVar<int32_t>((int32_t)value.color, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::pot_decorations& value) {
            WriteVar<int32_t>(61, data);
            WriteVar<int32_t>(4, data);
            WriteVar<int32_t>(block::get_block_id(value.decorations[0], 767), data);
            WriteVar<int32_t>(block::get_block_id(value.decorations[1], 767), data);
            WriteVar<int32_t>(block::get_block_id(value.decorations[2], 767), data);
            WriteVar<int32_t>(block::get_block_id(value.decorations[3], 767), data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::container& value) {
            WriteVar<int32_t>(62, data);
            WriteVar<int32_t>(value.count(), data);
            value.for_each([&data](auto& slot, size_t i) {
                WriteSlotItem(data, slot, 767);
            });
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::block_state& value) {
            WriteVar<int32_t>(63, data);
            WriteVar<int32_t>(value.properties.size(), data);
            value.properties.for_each([&data](auto& property) {
                WriteString(data, property.name);
                WriteString(data, property.value);
            });
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::bees& value) {
            WriteVar<int32_t>(64, data);
            WriteVar<int32_t>(value.values.size(), data);
            for (auto& it : value.values) {
                data.push_back(NBT::build(it.entity_data).get_as_network());
                WriteVar<int32_t>(it.ticks_in_hive, data);
                WriteVar<int32_t>(it.min_ticks_in_hive, data);
            }
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::lock& value) {
            WriteVar<int32_t>(65, data);
            data.push_back(NBT::build(enbt::value(value.key)).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::container_loot& value) {
            WriteVar<int32_t>(66, data);
            enbt::compound dat;
            dat["loot_table"] = value.loot_table;
            dat["seed"] = value.seed;
            data.push_back(NBT::build((enbt::value&)dat).get_as_network());
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::consumable& value) {
            int32_t animation_id = [&value]() {
                static std::unordered_map<std::string, int32_t> map{
                    {"minecraft:none", 0},
                    {"minecraft:eat", 1},
                    {"minecraft:drink", 2},
                    {"minecraft:block", 3},
                    {"minecraft:bow", 4},
                    {"minecraft:spear", 5},
                    {"minecraft:crossbow", 6},
                    {"minecraft:spyglass", 7},
                    {"minecraft:toot_horn", 8},
                    {"minecraft:brush", 9},
                    {"none", 0},
                    {"eat", 1},
                    {"drink", 2},
                    {"block", 3},
                    {"bow", 4},
                    {"spear", 5},
                    {"crossbow", 6},
                    {"spyglass", 7},
                    {"toot_horn", 8},
                    {"brush", 9},
                };
                auto it = map.find(value.animation);
                if (it == map.end())
                    return -1;
                return it->second;
            }();
            if (animation_id == -1) {
                animation_id = 1; //ignore unsupported and set eating animation
            }

            WriteVar<int32_t>(22, data);
            WriteValue<float>(value.consume_seconds, data);
            WriteVar<int32_t>(animation_id, data);
            std::visit(
                [&data](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                        int32_t id = 0;
                        try {
                            id = registers::view_reg_pro_id("minecraft:sound_event", it, 769);
                        } catch (...) {
                            id = registers::view_reg_pro_id("minecraft:sound_event", "minecraft:entity.generic.eat", 769);
                        }
                        WriteVar<int32_t>(1 + id, data);
                    } else {
                        WriteVar<int32_t>(0, data); //custom
                        WriteIdentifier(data, it.sound_name);
                        data.push_back((bool)it.fixed_range);
                        if (it.fixed_range)
                            WriteValue<float>(*it.fixed_range, data);
                    }
                },
                value.sound
            );
            data.push_back(value.has_consume_particles);
            WriteVar<int32_t>(value.on_consume_effects.size(), data);
            for (auto& it : value.on_consume_effects)
                __consume_effect_encoder(data, it);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::death_protection& value) {
            WriteVar<int32_t>(32, data);
            WriteVar<int32_t>(value.death_effects.size(), data);
            for (auto& it : value.death_effects)
                __consume_effect_encoder(data, it);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::enchantable& value) {
            WriteVar<int32_t>(27, data);
            WriteVar<int32_t>(value.value, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::equippable& value) {
            int32_t slot_id = [&value]() {
                static std::unordered_map<std::string, int32_t> map{
                    {"minecraft:mainhand", 0},
                    {"minecraft:feet", 1},
                    {"minecraft:legs", 2},
                    {"minecraft:chest", 3},
                    {"minecraft:head", 4},
                    {"minecraft:offhand", 5},
                    {"minecraft:body", 6},
                    {"mainhand", 0},
                    {"feet", 1},
                    {"legs", 2},
                    {"chest", 3},
                    {"head", 4},
                    {"offhand", 5},
                    {"body", 6},
                };
                auto it = map.find(value.slot);
                if (it == map.end())
                    return 6; //body
                return it->second;
            }();
            WriteVar<int32_t>(28, data);
            WriteVar<int32_t>(slot_id, data);
            std::visit(
                [&data](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                        int32_t id = 0;
                        try {
                            id = registers::view_reg_pro_id("minecraft:sound_event", it, 769);
                        } catch (...) {
                            id = registers::view_reg_pro_id("minecraft:sound_event", "minecraft:item.armor.equip_generic", 769);
                        }
                        WriteVar<int32_t>(1 + id, data);
                    } else if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::nullptr_t>) {
                        WriteVar<int32_t>(1 + registers::view_reg_pro_id("minecraft:sound_event", "minecraft:item.armor.equip_generic", 769), data);
                    } else {
                        WriteVar<int32_t>(0, data); //custom
                        WriteIdentifier(data, it.sound_name);
                        data.push_back((bool)it.fixed_range);
                        if (it.fixed_range)
                            WriteValue<float>(*it.fixed_range, data);
                    }
                },
                value.equip_sound
            );
            data.push_back((bool)value.model);
            if (value.model)
                WriteIdentifier(data, *value.model);
            data.push_back((bool)value.camera_overlay);
            if (value.camera_overlay)
                WriteIdentifier(data, *value.camera_overlay);
            if (!std::holds_alternative<std::nullptr_t>(value.allowed_entities)) {
                data.push_back(true);
                std::visit(
                    [&data](auto& it) {
                        if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                            WriteVar<int32_t>(0, data);
                            WriteIdentifier(data, it);
                        } else if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::nullptr_t>) {

                        } else {
                            WriteVar<int32_t>(1 + it.size(), data); //type 1 + arr size 1
                            for (auto& i : it) {
                                int32_t id = 0;
                                try {
                                    id = registers::view_reg_pro_id("minecraft:entity_type", i, 769);
                                } catch (...) {
                                    id = registers::view_reg_pro_id("minecraft:entity_type", "minecraft:player", 769);
                                }
                                WriteVar<int32_t>(id, data);
                            }
                        }
                    },
                    value.allowed_entities
                );
            }
            data.push_back((bool)value.dispensable);
            data.push_back((bool)value.swappable);
            data.push_back((bool)value.damage_on_hurt);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::glider& value) {
            WriteVar<int32_t>(30, data);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::item_model& value) {
            WriteVar<int32_t>(7, data);
            WriteIdentifier(data, value.value);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::repairable& value) {
            WriteVar<int32_t>(29, data);
            std::visit(
                [&data](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                        WriteVar<int32_t>(0, data);
                        WriteIdentifier(data, it);
                    } else {
                        WriteVar<int32_t>(1 + it.size(), data); //type 1 + arr size 1
                        for (auto& i : it) {
                            int32_t id = slot_data::get_slot_data(i).internal_item_aliases.at(769).local_id;
                            WriteVar<int32_t>(id, data);
                        }
                    }
                },
                value.items
            );
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::tooltip_style& value) {
            WriteVar<int32_t>(31, data);
            WriteIdentifier(data, value.value);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::use_cooldown& value) {
            WriteVar<int32_t>(24, data);
            WriteValue<float>(value.seconds, data);
            data.push_back((bool)value.cooldown_group);
            if (value.cooldown_group)
                WriteIdentifier(data, *value.cooldown_group);
        }

        void encode(const slot_data& slot, list_array<uint8_t>& data, const slot_component::use_remainder& value) {
            WriteVar<int32_t>(23, data);
            std::visit(
                [&data](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, slot_data*>) {
                        reader::WriteSlot(data, *it);
                    } else
                        reader::WriteSlot(data, slot_data::create_item(it.id, it.count));
                },
                value.proxy_value
            );
        }
    }

    namespace slot_component_decoder {
        using namespace base_objects;

        list_array<std::string> variants(int32_t id) {
            return {
                registers::view_reg_pro_name("minecraft:data_component_type", id, 769)
            };
        }

        slot_component::unified decode(ArrayStream& data) {
            switch (ReadVar<int32_t>(data)) {
            case 0: {
                slot_component::custom_data res;
                res.value = ReadNBT(data).get_as_enbt();
                return res;
            }
            case 1:
                return slot_component::max_stack_size{.value = (uint8_t)ReadVar<int32_t>(data)};
            case 2:
                return slot_component::max_damage{.value = ReadVar<int32_t>(data)};
            case 3:
                return slot_component::damage{.value = ReadVar<int32_t>(data)};
            case 4:
                return slot_component::unbreakable{.value = ReadValue<bool>(data)};
            case 5:
                return slot_component::custom_name{.value = Chat::fromEnbt(ReadNBT(data).get_as_enbt())};
            case 6:
                return slot_component::item_name{.value = Chat::fromEnbt(ReadNBT(data).get_as_enbt())};
            case 8: {
                slot_component::lore res;
                int32_t count = ReadVar<int32_t>(data);
                res.value.reserve(count);
                for (int32_t i = 0; i < count; i++)
                    res.value.push_back(Chat::fromEnbt(ReadNBT(data).get_as_enbt()));
                return res;
            }
            case 9:
                return slot_component::rarity{(slot_component::rarity)ReadVar<int32_t>(data)};
            case 10: {
                slot_component::enchantments res;
                int32_t count = ReadVar<int32_t>(data);
                res.enchants.reserve(count);
                for (int32_t i = 0; i < count; i++) {
                    int32_t id = ReadVar<int32_t>(data);
                    int32_t lvl = ReadVar<int32_t>(data);
                    res.enchants.push_back(
                        {registers::enchantments.at(registers::view_reg_pro_name("minecraft:enchantment", id, 769)).id, lvl}
                    );
                }
                res.show_in_tooltip = ReadValue<bool>(data);
                return res;
            }
            case 11: {
                slot_component::can_place_on res;
                int32_t count = ReadVar<int32_t>(data);
                res.predicates.reserve(count);
                for (int32_t i = 0; i < count; i++)
                    ;
                //res.predicates.push_back(ReadNBT(data).get_as_enbt()); TODO
                res.show_in_tooltip = ReadValue<bool>(data);
                return res;
            }
            case 12: {
                slot_component::can_break res;
                int32_t count = ReadVar<int32_t>(data);
                res.predicates.reserve(count);
                for (int32_t i = 0; i < count; i++)
                    ;
                //res.predicates.push_back(ReadNBT(data).get_as_enbt()); TODO
                res.show_in_tooltip = ReadValue<bool>(data);
                return res;
            }
            case 13: {
                slot_component::attribute_modifiers res;
                int32_t count = ReadVar<int32_t>(data);
                res.attributes.reserve(count);
                for (int32_t i = 0; i < count; i++) {
                    item_attribute attribute;
                    attribute.type = registers::view_reg_pro_name("minecraft:attribute", ReadVar<int32_t>(data), 769);
                    ReadUUID(data); //ignore UUID
                    attribute.id = ReadString(data);
                    attribute.amount = ReadValue<double>(data);
                    attribute.operation = (item_attribute::operation_e)ReadVar<int32_t>(data);
                    attribute.slot = (item_attribute::slot_filter)ReadVar<int32_t>(data);
                    res.attributes.push_back(attribute);
                }
                res.show_in_tooltip = ReadValue<bool>(data);
                return res;
            }
            case 14:
                return slot_component::custom_model_data{.value = ReadVar<int32_t>(data)};
            case 15:
                return slot_component::hide_additional_tooltip{};
            case 16:
                return slot_component::hide_tooltip{};
            case 17: {
                slot_component::repair_cost res;
                res.value = ReadVar<int32_t>(data);
                return res;
            }
            case 18:
                return slot_component::creative_slot_lock{};
            case 19:
                return slot_component::enchantment_glint_override{.has_glint = ReadVar<int32_t>(data)};
            case 20:
                return slot_component::intangible_projectile{};
            case 21: {
                slot_component::food res;
                res.nutrition = ReadVar<int32_t>(data);
                res.saturation = ReadValue<float>(data);
                res.can_always_eat = ReadValue<bool>(data);
                return res;
            }
            case 22: {
                slot_component::consumable res;
                //TODO
                return res;
            }
            case 25:
                return slot_component::damage_resistant{.types = ReadIdentifier(data)};
                //TODO
            case 23:
            case 24:
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
                WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", T::name, 769), data);
                if constexpr (
                    std::is_same_v<T, base_objects::slot_displays::minecraft::empty>
                    || std::is_same_v<T, base_objects::slot_displays::minecraft::any_fuel>
                ) {
                } else if constexpr (std::is_same_v<T, base_objects::slot_displays::minecraft::item>) {
                    WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:item", arg.item, 769), data);
                } else if constexpr (std::is_same_v<T, base_objects::slot_displays::minecraft::item_stack>) {
                    WriteSlotItem(data, arg.item, 769);
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
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", T_info::name, 769), data);
                        WriteVar<int32_t>(it.ingredients.size(), data);
                        for (auto& it : it.ingredients)
                            WriteIngredient(data, it);
                        WriteIngredient(data, it.result);
                        WriteIngredient(data, it.crafting_station);
                    } else if constexpr (std::is_same_v<T, base_objects::recipes::minecraft::crafting_shaped>) {
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", T_info::name, 769), data);
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
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", "minecraft:furnace", 769), data);
                        WriteIngredient(data, it.ingredient);
                        WriteIngredient(data, it.fuel);
                        WriteIngredient(data, it.result);
                        WriteIngredient(data, it.crafting_station);
                        WriteVar<int32_t>(it.cooking_time, data);
                        WriteValue<float>(it.experience, data);
                    } else if constexpr (std::is_same_v<T, base_objects::recipes::minecraft::stonecutting>) {
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", "minecraft:stonecutter", 769), data);
                        WriteIngredient(data, it.ingredient);
                        WriteIngredient(data, it.result);
                        WriteIngredient(data, it.crafting_station);
                    } else if constexpr (std::is_same_v<T, base_objects::recipes::minecraft::smithing_transform>) {
                        WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_display", "minecraft:smithing", 769), data);
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
        int32_t item_count = ReadVar<int32_t>(data);
        if (!item_count)
            return std::nullopt;
        int32_t item_id = ReadVar<int32_t>(data);
        base_objects::slot_data item = base_objects::slot_data::create_item(item_id);
        int32_t num_to_add = ReadVar<int32_t>(data);
        int32_t num_to_remove = ReadVar<int32_t>(data);
        for (int32_t i = 0; i < num_to_add; i++)
            item.add_component(slot_component_decoder::decode(data));
        for (int32_t i = 0; i < num_to_remove; i++)
            slot_component_decoder::variants(ReadVar<int32_t>(data)).for_each([&](auto& it) {
                item.remove_component(it);
            });
        return item;
    }
}