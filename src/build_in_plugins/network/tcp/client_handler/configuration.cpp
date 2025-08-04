#include <src/api/configuration.hpp>
#include <src/api/network/tcp.hpp>
#include <src/api/new_packets.hpp>
#include <src/api/tags.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/registers.hpp>
#include <src/resources/registers.hpp>

namespace copper_server::build_in_plugins::network::tcp::client_handler {
    struct tcp_configuration : public PluginAutoRegister<"network/tcp_configuration", tcp_configuration> {
        struct extra_data_t {
            enum class load_state_e {
                to_init,
                await_known_packs,
                await_processing,
                done
            } load_state
                = load_state_e::to_init;
            keep_alive_solution ka_solution;
            list_array<PluginRegistrationPtr> active_plugins;

            static extra_data_t& get(base_objects::SharedClientData& client) {
                if (!client.packets_state.extra_data) {
                    auto allocated = new extra_data_t{.ka_solution = client.get_session()};
                    client.packets_state.extra_data = std::shared_ptr<void>((void*)allocated, [](void* d) { delete reinterpret_cast<extra_data_t*>(d); });
                }
                return *reinterpret_cast<extra_data_t*>(client.packets_state.extra_data.get());
            }
        };

        static void send_tags(base_objects::SharedClientData& client) {
            api::new_packets::client_bound::configuration::update_tags::entry block;
            block.registry_id = "minecraft:block";
            for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::block, "minecraft"))
                block.tags.push_back({.tag_name = id, .values = values.to_container<std::vector<base_objects::var_int32>>()});

            api::new_packets::client_bound::configuration::update_tags::entry item;
            item.registry_id = "minecraft:item";
            for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::item, "minecraft"))
                item.tags.push_back({.tag_name = id, .values = values.to_container<std::vector<base_objects::var_int32>>()});

            api::new_packets::client_bound::configuration::update_tags::entry fluid;
            fluid.registry_id = "minecraft:fluid";
            for (auto& [id, values] : api::tags::view_tag("minecraft:fluid", "minecraft")) {
                fluid.tags.push_back(
                    {.tag_name = id, .values = registers::convert_reg_pro_id("minecraft:fluid", values).to_container<std::vector<base_objects::var_int32>>()}
                );
            }
            api::new_packets::client_bound::configuration::update_tags::entry worldgen_biome;
            worldgen_biome.registry_id = "minecraft:worldgen/biome";
            for (auto& [id, values] : api::tags::view_tag("minecraft:worldgen/biome", "minecraft")) {
                worldgen_biome.tags.push_back(
                    {.tag_name = id, .values = values.convert_fn([](auto& it) { return (base_objects::var_int32)registers::biomes.at(it).id; }).to_container<std::vector>()}
                );
            }
            api::new_packets::client_bound::configuration::update_tags::entry entity_type;
            entity_type.registry_id = "minecraft:entity_type";
            for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::entity_type, "minecraft"))
                entity_type.tags.push_back({.tag_name = id, .values = values.to_container<std::vector<base_objects::var_int32>>()});


            api::new_packets::client_bound::configuration::update_tags::entry game_event;
            game_event.registry_id = "minecraft:game_event";
            for (auto& [id, values] : api::tags::view_tag("minecraft:game_event", "minecraft")) {
                game_event.tags.push_back(
                    {.tag_name = id, .values = registers::convert_reg_pro_id("minecraft:game_event", values).to_container<std::vector<base_objects::var_int32>>()}
                );
            }

            client << api::new_packets::client_bound::configuration::update_tags{
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
            list_array<std::pair<std::string, enbt::value>> fixed_data;
            fixed_data.resize(values.size());
            for (auto& _it : values) {
                auto& [name, it] = *_it;
                if (it.id >= fixed_data.size())
                    throw std::out_of_range("Invalid registry values");
                fixed_data[it.id] = {name, serializer(it)};
            }


            api::new_packets::client_bound::configuration::registry_data res;
            res.registry_id = identifier;
            res.entries.reserve(fixed_data.size());
            fixed_data.for_each([&](const std::string& name, enbt::value& data) {
                api::new_packets::client_bound::configuration::registry_data::entry entry;
                entry.entry_id = name;
                if (data.get_type() != enbt::type::none)
                    if (data.size())
                        entry.data = std::move(data);
                res.entries.push_back(std::move(entry));
            });
            return api::new_packets::encode(std::move(res));
        }

        static void send_registry_data(base_objects::SharedClientData& client) {
            static base_objects::network::response data;
            if (!data.has_data()) {
                { // minecraft:trim_material
                    data += registry_data_serialize_entry<registers::ArmorTrimMaterial>("minecraft:trim_material", registers::armorTrimMaterials_cache, [](registers::ArmorTrimMaterial& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["asset_name"] = it.asset_name;
                        if (std::holds_alternative<std::string>(it.description))
                            element["description"] = std::get<std::string>(it.description);
                        else
                            element["description"] = std::get<Chat>(it.description).ToENBT();
                        return element;
                    });
                }
                { // minecraft:trim_pattern
                    data += registry_data_serialize_entry<registers::ArmorTrimPattern>("minecraft:trim_pattern", registers::armorTrimPatterns_cache, [](registers::ArmorTrimPattern& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["asset_id"] = it.asset_id;
                        if (std::holds_alternative<std::string>(it.description))
                            element["description"] = std::get<std::string>(it.description);
                        else
                            element["description"] = std::get<Chat>(it.description).ToENBT();
                        element["decal"] = it.decal;
                        return element;
                    });
                }
                { // minecraft:worldgen/biome
                    data += registry_data_serialize_entry<registers::Biome>("minecraft:worldgen/biome", registers::biomes_cache, [](registers::Biome& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["has_precipitation"] = it.has_precipitation;
                        element["temperature"] = it.temperature;
                        element["temperature_modifier"] = it.temperature_modifier;
                        element["downfall"] = it.downfall;
                        { //effects
                            enbt::compound effects;
                            effects["fog_color"] = it.effects.fog_color;
                            effects["water_color"] = it.effects.water_color;
                            effects["water_fog_color"] = it.effects.water_fog_color;
                            effects["sky_color"] = it.effects.sky_color;
                            if (it.effects.foliage_color)
                                effects["foliage_color"] = it.effects.foliage_color.value();
                            if (it.effects.grass_color)
                                effects["grass_color"] = it.effects.grass_color.value();
                            if (it.effects.grass_color_modifier)
                                effects["grass_color_modifier"] = it.effects.grass_color_modifier.value();
                            if (it.effects.particle) {
                                enbt::compound particle;
                                particle["probability"] = it.effects.particle->probability;
                                particle["options"] = it.effects.particle->options.options;
                                particle["options"]["type"] = it.effects.particle->options.type;
                                effects["particle"] = std::move(particle);
                            }
                            if (it.effects.ambient_sound) {
                                if (std::holds_alternative<std::string>(*it.effects.ambient_sound))
                                    effects["ambient_sound"] = std::get<std::string>(*it.effects.ambient_sound);
                                else if (std::holds_alternative<registers::Biome::AmbientSound>(*it.effects.ambient_sound)) {
                                    enbt::compound ambient_sound;
                                    ambient_sound["sound"] = std::get<registers::Biome::AmbientSound>(*it.effects.ambient_sound).sound;
                                    ambient_sound["range"] = std::get<registers::Biome::AmbientSound>(*it.effects.ambient_sound).range;
                                    effects["ambient_sound"] = std::move(ambient_sound);
                                }
                            }
                            if (it.effects.mood_sound) {
                                enbt::compound mood_sound;
                                mood_sound["sound"] = it.effects.mood_sound->sound;
                                mood_sound["tick_delay"] = it.effects.mood_sound->tick_delay;
                                mood_sound["block_search_extent"] = it.effects.mood_sound->block_search_extent;
                                mood_sound["offset"] = it.effects.mood_sound->offset;
                                effects["mood_sound"] = std::move(mood_sound);
                            }
                            if (it.effects.additions_sound) {
                                enbt::compound additions_sound;
                                additions_sound["sound"] = it.effects.additions_sound->sound;
                                additions_sound["tick_chance"] = it.effects.additions_sound->tick_chance;
                                effects["additions_sound"] = std::move(additions_sound);
                            }
                            {
                                enbt::fixed_array music_arr;
                                for (auto& music_it : it.effects.music) {
                                    enbt::compound music;
                                    music["sound"] = music_it.sound;
                                    music["min_delay"] = music_it.min_delay;
                                    music["max_delay"] = music_it.max_delay;
                                    music["replace_current_music"] = music_it.replace_current_music;
                                    music_arr.push_back(enbt::compound{{"weight", music_it.music_weight}, {"data", std::move(music)}});
                                }
                                effects["music"] = std::move(music_arr);
                            }
                            element["effects"] = std::move(effects);
                        }
                        return element;
                    });
                }
                { // minecraft:chat_type
                    data += registry_data_serialize_entry<registers::ChatType>("minecraft:chat_type", registers::chatTypes_cache, [](registers::ChatType& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        if (it.chat) {
                            enbt::compound chat;
                            chat["translation_key"] = it.chat->translation_key;
                            if (it.chat->style) {
                                it.chat->style->GetExtra().clear();
                                it.chat->style->SetText("");
                                enbt::value style = it.chat->style->ToENBT();
                                style.remove("text");
                                chat["style"] = std::move(style);
                            }
                            if (std::holds_alternative<std::string>(it.chat->parameters))
                                chat["parameters"] = std::get<std::string>(it.chat->parameters);
                            else
                                chat["parameters"] = std::get<std::vector<std::string>>(it.chat->parameters);
                            element["chat"] = std::move(chat);
                        }
                        if (it.narration) {
                            enbt::compound narration;
                            narration["translation_key"] = it.narration->translation_key;
                            if (std::holds_alternative<std::string>(it.narration->parameters))
                                narration["parameters"] = std::get<std::string>(it.narration->parameters);
                            else
                                narration["parameters"] = std::get<std::vector<std::string>>(it.narration->parameters);
                            element["narration"] = std::move(narration);
                        }
                        return element;
                    });
                }
                { // minecraft:damage_type
                    data += registry_data_serialize_entry<registers::DamageType>("minecraft:damage_type", registers::damageTypes_cache, [](registers::DamageType& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["message_id"] = it.message_id;
                        {
                            const char* scaling = nullptr;
                            switch (it.scaling) {
                            case registers::DamageType::ScalingType::never:
                                scaling = "never";
                                break;
                            case registers::DamageType::ScalingType::when_caused_by_living_non_player:
                                scaling = "when_caused_by_living_non_player";
                                break;
                            case registers::DamageType::ScalingType::always:
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
                            case registers::DamageType::EffectsType::hurt:
                                effect = "hurt";
                                break;
                            case registers::DamageType::EffectsType::thorns:
                                effect = "thorns";
                                break;
                            case registers::DamageType::EffectsType::drowning:
                                effect = "drowning";
                                break;
                            case registers::DamageType::EffectsType::burning:
                                effect = "burning";
                                break;
                            case registers::DamageType::EffectsType::poking:
                                effect = "poking";
                                break;
                            case registers::DamageType::EffectsType::freezing:
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
                            case registers::DamageType::DeathMessageType::_default:
                                death_message_type = "default";
                                break;
                            case registers::DamageType::DeathMessageType::fall_variants:
                                death_message_type = "fall_variants";
                                break;
                            case registers::DamageType::DeathMessageType::intentional_game_design:
                                death_message_type = "intentional_game_design";
                                break;
                            default:
                                break;
                            }
                            if (death_message_type)
                                element["death_message_type"] = death_message_type;
                        }
                        return element;
                    });
                }
                { // minecraft:dimension_type
                    data += registry_data_serialize_entry<registers::DimensionType>("minecraft:dimension_type", registers::dimensionTypes_cache, [](registers::DimensionType& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        if (std::holds_alternative<int32_t>(it.monster_spawn_light_level))
                            element["monster_spawn_light_level"] = std::get<int32_t>(it.monster_spawn_light_level);
                        else
                            element["monster_spawn_light_level"] = std::get<registers::IntegerDistribution>(it.monster_spawn_light_level).get_enbt();
                        if (it.fixed_time)
                            element["fixed_time"] = it.fixed_time.value();
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
                        return element;
                    });
                }
                { // minecraft:wolf_variant
                    data += registry_data_serialize_entry<registers::WolfVariant>("minecraft:wolf_variant", registers::wolfVariants_cache, [](registers::WolfVariant& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["assets"] = it.assets;
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element;
                    });
                }
                { // minecraft:painting_variant
                    data += registry_data_serialize_entry<registers::PaintingVariant>("minecraft:painting_variant", registers::paintingVariants_cache, [](registers::PaintingVariant& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["asset_id"] = it.asset_id;
                        element["height"] = it.height;
                        element["width"] = it.width;
                        element["title"] = it.title.ToENBT();
                        element["author"] = it.author.ToENBT();
                        return element;
                    });
                }
                { // minecraft:instrument
                    data += registry_data_serialize_entry<registers::Instrument>("minecraft:instrument", registers::instruments_cache, [](registers::Instrument& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["range"] = it.range;
                        element["use_duration"] = it.use_duration;
                        element["description"] = it.description.ToENBT();
                        std::visit(
                            [&](auto& it) {
                                using T = std::decay_t<decltype(it)>;
                                if constexpr (base_objects::is_id_source<T>) {
                                    element["sound_event"] = it.to_string();
                                } else {
                                    enbt::compound sound_event;
                                    sound_event["sound_name"] = it.sound_name;
                                    if (it.fixed_range)
                                        sound_event["fixed_range"] = *it.fixed_range;
                                    element["sound_event"] = sound_event;
                                }
                            },
                            it.sound_event
                        );
                        return element;
                    });
                }
                { // minecraft:cat_variant
                    data += registry_data_serialize_entry<registers::EntityVariant>("minecraft:cat_variant", registers::catVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = it.model.value();
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element;
                    });
                }
                { // minecraft:chicken_variant
                    data += registry_data_serialize_entry<registers::EntityVariant>("minecraft:chicken_variant", registers::chickenVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = it.model.value();
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element;
                    });
                }
                { // minecraft:cow_variant
                    data += registry_data_serialize_entry<registers::EntityVariant>("minecraft:cow_variant", registers::cowVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = it.model.value();
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element;
                    });
                }
                { // minecraft:frog_variant
                    data += registry_data_serialize_entry<registers::EntityVariant>("minecraft:frog_variant", registers::frogVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = it.model.value();
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element;
                    });
                }
                { // minecraft:pig_variant
                    data += registry_data_serialize_entry<registers::EntityVariant>("minecraft:pig_variant", registers::pigVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["asset_id"] = it.asset_id;
                        if (it.model)
                            element["model"] = it.model.value();
                        element["spawn_conditions"] = it.spawn_conditions;
                        return element;
                    });
                }
                { // minecraft:wolf_sound_variant
                    data += registry_data_serialize_entry<registers::WolfSoundVariant>("minecraft:wolf_sound_variant", registers::wolfSoundVariants_cache, [](registers::WolfSoundVariant& it) -> enbt::value {
                        if (!it.send_via_network_body)
                            return enbt::value{};
                        enbt::compound element;
                        element["ambient_sound"] = it.ambient_sound;
                        element["death_sound"] = it.death_sound;
                        element["growl_sound"] = it.growl_sound;
                        element["hurt_sound"] = it.hurt_sound;
                        element["pant_sound"] = it.pant_sound;
                        element["whine_sound"] = it.whine_sound;
                        return element;
                    });
                }
            }
            client.sendPacket(base_objects::network::response(data));
        }

        static void make_finish(base_objects::SharedClientData& client) {
            if (extra_data_t::get(client).active_plugins.empty()) {
                if (client.packets_state.pending_resource_packs.empty()) {
                    extra_data_t::get(client).load_state = extra_data_t::load_state_e::done;
                    client << api::new_packets::client_bound::configuration::finish_configuration{};
                }
            }
        }

        void OnRegister(const PluginRegistrationPtr&) override {
            using client_information = api::new_packets::server_bound::configuration::client_information;
            using cookie_response = api::new_packets::server_bound::configuration::cookie_response;
            using custom_payload = api::new_packets::server_bound::configuration::custom_payload;
            using keep_alive = api::new_packets::server_bound::configuration::keep_alive;
            using pong = api::new_packets::server_bound::configuration::pong;
            using resource_pack = api::new_packets::server_bound::configuration::resource_pack;
            using client_bound_resource_pack = api::new_packets::client_bound::configuration::resource_pack_push;
            using select_known_packs = api::new_packets::server_bound::configuration::select_known_packs;
            using custom_click_action = api::new_packets::server_bound::configuration::custom_click_action;

            api::new_packets::register_viewer_client_bound<client_bound_resource_pack>([](client_bound_resource_pack&& packet, base_objects::SharedClientData& client) {
                client.packets_state.pending_resource_packs[packet.uuid] = {.required = packet.forced};
            });
            api::new_packets::register_server_bound_processor<client_information>([](client_information&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).load_state == extra_data_t::load_state_e::to_init) {
                    client.locale = packet.locale;
                    client.view_distance = packet.view_distance;
                    client.chat_mode = (base_objects::SharedClientData::ChatMode)packet.chat_mode.value;
                    client.enable_chat_colors = packet.enable_chat_colors;
                    client.skin_parts.mask = packet.displayed_skin_parts.get();
                    client.main_hand = (base_objects::SharedClientData::MainHand)packet.main_hand.value;
                    client.enable_filtering = packet.enable_text_filtering;
                    client.allow_server_listings = packet.allow_server_listings;
                    client.particle_status = (base_objects::SharedClientData::ParticleStatus)packet.particle_status.value;
                    if (client.get_session())
                        client.get_session()->request_buffer(api::configuration::get().protocol.buffer);
                    extra_data_t::get(client).load_state = extra_data_t::load_state_e::await_known_packs;
                    extra_data_t::get(client).ka_solution.set_callback([](int64_t res, base_objects::SharedClientData& client) {
                        client << api::new_packets::client_bound::configuration::keep_alive{.keep_alive_id = (uint64_t)res};
                    });
                    client << api::new_packets::client_bound::configuration::select_known_packs{
                        .packs = resources::loaded_packs()
                                     .convert_fn([](auto& it) {
                                         return api::new_packets::client_bound::configuration::select_known_packs::pack{
                                             .pack_namespace = it.namespace_,
                                             .id = it.id,
                                             .version = it.version
                                         };
                                     })
                                     .to_container<std::vector>()
                    };
                    extra_data_t::get(client).ka_solution.make_keep_alive_packet();
                } else
                    client << api::new_packets::client_bound::configuration::disconnect{.reason = "Invalid protocol state, 0"};
            });
            api::new_packets::register_server_bound_processor<cookie_response>([](cookie_response&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).load_state == extra_data_t::load_state_e::await_processing) {
                    if (auto plugin = pluginManagement.get_bind_cookies(PluginManagement::registration_on::configuration, packet.key); plugin)
                        if (plugin->OnConfigurationCookie(plugin, packet.key, packet.payload ? *packet.payload : list_array<uint8_t>{}, client)) {
                            extra_data_t::get(client).active_plugins.remove(plugin);
                            make_finish(client);
                        }
                } else
                    client << api::new_packets::client_bound::configuration::disconnect{.reason = "Invalid protocol state, 2"};
            });
            api::new_packets::register_server_bound_processor<custom_payload>([](custom_payload&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).load_state == extra_data_t::load_state_e::await_processing) {
                    auto it = pluginManagement.get_bind_plugin(PluginManagement::registration_on::configuration, packet.channel);
                    if (it != nullptr)
                        if (it->OnConfigurationHandle(it, packet.channel, packet.payload, client)) {
                            extra_data_t::get(client).active_plugins.remove(it);
                            make_finish(client);
                        }
                } else
                    client << api::new_packets::client_bound::configuration::disconnect{.reason = "Invalid protocol state, 2"};
            });
            api::new_packets::register_viewer_server_bound<api::new_packets::server_bound::configuration::finish_configuration>([](api::new_packets::server_bound::configuration::finish_configuration&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).load_state == extra_data_t::load_state_e::done) {
                    if (extra_data_t::get(client).active_plugins.empty()) {
                        if (client.packets_state.pending_resource_packs.empty())
                            return false;
                        else
                            client << api::new_packets::client_bound::play::disconnect{.reason = "Pending resource packs"};
                    } else
                        client << api::new_packets::client_bound::play::disconnect{.reason = "Invalid protocol state, 3"};
                } else
                    client << api::new_packets::client_bound::play::disconnect{.reason = "Invalid protocol state, 3"};
                return true;
            });
            api::new_packets::register_server_bound_processor<keep_alive>([](keep_alive&& packet, base_objects::SharedClientData& client) {
                auto delay = extra_data_t::get(client).ka_solution.got_valid_keep_alive((int64_t)packet.keep_alive_id);
                client.packets_state.keep_alive_ping_ms = (int32_t)std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(delay).count(), INT32_MAX);
            });
            api::new_packets::register_server_bound_processor<pong>([](pong&& packet, base_objects::SharedClientData& client) {
                client.ping = std::chrono::duration_cast<std::chrono::milliseconds>(client.packets_state.pong_timer - std::chrono::system_clock::now());
            });
            api::new_packets::register_server_bound_processor<resource_pack>([](resource_pack&& packet, base_objects::SharedClientData& client) {
                auto res = client.packets_state.pending_resource_packs.find(packet.uuid);
                if (res != client.packets_state.pending_resource_packs.end()) {
                    switch (packet.result.value) {
                    case resource_pack::result_e::success:
                        client.packets_state.active_resource_packs.insert(packet.uuid);
                        client.packets_state.pending_resource_packs.erase(res);
                        break;
                    case resource_pack::result_e::accepted:
                    case resource_pack::result_e::downloaded:
                        break;
                    default:
                        if (res->second.required)
                            client << api::new_packets::client_bound::configuration::disconnect{.reason = "Resource pack is required"};
                        else
                            client.packets_state.pending_resource_packs.erase(res);
                    }
                    make_finish(client);
                }
            });
            api::new_packets::register_server_bound_processor<select_known_packs>([](select_known_packs&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).load_state == extra_data_t::load_state_e::await_known_packs) {
                    send_registry_data(client);
                    send_tags(client);
                    extra_data_t::get(client).load_state = extra_data_t::load_state_e::await_processing;
                    pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::configuration, [&client, &packet](PluginRegistrationPtr plugin) {
                        if (!plugin->OnConfiguration(client)) {
                            if (!plugin->OnConfiguration_gotKnownPacks(client, packet))
                                extra_data_t::get(client).active_plugins.push_back(plugin);
                        }
                    });
                    make_finish(client);
                } else
                    client << api::new_packets::client_bound::configuration::disconnect{.reason = "Invalid protocol state, 1"};
            });
            api::new_packets::register_server_bound_processor<custom_click_action>([](custom_click_action&& packet, base_objects::SharedClientData& client) {
                //TODO
            });
        }
    };
}