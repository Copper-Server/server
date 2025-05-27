#include <src/api/configuration.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/mojang/session_server.hpp>
#include <src/api/network.hpp>
#include <src/api/protocol.hpp>

#include <src/base_objects/network/tcp/accept_packet_registry.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>

#include <src/build_in_plugins/network/tcp/protocol/770/writers_readers.hpp>
#include <src/build_in_plugins/network/tcp/protocol/shared_encoding_packets.hpp>

#include <src/plugin/main.hpp>

#include <src/api/packets.hpp>
#include <src/build_in_plugins/network/tcp/client_handler/abstract.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>

#include <src/registers.hpp>

namespace copper_server::build_in_plugins::network::tcp::protocol::play_770 {
    namespace encoding {
        class login_functions : public api::packets::registry::login_functions {
            base_objects::network::response login(int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data) override {
                base_objects::network::response::item packet;
                packet.write_id(0x04);
                packet.write_var32(plugin_message_id);
                packet.write_identifier(chanel);
                packet.write_direct(data);
                return packet;
            }

            base_objects::network::response kick(const Chat& reason) override {
                base_objects::network::response::item packet;
                packet.write_id(0x00);
                packet.write_direct(reason.ToTextComponent());
                return base_objects::network::response::disconnect({std::move(packet)});
            }

            base_objects::network::response disableCompression() override {
                return base_objects::network::response::enable_compress_answer(list_array<uint8_t>::concat(0x03, 0), 0);
            }

            base_objects::network::response setCompression(int32_t threshold) override {
                list_array<uint8_t> packet;
                packet.push_back(0x03);
                WriteVar<int32_t>(threshold, packet);
                return base_objects::network::response::enable_compress_answer(std::move(packet), threshold);
            }

            base_objects::network::response requestCookie(const std::string& key) override {
                base_objects::network::response::item packet;
                packet.write_id(0x05);
                packet.write_string(key);
                return packet;
            }

            base_objects::network::response loginSuccess(base_objects::SharedClientData& client) override {
                if (api::configuration::get().server.offline_mode)
                    client.data = api::mojang::get_session_server().hasJoined(client.name, "", false);
                if (!client.data)
                    return kick("Internal error");
                base_objects::network::response::item packet;
                packet.write_id(0x02);
                packet.write_value(client.data->uuid);
                packet.write_string(client.name, 16);
                auto& properties = client.data->properties;
                packet.write_var32(properties.size());
                for (auto& it : properties) {
                    packet.write_string(it.name, 32767);
                    packet.write_string(it.value, 32767);
                    packet.write_value(it.signature.has_value());
                    if (it.signature.has_value())
                        packet.write_string(*it.signature, 32767);
                }
                return packet;
            }

            base_objects::network::response encryptionRequest(const std::string& server_id, uint8_t (&verify_token)[4]) override {
                auto public_key = api::network::tcp::public_key_buffer();
                base_objects::network::response::item packet;
                packet.write_id(0x01);
                packet.write_string(server_id, 20);
                packet.write_var32_check(public_key.size());
                packet.write_direct((const uint8_t*)public_key.data(), public_key.size());
                packet.write_var32(4);
                packet.write_direct(verify_token, 4);
                packet.write_value(!api::configuration::get().server.offline_mode);
                return packet;
            }
        };

        class configuration_functions : public api::packets::registry::configuration_functions {
            base_objects::network::response requestCookie(const std::string& key) override {
                base_objects::network::response::item packet;
                packet.write_id(0x00);
                packet.write_var32(key.size());
                packet.write_direct(key);
                return packet;
            }

            base_objects::network::response configuration(const std::string& chanel, const list_array<uint8_t>& data) override {
                base_objects::network::response::item packet;
                packet.write_id(0x01);
                packet.write_identifier(chanel);
                packet.write_direct(data);
                return packet;
            }

            base_objects::network::response kick(const Chat& reason) override {
                base_objects::network::response::item packet;
                packet.write_id(0x02);
                packet.write_direct(reason.ToTextComponent());
                return base_objects::network::response::disconnect({std::move(packet)});
            }

            base_objects::network::response finish() override {
                base_objects::network::response::item packet;
                packet.write_id(0x03);
                return packet;
            }

            base_objects::network::response keep_alive(int64_t keep_alive_packet) override {
                base_objects::network::response::item packet;
                packet.write_id(0x04);
                packet.write_value(keep_alive_packet);
                return packet;
            }

            base_objects::network::response ping(int32_t excepted_pong) override {
                base_objects::network::response::item packet;
                packet.write_id(0x05);
                packet.write_value(excepted_pong);
                return packet;
            }

            template <class RegistryT, class FN>
            static base_objects::network::response::item registry_data_serialize_entry(const std::string& identifier, list_array<typename std::unordered_map<std::string, RegistryT>::iterator>& values, FN&& serializer) {
                list_array<std::pair<std::string, enbt::value>> fixed_data;
                fixed_data.resize(values.size());
                for (auto& _it : values) {
                    auto& [name, it] = *_it;
                    if (it.id >= fixed_data.size())
                        throw std::out_of_range("Invalid registry values");
                    fixed_data[it.id] = {name, serializer(it)};
                }

                base_objects::network::response::item packet;
                packet.write_id(0x07);
                packet.write_identifier(identifier);
                packet.write_var32_check(fixed_data.size());
                fixed_data.for_each([&](const std::string& name, enbt::value& data) {
                    packet.write_identifier(name);
                    if (data.get_type() == enbt::type::none) {
                        packet.write_value(false);
                    } else {
                        packet.write_value(bool(data.size()));
                        if (data.size())
                            packet.write_direct(NBT::build(data).get_as_network());
                    }
                });
                return packet;
            }

            base_objects::network::response registry_data() override {
                using namespace registers;
                static list_array<base_objects::network::response::item> data;
                if (data.empty()) {
                    { // minecraft:trim_material
                        data.push_back(registry_data_serialize_entry<ArmorTrimMaterial>("minecraft:trim_material", registers::armorTrimMaterials_cache, [](registers::ArmorTrimMaterial& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["asset_name"] = it.asset_name;
                            if (std::holds_alternative<std::string>(it.description))
                                element["description"] = std::get<std::string>(it.description);
                            else
                                element["description"] = std::get<Chat>(it.description).ToENBT();
                            return element;
                        }));
                    }
                    { // minecraft:trim_pattern
                        data.push_back(registry_data_serialize_entry<ArmorTrimPattern>("minecraft:trim_pattern", registers::armorTrimPatterns_cache, [](registers::ArmorTrimPattern& it) -> enbt::value {
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
                        }));
                    }
                    { // minecraft:worldgen/biome
                        data.push_back(registry_data_serialize_entry<Biome>("minecraft:worldgen/biome", registers::biomes_cache, [](registers::Biome& it) -> enbt::value {
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
                                    else if (std::holds_alternative<Biome::AmbientSound>(*it.effects.ambient_sound)) {
                                        enbt::compound ambient_sound;
                                        ambient_sound["sound"] = std::get<Biome::AmbientSound>(*it.effects.ambient_sound).sound;
                                        ambient_sound["range"] = std::get<Biome::AmbientSound>(*it.effects.ambient_sound).range;
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
                        }));
                    }
                    { // minecraft:chat_type
                        data.push_back(registry_data_serialize_entry<ChatType>("minecraft:chat_type", registers::chatTypes_cache, [](registers::ChatType& it) -> enbt::value {
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
                        }));
                    }
                    { // minecraft:damage_type
                        data.push_back(registry_data_serialize_entry<DamageType>("minecraft:damage_type", registers::damageTypes_cache, [](registers::DamageType& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["message_id"] = it.message_id;
                            {
                                const char* scaling = nullptr;
                                switch (it.scaling) {
                                case DamageType::ScalingType::never:
                                    scaling = "never";
                                    break;
                                case DamageType::ScalingType::when_caused_by_living_non_player:
                                    scaling = "when_caused_by_living_non_player";
                                    break;
                                case DamageType::ScalingType::always:
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
                                case DamageType::EffectsType::hurt:
                                    effect = "hurt";
                                    break;
                                case DamageType::EffectsType::thorns:
                                    effect = "thorns";
                                    break;
                                case DamageType::EffectsType::drowning:
                                    effect = "drowning";
                                    break;
                                case DamageType::EffectsType::burning:
                                    effect = "burning";
                                    break;
                                case DamageType::EffectsType::poking:
                                    effect = "poking";
                                    break;
                                case DamageType::EffectsType::freezing:
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
                                case DamageType::DeathMessageType::_default:
                                    death_message_type = "default";
                                    break;
                                case DamageType::DeathMessageType::fall_variants:
                                    death_message_type = "fall_variants";
                                    break;
                                case DamageType::DeathMessageType::intentional_game_design:
                                    death_message_type = "intentional_game_design";
                                    break;
                                default:
                                    break;
                                }
                                if (death_message_type)
                                    element["death_message_type"] = death_message_type;
                            }
                            return element;
                        }));
                    }
                    { // minecraft:dimension_type
                        data.push_back(registry_data_serialize_entry<DimensionType>("minecraft:dimension_type", registers::dimensionTypes_cache, [](registers::DimensionType& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            if (std::holds_alternative<int32_t>(it.monster_spawn_light_level))
                                element["monster_spawn_light_level"] = std::get<int32_t>(it.monster_spawn_light_level);
                            else
                                element["monster_spawn_light_level"] = std::get<IntegerDistribution>(it.monster_spawn_light_level).get_enbt();
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
                        }));
                    }
                    { // minecraft:wolf_variant
                        data.push_back(registry_data_serialize_entry<WolfVariant>("minecraft:wolf_variant", registers::wolfVariants_cache, [](registers::WolfVariant& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["assets"] = it.assets;
                            element["spawn_conditions"] = it.spawn_conditions;
                            return element;
                        }));
                    }
                    { // minecraft:painting_variant
                        data.push_back(registry_data_serialize_entry<PaintingVariant>("minecraft:painting_variant", registers::paintingVariants_cache, [](registers::PaintingVariant& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["asset_id"] = it.asset_id;
                            element["height"] = it.height;
                            element["width"] = it.width;
                            element["title"] = it.title.ToENBT();
                            element["author"] = it.author.ToENBT();
                            return element;
                        }));
                    }
                    { // minecraft:instrument
                        data.push_back(registry_data_serialize_entry<Instrument>("minecraft:instrument", registers::instruments_cache, [](registers::Instrument& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["range"] = it.range;
                            element["use_duration"] = it.use_duration;
                            element["description"] = it.description.ToENBT();
                            std::visit(
                                [&](auto& it) {
                                    using T = std::decay_t<decltype(it)>;
                                    if constexpr (std::is_same_v<T, std::string>) {
                                        element["sound_event"] = it;
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
                        }));
                    }
                    { // minecraft:cat_variant
                        data.push_back(registry_data_serialize_entry<EntityVariant>("minecraft:cat_variant", registers::catVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["asset_id"] = it.asset_id;
                            if (it.model)
                                element["model"] = it.model.value();
                            element["spawn_conditions"] = it.spawn_conditions;
                            return element;
                        }));
                    }
                    { // minecraft:chicken_variant
                        data.push_back(registry_data_serialize_entry<EntityVariant>("minecraft:chicken_variant", registers::chickenVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["asset_id"] = it.asset_id;
                            if (it.model)
                                element["model"] = it.model.value();
                            element["spawn_conditions"] = it.spawn_conditions;
                            return element;
                        }));
                    }
                    { // minecraft:cow_variant
                        data.push_back(registry_data_serialize_entry<EntityVariant>("minecraft:cow_variant", registers::cowVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["asset_id"] = it.asset_id;
                            if (it.model)
                                element["model"] = it.model.value();
                            element["spawn_conditions"] = it.spawn_conditions;
                            return element;
                        }));
                    }
                    { // minecraft:frog_variant
                        data.push_back(registry_data_serialize_entry<EntityVariant>("minecraft:frog_variant", registers::frogVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["asset_id"] = it.asset_id;
                            if (it.model)
                                element["model"] = it.model.value();
                            element["spawn_conditions"] = it.spawn_conditions;
                            return element;
                        }));
                    }
                    { // minecraft:pig_variant
                        data.push_back(registry_data_serialize_entry<EntityVariant>("minecraft:pig_variant", registers::pigVariants_cache, [](registers::EntityVariant& it) -> enbt::value {
                            if (!it.send_via_network_body)
                                return enbt::value{};
                            enbt::compound element;
                            element["asset_id"] = it.asset_id;
                            if (it.model)
                                element["model"] = it.model.value();
                            element["spawn_conditions"] = it.spawn_conditions;
                            return element;
                        }));
                    }
                    { // minecraft:wolf_sound_variant
                        data.push_back(registry_data_serialize_entry<WolfSoundVariant>("minecraft:wolf_sound_variant", registers::wolfSoundVariants_cache, [](registers::WolfSoundVariant& it) -> enbt::value {
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
                        }));
                    }
                }
                return base_objects::network::response::answer(data);
            }

            base_objects::network::response registry_item(const std::string& registry_id, list_array<std::pair<std::string, enbt::compound>>& entries) override {
                base_objects::network::response::item packet;
                packet.write_id(0x07);
                packet.write_identifier(registry_id);
                packet.write_var32_check(entries.size());
                entries.for_each([&](const std::string& name, enbt::compound& data) {
                    packet.write_identifier(name);
                    packet.write_value(bool(data.size()));
                    if (data.size())
                        packet.write_direct(NBT::build((enbt::value&)data).get_as_network());
                });
                return packet;
            }

            base_objects::network::response resetChat() override {
                base_objects::network::response::item packet;
                packet.write_id(0x05);
                return packet;
            }

            base_objects::network::response removeResourcePacks() override {
                base_objects::network::response::item packet;
                packet.write_id(0x08);
                packet.write_value(false);
                return packet;
            }

            base_objects::network::response removeResourcePack(const enbt::raw_uuid& pack_id) override {
                base_objects::network::response::item packet;
                packet.write_id(0x08);
                packet.write_value(true);
                packet.write_value(pack_id);
                return packet;
            }

            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced) override {
                base_objects::network::response::item packet;
                packet.write_id(0x09);
                packet.write_value(pack_id);
                packet.write_string(url, 32767);
                packet.write_string(hash, 40);
                packet.write_value(forced);
                packet.write_value(false);
                client.packets_state.pending_resource_packs[pack_id] = {.required = forced};
                return packet;
            }

            base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt) override {
                base_objects::network::response::item packet;
                packet.write_id(0x09);
                packet.write_value(pack_id);
                packet.write_string(url, 32767);
                packet.write_string(hash, 40);
                packet.write_value(forced);
                packet.write_value(true);
                packet.write_direct(prompt.ToTextComponent());
                client.packets_state.pending_resource_packs[pack_id] = {.required = forced};
                return packet;
            }

            base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload) override {
                if (payload.size() > 5120)
                    throw std::runtime_error("Payload size is too big");
                base_objects::network::response::item packet;
                packet.write_id(0x0A);
                packet.write_identifier(key);
                packet.write_direct(payload);
                return packet;
            }

            base_objects::network::response transfer(const std::string& host, int32_t port) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0B);
                packet.write_string(host);
                packet.write_var32(port);
                return packet;
            }

            base_objects::network::response setFeatureFlags(const list_array<std::string>& features) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0C);
                packet.write_var32_check(features.size());
                for (auto& it : features)
                    packet.write_identifier(it);
                return packet;
            }

            base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0D);
                packet.write_var32_check(tags_entries.size());
                for (auto& [registry, tags] : tags_entries) {
                    packet.write_identifier(registry);
                    packet.write_var32_check(tags.size());
                    for (auto& it : tags) {
                        packet.write_identifier(it.tag_name);
                        packet.write_var32_check(it.entires.size());
                        for (auto& entry : it.entires)
                            packet.write_var32(entry);
                    }
                }
                return packet;
            }

            base_objects::network::response knownPacks(const list_array<base_objects::data_packs::known_pack>& packs) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0E);
                packet.write_var32_check(packs.size());
                for (const auto& pack : packs) {
                    packet.write_string(pack.namespace_);
                    packet.write_string(pack.id);
                    packet.write_string(pack.version);
                }
                return packet;
            }

            base_objects::network::response custom_report(const list_array<std::pair<std::string, std::string>>& values) override {
                if (values.size() > 32)
                    throw std::invalid_argument("Report cannot contain more than 32 entry's.");
                base_objects::network::response::item packet;
                packet.write_id(0x0F);
                packet.write_var32(values.size());
                for (auto&& [title, desc] : values) {
                    packet.write_string(title, 128);
                    packet.write_string(desc, 4096);
                }
                return packet;
            }

            base_objects::network::response server_links(const list_array<base_objects::packets::server_link>& links) override {
                base_objects::network::response::item packet;
                packet.write_id(0x10);
                packet.write_var32_check(links.size());
                for (auto&& [label, url] : links) {
                    std::visit(
                        [&](auto& item) {
                            using type = std::decay_t<decltype(item)>;
                            if constexpr (std::is_same_v<type, base_objects::packets::server_link::label_type>) {
                                packet.write_value(false);
                                packet.write_var32((int32_t)item);
                            } else {
                                packet.write_value(true);
                                packet.write_direct(item.ToTextComponent());
                            }
                        },
                        label
                    );
                    packet.write_string(url, 2048);
                }
                return packet;
            }
        };

        class play_functions : public api::packets::registry::play_functions {
            base_objects::network::response bundleResponse(base_objects::network::response&& response) override {
                if (response.do_disconnect)
                    return std::move(response);
                else {
                    response.data.remove_if([](auto& it) {
                        if (it.data.size() == 1)
                            return it.data[0] == 0x00;
                        else
                            return false;
                    });
                    if (response.data.empty())
                        return base_objects::network::response::empty();
                    else if (response.data.size() == 1)
                        return std::move(response);
                    base_objects::network::response answer(base_objects::network::response::answer({list_array<uint8_t>::concat(0x00)}));
                    answer.data.push_back(response.data);
                    answer.data.push_back({0x00});
                    return answer;
                }
            }

            base_objects::network::response spawnEntity(const base_objects::entity& entity, uint16_t protocol) override {
                if (protocol == UINT16_MAX)
                    protocol = 770;
                base_objects::network::response::item packet;
                packet.write_id(0x01);
                auto view = base_objects::entity_data::view(entity);
                packet.write_var32(api::entity_id_map::get_id(entity.id));
                packet.write_value(entity.id);
                packet.write_var32(view.internal_entity_aliases.at(protocol));
                packet.write_value(entity.position.x);
                packet.write_value(entity.position.y);
                packet.write_value(entity.position.z);
                {
                    auto res = util::to_yaw_pitch(entity.rotation);
                    packet.write_value((int8_t)res.x);
                    packet.write_value((int8_t)res.y);
                }
                packet.write_value((int8_t)util::to_yaw_pitch(entity.head_rotation).y);
                if (view.get_object_field) {
                    if (auto value = view.get_object_field(entity))
                        packet.write_var32(value);
                } else
                    packet.write_var32(0);
                auto velocity = util::minecraft::packets::velocity(entity.motion);
                packet.write_value(velocity.x);
                packet.write_value(velocity.y);
                packet.write_value(velocity.z);
                return packet;
            }

            base_objects::network::response entityAnimation(const base_objects::entity& entity, base_objects::entity_animation animation) override {
                base_objects::network::response::item packet;
                packet.write_id(0x02);
                packet.write_var32(api::entity_id_map::get_id(entity.id));
                packet.write_value((uint8_t)animation);
                return packet;
            }

            base_objects::network::response awardStatistics(const list_array<base_objects::packets::statistics>& statistics) override {
                base_objects::network::response::item packet;
                packet.write_id(0x03);
                packet.write_var32_check(statistics.size());
                for (auto& [category_id, statistic_id, value] : statistics) {
                    packet.write_var32(category_id);
                    packet.write_var32(statistic_id);
                    packet.write_var32(value);
                }
                return packet;
            }

            base_objects::network::response acknowledgeBlockChange(base_objects::SharedClientData& client) override {
                base_objects::network::response::item packet;
                packet.write_id(0x04);
                packet.write_var32(client.packets_state.play_data->current_block_sequence_id);
                return packet;
            }

            base_objects::network::response setBlockDestroyStage(const base_objects::entity& entity, base_objects::position block, uint8_t stage) override {
                base_objects::network::response::item packet;
                packet.write_id(0x05);
                packet.write_var32(api::entity_id_map::get_id(entity.id));
                packet.write_value(block.raw);
                packet.write_value(stage);
                return packet;
            }

            base_objects::network::response blockEntityData(base_objects::position block, int32_t type, const enbt::value& data) override {
                base_objects::network::response::item packet;
                packet.write_id(0x06);
                packet.write_value(block.raw);
                packet.write_var32(type);
                packet.write_direct(NBT::build(data).get_as_network());
                return packet;
            }

            //block_type is from "minecraft:block" registry, not a block state.
            base_objects::network::response blockAction(base_objects::position block, int32_t action_id, int32_t param, int32_t block_type) override {
                base_objects::network::response::item packet;
                packet.write_id(0x07);
                packet.write_value(block.raw);
                packet.write_var32(action_id);
                packet.write_var32(param);
                packet.write_var32(block_type);
                return packet;
            }

            //block_type is from "minecraft:block" registry, not a block state.
            base_objects::network::response blockUpdate(base_objects::position block, int32_t block_type) override {
                base_objects::network::response::item packet;
                packet.write_id(0x08);
                packet.write_value(block.raw);
                packet.write_var32(block_type);
                return packet;
            }

            base_objects::network::response bossBarAdd(const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags) override {
                base_objects::network::response::item packet;
                packet.write_id(0x09);
                packet.write_value(id);
                packet.write_value(0ui8);
                packet.write_string(title.ToStr(), 32767);
                packet.write_value(health);
                packet.write_var32(color);
                packet.write_var32(division);
                packet.write_value(flags);
                return packet;
            }

            base_objects::network::response bossBarRemove(const enbt::raw_uuid& id) override {
                base_objects::network::response::item packet;
                packet.write_id(0x09);
                packet.write_value(id);
                packet.write_value(0ui8);
                return packet;
            }

            base_objects::network::response bossBarUpdateHealth(const enbt::raw_uuid& id, float health) override {
                base_objects::network::response::item packet;
                packet.write_id(0x09);
                packet.write_value(id);
                packet.write_value(2ui8);
                packet.write_value(health);
                return packet;
            }

            base_objects::network::response bossBarUpdateTitle(const enbt::raw_uuid& id, const Chat& title) override {
                base_objects::network::response::item packet;
                packet.write_id(0x09);
                packet.write_value(id);
                packet.write_value(3ui8);
                packet.write_string(title.ToStr(), 32767);
                return packet;
            }

            base_objects::network::response bossBarUpdateStyle(const enbt::raw_uuid& id, int32_t color, int32_t division) override {
                base_objects::network::response::item packet;
                packet.write_id(0x09);
                packet.write_value(id);
                packet.write_value(4ui8);
                packet.write_value(color);
                packet.write_value(division);
                return packet;
            }

            base_objects::network::response bossBarUpdateFlags(const enbt::raw_uuid& id, uint8_t flags) override {
                base_objects::network::response::item packet;
                packet.write_id(0x09);
                packet.write_value(id);
                packet.write_value(5ui8);
                packet.write_value(flags);
                return packet;
            }

            base_objects::network::response changeDifficulty(uint8_t difficulty, bool locked) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0A);
                packet.write_value(difficulty);
                packet.write_value(locked);
                return packet;
            }

            base_objects::network::response chunkBatchFinished(int32_t count) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0B);
                packet.write_var32(count);
                return packet;
            }

            base_objects::network::response chunkBatchStart() override {
                base_objects::network::response::item packet;
                packet.write_id(0x0C);
                return packet;
            }

            base_objects::network::response chunkBiomes(list_array<base_objects::chunk::chunk_biomes>& chunk) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0D);
                packet.write_var32_check(chunk.size());
                for (auto& section : chunk) {
                    packet.write_var32(section.z);
                    packet.write_var32(section.x);
                    packet.write_var32_check(section.biomes.size());
                    for (auto& biome : section.biomes)
                        packet.write_direct(biome.serialize());
                }
                return packet;
            }

            base_objects::network::response clearTitles(bool reset) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0E);
                packet.write_value(reset);
                return packet;
            }

            base_objects::network::response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions) override {
                base_objects::network::response::item packet;
                packet.write_id(0x0F);
                packet.write_var32(transaction_id);
                packet.write_var32(start_pos);
                packet.write_var32(length);
                packet.write_var32_check(suggestions.size());
                for (auto& [suggestion, tooltip] : suggestions) {
                    packet.write_string(suggestion, 32767);
                    packet.write_value(!!tooltip);
                    if (tooltip)
                        packet.write_direct(tooltip->ToTextComponent());
                }
                return packet;
            }

            base_objects::network::response commands(int32_t root_id, const list_array<uint8_t>& compiled_graph) override {
                base_objects::network::response::item packet;
                packet.write_id(0x10);
                packet.write_direct(compiled_graph);
                packet.write_var32(root_id);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response closeContainer(uint8_t container_id) override {
                base_objects::network::response::item packet;
                packet.write_id(0x11);
                packet.write_value(container_id);
                return packet;
            }

            base_objects::network::response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item) override {
                list_array<uint8_t> packet;
                packet.push_back(0x12);
                packet.push_back(windows_id);
                WriteVar<int32_t>(state_id, packet);
                WriteVar<int32_t>(slots.size(), packet);
                for (auto& it : slots)
                    reader::WriteSlot(packet, it);
                reader::WriteSlot(packet, carried_item);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value) override {
                base_objects::network::response::item packet;
                packet.write_id(0x13);
                packet.write_value(windows_id);
                packet.write_value(property);
                packet.write_value(value);
                return packet;
            }

            base_objects::network::response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 1 + 4 + 2 + (bool)item);
                packet.push_back(0x14);
                packet.push_back(windows_id);
                WriteVar<int32_t>(state_id, packet);
                WriteValue<int16_t>(slot, packet);
                reader::WriteSlot(packet, item);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response cookieRequest(const std::string& key) override {
                base_objects::network::response::item packet;
                packet.write_id(0x15);
                packet.write_identifier(key);
                return packet;
            }

            base_objects::network::response setCooldown(int32_t item_id, int32_t cooldown) override {
                base_objects::network::response::item packet;
                packet.write_id(0x16);
                packet.write_var32(item_id);
                packet.write_var32(cooldown);
                return packet;
            }

            base_objects::network::response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions) override {
                base_objects::network::response::item packet;
                packet.write_id(0x17);
                packet.write_var32(action);
                packet.write_var32(count);
                for (auto& it : suggestions)
                    packet.write_string(it, 32767);
                return packet;
            }

            base_objects::network::response customPayload(const std::string& channel, const list_array<uint8_t>& data) override {
                base_objects::network::response::item packet;
                packet.write_id(0x18);
                packet.write_string(channel, 32767);
                packet.write_direct(data);
                return packet;
            }

            base_objects::network::response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<util::VECTOR> xyz) override {
                base_objects::network::response::item packet;
                packet.write_id(0x19);
                packet.write_var32(entity_id);
                packet.write_var32(source_type_id);
                packet.write_var32(source_cause_id);
                packet.write_var32(source_direct_id);
                packet.write_value((bool)xyz);
                if (xyz) {
                    packet.write_value((float)xyz->x);
                    packet.write_value((float)xyz->y);
                    packet.write_value((float)xyz->z);
                }
                return packet;
            }

            base_objects::network::response debugSample(const list_array<uint64_t>& sample, int32_t sample_type) override { //ADD
                base_objects::network::response::item packet;
                packet.write_id(0x1A);
                packet.write_array(sample);
                packet.write_var32(sample_type);
                return packet;
            }

            base_objects::network::response deleteMessageBySignature(uint8_t (&signature)[256]) override {
                list_array<uint8_t> packet;
                packet.reserve(2 + 256);
                packet.push_back(0x1B);
                packet.push_back(0);
                packet.push_back(signature, 256);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response deleteMessageByID(int32_t message_id) override {
                list_array<uint8_t> packet;
                packet.reserve(12 + 4);
                packet.push_back(0x1B);
                if (message_id == -1)
                    throw std::runtime_error("message id can't be -1");
                WriteVar<int32_t>(message_id + 1, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response kick(const Chat& reason) override {
                return base_objects::network::response::disconnect({list_array<uint8_t>::concat(0x1C, reason.ToTextComponent())});
            }

            base_objects::network::response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 2 + 2 + 1);
                packet.push_back(0x1D);
                packet.push_back(message.ToTextComponent());
                WriteVar<int32_t>(chat_type, packet);
                packet.push_back(sender.ToTextComponent());
                packet.push_back((bool)target_name);
                if (target_name)
                    packet.push_back(target_name->ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response entityEvent(int32_t entity_id, base_objects::entity_event entity_status) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 1);
                packet.push_back(0x1E);
                WriteValue<int32_t>(entity_id, packet);
                packet.push_back((uint8_t)entity_status);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response teleportEntity(int32_t entity_id, util::VECTOR pos, float yaw, float pitch, bool on_ground) override {
                base_objects::packets::teleport_flags flags;
                flags.velocity_x_relative = true;
                flags.velocity_y_relative = true;
                return teleportEntityEX(entity_id, pos, {0, 0, 0}, yaw, pitch, on_ground, flags);
            }

            //minecraft:entity_position_sync
            base_objects::network::response teleportEntityVelocity(int32_t entity_id, util::VECTOR pos, util::VECTOR velocity, float yaw, float pitch, bool on_ground) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 8 * 6 + 4 * 2 + 1);
                packet.push_back(0x1F);
                WriteVar<int32_t>(entity_id, packet);
                WriteValue<double>(pos.x, packet);
                WriteValue<double>(pos.y, packet);
                WriteValue<double>(pos.z, packet);
                WriteValue<double>(velocity.x, packet);
                WriteValue<double>(velocity.y, packet);
                WriteValue<double>(velocity.z, packet);
                WriteValue<float>(yaw, packet);
                WriteValue<float>(pitch, packet);
                packet.push_back(on_ground);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response explosion(
                util::VECTOR pos,
                float strength,
                const list_array<util::XYZ<int8_t>>& affected_blocks,
                util::VECTOR player_motion,
                int32_t block_interaction,
                int32_t small_explosion_particle_id,
                const base_objects::particle_data& small_explosion_particle_data,
                int32_t large_explosion_particle_id,
                const base_objects::particle_data& large_explosion_particle_data,
                const std::string& sound_name,
                std::optional<float> fixed_range
            ) {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 4 + 4 + 4 * 3 * affected_blocks.size() + 4 * 3);
                packet.push_back(0x20);
                WriteValue<float>(pos.x, packet);
                WriteValue<float>(pos.y, packet);
                WriteValue<float>(pos.z, packet);
                WriteValue<float>(strength, packet);
                WriteVar<int32_t>(affected_blocks.size(), packet);
                for (auto& it : affected_blocks) {
                    WriteValue<int8_t>(it.x, packet);
                    WriteValue<int8_t>(it.y, packet);
                    WriteValue<int8_t>(it.z, packet);
                }
                WriteValue<float>(player_motion.x, packet);
                WriteValue<float>(player_motion.y, packet);
                WriteValue<float>(player_motion.z, packet);
                WriteVar<int32_t>(block_interaction, packet);
                WriteVar<int32_t>(small_explosion_particle_id, packet);
                for (auto& it : small_explosion_particle_data.data)
                    std::visit(
                        [&packet](auto&& arg) {
                            if constexpr (std::is_same_v<decltype(arg), std::string>)
                                WriteString(packet, arg, 32767);
                            else
                                WriteValue(arg, packet);
                        },
                        it
                    );
                WriteVar<int32_t>(large_explosion_particle_id, packet);
                for (auto& it : large_explosion_particle_data.data)
                    std::visit(
                        [&packet](auto&& arg) {
                            if constexpr (std::is_same_v<decltype(arg), std::string>)
                                WriteString(packet, arg, 32767);
                            else
                                WriteValue(arg, packet);
                        },
                        it
                    );
                WriteIdentifier(packet, sound_name);
                packet.push_back((bool)fixed_range);
                if (fixed_range)
                    WriteValue<float>(*fixed_range, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response unloadChunk(int32_t x, int32_t z) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 2);
                packet.push_back(0x21);
                WriteValue<int32_t>(z, packet);
                WriteValue<int32_t>(x, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response gameEvent(base_objects::packets::game_event_id event_id, float value) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 1 + 4);
                packet.push_back(0x22);
                packet.push_back((uint8_t)event_id);
                WriteValue<float>(value, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 2 + 16);
                packet.push_back(0x23);
                packet.push_back(window_id);
                WriteVar<int32_t>(slots, packet);
                WriteValue<int32_t>(entity_id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response hurtAnimation(int32_t entity_id, float yaw) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 4);
                packet.push_back(0x24);
                WriteVar<int32_t>(entity_id, packet);
                WriteValue<float>(yaw, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 8 * 2 + 8 * 2 + 8 + 8 + 8 + 4 * 3);
                packet.push_back(0x25);
                WriteValue<double>(x, packet);
                WriteValue<double>(z, packet);
                WriteValue<double>(old_diameter, packet);
                WriteValue<double>(new_diameter, packet);
                WriteVar<int64_t>(speed_ms, packet);
                WriteVar<int32_t>(portal_teleport_boundary, packet);
                WriteVar<int32_t>(warning_blocks, packet);
                WriteVar<int32_t>(warning_time, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            //internal use
            base_objects::network::response keepAlive(int64_t id) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 8);
                packet.push_back(0x26);
                WriteValue<int64_t>(id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateChunkDataWLights(
                int32_t chunk_x,
                int32_t chunk_z,
                const NBT& heightmaps,
                const std::vector<uint8_t>& data,
                //block_entries not implemented, this is legal to send later by blockEntityData,
                const bit_list_array<>& sky_light_mask,
                const bit_list_array<>& block_light_mask,
                const bit_list_array<>& empty_skylight_mask,
                const bit_list_array<>& empty_block_light_mask,
                const list_array<std::vector<uint8_t>>& sky_light_arrays,
                const list_array<std::vector<uint8_t>>& block_light_arrays
            ) {
                if (
                    sky_light_mask.need_commit() | block_light_mask.need_commit() | empty_skylight_mask.need_commit() | empty_block_light_mask.need_commit()
                )
                    throw std::runtime_error("bit_list_array not commited");

                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 2 + sky_light_mask.data().size() + block_light_mask.data().size() + empty_skylight_mask.data().size() + empty_block_light_mask.data().size());
                packet.push_back(0x27);
                WriteVar<int32_t>(chunk_x, packet);
                WriteVar<int32_t>(chunk_z, packet);
                packet.push_back(heightmaps.get_as_normal());
                packet.push_back(list_array<uint8_t>(data));
                packet.push_back(0); //ignore Block Entity
                packet.push_back(sky_light_mask.data());
                packet.push_back(block_light_mask.data());
                packet.push_back(empty_skylight_mask.data());
                packet.push_back(empty_block_light_mask.data());
                WriteVar<int32_t>(sky_light_arrays.size(), packet);
                for (auto& it : sky_light_arrays) {
                    WriteVar<int32_t>(it.size(), packet);
                    packet.push_back(list_array<uint8_t>(it));
                }
                WriteVar<int32_t>(block_light_arrays.size(), packet);
                for (auto& it : block_light_arrays) {
                    WriteVar<int32_t>(it.size(), packet);
                    packet.push_back(list_array<uint8_t>(it));
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response worldEvent(int32_t event, base_objects::position pos, int32_t data, bool global) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 8 + 4 + 1);
                packet.push_back(0x28);
                WriteVar<int32_t>(event, packet);
                WriteValue<uint64_t>(pos.raw, packet);
                WriteVar<int32_t>(data, packet);
                packet.push_back(global);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response particle(int32_t particle_id, bool long_distance, util::VECTOR pos, util::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 1 + 4 * 3 + 4 * 3 + 4 + 4 + 4 * data.size());
                packet.push_back(0x29);
                WriteVar<int32_t>(particle_id, packet);
                packet.push_back(long_distance);
                WriteValue<double>(pos.x, packet);
                WriteValue<double>(pos.y, packet);
                WriteValue<double>(pos.z, packet);
                WriteValue<float>(offset.x, packet);
                WriteValue<float>(offset.y, packet);
                WriteValue<float>(offset.z, packet);
                WriteValue<float>(max_speed, packet);
                WriteVar<int32_t>(count, packet);
                packet.push_back(data);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateLight(
                int32_t chunk_x,
                int32_t chunk_z,
                const bit_list_array<>& sky_light_mask,
                const bit_list_array<>& block_light_mask,
                const bit_list_array<>& empty_skylight_mask,
                const bit_list_array<>& empty_block_light_mask,
                const list_array<std::vector<uint8_t>>& sky_light_arrays,
                const list_array<std::vector<uint8_t>>& block_light_arrays
            ) {
                if (
                    sky_light_mask.need_commit() | block_light_mask.need_commit() | empty_skylight_mask.need_commit() | empty_block_light_mask.need_commit()
                )
                    throw std::runtime_error("bit_list_array not commited");

                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 2 + sky_light_mask.data().size() + block_light_mask.data().size() + empty_skylight_mask.data().size() + empty_block_light_mask.data().size());
                packet.push_back(0x2A);
                WriteVar<int32_t>(chunk_x, packet);
                WriteVar<int32_t>(chunk_z, packet);
                packet.push_back(sky_light_mask.data());
                packet.push_back(block_light_mask.data());
                packet.push_back(empty_skylight_mask.data());
                packet.push_back(empty_block_light_mask.data());
                WriteVar<int32_t>(sky_light_arrays.size(), packet);
                for (auto& it : sky_light_arrays) {
                    WriteVar<int32_t>(it.size(), packet);
                    packet.push_back(list_array<uint8_t>(it));
                }
                WriteVar<int32_t>(block_light_arrays.size(), packet);
                for (auto& it : block_light_arrays) {
                    WriteVar<int32_t>(it.size(), packet);
                    packet.push_back(list_array<uint8_t>(it));
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool enforces_secure_chat) override {
                list_array<uint8_t> packet;
                packet.push_back(0x2B);
                WriteValue<int32_t>(entity_id, packet);
                packet.push_back(is_hardcore);
                WriteVar<int32_t>(dimension_names.size(), packet);
                for (auto& it : dimension_names)
                    WriteIdentifier(packet, it);
                WriteVar<int32_t>(max_players, packet);
                WriteVar<int32_t>(view_distance, packet);
                WriteVar<int32_t>(simulation_distance, packet);
                packet.push_back(reduced_debug_info);
                packet.push_back(enable_respawn_screen);
                packet.push_back(do_limited_crafting);
                WriteVar<int32_t>(current_dimension_type, packet);
                WriteIdentifier(packet, dimension_name);
                WriteValue<int64_t>(hashed_seed, packet);
                packet.push_back(gamemode);
                packet.push_back(prev_gamemode);
                packet.push_back(is_debug);
                packet.push_back(is_flat);
                packet.push_back((bool)death_location);
                if (death_location) {
                    WriteValue(death_location->position.raw, packet);
                    WriteIdentifier(packet, death_location->dimension);
                }
                WriteVar<int32_t>(portal_cooldown, packet);
                WriteVar<int32_t>(0, packet); //sea level
                packet.push_back(enforces_secure_chat);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t columns, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 1 + 1 + 4 * 2 + 1 + 1 + 1 + 1 + 2 * icons.size() + 4 * 4 + data.size());
                packet.push_back(0x2C);
                WriteVar<int32_t>(map_id, packet);
                packet.push_back(scale);
                packet.push_back(locked);
                packet.push_back(!icons.empty());
                if (!icons.empty()) {
                    WriteVar<int32_t>(icons.size(), packet);
                    for (auto& [name, type, x, z, direction] : icons) {
                        WriteVar<int32_t>(type, packet);
                        packet.push_back(x);
                        packet.push_back(z);
                        packet.push_back(direction);
                        packet.push_back((bool)name);
                        if (name)
                            packet.push_back(name->ToTextComponent());
                    }
                }
                packet.push_back(columns);
                if (columns) {
                    packet.push_back(rows);
                    packet.push_back(x);
                    packet.push_back(z);
                    WriteVar<int32_t>(data.size(), packet);
                    packet.push_back(data);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade>& trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock) override {
                list_array<uint8_t> packet;
                packet.push_back(0x2D);
                packet.push_back(window_id);
                packet.push_back(trade_id);
                WriteVar<int32_t>(trades.size(), packet);
                for (auto& [input1, output, input2, number_of_trade_uses, max_number_of_trades, xp, spec_price, price_multiplier, demand, trade_disabled] : trades) {
                    //TODO check
                    reader::WriteTradeItem(packet, input1);
                    reader::WriteSlot(packet, output);
                    reader::WriteTradeItem(packet, input2);
                    WriteValue<int32_t>(number_of_trade_uses, packet);
                    WriteValue<int32_t>(max_number_of_trades, packet);
                    WriteValue<int32_t>(xp, packet);
                    WriteValue<int32_t>(spec_price, packet);
                    WriteValue<int32_t>(price_multiplier, packet);
                    WriteValue<int32_t>(demand, packet);
                }
                packet.push_back(level);
                WriteVar<int32_t>(experience, packet);
                packet.push_back(regular_villager);
                packet.push_back(can_restock);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateEntityPosition(int32_t entity_id, util::XYZ<float> pos, bool on_ground) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 4 * 3 + 1);
                packet.push_back(0x2E);
                WriteVar<int32_t>(entity_id, packet);
                auto res = util::minecraft::packets::delta_move(pos);
                WriteValue<int16_t>(res.x, packet);
                WriteValue<int16_t>(res.y, packet);
                WriteValue<int16_t>(res.z, packet);
                packet.push_back(on_ground);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateEntityPositionAndRotation(int32_t entity_id, util::XYZ<float> pos, util::VECTOR rot, bool on_ground) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 4 * 3 + 4 * 3 + 1);
                packet.push_back(0x2F);
                WriteVar<int32_t>(entity_id, packet);
                auto res = util::minecraft::packets::delta_move(pos);
                WriteValue<int16_t>(res.x, packet);
                WriteValue<int16_t>(res.y, packet);
                WriteValue<int16_t>(res.z, packet);
                auto res2 = util::to_yaw_pitch_256(rot);
                WriteValue<uint8_t>(res2.x, packet);
                WriteValue<uint8_t>(res2.y, packet);
                packet.push_back(on_ground);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response moveMinecartAlongTrack(int32_t entity_id, list_array<base_objects::packets::minecart_state>& states) override {
                list_array<uint8_t> packet;
                packet.push_back(0x30);
                WriteVar<int32_t>(entity_id, packet);
                WriteVar<int32_t>((int32_t)states.size(), packet);
                for (auto& state : states) {
                    WriteValue<double>(state.x, packet);
                    WriteValue<double>(state.y, packet);
                    WriteValue<double>(state.z, packet);
                    WriteValue<double>(state.velocity_x, packet);
                    WriteValue<double>(state.velocity_y, packet);
                    WriteValue<double>(state.velocity_z, packet);
                    auto yp = util::to_yaw_pitch_256(util::ANGLE_DEG{state.yaw, state.pitch});
                    packet.push_back(yp.x);
                    packet.push_back(yp.y);
                    WriteValue<float>(state.weight, packet);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateEntityRotation(int32_t entity_id, util::VECTOR rot, bool on_ground) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 4 * 2 + 1);
                packet.push_back(0x31);
                WriteVar<int32_t>(entity_id, packet);
                auto res = util::to_yaw_pitch_256(rot);
                WriteValue<uint8_t>(res.x, packet);
                WriteValue<uint8_t>(res.y, packet);
                packet.push_back(on_ground);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response moveVehicle(util::VECTOR pos, util::VECTOR rot) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 3 + 4 * 3);
                packet.push_back(0x32);
                WriteValue<double>(pos.x, packet);
                WriteValue<double>(pos.y, packet);
                WriteValue<double>(pos.z, packet);
                auto res = util::to_yaw_pitch(rot);
                WriteValue<float>(res.x, packet);
                WriteValue<float>(res.y, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response openBook(int32_t hand) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4);
                packet.push_back(0x33);
                WriteVar<int32_t>(hand, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response openScreen(int32_t window_id, int32_t type, const Chat& title) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 2);
                packet.push_back(0x34);
                packet.push_back(window_id);
                WriteVar<int32_t>(type, packet);
                packet.push_back(title.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response openSignEditor(base_objects::position pos, bool is_front_text) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 8 + 1);
                packet.push_back(0x35);
                WriteValue<uint64_t>(pos.raw, packet);
                packet.push_back(is_front_text);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response ping(int32_t id) override {
                if (id == -1)
                    throw std::runtime_error("ping id can't be -1");
                list_array<uint8_t> packet;
                packet.reserve(1 + 4);
                packet.push_back(0x36);
                WriteVar<int32_t>(id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response pingResponse(int32_t id) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4);
                packet.push_back(0x37);
                WriteVar<int32_t>(id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + recipe_id.size());
                packet.push_back(0x38);
                packet.push_back(windows_id);
                WriteIdentifier(packet, recipe_id);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerAbilities(uint8_t flags, float flying_speed, float field_of_view) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 1 + 4 * 2);
                packet.push_back(0x39);
                packet.push_back(flags);
                WriteValue<float>(flying_speed, packet);
                WriteValue<float>(field_of_view, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerChatMessage(enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, const std::optional<enbt::value>& unsigned_content, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name) override {
                if (prev_messages.size() > 20)
                    throw std::runtime_error("too many prev messages");
                list_array<uint8_t> packet;
                packet.reserve(1 + 16 + 4 + 1 + message.size() + 8 + 8 + 4 * prev_messages.size() + 1 + 1 + 1 + 2 * 4 + 1 + (bool)target_name * 2);
                packet.push_back(0x3A);
                WriteUUID(sender, packet);
                WriteVar<int32_t>(index, packet);
                packet.push_back((bool)signature);
                if (signature)
                    packet.push_back(signature->data(), signature->size());
                WriteString(packet, message, 256);
                WriteValue<int64_t>(timestamp, packet);
                WriteValue<int64_t>(salt, packet);
                WriteVar<int32_t>(prev_messages.size(), packet);
                for (auto& msg_signature : prev_messages) {
                    WriteVar<int32_t>(0, packet);
                    packet.push_back(msg_signature.data(), msg_signature.size());
                }
                packet.push_back((bool)unsigned_content);
                if (unsigned_content)
                    packet.push_back(NBT::build(*unsigned_content).get_as_network());
                WriteVar<int32_t>(filter_type, packet);
                if (filter_type == 2) { //PARTIALLY_FILTERED
                    size_t need_bytes_for_message = (message.size() + 7) / 8;
                    if (filtered_symbols_bitfield.size() != need_bytes_for_message)
                        throw std::runtime_error("invalid filtered symbols bitfield size");
                    packet.push_back(filtered_symbols_bitfield);
                }
                WriteVar<int32_t>(chat_type, packet);
                packet.push_back(sender_name.ToTextComponent());
                packet.push_back((bool)target_name);
                if (target_name)
                    packet.push_back(target_name->ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response endCombat(int32_t duration) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4);
                packet.push_back(0x3B);
                WriteVar<int32_t>(duration, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response enterCombat() override {
                return base_objects::network::response::answer({list_array<uint8_t>::concat(0x3C)});
            }

            base_objects::network::response combatDeath(int32_t player_id, const Chat& message) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4);
                packet.push_back(0x3D);
                WriteVar<int32_t>(player_id, packet);
                packet.push_back(message.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerInfoRemove(const list_array<enbt::raw_uuid>& players) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + players.size() * sizeof(enbt::raw_uuid));
                packet.push_back(0x3E);
                WriteVar<int32_t>(players.size(), packet);
                for (auto& it : players)
                    WriteUUID(it, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerInfoAdd(const list_array<base_objects::packets::player_actions_add>& add_players) override {
                list_array<uint8_t> packet;
                packet.push_back(0x3F);
                packet.push_back(0x01);
                WriteVar<int32_t>(add_players.size(), packet);
                for (auto& [uuid, name, properties] : add_players) {
                    WriteUUID(uuid, packet);
                    WriteString(packet, name, 16);
                    WriteVar<int32_t>(properties.size(), packet);
                    for (auto& [name, value, signature] : properties) {
                        WriteString(packet, name, 32767);
                        WriteString(packet, value, 32767);
                        packet.push_back((bool)signature);
                        if (signature)
                            packet.push_back((uint8_t*)signature->data(), signature->size());
                    }
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerInfoInitializeChat(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat) override {
                list_array<uint8_t> packet;
                packet.push_back(0x3F);
                packet.push_back(0x02);
                WriteVar<int32_t>(initialize_chat.size(), packet);
                for (auto& [uuid, chat_session_id, public_key_expiry_time, public_key, public_key_signature] : initialize_chat) {
                    if (public_key.size() > 512)
                        throw std::runtime_error("public key too long");
                    if (public_key_signature.size() > 4096)
                        throw std::runtime_error("public key signature too long");
                    WriteUUID(uuid, packet);
                    packet.push_back((bool)chat_session_id);
                    if (chat_session_id)
                        WriteUUID(*chat_session_id, packet);
                    WriteValue<int64_t>(public_key_expiry_time, packet);
                    WriteVar<int32_t>(public_key.size(), packet);
                    packet.push_back(public_key);
                    WriteVar<int32_t>(public_key_signature.size(), packet);
                    packet.push_back(public_key_signature);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerInfoUpdateGameMode(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode) override {
                list_array<uint8_t> packet;
                packet.push_back(0x3F);
                packet.push_back(0x04);
                WriteVar<int32_t>(update_game_mode.size(), packet);
                for (auto& [uuid, game_mode] : update_game_mode) {
                    WriteUUID(uuid, packet);
                    packet.push_back(game_mode);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerInfoUpdateListed(const list_array<base_objects::packets::player_actions_update_listed>& update_listed) override {
                list_array<uint8_t> packet;
                packet.push_back(0x3F);
                packet.push_back(0x08);
                WriteVar<int32_t>(update_listed.size(), packet);
                for (auto& [uuid, listed] : update_listed) {
                    WriteUUID(uuid, packet);
                    packet.push_back(listed);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerInfoUpdateLatency(const list_array<base_objects::packets::player_actions_update_latency>& update_latency) override {
                list_array<uint8_t> packet;
                packet.push_back(0x3F);
                packet.push_back(0x10);
                WriteVar<int32_t>(update_latency.size(), packet);
                for (auto& [uuid, latency] : update_latency) {
                    WriteUUID(uuid, packet);
                    WriteVar<int32_t>(latency, packet);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response playerInfoUpdateDisplayName(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4);
                packet.push_back(0x3F);
                packet.push_back(0x20);
                WriteVar<int32_t>(update_display_name.size(), packet);
                for (auto& [uuid, display_name] : update_display_name) {
                    WriteUUID(uuid, packet);
                    packet.push_back((bool)display_name);
                    if (display_name)
                        packet.push_back(display_name->ToTextComponent());
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response lookAt(bool from_feet_or_eyes, util::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 1 + 4 * 3 + (bool)entity_id * 4 * 2);
                packet.push_back(0x30);
                packet.push_back(from_feet_or_eyes);
                WriteValue<float>(target.x, packet);
                WriteValue<float>(target.y, packet);
                WriteValue<float>(target.z, packet);
                packet.push_back((bool)entity_id);
                if (entity_id) {
                    WriteVar<int32_t>(entity_id->first, packet); //entity id
                    packet.push_back(entity_id->second);         // to entity foot or eyes
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response synchronizePlayerPosition(util::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 3 + 4 * 2 + 1);
                packet.push_back(0x41);
                WriteValue<double>(pos.x, packet);
                WriteValue<double>(pos.y, packet);
                WriteValue<double>(pos.z, packet);
                WriteValue<float>(yaw, packet);
                WriteValue<float>(pitch, packet);
                packet.push_back(flags);
                WriteVar<int32_t>(teleport_id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response synchronizePlayerRotation(float yaw, float pitch) override {
                list_array<uint8_t> packet;
                packet.push_back(0x42);
                WriteValue(yaw, packet);
                WriteValue(pitch, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& displayed_recipe_ids, const list_array<std::string>& had_access_to_recipe_ids) override {
                return addRecipeBook(
                    crafting_recipe_book_open,
                    crafting_recipe_book_filter_active,
                    smelting_recipe_book_open,
                    smelting_recipe_book_filter_active,
                    blast_furnace_recipe_book_open,
                    blast_furnace_recipe_book_filter_active,
                    smoker_recipe_book_open,
                    smoker_recipe_book_filter_active,
                    displayed_recipe_ids.concat(had_access_to_recipe_ids).unify()
                );
            }

            base_objects::network::response addRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) override {
                auto res = updateRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active);
                list_array<uint8_t> packet;
                size_t count = 0;
                for (auto& recipe_id : recipe_ids) {
                    auto& recipe = registers::recipe_table.at(recipe_id);
                    {
                        list_array<uint8_t> tmp;
                        if (!reader::WriteRecipeDisplay(tmp, recipe))
                            continue;
                        WriteVar<int32_t>(recipe.id, packet);
                        packet += tmp.take();
                    }
                    WriteVar<int32_t>((int32_t)std::hash<std::string>()(recipe.group), packet);
                    WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_book_category", recipe.category, 770), packet);
                    packet.push_back((bool)recipe.ingredients.size());
                    if (recipe.ingredients.size()) {
                        WriteVar<int32_t>(recipe.ingredients.size(), packet);
                        for (auto& items : recipe.ingredients) {
                            std::visit(
                                [&](auto&& item) {
                                    if constexpr (std::is_same_v<std::decay_t<decltype(item)>, std::string>) {
                                        packet.push_back(false);
                                        WriteIdentifier(packet, item);
                                    } else {
                                        WriteVar<int32_t>(item.size() + 1, packet);
                                        for (auto& i : registers::convert_reg_pro_id("minecraft:item", item, 770)) {
                                            WriteVar<int32_t>(i, packet);
                                        }
                                    }
                                },
                                items
                            );
                        }
                    }
                    uint8_t flags = 0;
                    if (recipe.show_notification)
                        flags |= 1;
                    if (recipe.highlight_as_new)
                        flags |= 2;
                    packet.push_back(flags);
                }
                list_array<uint8_t> header;
                header.push_back(0x43);
                WriteVar<int32_t>(count, header);
                packet.push_back(header.take());
                res += base_objects::network::response::answer({std::move(packet)});
                return res;
            }

            base_objects::network::response removeRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) override {
                auto res = updateRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active);
                list_array<uint8_t> packet;
                packet.push_back(0x44);
                WriteVar<int32_t>(recipe_ids.size(), packet);
                for (auto& recipe_id : recipe_ids)
                    WriteVar<int32_t>(registers::recipe_table.at(recipe_id).id, packet);
                res += base_objects::network::response::answer({std::move(packet)});
                return res;
            }

            base_objects::network::response updateRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active) override {
                list_array<uint8_t> packet;
                packet.reserve(10);
                packet.push_back(0x45);
                packet.push_back(crafting_recipe_book_open);
                packet.push_back(crafting_recipe_book_filter_active);
                packet.push_back(smelting_recipe_book_open);
                packet.push_back(smelting_recipe_book_filter_active);
                packet.push_back(blast_furnace_recipe_book_open);
                packet.push_back(blast_furnace_recipe_book_filter_active);
                packet.push_back(smoker_recipe_book_open);
                packet.push_back(smoker_recipe_book_filter_active);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response RecipeBookAdd(const list_array<base_objects::recipe>& recipes) override {
                list_array<uint8_t> packet;
                packet.push_back(0x43);
                WriteVar<int32_t>(recipes.size(), packet);
                for (auto& recipe : recipes) {
                    WriteVar<int32_t>(recipe.id, packet);
                    if (!reader::WriteRecipeDisplay(packet, recipe))
                        throw std::runtime_error("Recipe must to have the must_display property set to true");
                    WriteVar<int32_t>((int32_t)std::hash<std::string>()(recipe.group), packet);
                    WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_book_category", recipe.category, 770), packet);
                    packet.push_back((bool)recipe.ingredients.size());
                    if (recipe.ingredients.size()) {
                        WriteVar<int32_t>(recipe.ingredients.size(), packet);
                        for (auto& items : recipe.ingredients) {
                            std::visit(
                                [&](auto&& item) {
                                    if constexpr (std::is_same_v<std::decay_t<decltype(item)>, std::string>) {
                                        packet.push_back(false);
                                        WriteIdentifier(packet, item);
                                    } else {
                                        WriteVar<int32_t>(item.size() + 1, packet);
                                        for (auto& i : registers::convert_reg_pro_id("minecraft:item", item, 770)) {
                                            WriteVar<int32_t>(i, packet);
                                        }
                                    }
                                },
                                items
                            );
                        }
                    }
                    uint8_t flags = 0;
                    if (recipe.show_notification)
                        flags |= 1;
                    if (recipe.highlight_as_new)
                        flags |= 2;
                    packet.push_back(flags);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response RecipeBookRemove(const list_array<uint32_t>& recipe_ids) override {
                list_array<uint8_t> packet;
                packet.push_back(0x44);
                WriteVar<int32_t>(recipe_ids.size(), packet);
                for (auto& recipe_id : recipe_ids)
                    WriteVar<int32_t>(recipe_id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response RecipeBookSettings(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active) override {
                return updateRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active);
            }

            base_objects::network::response removeEntities(const list_array<int32_t>& entity_ids) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 4 * entity_ids.size());
                packet.push_back(0x46);
                WriteVar<int32_t>(entity_ids.size(), packet);
                for (auto& it : entity_ids)
                    WriteVar<int32_t>(it, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response removeEntityEffect(int32_t entity_id, int32_t effect_id) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 * 2);
                packet.push_back(0x47);
                WriteVar<int32_t>(entity_id, packet);
                WriteVar<int32_t>(effect_id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + entity_name.size() + (bool)objective_name * objective_name->size());
                packet.push_back(0x48);
                WriteString(packet, entity_name, 32767);
                packet.push_back((bool)objective_name);
                if (objective_name)
                    WriteString(packet, *objective_name, 32767);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response removeResourcePacks() override {
                return base_objects::network::response::answer({list_array<uint8_t>::concat(0x49, false)});
            }

            base_objects::network::response removeResourcePack(enbt::raw_uuid id) override {
                list_array<uint8_t> packet;
                packet.push_back(0x49);
                packet.push_back(true);
                WriteUUID(id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response addResourcePack(enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt) override {
                list_array<uint8_t> packet;
                packet.push_back(0x4A);
                WriteUUID(id, packet);
                WriteString(packet, url, 32767);
                WriteString(packet, hash, 40);
                packet.push_back(forced);
                packet.push_back((bool)prompt);
                if (prompt)
                    packet.push_back(prompt->ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response respawn(int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata) override {
                list_array<uint8_t> packet;
                packet.push_back(0x4B);
                WriteVar<int32_t>(dimension_type, packet);
                WriteIdentifier(packet, dimension_name);
                WriteValue<int64_t>(hashed_seed, packet);
                packet.push_back(gamemode);
                packet.push_back(previous_gamemode);
                packet.push_back(is_debug);
                packet.push_back(is_flat);
                packet.push_back((bool)death_location);
                if (death_location) {
                    WriteValue(death_location->position.raw, packet);
                    WriteIdentifier(packet, death_location->dimension);
                }
                WriteVar<int32_t>(portal_cooldown, packet);
                uint8_t data_kept = keep_attributes;
                data_kept |= keep_metadata << 1;
                packet.push_back(data_kept);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setHeadRotation(int32_t entity_id, util::VECTOR head_rotation) override {
                list_array<uint8_t> packet;
                packet.push_back(0x4C);
                WriteVar<int32_t>(entity_id, packet);
                auto res = util::to_yaw_pitch_256(head_rotation);
                packet.push_back(res.x);
                packet.push_back(res.y);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks) override {
                list_array<uint8_t> packet;
                int64_t section_pos = (int64_t(section_x & 0x3FFFFF) << 42) | ((section_z & 0x3FFFFF) << 20) | int64_t(section_y & 0xFFFFF);
                packet.push_back(0x4D);
                WriteValue<int64_t>(section_pos, packet);
                WriteVar<int32_t>(blocks.size(), packet);
                for (auto& it : blocks)
                    WriteVar<int64_t>(it.value, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setAdvancementsTab(const std::optional<std::string>& tab_id) override {
                list_array<uint8_t> packet;
                packet.push_back(0x4E);
                packet.push_back((bool)tab_id);
                if (tab_id)
                    WriteIdentifier(packet, *tab_id);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png) override {
                list_array<uint8_t> packet;
                packet.push_back(0x4F);
                packet.push_back(motd.ToTextComponent());
                packet.push_back((bool)icon_png);
                if (icon_png) {
                    WriteVar<int32_t>(icon_png->size(), packet);
                    packet.push_back(*icon_png);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setActionBarText(const Chat& text) override {
                list_array<uint8_t> packet;
                packet.push_back(0x50);
                packet.push_back(text.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setBorderCenter(double x, double z) override {
                list_array<uint8_t> packet;
                packet.push_back(0x51);
                WriteValue<double>(x, packet);
                WriteValue<double>(z, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms) override {
                list_array<uint8_t> packet;
                packet.push_back(0x52);
                WriteValue<double>(old_diameter, packet);
                WriteValue<double>(new_diameter, packet);
                WriteVar<int64_t>(speed_ms, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setBorderSize(double diameter) override {
                list_array<uint8_t> packet;
                packet.push_back(0x53);
                WriteValue<double>(diameter, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setBorderWarningDelay(int32_t warning_delay) override {
                list_array<uint8_t> packet;
                packet.push_back(0x54);
                WriteVar<int32_t>(warning_delay, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setBorderWarningDistance(int32_t warning_distance) override {
                list_array<uint8_t> packet;
                packet.push_back(0x55);
                WriteVar<int32_t>(warning_distance, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setCamera(int32_t entity_id) override {
                list_array<uint8_t> packet;
                packet.push_back(0x56);
                WriteVar<int32_t>(entity_id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setCenterChunk(int32_t x, int32_t z) override {
                list_array<uint8_t> packet;
                packet.push_back(0x57);
                WriteVar<int32_t>(x, packet);
                WriteVar<int32_t>(z, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setRenderDistance(int32_t render_distance) override {
                list_array<uint8_t> packet;
                packet.push_back(0x58);
                WriteVar<int32_t>(render_distance, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setCursorItem(const base_objects::slot& item) override {
                list_array<uint8_t> data;
                data.push_back(0x59);
                reader::WriteSlotItem(data, item);
                return base_objects::network::response::answer({std::move(data)});
            }

            base_objects::network::response setDefaultSpawnPosition(base_objects::position pos, float angle) override {
                list_array<uint8_t> packet;
                packet.push_back(0x5A);
                WriteValue(pos.raw, packet);
                WriteValue<float>(angle, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response displayObjective(int32_t position, const std::string& objective_name) override {
                list_array<uint8_t> packet;
                packet.push_back(0x5B);
                packet.push_back(position);
                WriteString(packet, objective_name, 32767);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata) override {
                list_array<uint8_t> packet;
                packet.push_back(0x5C);
                WriteVar<int32_t>(entity_id, packet);
                packet.push_back(metadata);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id) override {
                list_array<uint8_t> packet;
                packet.push_back(0x5D);
                WriteValue<int32_t>(attached_entity_id, packet);
                WriteValue<int32_t>(holder_entity_id, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setEntityMotion(int32_t entity_id, util::VECTOR velocity) override {
                list_array<uint8_t> packet;
                packet.push_back(0x5E);
                WriteVar<int32_t>(entity_id, packet);
                auto res = util::minecraft::packets::velocity(velocity);
                WriteValue<int16_t>(res.x, packet);
                WriteValue<int16_t>(res.y, packet);
                WriteValue<int16_t>(res.z, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::slot& item) override {
                list_array<uint8_t> packet;
                packet.push_back(0x5F);
                WriteVar<int32_t>(entity_id, packet);
                packet.push_back(slot);
                reader::WriteSlot(packet, item);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setExperience(float experience_bar, int32_t level, int32_t total_experience) override {
                list_array<uint8_t> packet;
                packet.push_back(0x60);
                WriteValue<float>(experience_bar, packet);
                WriteVar<int32_t>(level, packet);
                WriteVar<int32_t>(total_experience, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setHealth(float health, int32_t food, float saturation) override {
                list_array<uint8_t> packet;
                packet.push_back(0x61);
                WriteValue<float>(health, packet);
                WriteVar<int32_t>(food, packet);
                WriteValue<float>(saturation, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setHeldSlot(int32_t slot) override {
                list_array<uint8_t> packet;
                packet.push_back(0x62);
                WriteVar<int32_t>(slot, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type) override {
                list_array<uint8_t> packet;
                packet.push_back(0x63);
                WriteString(packet, objective_name, 32767);
                packet.push_back(0);
                packet.push_back(display_name.ToTextComponent());
                packet.push_back(render_type);
                packet.push_back(0);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) override {
                list_array<uint8_t> packet;
                packet.push_back(0x63);
                WriteString(packet, objective_name, 32767);
                packet.push_back(0);
                packet.push_back(display_name.ToTextComponent());
                packet.push_back(render_type);
                packet.push_back(true);
                packet.push_back(1);
                packet.push_back(NBT::build(style).get_as_network());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) override {
                list_array<uint8_t> packet;
                packet.push_back(0x63);
                WriteString(packet, objective_name, 32767);
                packet.push_back(0);
                packet.push_back(display_name.ToTextComponent());
                packet.push_back(render_type);
                packet.push_back(true);
                packet.push_back(2);
                packet.push_back(content.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateObjectivesRemove(const std::string& objective_name) override {
                list_array<uint8_t> packet;
                packet.push_back(0x63);
                WriteString(packet, objective_name, 32767);
                packet.push_back(1);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type) override {
                list_array<uint8_t> packet;
                packet.push_back(0x63);
                WriteString(packet, objective_name, 32767);
                packet.push_back(2);
                packet.push_back(display_name.ToTextComponent());
                packet.push_back(render_type);
                packet.push_back(false);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) override {
                list_array<uint8_t> packet;
                packet.push_back(0x63);
                WriteString(packet, objective_name, 32767);
                packet.push_back(2);
                packet.push_back(display_name.ToTextComponent());
                packet.push_back(render_type);
                packet.push_back(true);
                packet.push_back(1);
                packet.push_back(NBT::build(style).get_as_network());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) override {
                list_array<uint8_t> packet;
                packet.push_back(0x63);
                WriteString(packet, objective_name, 32767);
                packet.push_back(2);
                packet.push_back(display_name.ToTextComponent());
                packet.push_back(render_type);
                packet.push_back(true);
                packet.push_back(2);
                packet.push_back(content.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers) override {
                list_array<uint8_t> packet;
                packet.push_back(0x64);
                WriteVar<int32_t>(vehicle_entity_id, packet);
                WriteVar<int32_t>(passengers.size(), packet);
                for (auto& it : passengers)
                    WriteVar<int32_t>(it, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setPlayerInventory(int32_t slot, const base_objects::slot& item) override {
                list_array<uint8_t> packet;
                packet.push_back(0x65);
                WriteVar<int32_t>(slot, packet);
                reader::WriteSlot(packet, item);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities) override {
                list_array<uint8_t> packet;
                packet.push_back(0x66);
                WriteString(packet, team_name, 32767);
                packet.push_back(0);
                packet.push_back(display_name.ToTextComponent());
                packet.push_back(int8_t(allow_fire_co_teamer) & (int8_t(see_invisible_co_teamer) << 1));
                WriteString(packet, name_tag_visibility, 40);
                WriteString(packet, collision_rule, 40);
                WriteVar<int32_t>(team_color, packet);
                packet.push_back(prefix.ToTextComponent());
                packet.push_back(suffix.ToTextComponent());
                WriteVar<int32_t>(entities.size(), packet);
                for (auto& it : entities)
                    WriteString(packet, it, 32767);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateTeamRemove(const std::string& team_name) override {
                list_array<uint8_t> packet;
                packet.push_back(0x66);
                WriteString(packet, team_name, 32767);
                packet.push_back(1);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix) override {
                list_array<uint8_t> packet;
                packet.push_back(0x66);
                WriteString(packet, team_name, 32767);
                packet.push_back(2);
                packet.push_back(display_name.ToTextComponent());
                packet.push_back(int8_t(allow_fire_co_teamer) & (int8_t(see_invisible_co_teamer) << 1));
                WriteString(packet, name_tag_visibility, 40);
                WriteString(packet, collision_rule, 40);
                WriteVar<int32_t>(team_color, packet);
                packet.push_back(prefix.ToTextComponent());
                packet.push_back(suffix.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities) override {
                list_array<uint8_t> packet;
                packet.push_back(0x66);
                WriteString(packet, team_name, 32767);
                packet.push_back(3);
                WriteVar<int32_t>(entities.size(), packet);
                for (auto& it : entities)
                    WriteString(packet, it, 32767);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 1 + team_name.size() + 4);
                packet.push_back(0x66);
                WriteString(packet, team_name, 32767);
                packet.push_back(4);
                WriteVar<int32_t>(entities.size(), packet);
                for (auto& it : entities)
                    WriteString(packet, it, 32767);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name) override {
                list_array<uint8_t> packet;
                packet.push_back(0x67);
                WriteString(packet, entity_name, 32767);
                WriteString(packet, objective_name, 32767);
                WriteVar<int32_t>(value, packet);
                packet.push_back((bool)display_name);
                if (display_name)
                    packet.push_back(display_name->ToTextComponent());
                packet.push_back(0);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled) override {
                list_array<uint8_t> packet;
                packet.push_back(0x67);
                WriteString(packet, entity_name, 32767);
                WriteString(packet, objective_name, 32767);
                WriteVar<int32_t>(value, packet);
                packet.push_back((bool)display_name);
                if (display_name)
                    packet.push_back(display_name->ToTextComponent());
                packet.push_back(1);
                packet.push_back(NBT::build(styled).get_as_network());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content) override {
                list_array<uint8_t> packet;
                packet.push_back(0x67);
                WriteString(packet, entity_name, 32767);
                WriteString(packet, objective_name, 32767);
                WriteVar<int32_t>(value, packet);
                packet.push_back((bool)display_name);
                if (display_name)
                    packet.push_back(display_name->ToTextComponent());
                packet.push_back(2);
                packet.push_back(content.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setSimulationDistance(int32_t distance) override {
                list_array<uint8_t> packet;
                packet.push_back(0x68);
                WriteVar<int32_t>(distance, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setSubtitleText(const Chat& text) override {
                list_array<uint8_t> packet;
                packet.push_back(0x69);
                packet.push_back(text.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateTime(int64_t world_age, int64_t time_of_day, bool increase_time) override {
                list_array<uint8_t> packet;
                packet.push_back(0x6A);
                WriteValue<int64_t>(world_age, packet);
                WriteValue<int64_t>(time_of_day, packet);
                packet.push_back(increase_time);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setTitleText(const Chat& text) override {
                list_array<uint8_t> packet;
                packet.push_back(0x6B);
                packet.push_back(text.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out) override {
                list_array<uint8_t> packet;
                packet.push_back(0x6C);
                WriteValue<int32_t>(fade_in, packet);
                WriteValue<int32_t>(stay, packet);
                WriteValue<int32_t>(fade_out, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) override {
                volume = std::clamp(volume, 0.0f, 1.0f);
                pitch = std::clamp(pitch, 0.5f, 2.0f);
                list_array<uint8_t> packet;
                packet.push_back(0x6D);
                WriteVar<int32_t>(sound_id + 1, packet);
                WriteVar<int32_t>(category, packet);
                WriteVar<int32_t>(entity_id, packet);
                WriteValue<float>(volume, packet);
                WriteValue<float>(pitch, packet);
                WriteValue<int64_t>(seed, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response entitySoundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) override {
                volume = std::clamp(volume, 0.0f, 1.0f);
                pitch = std::clamp(pitch, 0.5f, 2.0f);

                list_array<uint8_t> packet;
                packet.push_back(0x6D);
                packet.push_back(0);
                WriteIdentifier(packet, sound_id);
                packet.push_back((bool)range);
                if (range)
                    WriteValue<float>(*range, packet);
                WriteVar<int32_t>(category, packet);
                WriteVar<int32_t>(entity_id, packet);
                WriteValue<float>(volume, packet);
                WriteValue<float>(pitch, packet);
                WriteValue<int64_t>(seed, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) override {
                volume = std::clamp(volume, 0.0f, 1.0f);
                pitch = std::clamp(pitch, 0.5f, 2.0f);

                list_array<uint8_t> packet;
                packet.push_back(0x6E);
                WriteVar<int32_t>(sound_id + 1, packet);
                WriteVar<int32_t>(category, packet);
                WriteValue<int32_t>(x, packet);
                WriteValue<int32_t>(y, packet);
                WriteValue<int32_t>(z, packet);
                WriteValue<float>(volume, packet);
                WriteValue<float>(pitch, packet);
                WriteValue<int64_t>(seed, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response soundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) override {
                volume = std::clamp(volume, 0.0f, 1.0f);
                pitch = std::clamp(pitch, 0.5f, 2.0f);

                list_array<uint8_t> packet;
                packet.push_back(0x6E);
                packet.push_back(0);
                WriteIdentifier(packet, sound_id);
                packet.push_back((bool)range);
                if (range)
                    WriteValue<float>(*range, packet);
                WriteVar<int32_t>(category, packet);
                WriteValue<int32_t>(x, packet);
                WriteValue<int32_t>(y, packet);
                WriteValue<int32_t>(z, packet);
                WriteValue<float>(volume, packet);
                WriteValue<float>(pitch, packet);
                WriteValue<int64_t>(seed, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response startConfiguration() override {
                list_array<uint8_t> packet;
                packet.push_back(0x6F);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response stopSound(uint8_t flags) override {
                list_array<uint8_t> packet;
                packet.push_back(0x70);
                packet.push_back(flags | (~3));
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response stopSoundBySource(uint8_t flags, int32_t source) override {
                list_array<uint8_t> packet;
                packet.push_back(0x70);
                packet.push_back((flags & 1) | (~2));
                WriteVar<int32_t>(source, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response stopSoundBySound(uint8_t flags, const std::string& sound) override {
                list_array<uint8_t> packet;
                packet.push_back(0x70);
                packet.push_back((flags & 2) | (~1));
                WriteIdentifier(packet, sound);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response stopSoundBySourceAndSound(uint8_t flags, int32_t source, const std::string& sound) override {
                list_array<uint8_t> packet;
                packet.push_back(0x70);
                packet.push_back(flags & 3);
                WriteVar<int32_t>(source, packet);
                WriteIdentifier(packet, sound);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload) override {
                if (payload.size() > 5120)
                    throw std::runtime_error("Payload size is too big");

                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + key.size() + payload.size());
                packet.push_back(0x71);
                WriteIdentifier(packet, key);
                packet.push_back(payload);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response systemChatMessage(const Chat& message) override {
                list_array<uint8_t> packet;
                packet.push_back(0x72);
                packet.push_back(message.ToTextComponent());
                packet.push_back(false);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response systemChatMessageOverlay(const Chat& message) override {
                list_array<uint8_t> packet;
                packet.push_back(0x72);
                packet.push_back(message.ToTextComponent());
                packet.push_back(true);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setTabListHeaderAndFooter(const Chat& header, const Chat& footer) override {
                list_array<uint8_t> packet;
                packet.push_back(0x73);
                packet.push_back(header.ToTextComponent());
                packet.push_back(footer.ToTextComponent());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response tagQueryResponse(int32_t transaction_id, const enbt::value& nbt) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4);
                packet.push_back(0x74);
                WriteVar<int32_t>(transaction_id, packet);
                packet.push_back(NBT::build(nbt).get_as_network());
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count) override {
                list_array<uint8_t> packet;
                packet.push_back(0x76);
                WriteVar<int32_t>(collected_entity_id, packet);
                WriteVar<int32_t>(collector_entity_id, packet);
                WriteVar<int32_t>(pickup_item_count, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            //minecraft:teleport_entity
            base_objects::network::response teleportEntityEX(int32_t entity_id, util::VECTOR pos, util::VECTOR velocity, float yaw, float pitch, bool on_ground, base_objects::packets::teleport_flags flags) override {
                list_array<uint8_t> packet;
                packet.reserve(1 + 4 + 4 * 3 + 4 * 2 + 1);
                packet.push_back(0x77);
                WriteVar<int32_t>(entity_id, packet);
                WriteValue<double>(pos.x, packet);
                WriteValue<double>(pos.y, packet);
                WriteValue<double>(pos.z, packet);
                WriteValue<double>(velocity.x, packet);
                WriteValue<double>(velocity.y, packet);
                WriteValue<double>(velocity.z, packet);
                WriteValue<float>(yaw, packet);
                WriteValue<float>(pitch, packet);
                WriteValue<int32_t>((int32_t)flags.raw, packet);
                packet.push_back(on_ground);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response test_instance_block_status(const Chat& status, std::optional<util::VECTOR> size) {
                list_array<uint8_t> packet;
                packet.push_back(0x77);
                packet.push_back(status.ToTextComponent());
                packet.push_back((bool)size);
                if (size) {
                    WriteValue<double>(size->x, packet);
                    WriteValue<double>(size->y, packet);
                    WriteValue<double>(size->z, packet);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response setTickingState(float tick_rate, bool is_frozen) override {
                list_array<uint8_t> packet;
                packet.push_back(0x78);
                WriteValue<float>(tick_rate, packet);
                packet.push_back(is_frozen);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response stepTick(int32_t step_count) override {
                list_array<uint8_t> packet;
                packet.push_back(0x79);
                WriteVar<int32_t>(step_count, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response transfer(const std::string& host, int32_t port) override {
                list_array<uint8_t> packet;
                packet.push_back(0x7A);
                WriteString(packet, host);
                WriteVar<int32_t>(port, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping>& advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress>& progress_advancements) override {
                list_array<uint8_t> packet;
                packet.push_back(0x7B);
                packet.push_back(reset);
                WriteVar<int32_t>(advancement_mapping.size(), packet);
                for (auto& item : advancement_mapping) {
                    WriteIdentifier(packet, item.key);
                    packet.push_back((bool)item.parent);
                    if (item.parent)
                        WriteIdentifier(packet, *item.parent);
                    packet.push_back((bool)item.display);
                    if (item.display) {
                        auto& display = *item.display;
                        packet.push_back(display.title.ToTextComponent());
                        packet.push_back(display.description.ToTextComponent());
                        reader::WriteSlot(packet, display.icon);
                        WriteVar<int32_t>(display.frame_type, packet);
                        //automaticaly set background_texture flag(0x01) to true if it has value
                        WriteValue(display.flags & (int32_t)display.background_texture.has_value(), packet);
                        if (display.background_texture)
                            WriteIdentifier(packet, *display.background_texture);
                        WriteValue(display.x, packet);
                        WriteValue(display.y, packet);
                    }
                }
                WriteVar<int32_t>(remove_advancements.size(), packet);
                for (auto& it : remove_advancements)
                    WriteIdentifier(packet, it);
                WriteVar<int32_t>(progress_advancements.size(), packet);
                for (auto& [advancement, progress] : progress_advancements) {
                    WriteIdentifier(packet, advancement);
                    WriteVar<int32_t>(progress.size(), packet);
                    for (auto& [criterion, criterion_progress] : progress) {
                        WriteIdentifier(packet, criterion);
                        packet.push_back(criterion_progress.achieved);
                        if (criterion_progress.achieved)
                            WriteValue<int64_t>(criterion_progress.date, packet);
                    }
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) override {
                auto res = protocol::play::updateAttributes__(entity_id, properties, 770);
                res.data[0].data[0] = 0x7C;
                return res;
            }

            base_objects::network::response entityEffect(int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, base_objects::packets::effect_flags flags) override {
                list_array<uint8_t> packet;
                packet.push_back(0x7D);
                WriteVar<int32_t>(entity_id, packet);
                WriteVar<int32_t>(effect_id, packet);
                WriteVar<int32_t>(amplifier, packet);
                WriteVar<int32_t>(duration, packet);
                packet.push_back(flags.raw);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateRecipes(const std::vector<base_objects::recipe>& recipes) override {
                list_array<uint8_t> packet;
                packet.push_back(0x7E);
                WriteVar<int32_t>(recipes.size(), packet);
                for (auto& recipe : recipes) {
                    std::visit(
                        [&](auto&& item) {
                            using type = std::decay_t<decltype(item)>;
                            WriteIdentifier(packet, recipe.full_id);
                            WriteVar<int32_t>(registers::view_reg_pro_id("minecraft:recipe_serializer", base_objects::recipes::variant_data<type>::name, 770), packet);
                            if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::crafting_shaped>) {
                                WriteString(packet, recipe.group, 32767);
                                WriteVar<int32_t>((int32_t)registers::view_reg_pro_id("minecraft:recipe_book_category", recipe.category, 770), packet);
                                WriteVar<int32_t>(item.width, packet);
                                WriteVar<int32_t>(item.height, packet);
                                for (auto& item : item.ingredients)
                                    reader::WriteIngredient(packet, item);
                                reader::WriteIngredient(packet, item.result);
                                packet.push_back(item.show_notification);
                            } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::crafting_shapeless>) {
                                WriteString(packet, recipe.group, 32767);
                                WriteVar<int32_t>((int32_t)registers::view_reg_pro_id("minecraft:recipe_book_category", recipe.category, 770), packet);
                                WriteVar<int32_t>(item.ingredients.size(), packet);
                                for (auto& item : item.ingredients)
                                    reader::WriteIngredient(packet, item);

                                reader::WriteIngredient(packet, item.result);
                            } else if constexpr (
                                std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_armordye>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_bookcloning>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_mapcloning>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_mapextending>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_firework_rocket>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_firework_star>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_firework_star_fade>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_tippedarrow>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_bannerduplicate>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_shielddecoration>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_repairitem>
                                | std::is_same_v<type, base_objects::recipes::minecraft::crafting_decorated_pot>
                            ) {
                                WriteVar<int32_t>((int32_t)registers::view_reg_pro_id("minecraft:recipe_book_category", recipe.category, 770), packet);
                            } else if constexpr (
                                std::is_same_v<type, base_objects::recipes::minecraft::smelting>
                                | std::is_same_v<type, base_objects::recipes::minecraft::blasting>
                                | std::is_same_v<type, base_objects::recipes::minecraft::smoking>
                                | std::is_same_v<type, base_objects::recipes::minecraft::campfire_cooking>
                            ) {
                                WriteString(packet, recipe.group, 32767);
                                WriteVar<int32_t>((int32_t)registers::view_reg_pro_id("minecraft:recipe_book_category", recipe.category, 770), packet);
                                reader::WriteIngredient(packet, item.ingredient);
                                reader::WriteIngredient(packet, item.result);
                                WriteValue<float>(item.experience, packet);
                                WriteVar<int32_t>(item.cooking_time, packet);
                            } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::stonecutting>) {
                                WriteString(packet, recipe.group, 32767);
                                reader::WriteIngredient(packet, item.ingredient);
                                reader::WriteIngredient(packet, item.result);
                            } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::smithing_transform>) {
                                reader::WriteIngredient(packet, item._template);
                                reader::WriteIngredient(packet, item.base);
                                reader::WriteIngredient(packet, item.addition);
                                reader::WriteIngredient(packet, item.result);
                            } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::smithing_trim>) {
                                reader::WriteIngredient(packet, item._template);
                                reader::WriteIngredient(packet, item.base);
                                reader::WriteIngredient(packet, item.addition);
                            } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::crafting_transmute>) {
                                reader::WriteIngredient(packet, item.input);
                                reader::WriteIngredient(packet, item.material);
                                reader::WriteIngredient(packet, item.result);
                            } else if constexpr (std::is_same_v<type, base_objects::recipes::custom>) {
                                WriteVar<int32_t>(item.data.size(), packet);
                                packet.push_back(item.data);
                            } else
                                throw std::runtime_error("invalid recipe type");
                        },
                        recipe.data
                    );
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings) override {
                list_array<uint8_t> packet;
                packet.push_back(0x7F);
                WriteVar<int32_t>(tag_mappings.size(), packet);
                for (auto& [registry, values] : tag_mappings) {
                    WriteIdentifier(packet, registry);
                    WriteVar<int32_t>(values.size(), packet);
                    for (auto& [tag, entries] : values) {
                        WriteIdentifier(packet, tag);
                        WriteVar<int32_t>(entries.size(), packet);
                        for (auto& entry : entries)
                            WriteVar<int32_t>(entry, packet);
                    }
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response projectilePower(int32_t entity_id, double power_x, double power_y, double power_z) override {
                list_array<uint8_t> packet;
                packet.reserve(32);
                packet.push_back(0x80);
                WriteVar<int32_t>(entity_id, packet);
                WriteValue(power_x, packet);
                WriteValue(power_y, packet);
                WriteValue(power_z, packet);
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response custom_report(const list_array<std::pair<std::string, std::string>>& values) override {
                if (values.size() > 32)
                    throw std::invalid_argument("Report cannot contain more than 32 entry's.");
                list_array<uint8_t> packet;
                packet.push_back(0x81);
                WriteVar<int32_t>(values.size(), packet);
                for (auto&& [title, desc] : values) {
                    WriteString(packet, title, 128);
                    WriteString(packet, desc, 4096);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }

            base_objects::network::response server_links(const list_array<base_objects::packets::server_link>& links) override {
                list_array<uint8_t> packet;
                packet.push_back(0x82);
                WriteVar<int32_t>(links.size(), packet);
                for (auto&& [label, url] : links) {
                    std::visit(
                        [&](auto& item) {
                            using type = std::decay_t<decltype(item)>;
                            if constexpr (std::is_same_v<type, base_objects::packets::server_link::label_type>) {
                                packet.push_back(false);
                                WriteVar<int32_t>((int32_t)item, packet);
                            } else {
                                packet.push_back(true);
                                packet.push_back(item.ToTextComponent());
                            }
                        },
                        label
                    );
                    WriteString(packet, url);
                }
                return base_objects::network::response::answer({std::move(packet)});
            }
        };
    }

    namespace decoding::play {
        void teleport_confirm(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::teleport_request_completion data;
            data.teleport_id = packet.read_var<int32_t>();
            data.success = session->shared_data().packets_state.play_data->pending_teleport_ids.front() != data.teleport_id;
            if (data.success)
                session->shared_data().packets_state.play_data->pending_teleport_ids.pop_front();
            api::protocol::on_teleport_request_completion.async_notify({data, *session, session->shared_data_ref()});
        }

        void query_block_nbt(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::block_nbt_request data;
            data.transaction_id = packet.read_var<int32_t>();
            data.position.raw = packet.read_value<int64_t>();
            api::protocol::on_block_nbt_request.async_notify({data, *session, session->shared_data_ref()});
        }

        void bundle_item_selected(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::bundle_item_selected data;
            data.inventory_slot = packet.read_var<int32_t>();
            data.bundle_slot = packet.read_var<int32_t>();
            api::protocol::on_bundle_item_selected.async_notify({data, *session, session->shared_data_ref()});
        }

        void change_difficulty(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_change_difficulty.async_notify({packet.read_value<uint8_t>(), *session, session->shared_data_ref()});
        }

        void acknowledge_message(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_acknowledge_message.async_notify({packet.read_var<int32_t>(), *session, session->shared_data_ref()});
        }

        void chat_command(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_chat_command.async_notify({packet.read_string(32769), *session, session->shared_data_ref()});
        }

        void signed_chat_command(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::signed_chat_command data;
            data.command = packet.read_string(32769);
            data.timestamp = packet.read_value<int64_t>();
            data.salt = packet.read_value<int64_t>();
            int32_t arguments_count = packet.read_var<int32_t>();
            data.arguments_signature.reserve(arguments_count);
            for (int32_t i = 0; i < arguments_count; i++) {
                api::protocol::data::signed_chat_command::argument_signature arg;
                arg.argument_name = packet.read_string(16);
                for (int i = 0; i < 256; i++)
                    arg.signature[i] = packet.read();
                data.arguments_signature.push_back(arg);
            }
            data.message_count = packet.read_var<int32_t>();
            data.acknowledged.arr.push_back(packet.read());
            data.acknowledged.arr.push_back(packet.read());
            data.acknowledged.arr.push_back(packet.read());
            api::protocol::on_signed_chat_command.async_notify({data, *session, session->shared_data_ref()});
        }

        void chat_message(api::network::tcp::session* session, ArrayStream& packet) {
            std::string message = packet.read_string(256);
            int64_t timestamp = packet.read_value<int64_t>();
            int64_t salt = packet.read_value<int64_t>();
            if (packet.read() == 0) {
                api::protocol::data::chat_message_unsigned data;
                data.message = message;
                data.timestamp = timestamp;
                data.salt = salt;
                data.message_count = packet.read_var<int32_t>();
                data.acknowledged.arr.push_back(packet.read());
                data.acknowledged.arr.push_back(packet.read());
                data.acknowledged.arr.push_back(packet.read());
                api::protocol::on_chat_message_unsigned.async_notify({data, *session, session->shared_data_ref()});
            } else {
                api::protocol::data::chat_message_signed data;
                for (int i = 0; i < 256; i++)
                    data.signature[i] = packet.read();
                data.message = message;
                data.timestamp = timestamp;
                data.salt = salt;
                data.message_count = packet.read_var<int32_t>();
                data.acknowledged.arr.push_back(packet.read());
                data.acknowledged.arr.push_back(packet.read());
                data.acknowledged.arr.push_back(packet.read());
                api::protocol::on_chat_message_signed.async_notify({data, *session, session->shared_data_ref()});
            }
        }

        void player_session(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::player_session data;
            data.session_id = packet.read_uuid();
            data.public_key.expiries_at = packet.read_value<int64_t>();
            data.public_key.public_key = packet.read_list_array();
            data.public_key.key_signature = packet.read_list_array();
            api::protocol::on_player_session.async_notify({data, *session, session->shared_data_ref()});
        }

        void chunk_batch_received(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_chunk_batch_received.async_notify({packet.read_value<float>(), *session, session->shared_data_ref()});
        }

        void client_status(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_client_status.async_notify({packet.read_var<int32_t>(), *session, session->shared_data_ref()});
        }

        void client_tick_end(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_client_tick_end.async_notify({true, *session, session->shared_data_ref()});
        }

        void client_information(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::client_information data;
            data.locale = packet.read_string(16);
            data.view_distance = packet.read();
            data.chat_mode = packet.read_var<int32_t>();
            data.chat_colors = packet.read();
            data.displayed_skin_parts = packet.read();
            data.main_hand = packet.read_var<int32_t>();
            data.enable_text_filtering = packet.read();
            data.allow_server_listings = packet.read();
            data.particle_status = packet.read_var<int32_t>();
            api::protocol::on_client_information.async_notify({data, *session, session->shared_data_ref()});
        }

        void command_suggestion(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::command_suggestion data;
            data.transaction_id = packet.read_var<int32_t>();
            data.text = packet.read_string(32500);
            api::protocol::on_command_suggestion.async_notify({data, *session, session->shared_data_ref()});
        }

        void switch_to_configuration(api::network::tcp::session* session, ArrayStream& packet) {
            session->shared_data().packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::configuration;
            session->shared_data().switchToHandler(client_handler::abstract::createhandle_configuration(session));
        }

        void click_container_button(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::click_container_button data;
            data.window_id = packet.read();
            data.button_id = packet.read();
            api::protocol::on_click_container_button.async_notify({data, *session, session->shared_data_ref()});
        }

        void click_container(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::click_container data;
            data.window_id = packet.read();
            data.state_id = packet.read_var<int32_t>();
            data.slot = packet.read_value<int16_t>();
            data.button = packet.read();
            data.mode = packet.read_var<int32_t>();
            int32_t changed_slots_count = packet.read_var<int32_t>();
            data.changed_slots.reserve(changed_slots_count);
            for (int32_t i = 0; i < changed_slots_count; i++) {
                api::protocol::data::click_container::changed_slot slot;
                slot.slot = packet.read_value<int16_t>();
                slot.item = reader::ReadSlot(packet);
                data.changed_slots.push_back(slot);
            }
            data.carried_item = reader::ReadSlot(packet);
            api::protocol::on_click_container.async_notify({data, *session, session->shared_data_ref()});
        }

        void close_container(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_close_container.async_notify({packet.read_var<int32_t>(), *session, session->shared_data_ref()});
        }

        void change_container_slot_state(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::change_container_slot_state data;
            data.slot_id = packet.read_var<int32_t>();
            data.window_id = packet.read_var<int32_t>();
            data.state = packet.read();
            api::protocol::on_change_container_slot_state.async_notify({data, *session, session->shared_data_ref()});
        }

        void cookie_response(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::cookie_response data;
            data.key = packet.read_identifier();
            if (packet.read())
                data.payload = packet.read_array<uint8_t>(5120);
            api::protocol::on_cookie_response.async_notify({data, *session, session->shared_data_ref()});
            if (auto plugin = pluginManagement.get_bind_cookies(PluginManagement::registration_on::play, data.key); plugin) {
                if (auto response = plugin->OnPlayCookie(plugin, data.key, data.payload ? *data.payload : list_array<uint8_t>{}, session->shared_data_ref()); response)
                    session->shared_data().sendPacket(std::move(*response));
            }
        }

        void plugin_message(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::plugin_message msg;
            msg.channel = packet.read_identifier();
            msg.data = packet.read_left(32767).to_vector();
            auto plugin = pluginManagement.get_bind_plugin(PluginManagement::registration_on::play, msg.channel);
            if (plugin)
                if (auto res = plugin->OnPlayHandle(plugin, msg.channel, msg.data, session->shared_data_ref()); res != std::nullopt)
                    session->shared_data().sendPacket(std::move(*res));
        }

        void subscribe_to_debug_sample(api::network::tcp::session* session, ArrayStream& packet) {
            auto data = (api::protocol::data::debug_sample_subscription)packet.read_var<int32_t>();
            api::protocol::on_debug_sample_subscription.async_notify({data, *session, session->shared_data_ref()});
        }

        void edit_book(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::edit_book data;
            data.slot = packet.read_var<int32_t>();
            int32_t text_count = packet.read_var<int32_t>();
            data.text.reserve(text_count);
            data.text.push_back(packet.read_string(8192));
            if (packet.read())
                data.title = packet.read_string(128);
            api::protocol::on_edit_book.async_notify({data, *session, session->shared_data_ref()});
        }

        void query_entity_tag(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::query_entity_tag data;
            data.transaction_id = packet.read_var<int32_t>();
            data.entity_id = packet.read_var<int32_t>();
            api::protocol::on_query_entity_tag.async_notify({data, *session, session->shared_data_ref()});
        }

        void interact(api::network::tcp::session* session, ArrayStream& packet) {
            int32_t entity_id = packet.read_var<int32_t>();
            int32_t type = packet.read_var<int32_t>();
            switch (type) {
            case 0: {
                api::protocol::data::interact data;
                data.entity_id = entity_id;
                data.hand = packet.read();
                data.sneaking = packet.read();
                api::protocol::on_interact.async_notify({data, *session, session->shared_data_ref()});
            }
            case 1: {
                api::protocol::data::interact_attack data;
                data.entity_id = entity_id;
                data.sneaking = packet.read();
                api::protocol::on_interact_attack.async_notify({data, *session, session->shared_data_ref()});
            }
            case 2: {
                api::protocol::data::interact_at data;
                data.entity_id = entity_id;
                data.target_x = packet.read_value<float>();
                data.target_y = packet.read_value<float>();
                data.target_z = packet.read_value<float>();
                data.hand = packet.read();
                data.sneaking = packet.read();
                api::protocol::on_interact_at.async_notify({data, *session, session->shared_data_ref()});
            }
            default: {
                throw std::runtime_error("Unrecognized interact type.");
            }
            }
        }

        void jigsaw_generate(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::jigsaw_generate data;
            data.location.raw = packet.read_value<int64_t>();
            data.levels = packet.read_var<int32_t>();
            data.keep_jigsaws = packet.read();
            api::protocol::on_jigsaw_generate.async_notify({data, *session, session->shared_data_ref()});
        }

        void lock_difficulty(api::network::tcp::session* session, ArrayStream& packet) {
            bool lock = packet.read();
            api::protocol::on_lock_difficulty.async_notify({lock, *session, session->shared_data_ref()});
        }

        void set_player_position(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::set_player_position data;
            data.x = packet.read_value<double>();
            data.y = packet.read_value<double>();
            data.z = packet.read_value<double>();
            data.on_ground = packet.read();
            api::protocol::on_set_player_position.async_notify({data, *session, session->shared_data_ref()});
        }

        void set_player_position_and_rotation(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::set_player_position_and_rotation data;
            data.x = packet.read_value<double>();
            data.y = packet.read_value<double>();
            data.z = packet.read_value<double>();
            data.yaw = packet.read_value<float>();
            data.pitch = packet.read_value<float>();
            data.on_ground = packet.read();
            api::protocol::on_set_player_position_and_rotation.async_notify({data, *session, session->shared_data_ref()});
        }

        void set_player_rotation(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::set_player_rotation data;
            data.yaw = packet.read_value<float>();
            data.pitch = packet.read_value<float>();
            data.on_ground = packet.read();
            api::protocol::on_set_player_rotation.async_notify({data, *session, session->shared_data_ref()});
        }

        void set_player_movement_flags(api::network::tcp::session* session, ArrayStream& packet) {
            uint8_t flags = packet.read();
            api::protocol::on_set_player_movement_flags.async_notify({flags, *session, session->shared_data_ref()});
        }

        void move_vehicle(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::move_vehicle data;
            data.x = packet.read_value<double>();
            data.y = packet.read_value<double>();
            data.z = packet.read_value<double>();
            data.yaw = packet.read_value<float>();
            data.pitch = packet.read_value<float>();
            data.on_ground = packet.read();
            api::protocol::on_move_vehicle.async_notify({data, *session, session->shared_data_ref()});
        }

        void paddle_boat(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::paddle_boat data;
            data.left_paddle = packet.read();
            data.right_paddle = packet.read();
            api::protocol::on_paddle_boat.async_notify({data, *session, session->shared_data_ref()});
        }

        void pick_item_from_block(api::network::tcp::session* session, ArrayStream& packet) {
            base_objects::position pos;
            pos.raw = packet.read_value<uint64_t>();
            api::protocol::on_pick_item_from_block.async_notify({{pos, (bool)packet.read()}, *session, session->shared_data_ref()});
        }

        void pick_item_from_entity(api::network::tcp::session* session, ArrayStream& packet) {
            int32_t id = packet.read_var<int32_t>();
            api::protocol::on_pick_item_from_block.async_notify({{id, (bool)packet.read()}, *session, session->shared_data_ref()});
        }

        void pong(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::pong data;
            data.id = packet.read_value<int32_t>();
            data.elapsed = std::chrono::system_clock::now() - session->shared_data().packets_state.pong_timer;
            if (data.id == session->shared_data().packets_state.excepted_pong) {
                session->shared_data().packets_state.excepted_pong = -1;
                api::protocol::on_pong.async_notify({data, *session, session->shared_data_ref()});
            }
        }

        void keep_alive(api::network::tcp::session* session, ArrayStream& packet) {
            session->shared_data().gotKeepAlive(packet.read_value<int64_t>());
        }

        void ping_request(api::network::tcp::session* session, ArrayStream& packet) {
            int64_t ping = packet.read_value<int64_t>();
            api::protocol::on_ping_request.async_notify({ping, *session, session->shared_data_ref()});
            list_array<uint8_t> result;
            result.reserve(9);
            result.push_back(0x34);
            WriteValue<int64_t>(ping, result);
            session->shared_data_ref()->sendPacket(base_objects::network::response::answer({std::move(result)}));
        }

        void place_recipe(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::place_recipe data;
            data.window_id = packet.read_var<int32_t>();
            data.recipe_id = packet.read_var<int32_t>();
            data.make_all = packet.read_value<bool>();
            api::protocol::on_place_recipe.async_notify({data, *session, session->shared_data_ref()});
        }

        void player_abilities(api::network::tcp::session* session, ArrayStream& packet) {
            int8_t flags = packet.read_value<int8_t>();
            bool flying = flags & 0x02;
            api::protocol::on_player_abilities.async_notify({flags, *session, session->shared_data_ref()});
            session->shared_data().player_data.abilities.flags.flying = flying;
        }

        void player_action(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::player_action data;
            data.status = packet.read_var<int32_t>();
            data.location.raw = packet.read_value<int64_t>();
            data.face = packet.read_value<int8_t>();
            data.sequence_id = packet.read_var<int32_t>();
            api::protocol::on_player_action.async_notify({data, *session, session->shared_data_ref()});
        }

        void player_command(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::player_command data;
            data.entity_id = packet.read_var<int32_t>();
            data.action_id = packet.read_var<int32_t>();
            data.jump_boost = packet.read_var<int32_t>();
            api::protocol::on_player_command.async_notify({data, *session, session->shared_data_ref()});
        }

        void player_input(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::player_input data;
            auto flags = packet.read_value<int8_t>();
            data.flags.forward = flags & 1;
            data.flags.backward = flags & 2;
            data.flags.left = flags & 4;
            data.flags.right = flags & 8;
            data.flags.jump = flags & 16;
            data.flags.sneaking = flags & 32;
            data.flags.sprint = flags & 64;
            api::protocol::on_player_input.async_notify({data, *session, session->shared_data_ref()});
        }

        void player_loaded(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_player_loaded.sync_notify({true, *session, session->shared_data_ref()});
        }

        void change_recipe_book_settings(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::change_recipe_book_settings data;
            data.book_id = packet.read_var<int32_t>();
            data.book_open = packet.read_value<bool>();
            data.filter_active = packet.read_value<bool>();
            api::protocol::on_change_recipe_book_settings.async_notify({data, *session, session->shared_data_ref()});
        }

        void set_seen_recipe(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_set_seen_recipe.async_notify({packet.read_identifier(), *session, session->shared_data_ref()});
        }

        void rename_item(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_rename_item.async_notify({packet.read_identifier(), *session, session->shared_data_ref()});
        }

        void resource_pack_response(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::resource_pack_response data;
            data.uuid = packet.read_uuid();
            data.result = packet.read_var<int32_t>();
            api::protocol::on_resource_pack_response.async_notify({data, *session, session->shared_data_ref()});
        }

        void seen_advancements(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::seen_advancements data;
            data.action = packet.read_var<int32_t>();
            if (data.action == 1)
                data.tab_id = packet.read_identifier();
            api::protocol::on_seen_advancements.async_notify({data, *session, session->shared_data_ref()});
        }

        void select_trade(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_select_trade.async_notify({packet.read_var<int32_t>(), *session, session->shared_data_ref()});
        }

        void set_beacon_effect(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::set_beacon_effect data;
            if (packet.read_value<bool>())
                data.primary_effect = packet.read_var<int32_t>();
            if (packet.read_value<bool>())
                data.secondary_effect = packet.read_var<int32_t>();
            api::protocol::on_set_beacon_effect.async_notify({data, *session, session->shared_data_ref()});
        }

        void set_held_item(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_set_held_item.async_notify({packet.read_value<int16_t>(), *session, session->shared_data_ref()});
        }

        void program_command_block(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::program_command_block data;
            data.location.raw = packet.read_value<int64_t>();
            data.command = packet.read_identifier();
            data.mode = packet.read_var<int32_t>();
            data.flags = packet.read_value<int8_t>();
            api::protocol::on_program_command_block.async_notify({data, *session, session->shared_data_ref()});
        }

        void program_command_cart(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::program_command_cart data;
            data.entity_id = packet.read_var<int32_t>();
            data.command = packet.read_identifier();
            data.track_output = packet.read_value<bool>();
            api::protocol::on_program_command_cart.async_notify({data, *session, session->shared_data_ref()});
        }

        void set_creative_slot(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::set_creative_slot data;
            data.slot = packet.read_value<int16_t>();
            data.item = reader::ReadSlot(packet);
            api::protocol::on_set_creative_slot.async_notify({data, *session, session->shared_data_ref()});
        }

        void program_jigsaw_block(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::program_jigsaw_block data;
            data.location.raw = packet.read_value<int64_t>();
            data.name = packet.read_identifier();
            data.target = packet.read_identifier();
            data.pool = packet.read_identifier();
            data.final_state = packet.read_identifier();
            data.joint_type = packet.read_identifier();
            data.selection_priority = packet.read_var<int32_t>();
            data.placement_priority = packet.read_var<int32_t>();
            api::protocol::on_program_jigsaw_block.async_notify({data, *session, session->shared_data_ref()});
        }

        void program_structure_block(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::program_structure_block data;
            data.location.raw = packet.read_value<int64_t>();
            data.action = packet.read_var<int32_t>();
            data.mode = packet.read_var<int32_t>();
            data.name = packet.read_identifier();
            data.offset_x = packet.read_value<int8_t>();
            data.offset_y = packet.read_value<int8_t>();
            data.offset_z = packet.read_value<int8_t>();
            data.size_x = packet.read_var<int8_t>();
            data.size_y = packet.read_var<int8_t>();
            data.size_z = packet.read_var<int8_t>();
            data.mirror = packet.read_var<int32_t>();
            data.rotation = packet.read_var<int32_t>();
            data.metadata = packet.read_string(128);
            data.integrity = packet.read_value<float>();
            data.seed = packet.read_value<int64_t>();
            data.flags = packet.read_value<int8_t>();
            api::protocol::on_program_structure_block.async_notify({data, *session, session->shared_data_ref()});
        }

        void update_sign(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::update_sign data;
            data.location.raw = packet.read_value<int64_t>();
            data.is_front_text = packet.read_value<bool>();
            data.line1 = packet.read_string(384);
            data.line2 = packet.read_string(384);
            data.line3 = packet.read_string(384);
            data.line4 = packet.read_string(384);
            api::protocol::on_update_sign.async_notify({data, *session, session->shared_data_ref()});
        }

        void swing_arm(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_swing_arm.async_notify({{packet.read_var<int32_t>()}, *session, session->shared_data_ref()});
        }

        void spectator_teleport(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::on_spectator_teleport.async_notify({{packet.read_uuid()}, *session, session->shared_data_ref()});
        }

        void use_item_on(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::use_item_on data;
            data.hand = packet.read_var<int32_t>();
            data.location.raw = packet.read_value<int64_t>();
            data.face = packet.read_var<int32_t>();
            data.cursor_x = packet.read_value<float>();
            data.cursor_y = packet.read_value<float>();
            data.cursor_z = packet.read_value<float>();
            data.inside_block = packet.read_value<bool>();
            data.world_border_hit = packet.read_value<bool>();
            data.sequence = packet.read_var<int32_t>();
            api::protocol::on_use_item_on.async_notify({data, *session, session->shared_data_ref()});
        }

        void use_item(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::use_item data;
            data.hand = packet.read_var<int32_t>();
            data.sequence = packet.read_var<int32_t>();
            data.yaw = packet.read_value<float>();
            data.pitch = packet.read_value<float>();
            api::protocol::on_use_item.async_notify({data, *session, session->shared_data_ref()});
        }
    }

    class ProtocolSupport_770 : public PluginAutoRegister<"protocol_support_for_770", ProtocolSupport_770> {
    public:
        void OnRegister(const PluginRegistrationPtr& self) {
            base_objects::network::tcp::packet_registry.serverbound.play.register_seq(
                770,
                {
                    {"teleport_confirm", decoding::play::teleport_confirm},
                    {"query_block_nbt", decoding::play::query_block_nbt},
                    {"bundle_item_selected", decoding::play::bundle_item_selected},
                    {"change_difficulty", decoding::play::change_difficulty},
                    {"acknowledge_message", decoding::play::acknowledge_message},
                    {"chat_command", decoding::play::chat_command},
                    {"signed_chat_command", decoding::play::signed_chat_command},
                    {"chat_message", decoding::play::chat_message},
                    {"player_session", decoding::play::player_session},
                    {"chunk_batch_received", decoding::play::chunk_batch_received},
                    {"client_status", decoding::play::client_status},
                    {"client_tick_end", decoding::play::client_tick_end},
                    {"client_information", decoding::play::client_information},
                    {"command_suggestion", decoding::play::command_suggestion},
                    {"switch_to_configuration", decoding::play::switch_to_configuration},
                    {"click_container_button", decoding::play::click_container_button},
                    {"click_container", decoding::play::click_container},
                    {"close_container", decoding::play::close_container},
                    {"change_container_slot_state", decoding::play::change_container_slot_state},
                    {"cookie_response", decoding::play::cookie_response},
                    {"plugin_message", decoding::play::plugin_message},
                    {"subscribe_to_debug_sample", decoding::play::subscribe_to_debug_sample},
                    {"edit_book", decoding::play::edit_book},
                    {"query_entity_tag", decoding::play::query_entity_tag},
                    {"interact", decoding::play::interact},
                    {"jigsaw_generate", decoding::play::jigsaw_generate},
                    {"keep_alive", decoding::play::keep_alive},
                    {"lock_difficulty", decoding::play::lock_difficulty},
                    {"set_player_position", decoding::play::set_player_position},
                    {"set_player_position_and_rotation", decoding::play::set_player_position_and_rotation},
                    {"set_player_rotation", decoding::play::set_player_rotation},
                    {"set_player_movement_flags", decoding::play::set_player_movement_flags},
                    {"move_vehicle", decoding::play::move_vehicle},
                    {"paddle_boat", decoding::play::paddle_boat},
                    {"pick_item_from_block", decoding::play::pick_item_from_block},
                    {"pick_item_from_entity", decoding::play::pick_item_from_entity},
                    {"ping_request", decoding::play::ping_request},
                    {"place_recipe", decoding::play::place_recipe},
                    {"player_abilities", decoding::play::player_abilities},
                    {"player_action", decoding::play::player_action},
                    {"player_command", decoding::play::player_command},
                    {"player_input", decoding::play::player_input},
                    {"player_loaded", decoding::play::player_loaded},
                    {"pong", decoding::play::pong},
                    {"change_recipe_book_settings", decoding::play::change_recipe_book_settings},
                    {"set_seen_recipe", decoding::play::set_seen_recipe},
                    {"rename_item", decoding::play::rename_item},
                    {"resource_pack_response", decoding::play::resource_pack_response},
                    {"seen_advancements", decoding::play::seen_advancements},
                    {"select_trade", decoding::play::select_trade},
                    {"set_beacon_effect", decoding::play::set_beacon_effect},
                    {"set_held_item", decoding::play::set_held_item},
                    {"program_command_block", decoding::play::program_command_block},
                    {"program_command_cart", decoding::play::program_command_cart},
                    {"set_creative_slot", decoding::play::set_creative_slot},
                    {"program_jigsaw_block", decoding::play::program_jigsaw_block},
                    {"program_structure_block", decoding::play::program_structure_block},
                    {"update_sign", decoding::play::update_sign},
                    {"swing_arm", decoding::play::swing_arm},
                    {"spectator_teleport", decoding::play::spectator_teleport},
                    {"use_item_on", decoding::play::use_item_on},
                    {"use_item", decoding::play::use_item},
                }
            );
            api::packets::registry::register_protocol(770, {new encoding::login_functions(), new encoding::configuration_functions(), new encoding::play_functions()});
        }

        void OnUnregister(const PluginRegistrationPtr& self) {
            base_objects::network::tcp::packet_registry.serverbound.play.unregister_protocol(770);
            api::packets::registry::erase_protocol(770);
        }
    };
}