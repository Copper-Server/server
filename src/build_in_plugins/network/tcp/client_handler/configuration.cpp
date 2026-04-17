/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/fast_task/include/files.hpp>
#include <src/api/configuration.hpp>
#include <src/api/dialogs.hpp>
#include <src/api/id.hpp>
#include <src/api/network/tcp.hpp>
#include <src/api/packets/client_bound/config.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/config.hpp>
#include <src/api/registers.hpp>
#include <src/api/tags.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>
#include <src/plugin/main.hpp>
#include <src/resources/registers.hpp>

namespace copper_server::build_in_plugins::network::tcp::client_handler {
    struct tcp_configuration : public plugin_auto_register<"network/tcp_configuration", tcp_configuration> {
        struct ResourcePackData {
            bool required : 1 = false;
        };

        struct extra_data_t {
            keep_alive_solution ka_solution;
            list_array<plugin_registration_ptr> active_plugins{};
            std::unordered_map<base_objects::uuid, ResourcePackData> pending_resource_packs;
            bool code_of_conduct_is_accepted = false;
            bool packs_requested = false;

            static extra_data_t& get(base_objects::shared_client_data& client) {
                return *client.packets_state.internal_data.set([&](auto& data) {
                    if (!data.extra_data)
                        data.extra_data = std::make_shared<extra_data_t>(client.get_session());
                    return reinterpret_cast<extra_data_t*>(data.extra_data.get());
                });
            }
        };

        static void send_tags(base_objects::shared_client_data& client) {
            api::packets::client_bound::config::update_tags::entry block;
            block.registry_id = "minecraft:block";
            for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::block, "minecraft"))
                block.tags.push_back({.tag_name = id, .values = values.convert<base_objects::var_int32>()});

            api::packets::client_bound::config::update_tags::entry item;
            item.registry_id = "minecraft:item";
            for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::item, "minecraft"))
                item.tags.push_back({.tag_name = id, .values = values.convert<base_objects::var_int32>()});

            api::packets::client_bound::config::update_tags::entry fluid;
            fluid.registry_id = "minecraft:fluid";
            for (auto& [id, values] : api::tags::view_tag("minecraft:fluid", "minecraft")) {
                fluid.tags.push_back(
                    {.tag_name = id, .values = api::registers::convert_reg_pro_id("minecraft:fluid", values).convert<base_objects::var_int32>()}
                );
            }
            api::packets::client_bound::config::update_tags::entry worldgen_biome;
            worldgen_biome.registry_id = "minecraft:worldgen/biome";
            for (auto& [id, values] : api::tags::view_tag("minecraft:worldgen/biome", "minecraft")) {
                worldgen_biome.tags.push_back(
                    {.tag_name = id, .values = values.convert_fn([](auto& it) { return (base_objects::var_int32)api::registers::biomes.at(it).id; })}
                );
            }
            api::packets::client_bound::config::update_tags::entry entity_type;
            entity_type.registry_id = "minecraft:entity_type";
            for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::entity_type, "minecraft"))
                entity_type.tags.push_back({.tag_name = id, .values = values.convert<base_objects::var_int32>()});


            api::packets::client_bound::config::update_tags::entry game_event;
            game_event.registry_id = "minecraft:game_event";
            for (auto& [id, values] : api::tags::view_tag("minecraft:game_event", "minecraft")) {
                game_event.tags.push_back(
                    {.tag_name = id, .values = api::registers::convert_reg_pro_id("minecraft:game_event", values).convert<base_objects::var_int32>()}
                );
            }

            client << api::packets::client_bound::config::update_tags{
                .entries{
                    std::move(block),
                    std::move(item),
                    std::move(fluid),
                    std::move(worldgen_biome),
                    std::move(entity_type),
                    std::move(game_event)
                }
            };
        }

        template <class RegistryT, class FN>
        static base_objects::network::response registry_data_serialize_entry(const std::string& identifier, list_array<typename std::unordered_map<std::string, RegistryT>::iterator>& values, FN&& serializer) {
            list_array<std::pair<std::string, util::nbt>> fixed_data;
            fixed_data.resize(values.size());
            for (auto& _it : values) {
                auto& [name, it] = *_it;
                if (it.id >= fixed_data.size())
                    throw std::out_of_range("Invalid registry values");
                fixed_data[it.id] = {name, serializer(it)};
            }


            api::packets::client_bound::config::registry_data res;
            res.registry_id = identifier;
            res.entries.reserve(fixed_data.size());
            fixed_data.for_each([&](const std::string& name, util::nbt& data) {
                api::packets::client_bound::config::registry_data::entry entry;
                entry.entry_id = name;
                if (!data.is_end())
                    entry.data = std::move(data);
                res.entries.push_back(std::move(entry));
            });
            return api::packets::encode(std::move(res));
        }

        static void send_registry_data(base_objects::shared_client_data& client) {
            static base_objects::network::response data;
            if (!data.has_data()) {
                { // minecraft:trim_material
                    data += registry_data_serialize_entry<api::registers::armor_trim_material>("minecraft:trim_material", api::registers::armor_trim_materials_cache, [](api::registers::armor_trim_material& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["asset_name"] = it.asset_name;
                        if (std::holds_alternative<std::string>(it.description))
                            element["description"] = std::get<std::string>(it.description);
                        else
                            element["description"] = std::get<base_objects::chat>(it.description).to_nbt();
                        return element.take_map();
                    });
                }
                { // minecraft:trim_pattern
                    data += registry_data_serialize_entry<api::registers::armor_trim_pattern>("minecraft:trim_pattern", api::registers::armor_trim_patterns_cache, [](api::registers::armor_trim_pattern& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["asset_id"] = it.asset_id;
                        if (std::holds_alternative<std::string>(it.description))
                            element["description"] = std::get<std::string>(it.description);
                        else
                            element["description"] = std::get<base_objects::chat>(it.description).to_nbt();
                        element["decal"] = it.decal;
                        return element.take_map();
                    });
                }
                { // minecraft:worldgen/biome
                    data += registry_data_serialize_entry<api::registers::biome>("minecraft:worldgen/biome", api::registers::biomes_cache, [](api::registers::biome& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["has_precipitation"] = it.has_precipitation;
                        element["temperature"] = it.temperature;
                        if (it.temperature_modifier)
                            element["temperature_modifier"] = *it.temperature_modifier;
                        element["downfall"] = it.downfall;
                        { //effects
                            util::nbt_compound effects;
                            effects["fog_color"] = it.effects.fog_color;
                            effects["water_color"] = it.effects.water_color;
                            effects["water_fog_color"] = it.effects.water_fog_color;
                            effects["sky_color"] = it.effects.sky_color;
                            if (it.effects.foliage_color)
                                effects["foliage_color"] = *it.effects.foliage_color;
                            if (it.effects.grass_color)
                                effects["grass_color"] = *it.effects.grass_color;
                            if (it.effects.grass_color_modifier)
                                effects["grass_color_modifier"] = *it.effects.grass_color_modifier;
                            if (it.effects.particle) {
                                util::nbt_compound particle;
                                particle["probability"] = it.effects.particle->probability;
                                particle["options"] = it.effects.particle->options.options;
                                particle["options"]["type"] = it.effects.particle->options.type.to_string();
                                effects["particle"] = std::move(particle).take_map();
                            }
                            if (it.effects.ambient_sound) {
                                if (std::holds_alternative<std::string>(*it.effects.ambient_sound))
                                    effects["ambient_sound"] = std::get<std::string>(*it.effects.ambient_sound);
                                else if (std::holds_alternative<api::registers::biome::ambient_sound>(*it.effects.ambient_sound)) {
                                    util::nbt_compound ambient_sound;
                                    ambient_sound["sound"] = std::get<api::registers::biome::ambient_sound>(*it.effects.ambient_sound).sound.to_string();
                                    ambient_sound["range"] = std::get<api::registers::biome::ambient_sound>(*it.effects.ambient_sound).range;
                                    effects["ambient_sound"] = std::move(ambient_sound).take_map();
                                }
                            }
                            if (it.effects.mood_sound) {
                                util::nbt_compound mood_sound;
                                mood_sound["sound"] = it.effects.mood_sound->sound.to_string();
                                mood_sound["tick_delay"] = it.effects.mood_sound->tick_delay;
                                mood_sound["block_search_extent"] = it.effects.mood_sound->block_search_extent;
                                mood_sound["offset"] = it.effects.mood_sound->offset;
                                effects["mood_sound"] = std::move(mood_sound).take_map();
                            }
                            if (it.effects.additions_sound) {
                                util::nbt_compound additions_sound;
                                additions_sound["sound"] = it.effects.additions_sound->sound.to_string();
                                additions_sound["tick_chance"] = it.effects.additions_sound->tick_chance;
                                effects["additions_sound"] = std::move(additions_sound).take_map();
                            }
                            {
                                list_array<util::nbt> music_arr;
                                for (auto& music_it : it.effects.music) {
                                    util::nbt_compound music;
                                    music["sound"] = music_it.sound.to_string();
                                    music["min_delay"] = music_it.min_delay;
                                    music["max_delay"] = music_it.max_delay;
                                    music["replace_current_music"] = music_it.replace_current_music;
                                    music_arr.push_back(util::nbt_compound{{"weight", music_it.music_weight}, {"data", std::move(music).take_map()}}.take_map());
                                }
                                effects["music"] = std::move(music_arr);
                            }
                            element["effects"] = std::move(effects).take_map();
                        }
                        return element.take_map();
                    });
                }
                { // minecraft:chat_type
                    data += registry_data_serialize_entry<api::registers::chat_type>("minecraft:chat_type", api::registers::chat_types_cache, [](api::registers::chat_type& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        if (it.chat) {
                            util::nbt_compound chat;
                            chat["translation_key"] = it.chat->translation_key;
                            if (it.chat->style) {
                                it.chat->style->get_extra().clear();
                                it.chat->style->set_text("");
                                util::nbt style = it.chat->style->to_nbt();
                                style.remove("text");
                                chat["style"] = std::move(style);
                            }
                            if (std::holds_alternative<std::string>(it.chat->parameters))
                                chat["parameters"] = std::get<std::string>(it.chat->parameters);
                            else
                                chat["parameters"] = list_array<util::nbt>(std::get<std::vector<std::string>>(it.chat->parameters));
                            element["chat"] = std::move(chat).take_map();
                        }
                        if (it.narration) {
                            util::nbt_compound narration;
                            narration["translation_key"] = it.narration->translation_key;
                            if (std::holds_alternative<std::string>(it.narration->parameters))
                                narration["parameters"] = std::get<std::string>(it.narration->parameters);
                            else
                                narration["parameters"] = list_array<util::nbt>(std::get<std::vector<std::string>>(it.narration->parameters));
                            element["narration"] = std::move(narration).take_map();
                        }
                        return element.take_map();
                    });
                }
                { // minecraft:damage_type
                    data += registry_data_serialize_entry<api::registers::damage_type>("minecraft:damage_type", api::registers::damage_types_cache, [](api::registers::damage_type& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["message_id"] = it.message_id;
                        {
                            const char* scaling = nullptr;
                            switch (it.scaling) {
                            case api::registers::damage_type::scaling_type::never:
                                scaling = "never";
                                break;
                            case api::registers::damage_type::scaling_type::when_caused_by_living_non_player:
                                scaling = "when_caused_by_living_non_player";
                                break;
                            case api::registers::damage_type::scaling_type::always:
                                scaling = "always";
                                break;
                            }
                            if (scaling)
                                element["scaling"] = scaling;
                        }
                        element["exhaustion"] = it.exhaustion;
                        if (it.effects) {
                            const char* effect = nullptr;
                            switch (*it.effects) {
                            case api::registers::damage_type::effects_type::hurt:
                                effect = "hurt";
                                break;
                            case api::registers::damage_type::effects_type::thorns:
                                effect = "thorns";
                                break;
                            case api::registers::damage_type::effects_type::drowning:
                                effect = "drowning";
                                break;
                            case api::registers::damage_type::effects_type::burning:
                                effect = "burning";
                                break;
                            case api::registers::damage_type::effects_type::poking:
                                effect = "poking";
                                break;
                            case api::registers::damage_type::effects_type::freezing:
                                effect = "freezing";
                                break;
                            default:
                                break;
                            }
                            if (effect)
                                element["effects"] = effect;
                        }
                        if (it.death_message_type) {
                            const char* death_message_type = nullptr;
                            switch (*it.death_message_type) {
                            case api::registers::damage_type::death_message_type::_default:
                                death_message_type = "default";
                                break;
                            case api::registers::damage_type::death_message_type::fall_variants:
                                death_message_type = "fall_variants";
                                break;
                            case api::registers::damage_type::death_message_type::intentional_game_design:
                                death_message_type = "intentional_game_design";
                                break;
                            default:
                                break;
                            }
                            if (death_message_type)
                                element["death_message_type"] = death_message_type;
                        }
                        return element.take_map();
                    });
                }
                { // minecraft:dimension_type
                    data += registry_data_serialize_entry<api::registers::dimension_type>("minecraft:dimension_type", api::registers::dimension_types_cache, [](api::registers::dimension_type& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        if (std::holds_alternative<int32_t>(it.monster_spawn_light_level))
                            element["monster_spawn_light_level"] = std::get<int32_t>(it.monster_spawn_light_level);
                        else
                            element["monster_spawn_light_level"] = std::get<std::shared_ptr<base_objects::number_provider>>(it.monster_spawn_light_level)->get_nbt();
                        if (it.fixed_time)
                            element["fixed_time"] = std::bit_cast<ptrdiff_t>(*it.fixed_time);
                        element["infiniburn"] = it.infiniburn;
                        element["effects"] = it.effects;
                        element["coordinate_scale"] = it.coordinate_scale;
                        element["ambient_light"] = it.ambient_light;
                        element["min_y"] = it.min_y;
                        element["height"] = it.height;
                        element["logical_height"] = it.logical_height;
                        element["monster_spawn_block_light_limit"] = it.monster_spawn_block_light_limit;
                        element["has_skylight"] = it.has_skylight;
                        element["has_ceiling"] = it.has_ceiling;
                        element["ultrawarm"] = it.ultrawarm;
                        element["natural"] = it.natural;
                        element["piglin_safe"] = it.piglin_safe;
                        element["has_raids"] = it.has_raids;
                        element["respawn_anchor_works"] = it.respawn_anchor_works;
                        element["bed_works"] = it.bed_works;
                        return element.take_map();
                    });
                }
                { // minecraft:wolf_variant
                    data += registry_data_serialize_entry<api::registers::wolf_variant>("minecraft:wolf_variant", api::registers::wolf_variants_cache, [](api::registers::wolf_variant& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["assets"] = it.assets.get_map();
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element.take_map();
                    });
                }
                { // minecraft:painting_variant
                    data += registry_data_serialize_entry<api::registers::painting_variant>("minecraft:painting_variant", api::registers::painting_variants_cache, [](api::registers::painting_variant& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["asset_id"] = it.asset_id;
                        element["height"] = std::bit_cast<int32_t>(it.height);
                        element["width"] = std::bit_cast<int32_t>(it.width);
                        element["title"] = it.title.to_nbt();
                        element["author"] = it.author.to_nbt();
                        return element.take_map();
                    });
                }
                { // minecraft:instrument
                    data += registry_data_serialize_entry<api::registers::instrument>("minecraft:instrument", api::registers::instruments_cache, [](api::registers::instrument& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["range"] = it.range;
                        element["use_duration"] = it.use_duration;
                        element["description"] = it.description.to_nbt();
                        std::visit(
                            [&](auto& it) {
                                using T = std::decay_t<decltype(it)>;
                                if constexpr (api::id::is_source<T>) {
                                    element["sound_event"] = it.to_string();
                                } else {
                                    util::nbt_compound sound_event;
                                    sound_event["sound_name"] = it.sound_name.to_string();
                                    if (it.fixed_range)
                                        sound_event["fixed_range"] = *it.fixed_range;
                                    element["sound_event"] = sound_event.take_map();
                                }
                            },
                            it.sound_event
                        );
                        return element.take_map();
                    });
                }
                { // minecraft:cat_variant
                    data += registry_data_serialize_entry<api::registers::entity_variant>("minecraft:cat_variant", api::registers::cat_variants_cache, [](api::registers::entity_variant& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = *it.model;
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element.take_map();
                    });
                }
                { // minecraft:chicken_variant
                    data += registry_data_serialize_entry<api::registers::entity_variant>("minecraft:chicken_variant", api::registers::chicken_variants_cache, [](api::registers::entity_variant& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = *it.model;
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element.take_map();
                    });
                }
                { // minecraft:cow_variant
                    data += registry_data_serialize_entry<api::registers::entity_variant>("minecraft:cow_variant", api::registers::cow_variants_cache, [](api::registers::entity_variant& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = *it.model;
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element.take_map();
                    });
                }
                { // minecraft:frog_variant
                    data += registry_data_serialize_entry<api::registers::entity_variant>("minecraft:frog_variant", api::registers::frog_variants_cache, [](api::registers::entity_variant& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = *it.model;
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element.take_map();
                    });
                }
                { // minecraft:pig_variant
                    data += registry_data_serialize_entry<api::registers::entity_variant>("minecraft:pig_variant", api::registers::pig_variants_cache, [](api::registers::entity_variant& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = *it.model;
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element.take_map();
                    });
                }
                { // minecraft:wolf_sound_variant
                    data += registry_data_serialize_entry<api::registers::wolf_sound_variant>("minecraft:wolf_sound_variant", api::registers::wolf_sound_variants_cache, [](api::registers::wolf_sound_variant& it) -> util::nbt {
                        if (!it.send_via_network_body)
                            return util::nbt{};
                        util::nbt_compound element;
                        element["ambient_sound"] = it.ambient_sound.to_string();
                        element["death_sound"] = it.death_sound.to_string();
                        element["growl_sound"] = it.growl_sound.to_string();
                        element["hurt_sound"] = it.hurt_sound.to_string();
                        element["pant_sound"] = it.pant_sound.to_string();
                        element["whine_sound"] = it.whine_sound.to_string();
                        return element.take_map();
                    });
                }
            }
            client.sendPacket(base_objects::network::response(data));
        }

        static void send_code_of_conduct_from_file(base_objects::shared_client_data& client, const std::filesystem::path& path) {
            fast_task::files::async_iofstream read(path, std::ios::in);
            if (!read.is_open()) {
                static std::string generic_code_of_conduct = "Be nice!";
                client << api::packets::client_bound::config::code_of_conduct{.text = generic_code_of_conduct};
            } else {
                //read all and send to player
                std::string res;
                read.seekg(0, std::ios::end);
                res.resize(read.tellg());
                read.seekg(0, std::ios::beg);
                read.read(res.data(), res.size());
                client << api::packets::client_bound::config::code_of_conduct{.text = res}; //TODO add caching
            }
        }

        static void send_code_of_conduct(base_objects::shared_client_data& client) {
            auto codeofconduct_path = api::configuration::get().server.get_storage_path() / "codeofconduct";
            auto player_code_of_conduct = codeofconduct_path / (client.locale + ".txt");
            auto default_code_of_conduct = codeofconduct_path / ("en_us.txt");
            if (std::filesystem::exists(player_code_of_conduct)) {
                send_code_of_conduct_from_file(client, player_code_of_conduct);
                return;
            } else if (std::filesystem::exists(default_code_of_conduct)) {
                send_code_of_conduct_from_file(client, default_code_of_conduct);
                return;
            } else {
                std::optional<std::filesystem::path> random_code_of_conduct;
                for (auto& entry : std::filesystem::directory_iterator(codeofconduct_path)) {
                    if (entry.path().extension() == ".txt") {
                        random_code_of_conduct = entry.path();
                        break;
                    }
                }
                if (random_code_of_conduct.has_value()) {
                    send_code_of_conduct_from_file(client, random_code_of_conduct.value());
                    return;
                }
            }

            static std::string generic_code_of_conduct = "Be nice!";
            client << api::packets::client_bound::config::code_of_conduct{.text = generic_code_of_conduct};
        }

        static void make_finish(base_objects::shared_client_data& client) {
            if (api::configuration::get().game_play.enable_code_of_conduct) {
                if (!extra_data_t::get(client).code_of_conduct_is_accepted) {
                    send_code_of_conduct(client);
                    return;
                }
            }
            auto& data = extra_data_t::get(client);
            if (data.packs_requested) {
                if (data.active_plugins.empty()) {
                    if (data.pending_resource_packs.empty()) {
                        client << api::packets::client_bound::config::finish_configuration{};
                    }
                }
            }
        }

        void on_register(const plugin_registration_ptr&) override {
            using client_information = api::packets::server_bound::config::client_information;
            using cookie_response = api::packets::server_bound::config::cookie_response;
            using custom_payload = api::packets::server_bound::config::custom_payload;
            using keep_alive = api::packets::server_bound::config::keep_alive;
            using pong = api::packets::server_bound::config::pong;
            using resource_pack = api::packets::server_bound::config::resource_pack;
            using client_bound_resource_pack = api::packets::client_bound::config::resource_pack_push;
            using select_known_packs = api::packets::server_bound::config::select_known_packs;
            using custom_click_action = api::packets::server_bound::config::custom_click_action;
            using accept_code_of_conduct = api::packets::server_bound::config::accept_code_of_conduct;

            api::packets::send_viewer(*this, [](const client_bound_resource_pack& packet, base_objects::shared_client_data& client) {
                extra_data_t::get(client).pending_resource_packs[packet.uuid] = {.required = packet.forced};
                return false;
            });
            api::packets::processor(*this, [](client_information&& packet, base_objects::shared_client_data& client) {
                client.locale = packet.locale.value;
                client.view_distance = (uint8_t)std::min<uint32_t>(packet.view_distance, api::configuration::get().game_play.view_distance);
                client.chat_mode = (base_objects::shared_client_data::ChatMode)packet.chat_mode.value;
                client.enable_chat_colors = packet.enable_chat_colors;
                client.skin_parts.mask = packet.displayed_skin_parts.get();
                client.main_hand = (base_objects::shared_client_data::MainHand)packet.main_hand.value;
                client.enable_filtering = packet.enable_text_filtering;
                client.allow_server_listings = packet.allow_server_listings;
                client.particle_status = (base_objects::shared_client_data::ParticleStatus)packet.particle_status.value;
                if (client.get_session())
                    client.get_session()->request_buffer(api::configuration::get().protocol.buffer);
                auto& data = extra_data_t::get(client);
                data.ka_solution.set_callback([](int64_t res, base_objects::shared_client_data& client) {
                    client << api::packets::client_bound::config::keep_alive{.keep_alive_id = (uint64_t)res};
                });
                client << api::packets::client_bound::config::select_known_packs{
                    .packs = resources::loaded_packs()
                                 .convert_fn([](auto& it) {
                                     return api::packets::client_bound::config::select_known_packs::pack{
                                         .pack_namespace = it.namespace_,
                                         .id = it.id,
                                         .version = it.version
                                     };
                                 })
                };
                data.packs_requested = true;
                data.ka_solution.start();
            });
            api::packets::processor(*this, [](cookie_response&& packet, base_objects::shared_client_data& client) {
                if (auto plugin = plugin_management.get_bind_cookies(plugin_management_system::registration_on::configuration, packet.key); plugin)
                    if (plugin->on_configuration_cookie(plugin, packet.key, packet.payload ? *packet.payload : list_array<uint8_t>{}, client)) {
                        extra_data_t::get(client).active_plugins.remove(plugin);
                        make_finish(client);
                    }
            });
            api::packets::processor(*this, [](custom_payload&& packet, base_objects::shared_client_data& client) {
                auto it = plugin_management.get_bind_plugin(plugin_management_system::registration_on::configuration, packet.channel);
                if (it == nullptr) {
                    extra_data_t::get(client).active_plugins.remove(it);
                    make_finish(client);
                    return;
                }
                if (it != nullptr)
                    packet.payload.commit();
                if (it->on_configuration_handle(it, packet.channel, packet.payload, client)) {
                    extra_data_t::get(client).active_plugins.remove(it);
                    make_finish(client);
                }
            });
            api::packets::receive_viewer(*this, [](const api::packets::server_bound::config::finish_configuration&, base_objects::shared_client_data& client) {
                if (extra_data_t::get(client).packs_requested) {
                    if (extra_data_t::get(client).active_plugins.empty()) {
                        if (extra_data_t::get(client).pending_resource_packs.empty()) {
                            if (api::configuration::get().game_play.enable_code_of_conduct) {
                                if (extra_data_t::get(client).code_of_conduct_is_accepted)
                                    return false;
                                else
                                    client << api::packets::client_bound::play::disconnect{.reason = "Code of conduct acceptance is required."};
                            } else
                                return false;
                        } else
                            client << api::packets::client_bound::play::disconnect{.reason = "Pending resource packs."};
                    } else
                        client << api::packets::client_bound::play::disconnect{.reason = "Requested more data."};
                } else
                    client << api::packets::client_bound::play::disconnect{.reason = "Nope, gimme packs!"};
                return true;
            });
            api::packets::processor(*this, [](keep_alive&& packet, base_objects::shared_client_data& client) {
                auto delay = extra_data_t::get(client).ka_solution.got_valid_keep_alive((int64_t)packet.keep_alive_id);
                client.packets_state.keep_alive_ping_ms = (int32_t)std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(delay).count(), INT32_MAX);
            });
            api::packets::send_viewer(*this, [](api::packets::client_bound::config::ping&, base_objects::shared_client_data& client) {
                client.packets_state.pong_timer = std::chrono::system_clock::now();
                return true;
            });
            api::packets::processor(*this, [](pong&& packet, base_objects::shared_client_data& client) {
                if (packet.ping_request_id.is_valid)
                    client.ping = std::chrono::duration_cast<std::chrono::milliseconds>(client.packets_state.pong_timer - std::chrono::system_clock::now());
            });
            api::packets::processor(*this, [](resource_pack&& packet, base_objects::shared_client_data& client) {
                auto& data = extra_data_t::get(client);
                auto res = data.pending_resource_packs.find(packet.uuid);
                if (res != data.pending_resource_packs.end()) {
                    switch (packet.result.value) {
                    case resource_pack::result_e::success:
                        client.packets_state.active_resource_packs.insert(packet.uuid);
                        data.pending_resource_packs.erase(res);
                        break;
                    case resource_pack::result_e::accepted:
                    case resource_pack::result_e::downloaded:
                        break;
                    default:
                        if (res->second.required)
                            client << api::packets::client_bound::config::disconnect{.reason = "Resource pack is required"};
                        else
                            data.pending_resource_packs.erase(res);
                    }
                    make_finish(client);
                }
            });
            api::packets::processor(*this, [](select_known_packs&& packet, base_objects::shared_client_data& client) {
                send_registry_data(client);
                send_tags(client);
                plugin_management.inspect_plugin_registration(plugin_management_system::registration_on::configuration, [&client, &packet](plugin_registration_ptr plugin) {
                    if (!plugin->on_configuration(client)) {
                        if (!plugin->on_configuration_got_known_packs(client, packet))
                            extra_data_t::get(client).active_plugins.push_back(plugin);
                    }
                });
                make_finish(client);
            });
            api::packets::processor(*this, [](custom_click_action&& packet, base_objects::shared_client_data& client) {
                api::dialogs::pass_dialog(packet.id, client, std::move(packet.payload));
            });
            api::packets::processor(*this, [](accept_code_of_conduct&& packet, base_objects::shared_client_data& client) {
                extra_data_t::get(client).code_of_conduct_is_accepted = true;
            });
        }
    };
}