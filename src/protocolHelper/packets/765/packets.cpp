#include <src/api/configuration.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/mojang/session_server.hpp>
#include <src/base_objects/network/tcp_server.hpp>
#include <src/base_objects/slot_display.hpp>
#include <src/protocolHelper/packets/765/packets.hpp>
#include <src/protocolHelper/packets/765/writers_readers.hpp>
#include <src/protocolHelper/util.hpp>
#include <src/registers.hpp>

namespace copper_server::packets::release_765 {
    namespace login {
        base_objects::network::response login(int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data) {
            base_objects::network::response::item packet;
            packet.write_id(0x04);
            packet.write_var32(plugin_message_id);
            packet.write_identifier(chanel);
            packet.write_direct(data);
            return packet;
        }

        base_objects::network::response kick(const Chat& reason) {
            base_objects::network::response::item packet;
            packet.write_id(0x00);
            packet.write_direct(reason.ToTextComponent());
            return base_objects::network::response::disconnect({std::move(packet)});
        }

        base_objects::network::response encryptionRequest(const std::string& server_id, uint8_t (&verify_token)[4]) {
            list_array<uint8_t> response;
            auto public_key = base_objects::network::tcp_server::instance().public_key_buffer();

            response.push_back(1);
            WriteString(response, server_id, 20); //server id
            WriteVar<int32_t>(public_key.size(), response);
            response.push_back((uint8_t*)public_key.data(), public_key.size());
            WriteVar<int32_t>(4, response);
            response.push_back(verify_token[0]);
            response.push_back(verify_token[1]);
            response.push_back(verify_token[2]);
            response.push_back(verify_token[3]);
            return base_objects::network::response::answer({std::move(response)});
        }

        base_objects::network::response loginSuccess(base_objects::SharedClientData& client) {
            if (api::configuration::get().server.offline_mode)
                client.data = api::mojang::get_session_server().hasJoined(client.name, "", false);
            if (!client.data)
                return kick("Internal error");

            list_array<uint8_t> response;
            response.push_back(2);
            WriteUUID(client.data->uuid, response);
            WriteString(response, client.name, 16);
            auto& properties = client.data->properties;
            WriteVar<int32_t>(properties.size(), response);
            for (auto& it : properties) {
                WriteString(response, it.name, 32767);
                WriteString(response, it.value, 32767);
                WriteValue(it.signature.has_value(), response);
                if (it.signature.has_value())
                    WriteString(response, *it.signature, 32767);
            }
            return base_objects::network::response::answer({std::move(response)});
        }

        base_objects::network::response disableCompression() {
            return base_objects::network::response::enable_compress_answer(list_array<uint8_t>::concat(0x03, 0), 0);
        }

        base_objects::network::response setCompression(int32_t threshold) {
            list_array<uint8_t> packet;
            packet.push_back(0x03);
            WriteVar<int32_t>(threshold, packet);
            return base_objects::network::response::enable_compress_answer(std::move(packet), threshold);
        }
    }

    namespace configuration {
        base_objects::network::response configuration(const std::string& chanel, const list_array<uint8_t>& data) {
            list_array<uint8_t> packet;
            packet.push_back(0x00);
            WriteIdentifier(packet, chanel);
            packet.push_back(data);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response kick(const Chat& reason) {
            list_array<uint8_t> packet = reason.ToTextComponent();
            packet.reserve_front(1);
            packet.push_front(0x01); //disconnect
            return base_objects::network::response::disconnect({packet});
        }

        base_objects::network::response finish() {
            list_array<uint8_t> packet;
            packet.push_back(0x02);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response keep_alive(int64_t keep_alive_packet) {
            list_array<uint8_t> response;
            response.reserve(9);
            response.push_back(0x03);
            WriteValue<int64_t>(keep_alive_packet, response);
            return base_objects::network::response::answer({std::move(response)});
        }

        base_objects::network::response ping(int32_t excepted_pong) {
            list_array<uint8_t> response;
            response.reserve(5);
            response.push_back(0x04);
            WriteVar<int32_t>(excepted_pong, response);
            return base_objects::network::response::answer({std::move(response)});
        }

        base_objects::network::response registry_data() {
            using namespace registers;
            static list_array<uint8_t> data;
            if (data.empty()) {
                data.push_back(0x05);
                enbt::compound nbt;
                { //minecraft:trim_material
                    enbt::dynamic_array _armorTrimMaterials;
                    for (auto& [name, it] : armorTrimMaterials) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["asset_name"] = it.asset_name;
                            element["ingredient"] = it.ingredient.to_protocol_name(765);
                            element["item_model_index"] = it.item_model_index;
                            {
                                enbt::compound override_armor_materials;
                                for (auto& [key, value] : it.override_armor_materials)
                                    override_armor_materials[key] = value;
                                element["override_armor_materials"] = std::move(override_armor_materials);
                            }
                            if (std::holds_alternative<std::string>(it.description))
                                element["description"] = std::get<std::string>(it.description);
                            else
                                element["description"] = std::get<Chat>(it.description).ToENBT();
                            tmp["element"] = std::move(element);
                        }
                        _armorTrimMaterials.push_back((enbt::value&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:trim_material";
                    entry["value"] = std::move(_armorTrimMaterials);
                    nbt["minecraft:trim_material"] = std::move(entry);
                }
                { //minecraft:trim_pattern
                    enbt::dynamic_array _armorTrimPatterns;
                    for (auto& [name, it] : armorTrimPatterns) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
                            enbt::compound element;
                            element["asset_id"] = it.asset_id;
                            element["template_item"] = it.template_item.to_protocol_name(765);
                            if (std::holds_alternative<std::string>(it.description))
                                element["description"] = std::get<std::string>(it.description);
                            else
                                element["description"] = std::get<Chat>(it.description).ToENBT();
                            element["decal"] = it.decal;
                            tmp["element"] = std::move(element);
                        }
                        _armorTrimPatterns.push_back((enbt::value&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:trim_pattern";
                    entry["value"] = std::move(_armorTrimPatterns);
                    nbt["minecraft:trim_pattern"] = std::move(entry);
                }
                { //minecraft:worldgen/biome
                    enbt::dynamic_array _biomes;
                    for (auto& [name, it] : biomes) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
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
                                    particle["type"] = it.effects.particle->options.type;
                                    particle["options"] = it.effects.particle->options.options;
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
                                if (it.effects.music.size()) {
                                    auto& music_it = *it.effects.music.begin();
                                    enbt::compound music;
                                    music["sound"] = music_it.sound;
                                    music["min_delay"] = music_it.min_delay;
                                    music["max_delay"] = music_it.max_delay;
                                    music["replace_current_music"] = music_it.replace_current_music;
                                    effects["music"] = std::move(music);
                                }
                                element["effects"] = std::move(effects);
                            }
                            tmp["element"] = std::move(element);
                        }
                        _biomes.push_back((enbt::value&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:worldgen/biome";
                    entry["value"] = std::move(_biomes);
                    nbt["minecraft:worldgen/biome"] = std::move(entry);
                }
                { // minecraft:chat_type
                    enbt::dynamic_array _chatTypes;
                    for (auto& [name, it] : chatTypes) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
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
                            tmp["element"] = std::move(element);
                        }
                        _chatTypes.push_back((enbt::value&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:chat_type";
                    entry["value"] = std::move(_chatTypes);
                    nbt["minecraft:chat_type"] = std::move(entry);
                }
                { // minecraft:damage_type
                    enbt::dynamic_array _damageTypes;
                    for (auto& [name, it] : damageTypes) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
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
                            tmp["element"] = std::move(element);
                        }
                        _damageTypes.push_back((enbt::value&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:damage_type";
                    entry["value"] = std::move(_damageTypes);
                    nbt["minecraft:damage_type"] = std::move(entry);
                }
                { // minecraft:dimension_type
                    enbt::dynamic_array _dimensionTypes;
                    for (auto& [name, it] : dimensionTypes) {
                        enbt::compound tmp;
                        tmp["name"] = name;
                        tmp["id"] = it.id;
                        { //element
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
                            tmp["element"] = std::move(element);
                        }
                        _dimensionTypes.push_back((enbt::value&&)std::move(tmp));
                    }
                    enbt::compound entry;
                    entry["type"] = "minecraft:dimension_type";
                    entry["value"] = std::move(_dimensionTypes);
                    nbt["minecraft:dimension_type"] = std::move(entry);
                }
                data.push_back(NBT::build((enbt::value&)nbt).get_as_network());
            }
            return base_objects::network::response::answer({data});
        }

        base_objects::network::response removeResourcePacks() {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x06, false)});
        }

        base_objects::network::response removeResourcePack(const enbt::raw_uuid& pack_id) {
            list_array<uint8_t> packet;
            packet.reserve(18);
            packet.push_back(0x06);
            packet.push_back(true);
            WriteUUID(pack_id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced) {
            list_array<uint8_t> packet;
            packet.reserve(20 + url.size() + 2 + hash.size());
            packet.push_back(0x07);
            WriteUUID(pack_id, packet);
            WriteString(packet, url, 32767);
            WriteString(packet, hash, 40);
            packet.push_back(forced);
            packet.push_back(false);
            client.packets_state.pending_resource_packs[pack_id] = {forced};
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt) {
            list_array<uint8_t> packet;
            packet.reserve(20 + url.size() + 2 + hash.size());
            packet.push_back(0x07);
            WriteUUID(pack_id, packet);
            WriteString(packet, url, 32767);
            WriteString(packet, hash, 40);
            packet.push_back(forced);
            packet.push_back(true);
            packet.push_back(prompt.ToTextComponent());
            client.packets_state.pending_resource_packs[pack_id] = {forced};
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setFeatureFlags(const list_array<std::string>& features) {
            list_array<uint8_t> packet;
            packet.push_back(0x08);
            WriteVar<int16_t>(features.size(), packet);
            for (auto& it : features)
                WriteIdentifier(packet, it);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries) {
            list_array<uint8_t> packet;
            packet.push_back(0x09);
            WriteVar<int32_t>(tags_entries.size(), packet);
            for (auto& [registry, tags] : tags_entries) {
                WriteIdentifier(packet, registry);
                WriteVar<int32_t>(tags.size(), packet);
                for (auto& it : tags) {
                    WriteIdentifier(packet, it.tag_name);
                    WriteVar<int32_t>(it.entires.size(), packet);
                    for (auto& entry : it.entires)
                        WriteVar<int32_t>(entry, packet);
                }
            }
            return base_objects::network::response::answer({std::move(packet)});
        }
    }

    namespace play {
        base_objects::network::response bundleResponse(base_objects::network::response&& response) {
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

        base_objects::network::response spawnEntity(const base_objects::entity& entity, uint16_t protocol) {
            if (protocol == UINT16_MAX)
                protocol = 765;
            list_array<uint8_t> packet;
            packet.reserve(58);
            packet.push_back(0x01);
            auto view = base_objects::entity_data::view(entity);
            WriteVar<int32_t>(api::entity_id_map::get_id(entity.id), packet);
            WriteUUID(entity.id, packet);
            WriteVar<int32_t>(view.internal_entity_aliases.at(protocol), packet);
            WriteValue<double>(entity.position.x, packet);
            WriteValue<double>(entity.position.y, packet);
            WriteValue<double>(entity.position.z, packet);
            auto res = util::to_yaw_pitch(entity.rotation);
            WriteValue<int8_t>(res.x, packet);
            WriteValue<int8_t>(res.y, packet);
            res = util::to_yaw_pitch(entity.head_rotation);
            WriteValue<int8_t>(res.y, packet);
            if (view.get_object_field) {
                if (auto value = view.get_object_field(entity))
                    WriteVar<int32_t>(value, packet);
            } else
                WriteVar<int32_t>(0, packet);

            auto velocity = util::minecraft::packets::velocity(entity.motion);
            WriteValue<int16_t>(velocity.x, packet);
            WriteValue<int16_t>(velocity.y, packet);
            WriteValue<int16_t>(velocity.z, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response spawnExperienceOrb(const base_objects::entity& entity, int16_t count) {
            list_array<uint8_t> packet;
            packet.reserve(27);
            packet.push_back(0x02);
            WriteVar<int32_t>(api::entity_id_map::get_id(entity.id), packet);
            WriteValue<double>(entity.position.x, packet);
            WriteValue<double>(entity.position.y, packet);
            WriteValue<double>(entity.position.z, packet);
            WriteValue<int16_t>(count, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response entityAnimation(const base_objects::entity& entity, uint8_t animation) {
            list_array<uint8_t> packet;
            packet.reserve(6);
            packet.push_back(0x03);
            WriteVar<int32_t>(api::entity_id_map::get_id(entity.id), packet);
            packet.push_back(animation);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response awardStatistics(const list_array<base_objects::packets::statistics>& statistics) {
            list_array<uint8_t> packet;
            packet.reserve(3 * statistics.size() + 1 + 4);
            packet.push_back(0x04);
            WriteVar<int32_t>(statistics.size(), packet);
            for (auto& [category_id, statistic_id, value] : statistics) {
                WriteVar<int32_t>(category_id, packet);
                WriteVar<int32_t>(statistic_id, packet);
                WriteVar<int32_t>(value, packet);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response acknowledgeBlockChange(base_objects::SharedClientData& client) {
            list_array<uint8_t> packet;
            packet.reserve(5);
            packet.push_back(0x05);
            WriteVar<int32_t>(client.packets_state.play_data->current_block_sequence_id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setBlockDestroyStage(const base_objects::entity& entity, base_objects::position block, uint8_t stage) {
            list_array<uint8_t> packet;
            packet.reserve(14);
            packet.push_back(0x06);
            WriteVar<int32_t>(api::entity_id_map::get_id(entity.id), packet);
            WriteValue<uint64_t>(block.raw, packet);
            packet.push_back(stage);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response blockEntityData(base_objects::position block, int32_t type, const enbt::value& data) {

            list_array<uint8_t> packet;
            packet.reserve(13);
            packet.push_back(0x07);
            WriteValue<uint64_t>(block.raw, packet);
            WriteVar<int32_t>(type, packet);
            packet.push_back(NBT::build(data).get_as_normal());
            return base_objects::network::response::answer({std::move(packet)});
        }

        //block_type is from "minecraft:block" registry, not a block state.
        base_objects::network::response blockAction(base_objects::position block, int32_t action_id, int32_t param, int32_t block_type) {
            list_array<uint8_t> packet;
            packet.reserve(21);
            packet.push_back(0x08);
            WriteValue<uint64_t>(block.raw, packet);
            WriteVar<int32_t>(action_id, packet);
            WriteVar<int32_t>(param, packet);
            WriteVar<int32_t>(block_type, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        //block_type is from "minecraft:block" registry, not a block state.
        base_objects::network::response blockUpdate(base_objects::position block, int32_t block_type) {
            list_array<uint8_t> packet;
            packet.reserve(13);
            packet.push_back(0x09);
            WriteValue<uint64_t>(block.raw, packet);
            WriteVar<int32_t>(block_type, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response bossBarAdd(const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags) {
            list_array<uint8_t> packet;
            packet.reserve(4 * 4 + 1 + 16 + 2);
            packet.push_back(0x0A);
            WriteUUID(id, packet);
            packet.push_back(0);
            WriteString(packet, title.ToStr(), 32767);
            WriteValue<float>(health, packet);
            WriteVar<int32_t>(color, packet);
            WriteVar<int32_t>(division, packet);
            WriteValue<uint8_t>(flags, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response bossBarRemove(const enbt::raw_uuid& id) {
            list_array<uint8_t> packet;
            packet.reserve(18);
            packet.push_back(0x0A);
            WriteUUID(id, packet);
            packet.push_back(0);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response bossBarUpdateHealth(const enbt::raw_uuid& id, float health) {
            list_array<uint8_t> packet;
            packet.reserve(21);
            packet.push_back(0x0A);
            WriteUUID(id, packet);
            packet.push_back(2);
            WriteValue<float>(health, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response bossBarUpdateTitle(const enbt::raw_uuid& id, const Chat& title) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 16 + 2);
            packet.push_back(0x0A);
            WriteUUID(id, packet);
            packet.push_back(3);
            WriteString(packet, title.ToStr(), 32767);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response bossBarUpdateStyle(const enbt::raw_uuid& id, int32_t color, int32_t division) {
            list_array<uint8_t> packet;
            packet.reserve(25);
            packet.push_back(0x0A);
            WriteUUID(id, packet);
            packet.push_back(4);
            WriteVar<int32_t>(color, packet);
            WriteVar<int32_t>(division, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response bossBarUpdateFlags(const enbt::raw_uuid& id, uint8_t flags) {
            list_array<uint8_t> packet;
            packet.reserve(21);
            packet.push_back(0x0A);
            WriteUUID(id, packet);
            packet.push_back(5);
            WriteValue<uint8_t>(flags, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response changeDifficulty(uint8_t difficulty, bool locked) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x0B, difficulty, locked)});
        }

        base_objects::network::response chunkBatchFinished(int32_t count) {
            list_array<uint8_t> packet;
            packet.reserve(5);
            packet.push_back(0x0C);
            WriteVar<int32_t>(count, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response chunkBatchStart() {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x0D)});
        }

        base_objects::network::response chunkBiomes(list_array<base_objects::chunk::chunk_biomes>& chunk) {
            list_array<uint8_t> packet;
            packet.push_back(0x0E);
            WriteVar<int32_t>(chunk.size(), packet);
            for (auto& section : chunk) {
                WriteVar<int32_t>(section.z, packet);
                WriteVar<int32_t>(section.x, packet);
                WriteVar<int32_t>(section.biomes.size(), packet);
                list_array<uint8_t> biomes;
                for (auto& biome : section.biomes)
                    biomes.push_back(biome.serialize());

                packet.push_back(biomes);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response clearTitles(bool reset) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x0F, reset)});
        }

        base_objects::network::response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions) {
            list_array<uint8_t> packet;
            packet.reserve(17);
            packet.push_back(0x10);
            WriteVar<int32_t>(transaction_id, packet);
            WriteVar<int32_t>(start_pos, packet);
            WriteVar<int32_t>(length, packet);
            WriteVar<int32_t>(suggestions.size(), packet);
            for (auto& [suggestion, tooltip] : suggestions) {
                WriteString(packet, suggestion, 32767);
                packet.push_back(!tooltip);
                if (tooltip)
                    packet.push_back(tooltip->ToTextComponent());
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response commands(int32_t root_id, const list_array<base_objects::packets::command_node>& nodes) {
            list_array<uint8_t> packet;
            packet.reserve(5);
            packet.push_back(0x11);
            WriteVar<int32_t>(nodes.size(), packet);
            for (auto& node : nodes) {
                WriteValue(node.flags.raw, packet);
                WriteVar<int32_t>(node.children.size(), packet);
                for (auto& child : node.children)
                    WriteVar<int32_t>(child, packet);
                if (node.flags.has_redirect)
                    WriteVar<int32_t>(node.redirect_node.value(), packet);
                if (node.flags.node_type == base_objects::packets::command_node::node_type::literal || node.flags.node_type == base_objects::packets::command_node::node_type::argument)
                    WriteIdentifier(packet, node.name.value());
                if (node.flags.has_suggestion)
                    WriteIdentifier(packet, node.suggestion_type.value());
                if (node.flags.node_type == base_objects::packets::command_node::node_type::argument) {
                    WriteVar<int32_t>((int32_t)node.parser_id.value(), packet);
                    if (node.properties) {
                        auto& properties = *node.properties;
                        if (properties.flags)
                            packet.push_back(*properties.flags);
                        if (properties.min)
                            std::visit([&packet](auto&& arg) { WriteValue(arg, packet); }, *properties.min);
                        if (properties.max)
                            std::visit([&packet](auto&& arg) { WriteValue(arg, packet); }, *properties.max);
                        if (properties.registry)
                            WriteIdentifier(packet, *properties.registry);
                    }
                }
                if (node.flags.has_suggestion)
                    WriteIdentifier(packet, node.suggestion_type.value());
            }
            WriteVar<int32_t>(root_id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response closeContainer(uint8_t container_id) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x12, container_id)});
        }

        base_objects::network::response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item) {
            list_array<uint8_t> packet;
            packet.reserve(2 * slots.size() + 1 + 1 + 4);
            packet.push_back(0x13);
            packet.push_back(windows_id);
            WriteVar<int32_t>(state_id, packet);
            WriteVar<int32_t>(slots.size(), packet);
            for (auto& it : slots)
                reader::WriteSlot(packet, it);

            reader::WriteSlot(packet, carried_item);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value) {
            list_array<uint8_t> packet;
            packet.reserve(6);
            packet.push_back(0x14);
            packet.push_back(windows_id);
            WriteValue<uint16_t>(property, packet);
            WriteValue<uint16_t>(value, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 4 + 2 + (bool)item);
            packet.push_back(0x15);
            packet.push_back(windows_id);
            WriteVar<int32_t>(state_id, packet);
            WriteValue<int16_t>(slot, packet);

            reader::WriteSlot(packet, item);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setCooldown(int32_t item_id, int32_t cooldown) {
            list_array<uint8_t> packet;
            packet.reserve(9);
            packet.push_back(0x16);
            WriteVar<int32_t>(item_id, packet);
            WriteVar<int32_t>(cooldown, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        //UNUSED by Notchian client
        base_objects::network::response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions) {
            list_array<uint8_t> packet;
            packet.reserve(9);
            packet.push_back(0x17);
            WriteVar<int32_t>(action, packet);
            WriteVar<int32_t>(count, packet);
            for (auto& it : suggestions)
                WriteString(packet, it, 32767);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response customPayload(const std::string& channel, const list_array<uint8_t>& data) {
            list_array<uint8_t> packet;
            packet.reserve(1 + channel.size() + 2);
            packet.push_back(0x18);
            WriteString(packet, channel, 32767);
            packet.push_back(data);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<util::VECTOR> xyz) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 4 + 1 + (3 * 4) * (bool)xyz);
            packet.push_back(0x19);
            WriteVar<int32_t>(entity_id, packet);
            WriteVar<int32_t>(source_type_id, packet);
            WriteVar<int32_t>(source_cause_id, packet);
            WriteVar<int32_t>(source_direct_id, packet);
            packet.push_back((bool)xyz);
            if (xyz) {
                WriteValue<float>(xyz->x, packet);
                WriteValue<float>(xyz->y, packet);
                WriteValue<float>(xyz->z, packet);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response deleteMessageBySignature(uint8_t (&signature)[256]) {
            list_array<uint8_t> packet;
            packet.reserve(12 + 256);
            packet.push_back(0x1A);
            packet.push_back(0);
            packet.push_back(signature, 256);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response deleteMessageByID(int32_t message_id) {
            list_array<uint8_t> packet;
            packet.reserve(12 + 4);
            packet.push_back(0x1A);
            if (message_id == -1)
                throw std::runtime_error("message id can't be -1");
            WriteVar<int32_t>(message_id + 1, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response kick(const Chat& reason) {
            return base_objects::network::response::disconnect({list_array<uint8_t>::concat(0x1B, reason.ToTextComponent())});
        }

        base_objects::network::response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 2 + 2 + 1);
            packet.push_back(0x1C);
            packet.push_back(message.ToTextComponent());
            WriteVar<int32_t>(chat_type, packet);
            packet.push_back(sender.ToTextComponent());
            packet.push_back((bool)target_name);
            if (target_name)
                packet.push_back(target_name->ToTextComponent());

            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response entityEvent(int32_t entity_id, uint8_t entity_status) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 1);
            packet.push_back(0x1D);
            WriteValue<int32_t>(entity_id, packet);
            packet.push_back(entity_status);
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
            packet.push_back(0x1E);
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

        base_objects::network::response unloadChunk(int32_t x, int32_t z) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 2);
            packet.push_back(0x1F);
            WriteValue<int32_t>(z, packet);
            WriteValue<int32_t>(x, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response gameEvent(uint8_t event_id, float value) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 4);
            packet.push_back(0x20);
            packet.push_back(event_id);
            WriteValue<float>(value, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 2 + 16);
            packet.push_back(0x21);
            packet.push_back(window_id);
            WriteVar<int32_t>(slots, packet);
            WriteValue<int32_t>(entity_id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response hurtAnimation(int32_t entity_id, float yaw) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4);
            packet.push_back(0x22);
            WriteVar<int32_t>(entity_id, packet);
            WriteValue<float>(yaw, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 8 * 2 + 8 * 2 + 8 + 8 + 8 + 4 * 3);
            packet.push_back(0x23);
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
        base_objects::network::response keepAlive(int64_t id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 8);
            packet.push_back(0x24);
            WriteValue<int64_t>(id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateChunkDataWLights(
            int32_t chunk_x,
            int32_t chunk_z,
            const NBT& heightmaps,
            const std::vector<uint8_t>& data,
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

            packet.push_back(0x25);
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

        base_objects::network::response worldEvent(int32_t event, base_objects::position pos, int32_t data, bool global) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 8 + 4 + 1);
            packet.push_back(0x26);
            WriteVar<int32_t>(event, packet);
            WriteValue<uint64_t>(pos.raw, packet);
            WriteVar<int32_t>(data, packet);
            packet.push_back(global);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response particle(int32_t particle_id, bool long_distance, util::VECTOR pos, util::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 1 + 4 * 3 + 4 * 3 + 4 + 4 + 4 * data.size());
            packet.push_back(0x27);
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

            packet.push_back(0x28);
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

        base_objects::network::response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool __unused) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 1 + 4 + 1 + 4 + 1 + 4);
            packet.push_back(0x29);
            WriteVar<int32_t>(entity_id, packet);
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
            WriteIdentifier(packet, registers::dimensionTypes_cache[current_dimension_type]->first);
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
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t columns, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 1 + 1 + 4 * 2 + 1 + 1 + 1 + 1 + 2 * icons.size() + 4 * 4 + data.size());
            packet.push_back(0x2A);
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

        base_objects::network::response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade>& trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 + 4 * 2 + 1 + 1 + 1 + 1 + 1 + 2 * trades.size());
            packet.push_back(0x2B);
            packet.push_back(window_id);
            packet.push_back(trade_id);
            WriteVar<int32_t>(trades.size(), packet);
            for (auto& [input1, output, input2, number_of_trade_uses, max_number_of_trades, xp, spec_price, price_multiplier, demand, trade_disabled] : trades) {
                reader::WriteSlot(packet, input1);
                reader::WriteSlot(packet, output);
                reader::WriteSlot(packet, input2);
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

        base_objects::network::response updateEntityPosition(int32_t entity_id, util::XYZ<float> pos, bool on_ground) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * 3 + 1);
            packet.push_back(0x2C);
            WriteVar<int32_t>(entity_id, packet);
            auto res = util::minecraft::packets::delta_move(pos);
            WriteValue<int16_t>(res.x, packet);
            WriteValue<int16_t>(res.y, packet);
            WriteValue<int16_t>(res.z, packet);
            packet.push_back(on_ground);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateEntityPositionAndRotation(int32_t entity_id, util::XYZ<float> pos, util::VECTOR rot, bool on_ground) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * 3 + 4 * 3 + 1);
            packet.push_back(0x2D);
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

        base_objects::network::response updateEntityRotation(int32_t entity_id, util::VECTOR rot, bool on_ground) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * 2 + 1);
            packet.push_back(0x2E);
            WriteVar<int32_t>(entity_id, packet);
            auto res = util::to_yaw_pitch_256(rot);
            WriteValue<uint8_t>(res.x, packet);
            WriteValue<uint8_t>(res.y, packet);
            packet.push_back(on_ground);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response moveVehicle(util::VECTOR pos, util::VECTOR rot) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 3 + 4 * 3);
            packet.push_back(0x2F);
            WriteValue<double>(pos.x, packet);
            WriteValue<double>(pos.y, packet);
            WriteValue<double>(pos.z, packet);
            auto res = util::to_yaw_pitch(rot);
            WriteValue<float>(res.x, packet);
            WriteValue<float>(res.y, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response openBook(int32_t hand) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x30);
            WriteVar<int32_t>(hand, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response openScreen(int32_t window_id, int32_t type, const Chat& title) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 2);
            packet.push_back(0x31);
            packet.push_back(window_id);
            WriteVar<int32_t>(type, packet);
            packet.push_back(title.ToTextComponent());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response openSignEditor(base_objects::position pos, bool is_front_text) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 8 + 1);
            packet.push_back(0x32);
            WriteValue<uint64_t>(pos.raw, packet);
            packet.push_back(is_front_text);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response ping(int32_t id) {
            if (id == -1)
                throw std::runtime_error("ping id can't be -1");
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x33);
            WriteVar<int32_t>(id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response pingResponse(int32_t id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x34);
            WriteVar<int32_t>(id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + recipe_id.size());
            packet.push_back(0x35);
            packet.push_back(windows_id);
            WriteIdentifier(packet, recipe_id);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response playerAbilities(uint8_t flags, float flying_speed, float field_of_view) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 4 * 2);
            packet.push_back(0x36);
            packet.push_back(flags);
            WriteValue<float>(flying_speed, packet);
            WriteValue<float>(field_of_view, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response playerChatMessage(enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, const std::optional<enbt::value>& unsigned_content, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name) {
            if (prev_messages.size() > 20)
                throw std::runtime_error("too many prev messages");
            list_array<uint8_t> packet;
            packet.reserve(1 + 16 + 4 + 1 + message.size() + 8 + 8 + 4 * prev_messages.size() + 1 + 1 + 1 + 2 * 4 + 1 + (bool)target_name * 2);
            packet.push_back(0x37);
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
                packet.push_back(NBT::build(*unsigned_content).get_as_normal());

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

        //UNUSED by Notchian client
        base_objects::network::response endCombat(int32_t duration) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x38);
            WriteVar<int32_t>(duration, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        //UNUSED by Notchian client
        base_objects::network::response enterCombat() {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x39)});
        }

        base_objects::network::response combatDeath(int32_t player_id, const Chat& message) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x3A);
            WriteVar<int32_t>(player_id, packet);
            packet.push_back(message.ToTextComponent());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response playerInfoRemove(const list_array<enbt::raw_uuid>& players) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + players.size() * sizeof(enbt::raw_uuid));
            packet.push_back(0x3B);
            WriteVar<int32_t>(players.size(), packet);
            for (auto& it : players)
                WriteUUID(it, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response playerInfoAdd(const list_array<base_objects::packets::player_actions_add>& add_players) {
            list_array<uint8_t> packet;
            packet.push_back(0x3C);
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

        base_objects::network::response playerInfoInitializeChat(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat) {
            list_array<uint8_t> packet;
            packet.push_back(0x3C);
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

        base_objects::network::response playerInfoUpdateGameMode(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode) {
            list_array<uint8_t> packet;
            packet.push_back(0x3C);
            packet.push_back(0x04);
            WriteVar<int32_t>(update_game_mode.size(), packet);
            for (auto& [uuid, game_mode] : update_game_mode) {
                WriteUUID(uuid, packet);
                packet.push_back(game_mode);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response playerInfoUpdateListed(const list_array<base_objects::packets::player_actions_update_listed>& update_listed) {
            list_array<uint8_t> packet;
            packet.push_back(0x3C);
            packet.push_back(0x08);
            WriteVar<int32_t>(update_listed.size(), packet);
            for (auto& [uuid, listed] : update_listed) {
                WriteUUID(uuid, packet);
                packet.push_back(listed);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response playerInfoUpdateLatency(const list_array<base_objects::packets::player_actions_update_latency>& update_latency) {
            list_array<uint8_t> packet;
            packet.push_back(0x3C);
            packet.push_back(0x10);
            WriteVar<int32_t>(update_latency.size(), packet);
            for (auto& [uuid, latency] : update_latency) {
                WriteUUID(uuid, packet);
                WriteVar<int32_t>(latency, packet);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response playerInfoUpdateDisplayName(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x3C);
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

        base_objects::network::response lookAt(bool from_feet_or_eyes, util::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 4 * 3 + (bool)entity_id * 4 * 2);
            packet.push_back(0x3D);
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

        base_objects::network::response synchronizePlayerPosition(util::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 3 + 4 * 2 + 1);
            packet.push_back(0x3E);
            WriteValue<double>(pos.x, packet);
            WriteValue<double>(pos.y, packet);
            WriteValue<double>(pos.z, packet);
            WriteValue<float>(yaw, packet);
            WriteValue<float>(pitch, packet);
            packet.push_back(flags);
            WriteVar<int32_t>(teleport_id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& displayed_recipe_ids, const list_array<std::string>& had_access_to_recipe_ids) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 2 * displayed_recipe_ids.size() + 2 * had_access_to_recipe_ids.size());
            packet.push_back(0x3F);
            packet.push_back(0);
            packet.push_back(crafting_recipe_book_open);
            packet.push_back(crafting_recipe_book_filter_active);
            packet.push_back(smelting_recipe_book_open);
            packet.push_back(smelting_recipe_book_filter_active);
            packet.push_back(blast_furnace_recipe_book_open);
            packet.push_back(blast_furnace_recipe_book_filter_active);
            packet.push_back(smoker_recipe_book_open);
            packet.push_back(smoker_recipe_book_filter_active);
            WriteVar<int32_t>(displayed_recipe_ids.size(), packet);
            for (auto& it : displayed_recipe_ids)
                WriteIdentifier(packet, it);
            WriteVar<int32_t>(had_access_to_recipe_ids.size(), packet);
            for (auto& it : had_access_to_recipe_ids)
                WriteIdentifier(packet, it);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response addRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 2 * recipe_ids.size());
            packet.push_back(0x3F);
            packet.push_back(1);
            packet.push_back(crafting_recipe_book_open);
            packet.push_back(crafting_recipe_book_filter_active);
            packet.push_back(smelting_recipe_book_open);
            packet.push_back(smelting_recipe_book_filter_active);
            packet.push_back(blast_furnace_recipe_book_open);
            packet.push_back(blast_furnace_recipe_book_filter_active);
            packet.push_back(smoker_recipe_book_open);
            packet.push_back(smoker_recipe_book_filter_active);
            WriteVar<int32_t>(recipe_ids.size(), packet);
            for (auto& it : recipe_ids)
                WriteIdentifier(packet, it);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response removeRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 2 * recipe_ids.size());
            packet.push_back(0x3F);
            packet.push_back(2);
            packet.push_back(crafting_recipe_book_open);
            packet.push_back(crafting_recipe_book_filter_active);
            packet.push_back(smelting_recipe_book_open);
            packet.push_back(smelting_recipe_book_filter_active);
            packet.push_back(blast_furnace_recipe_book_open);
            packet.push_back(blast_furnace_recipe_book_filter_active);
            packet.push_back(smoker_recipe_book_open);
            packet.push_back(smoker_recipe_book_filter_active);
            WriteVar<int32_t>(recipe_ids.size(), packet);
            for (auto& it : recipe_ids)
                WriteIdentifier(packet, it);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active) {
            return addRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, {});
        }

        base_objects::network::response removeEntities(const list_array<int32_t>& entity_ids) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * entity_ids.size());
            packet.push_back(0x40);
            WriteVar<int32_t>(entity_ids.size(), packet);
            for (auto& it : entity_ids)
                WriteVar<int32_t>(it, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response removeEntityEffect(int32_t entity_id, int32_t effect_id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 2);
            packet.push_back(0x41);
            WriteVar<int32_t>(entity_id, packet);
            WriteVar<int32_t>(effect_id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name) {
            list_array<uint8_t> packet;
            packet.reserve(1 + entity_name.size() + (bool)objective_name * objective_name->size());
            packet.push_back(0x42);
            WriteString(packet, entity_name, 32767);
            packet.push_back((bool)objective_name);
            if (objective_name)
                WriteString(packet, *objective_name, 32767);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response removeResourcePacks() {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x43, false)});
        }

        base_objects::network::response removeResourcePack(enbt::raw_uuid id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 16);
            packet.push_back(0x43);
            packet.push_back(true);
            WriteUUID(id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response addResourcePack(enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 16 + url.size() + hash.size() + 1);
            packet.push_back(0x44);
            WriteUUID(id, packet);
            WriteString(packet, url, 32767);
            WriteString(packet, hash, 40);
            packet.push_back(forced);
            packet.push_back((bool)prompt);
            if (prompt)
                packet.push_back(prompt->ToTextComponent());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response respawn(int32_t _dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata) {
            list_array<uint8_t> packet;
            auto& dimension_type = registers::dimensionTypes_cache.at(_dimension_type)->first;
            packet.reserve(1 + dimension_type.size() + dimension_name.size() + 8 + 1 + 1 + 1 + 1 + 4 + 1 + 1);
            packet.push_back(0x45);
            WriteIdentifier(packet, dimension_type);
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

        base_objects::network::response setHeadRotation(int32_t entity_id, util::VECTOR head_rotation) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * 2);
            packet.push_back(0x46);
            WriteVar<int32_t>(entity_id, packet);
            auto res = util::to_yaw_pitch_256(head_rotation);
            packet.push_back(res.x);
            packet.push_back(res.y);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks) {
            list_array<uint8_t> packet;
            int64_t section_pos = (int64_t(section_x & 0x3FFFFF) << 42) | (int64_t(section_z & 0x3FFFFF) << 20) | int64_t(section_y & 0xFFFFF);
            packet.reserve(1 + 8 + 5 * blocks.size() * 6);
            packet.push_back(0x47);
            WriteValue<int64_t>(section_pos, packet);
            WriteVar<int32_t>(blocks.size(), packet);
            for (auto& it : blocks)
                WriteVar<int64_t>(it.value, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setAdvancementsTab(const std::optional<std::string>& tab_id) {
            list_array<uint8_t> packet;
            packet.push_back(0x48);
            packet.push_back((bool)tab_id);
            if (tab_id)
                WriteIdentifier(packet, *tab_id);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool secure_chat) {
            list_array<uint8_t> packet;
            packet.push_back(0x49);
            packet.push_back(motd.ToTextComponent());
            packet.push_back((bool)icon_png);
            if (icon_png) {
                WriteVar<int32_t>(icon_png->size(), packet);
                packet.push_back(*icon_png);
            } else
                packet.push_back(0);
            packet.push_back(secure_chat);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setActionBarText(const Chat& text) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x4A, text.ToTextComponent())});
        }

        base_objects::network::response setBorderCenter(double x, double z) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 8 * 2);
            packet.push_back(0x4B);
            WriteValue<double>(x, packet);
            WriteValue<double>(z, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 8 * 2 + 8);
            packet.push_back(0x4C);
            WriteValue<double>(old_diameter, packet);
            WriteValue<double>(new_diameter, packet);
            WriteVar<int64_t>(speed_ms, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setBorderSize(double diameter) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 8);
            packet.push_back(0x4D);
            WriteValue<double>(diameter, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setBorderWarningDelay(int32_t warning_delay) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x4E);
            WriteVar<int32_t>(warning_delay, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setBorderWarningDistance(int32_t warning_distance) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x4F);
            WriteVar<int32_t>(warning_distance, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setCamera(int32_t entity_id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x50);
            WriteVar<int32_t>(entity_id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setHeldSlot(int32_t slot) {
            list_array<uint8_t> packet;
            packet.reserve(6);
            packet.push_back(0x51);
            WriteVar<int32_t>(slot, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setCenterChunk(int32_t x, int32_t z) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 2);
            packet.push_back(0x52);
            WriteVar<int32_t>(x, packet);
            WriteVar<int32_t>(z, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setRenderDistance(int32_t render_distance) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x53);
            WriteVar<int32_t>(render_distance, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setDefaultSpawnPosition(base_objects::position pos, float angle) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 8 + 1 + 8);
            packet.push_back(0x54);
            WriteValue(pos.raw, packet);
            WriteValue<float>(angle, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response displayObjective(int32_t position, const std::string& objective_name) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + objective_name.size());
            packet.push_back(0x55);
            packet.push_back(position);
            WriteString(packet, objective_name, 32767);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + metadata.size());
            packet.push_back(0x56);
            WriteVar<int32_t>(entity_id, packet);
            packet.push_back(metadata);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 2);
            packet.push_back(0x57);
            WriteValue<int32_t>(attached_entity_id, packet);
            WriteValue<int32_t>(holder_entity_id, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setEntityVelocity(int32_t entity_id, util::VECTOR velocity) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * 3);
            packet.push_back(0x58);
            WriteVar<int32_t>(entity_id, packet);
            auto res = util::minecraft::packets::velocity(velocity);
            WriteValue<int16_t>(res.x, packet);
            WriteValue<int16_t>(res.y, packet);
            WriteValue<int16_t>(res.z, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::slot& item) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 1);
            packet.push_back(0x59);
            WriteVar<int32_t>(entity_id, packet);
            packet.push_back(slot);
            reader::WriteSlot(packet, item);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setExperience(float experience_bar, int32_t level, int32_t total_experience) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * 2);
            packet.push_back(0x5A);
            WriteValue<float>(experience_bar, packet);
            WriteVar<int32_t>(level, packet);
            WriteVar<int32_t>(total_experience, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setHealth(float health, int32_t food, float saturation) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 + 4);
            packet.push_back(0x5B);
            WriteValue<float>(health, packet);
            WriteVar<int32_t>(food, packet);
            WriteValue<float>(saturation, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + objective_name.size() + 1 + 1 + 1);
            packet.push_back(0x5C);
            WriteString(packet, objective_name, 32767);
            packet.push_back(0);
            packet.push_back(display_name.ToTextComponent());
            packet.push_back(render_type);
            packet.push_back(0);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + objective_name.size() + 1 + 1 + 1);
            packet.push_back(0x5C);
            WriteString(packet, objective_name, 32767);
            packet.push_back(0);
            packet.push_back(display_name.ToTextComponent());
            packet.push_back(render_type);
            packet.push_back(true);
            packet.push_back(1);
            packet.push_back(NBT::build(style).get_as_network());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + objective_name.size() + 1 + 1 + 1);
            packet.push_back(0x5C);
            WriteString(packet, objective_name, 32767);
            packet.push_back(0);
            packet.push_back(display_name.ToTextComponent());
            packet.push_back(render_type);
            packet.push_back(true);
            packet.push_back(2);
            packet.push_back(content.ToTextComponent());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateObjectivesRemove(const std::string& objective_name) {
            list_array<uint8_t> packet;
            packet.reserve(1 + objective_name.size());
            packet.push_back(0x5C);
            WriteString(packet, objective_name, 32767);
            packet.push_back(1);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + objective_name.size() + 1 + 1 + 1);
            packet.push_back(0x5C);
            WriteString(packet, objective_name, 32767);
            packet.push_back(2);
            packet.push_back(display_name.ToTextComponent());
            packet.push_back(render_type);
            packet.push_back(false);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + objective_name.size() + 1 + 1 + 1);
            packet.push_back(0x5C);
            WriteString(packet, objective_name, 32767);
            packet.push_back(2);
            packet.push_back(display_name.ToTextComponent());
            packet.push_back(render_type);
            packet.push_back(true);
            packet.push_back(1);
            packet.push_back(NBT::build(style).get_as_network());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + objective_name.size() + 1 + 1 + 1);
            packet.push_back(0x5C);
            WriteString(packet, objective_name, 32767);
            packet.push_back(2);
            packet.push_back(display_name.ToTextComponent());
            packet.push_back(render_type);
            packet.push_back(true);
            packet.push_back(2);
            packet.push_back(content.ToTextComponent());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * passengers.size());
            packet.push_back(0x5D);
            WriteVar<int32_t>(vehicle_entity_id, packet);
            WriteVar<int32_t>(passengers.size(), packet);
            for (auto& it : passengers)
                WriteVar<int32_t>(it, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + team_name.size() + name_tag_visibility.size() + collision_rule.size() + 4 * 3 + 1 + 4);
            packet.push_back(0x5E);
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

        base_objects::network::response updateTeamRemove(const std::string& team_name) {
            list_array<uint8_t> packet;
            packet.reserve(1 + team_name.size());
            packet.push_back(0x5E);
            WriteString(packet, team_name, 32767);
            packet.push_back(1);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + team_name.size() + name_tag_visibility.size() + collision_rule.size() + 4 * 3);
            packet.push_back(0x5E);
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

        base_objects::network::response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + team_name.size() + 4);
            packet.push_back(0x5E);
            WriteString(packet, team_name, 32767);
            packet.push_back(3);
            WriteVar<int32_t>(entities.size(), packet);
            for (auto& it : entities)
                WriteString(packet, it, 32767);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + team_name.size() + 4);
            packet.push_back(0x5E);
            WriteString(packet, team_name, 32767);
            packet.push_back(4);
            WriteVar<int32_t>(entities.size(), packet);
            for (auto& it : entities)
                WriteString(packet, it, 32767);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name) {
            list_array<uint8_t> packet;
            packet.reserve(1 + entity_name.size() + objective_name.size() + 4 + 1 + 1);
            packet.push_back(0x5F);
            WriteString(packet, entity_name, 32767);
            WriteString(packet, objective_name, 32767);
            WriteVar<int32_t>(value, packet);
            packet.push_back((bool)display_name);
            if (display_name)
                packet.push_back(display_name->ToTextComponent());
            packet.push_back(0);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled) {
            list_array<uint8_t> packet;
            packet.reserve(1 + entity_name.size() + objective_name.size() + 4 + 1 + 1);
            packet.push_back(0x5F);
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

        base_objects::network::response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content) {
            list_array<uint8_t> packet;
            packet.reserve(1 + entity_name.size() + objective_name.size() + 4 + 1 + 1);
            packet.push_back(0x5F);
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

        base_objects::network::response setSimulationDistance(int32_t distance) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x60);
            WriteVar<int32_t>(distance, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setSubtitleText(const Chat& text) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x61, text.ToTextComponent())});
        }

        base_objects::network::response updateTime(int64_t world_age, int64_t time_of_day) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 8 * 2);
            packet.push_back(0x62);
            WriteVar<int64_t>(world_age, packet);
            WriteVar<int64_t>(time_of_day, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setTitleText(const Chat& text) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x63, text.ToTextComponent())});
        }

        base_objects::network::response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 3);
            packet.push_back(0x64);
            WriteValue<int32_t>(fade_in, packet);
            WriteValue<int32_t>(stay, packet);
            WriteValue<int32_t>(fade_out, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
            if (volume < 0 || volume > 1)
                throw std::runtime_error("invalid volume value");
            if (pitch < 0.5 || pitch > 2.0)
                throw std::runtime_error("invalid pitch value");

            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 3 + 4 * 2 + 8);
            packet.push_back(0x65);
            WriteVar<int32_t>(sound_id + 1, packet);
            WriteVar<int32_t>(category, packet);
            WriteVar<int32_t>(entity_id, packet);
            WriteValue<float>(volume, packet);
            WriteValue<float>(pitch, packet);
            WriteValue<int64_t>(seed, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response entitySoundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
            if (volume < 0 || volume > 1)
                throw std::runtime_error("invalid volume value");
            if (pitch < 0.5 || pitch > 2.0)
                throw std::runtime_error("invalid pitch value");

            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 3 + 4 * 2 + 8);
            packet.push_back(0x65);
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

        base_objects::network::response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
            if (volume < 0 || volume > 1)
                throw std::runtime_error("invalid volume value");
            if (pitch < 0.5 || pitch > 2.0)
                throw std::runtime_error("invalid pitch value");

            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 3 + 4 * 3 + 4 * 2 + 8);
            packet.push_back(0x66);
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

        base_objects::network::response soundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
            if (volume < 0 || volume > 1)
                throw std::runtime_error("invalid volume value");
            if (pitch < 0.5 || pitch > 2.0)
                throw std::runtime_error("invalid pitch value");

            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 3 + 4 * 3 + 4 * 2 + 8);
            packet.push_back(0x66);
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

        //internal
        base_objects::network::response startConfiguration() {
            list_array<uint8_t> packet;
            packet.reserve(1);
            packet.push_back(0x67);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response stopSound(uint8_t flags) {
            if (flags & 0x01 || flags & 0x02)
                throw std::runtime_error("invalid params for this flag");
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x68, flags)});
        }

        base_objects::network::response stopSoundBySource(uint8_t flags, int32_t source) {
            if (flags & 0x02)
                throw std::runtime_error("invalid params for this flag");
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 4);
            packet.push_back(0x68);
            packet.push_back(flags);
            WriteVar<int32_t>(source, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response stopSoundBySound(uint8_t flags, const std::string& sound) {
            if (flags & 0x01)
                throw std::runtime_error("invalid params for this flag");
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + sound.size());
            packet.push_back(0x68);
            packet.push_back(flags);
            WriteIdentifier(packet, sound);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response stopSoundBySourceAndSound(uint8_t flags, int32_t source, const std::string& sound) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 1 + 4 + sound.size());
            packet.push_back(0x68);
            packet.push_back(flags);
            WriteVar<int32_t>(source, packet);
            WriteIdentifier(packet, sound);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response systemChatMessage(const Chat& message) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x69, message.ToTextComponent(), false)});
        }

        base_objects::network::response systemChatMessageOverlay(const Chat& message) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x69, message.ToTextComponent(), true)});
        }

        base_objects::network::response setTabListHeaderAndFooter(const Chat& header, const Chat& footer) {
            return base_objects::network::response::answer({list_array<uint8_t>::concat(0x6A, header.ToTextComponent(), footer.ToTextComponent())});
        }

        base_objects::network::response tagQueryResponse(int32_t transaction_id, const enbt::value& nbt) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4);
            packet.push_back(0x6B);
            WriteVar<int32_t>(transaction_id, packet);
            packet.push_back(NBT::build(nbt).get_as_network());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 * 3);
            packet.push_back(0x6C);
            WriteVar<int32_t>(collected_entity_id, packet);
            WriteVar<int32_t>(collector_entity_id, packet);
            WriteVar<int32_t>(pickup_item_count, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response teleportEntity(int32_t entity_id, util::VECTOR pos, float yaw, float pitch, bool on_ground) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + 4 * 3 + 4 * 2 + 1);
            packet.push_back(0x6D);
            WriteVar<int32_t>(entity_id, packet);
            WriteValue<double>(pos.x, packet);
            WriteValue<double>(pos.y, packet);
            WriteValue<double>(pos.z, packet);
            WriteValue<float>(yaw, packet);
            WriteValue<float>(pitch, packet);
            packet.push_back(on_ground);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setTickingState(float tick_rate, bool is_frozen) {
            list_array<uint8_t> packet;
            packet.reserve(6);
            packet.push_back(0x6E);
            WriteValue<float>(tick_rate, packet);
            packet.push_back(is_frozen);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response stepTick(int32_t step_count) {
            list_array<uint8_t> packet;
            packet.reserve(5);
            packet.push_back(0x6F);
            WriteVar<int32_t>(step_count, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping>& advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress>& progress_advancements) {
            list_array<uint8_t> packet;
            packet.push_back(0x70);
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

        base_objects::network::response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) {
            list_array<uint8_t> packet;
            packet.push_back(0x71);
            WriteVar<int32_t>(entity_id, packet);
            WriteVar<int32_t>(properties.size(), packet);
            for (auto& [key, value, modifiers] : properties) {
                WriteString(packet, key, 32767);
                WriteValue<double>(value, packet);
                WriteVar<int32_t>(modifiers.size(), packet);
                for (auto& [uuid, amount, operation] : modifiers) {
                    WriteUUID(uuid, packet);
                    WriteValue<double>(amount, packet);
                    packet.push_back(operation);
                }
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response entityEffect(int32_t entity_id, int32_t effect_id, int8_t amplifier, int32_t duration, int8_t flags, const std::optional<enbt::value>& factor_codec) {
            list_array<uint8_t> packet;
            packet.push_back(0x72);
            WriteVar<int32_t>(entity_id, packet);
            WriteVar<int32_t>(effect_id, packet);
            packet.push_back(amplifier);
            WriteVar<int32_t>(duration, packet);
            packet.push_back(flags);
            packet.push_back((bool)factor_codec);
            if (factor_codec)
                packet.push_back(NBT::build(*factor_codec).get_as_network());
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response entityEffect(int32_t entity_id, int32_t effect_id, int8_t amplifier, int32_t duration, int8_t flags) {
            return entityEffect(entity_id, effect_id, amplifier, duration, flags, std::nullopt);
        }

        list_array<list_array<uint8_t>> unfold_recipes(list_array<uint8_t>& header, list_array<base_objects::slot_display>&& recipes, const list_array<uint8_t>& footer = {}) {
            list_array<list_array<list_array<uint8_t>>> tmp
                = recipes
                      .convert_fn([](auto& recipe) {
                          return recipe.to_slots();
                      })
                      .convert_fn([](auto& slots) {
                          return slots.convert_fn([](auto& slot) {
                              list_array<uint8_t> res;
                              reader::WriteSlotItem(res, slot);
                              return res;
                          });
                      });

            list_array<list_array<uint8_t>> results;
            results.resize(tmp.count([](auto& arr) {
                return arr.size();
            }));
            if (header.size())
                results.push_back_for(header);

            results.transform_with(tmp, [](list_array<uint8_t>& it, const list_array<uint8_t>& ss) {
                it += ss;
            });
            if (footer.size())
                results.push_back_for(footer);
            return results;
        }

        base_objects::network::response updateRecipes(const std::vector<base_objects::recipe>& recipes) {
            list_array<uint8_t> packet;
            size_t recipe_count = 0;
            for (auto& recipe : recipes) {
                std::visit(
                    [&](auto&& item) {
                        using type = std::decay_t<decltype(item)>;
                        list_array<uint8_t> header;
                        WriteIdentifier(header, base_objects::recipes::variant_data<type>::name);
                        WriteIdentifier(header, recipe.full_id);
                        if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::crafting_shaped>) {
                            WriteString(header, recipe.group, 32767);
                            WriteVar<int32_t>((int32_t)item.category, header);
                            WriteVar<int32_t>(item.width, header);
                            WriteVar<int32_t>(item.height, header);
                            auto results = unfold_recipes(header, to_list_array(item.ingredients).push_back(item.result), {item.show_notification});
                            recipe_count += results.size();
                            packet += results.take().concat();
                        } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::crafting_shapeless>) {
                            WriteString(header, recipe.group, 32767);
                            WriteVar<int32_t>((int32_t)item.category, header);
                            WriteVar<int32_t>(item.ingredients.size(), header);
                            auto results = unfold_recipes(header, to_list_array(item.ingredients).push_back(item.result));
                            recipe_count += results.size();
                            packet += results.take().concat();
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
                            | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_shulkerboxcoloring>
                            | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_suspiciousstew>
                            | std::is_same_v<type, base_objects::recipes::minecraft::crafting_special_repairitem>
                            | std::is_same_v<type, base_objects::recipes::minecraft::crafting_decorated_pot>
                        ) {
                            WriteVar<int32_t>((int32_t)item.category, header);
                            packet += header.take();
                            recipe_count += 1;
                        } else if constexpr (
                            std::is_same_v<type, base_objects::recipes::minecraft::smelting>
                            | std::is_same_v<type, base_objects::recipes::minecraft::blasting>
                            | std::is_same_v<type, base_objects::recipes::minecraft::smoking>
                            | std::is_same_v<type, base_objects::recipes::minecraft::campfire_cooking>
                        ) {
                            WriteString(header, recipe.group, 32767);
                            WriteVar<int32_t>((int32_t)item.category, header);
                            list_array<uint8_t> footer;
                            WriteValue<float>(item.experience, footer);
                            WriteVar<int32_t>(item.cooking_time, footer);
                            auto results = unfold_recipes(header, {item.ingredient, item.result}, footer);
                            recipe_count += results.size();
                            packet += results.take().concat();
                        } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::stonecutting>) {
                            WriteString(header, recipe.group, 32767);
                            auto results = unfold_recipes(header, {item.ingredient, item.result});
                            recipe_count += results.size();
                            packet += results.take().concat();
                        } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::smithing_transform>) {
                            auto results = unfold_recipes(header, {item._template, item.base, item.addition, item.result});
                            recipe_count += results.size();
                            packet += results.take().concat();
                        } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::smithing_trim>) {
                            auto results = unfold_recipes(header, {item._template, item.base, item.addition});
                            recipe_count += results.size();
                            packet += results.take().concat();
                        } else if constexpr (std::is_same_v<type, base_objects::recipes::custom>) {
                            header.push_back(item.data);
                            packet.push_back(header.take());
                            recipe_count += 1;
                        } else
                            throw std::runtime_error("invalid recipe type");
                    },
                    recipe.data
                );
            }
            list_array<uint8_t> header;

            header.push_back(0x73);
            WriteVar<int32_t>(recipe_count, header);
            packet.push_front(header.take());

            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings) {
            list_array<uint8_t> packet;
            packet.push_back(0x74);
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
    }
}