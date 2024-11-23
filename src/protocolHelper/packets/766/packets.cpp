#include <src/api/configuration.hpp>
#include <src/api/mojang/session_server.hpp>
#include <src/protocolHelper/packets/765/packets.hpp>
#include <src/protocolHelper/packets/766/packets.hpp>
#include <src/protocolHelper/packets/766/writers_readers.hpp>
#include <src/protocolHelper/util.hpp>

//packets for 1.20.4, protocol 766
//changes between 765:
// encryption in offline mode is now optional instead of disabled
//
//# Packets changes:
//## Play
//Server bound@0x00 <- @0x00
//Server bound@0x01 <- @0x01
//Server bound@0x02 <- @0x02
//Server bound@0x03 <- @0x03
//Server bound@0x04 new (Chat command)
//  [String] Command (max 32767)
//Server bound@0x05 <- @0x04 rename(Signed Chat Command)
//  Command length limit now extended to 32767
//Server bound@0x06 <- @0x05
//Server bound@0x07 <- @0x06
//Server bound@0x08 <- @0x07
//Server bound@0x09 <- @0x08
//Server bound@0x0A <- @0x09
//Server bound@0x0B <- @0x0A
//Server bound@0x0C <- @0x0B
//Server bound@0x0D <- @0x0C
//Server bound@0x0E <- @0x0D
//Server bound@0x0F <- @0x0E
//Server bound@0x10 <- @0x0F
//Server bound@0x11 new (Cookie response)
//  [Identifier] Cookie key
//  [boolean] payload been registered
//  [var_int|optional] payload length
//  [payload|optional]
//Server bound@0x12 <- @0x10
//Server bound@0x13 new (Subscribe to debug Sample)
//  [var_int] sample type
//Server bound@0x14 <- @0x11
//Server bound@0x15 <- @0x12
//Server bound... keeps but ID + 3


namespace copper_server {
    namespace packets {
        namespace release_766 {
            namespace login {
                Response login(int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data) {
                    return release_765::login::login(plugin_message_id, chanel, data);
                }

                Response kick(const Chat& reason) {
                    return release_765::login::kick(reason);
                }

                Response disableCompression() {
                    return release_765::login::disableCompression();
                }

                Response setCompression(int32_t threshold) {
                    return release_765::login::setCompression(threshold);
                }

                Response loginSuccess(SharedClientData& client) {
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
                    response.push_back(false);
                    return Response::Answer({std::move(response)});
                }

                Response encryptionRequest(const std::string& server_id, uint8_t (&verify_token)[4]) {
                    list_array<uint8_t> response;
                    auto public_key = Server::instance().public_key_buffer();

                    response.push_back(1);
                    WriteString(response, server_id, 20); //server id
                    WriteVar<int32_t>(public_key.size(), response);
                    response.push_back((uint8_t*)public_key.data(), public_key.size());
                    WriteVar<int32_t>(4, response);
                    response.push_back(verify_token[0]);
                    response.push_back(verify_token[1]);
                    response.push_back(verify_token[2]);
                    response.push_back(verify_token[3]);
                    response.push_back(!api::configuration::get().server.offline_mode);
                    return Response::Answer({std::move(response)});
                }

                Response requestCookie(const std::string& key) {
                    list_array<uint8_t> packet;
                    packet.reserve(1 + 4 + key.size());
                    packet.push_back(0x05);
                    WriteVar<int32_t>(key.size(), packet);
                    packet.push_back((const uint8_t*)key.data(), key.size());
                    return Response::Answer({std::move(packet)});
                }
            }

            namespace configuration {
                Response requestCookie(const std::string& key) {
                    list_array<uint8_t> packet;
                    packet.reserve(1 + 4 + key.size());
                    packet.push_back(0x00);
                    WriteVar<int32_t>(key.size(), packet);
                    packet.push_back((const uint8_t*)key.data(), key.size());
                    return Response::Answer({std::move(packet)});
                }

                Response configuration(const std::string& chanel, const list_array<uint8_t>& data) {
                    auto res = release_765::configuration::configuration(chanel, data);
                    res.data[0].data[0] = 0x01;
                    return res;
                }

                Response kick(const Chat& reason) {
                    auto res = release_765::configuration::kick(reason);
                    res.data[0].data[0] = 0x02;
                    return res;
                }

                Response finish() {
                    return Response::Answer({{3}});
                }

                Response keep_alive(int64_t keep_alive_packet) {
                    list_array<uint8_t> response;
                    response.reserve(9);
                    response.push_back(0x04);
                    WriteValue<int64_t>(keep_alive_packet, response);
                    return Response::Answer({std::move(response)});
                }

                Response ping(int32_t excepted_pong) {
                    list_array<uint8_t> response;
                    response.reserve(5);
                    response.push_back(0x05);
                    WriteVar<int32_t>(excepted_pong, response);
                    return Response::Answer({std::move(response)});
                }

                template <class FN, class RegistryT>
                list_array<uint8_t> registry_data_serialize_entry(const std::string& identifier, std::unordered_map<std::string, RegistryT>& values, FN&& serializer) {
                    list_array<std::pair<std::string, enbt::compound>> fixed_data;
                    fixed_data.resize(values.size());
                    for (auto& [name, it] : values) {
                        if (it.id >= fixed_data.size())
                            throw std::out_of_range("Invalid registry values");
                        fixed_data[it.id] = {name, serializer(it)};
                    }
                    list_array<uint8_t> part;
                    part.push_back(0x07);
                    WriteIdentifier(part, identifier);
                    WriteVar<int32_t>(fixed_data.size(), part);
                    fixed_data.for_each([&](const std::string& name, enbt::compound& data) {
                        WriteIdentifier(part, name);
                        part.push_back(data.size());
                        if (data.size())
                            part.push_back(NBT::build((enbt::value&)data).get_as_network());
                    });
                    return part;
                }

                Response registry_data() {
                    using namespace registers;
                    static list_array<list_array<uint8_t>> data;
                    if (data.empty()) {
                        { //minecraft:trim_material
                            data.push_back(registry_data_serialize_entry(
                                "minecraft:trim_material",
                                registers::armorTrimMaterials,
                                [](registers::ArmorTrimMaterial& it) {
                                    enbt::compound element;
                                    element["asset_name"] = it.asset_name;
                                    element["ingredient"] = it.ingredient;
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
                                    return element;
                                }
                            ));
                        }
                        { //minecraft:trim_pattern
                            data.push_back(registry_data_serialize_entry(
                                "minecraft:trim_pattern",
                                registers::armorTrimPatterns,
                                [](registers::ArmorTrimPattern& it) {
                                    enbt::compound element;
                                    element["asset_id"] = it.asset_id;
                                    element["template_item"] = it.template_item;
                                    if (std::holds_alternative<std::string>(it.description))
                                        element["description"] = std::get<std::string>(it.description);
                                    else
                                        element["description"] = std::get<Chat>(it.description).ToENBT();
                                    element["decal"] = it.decal;
                                    return element;
                                }
                            ));
                        }
                        { //minecraft:worldgen/biome
                            data.push_back(registry_data_serialize_entry(
                                "minecraft:worldgen/biome",
                                registers::biomes,
                                [](registers::Biome& it) { //element
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
                                            mood_sound["block_search_extend"] = it.effects.mood_sound->block_search_extend;
                                            mood_sound["offset"] = it.effects.mood_sound->offset;
                                            effects["mood_sound"] = std::move(mood_sound);
                                        }
                                        if (it.effects.additions_sound) {
                                            enbt::compound additions_sound;
                                            additions_sound["sound"] = it.effects.additions_sound->sound;
                                            additions_sound["tick_chance"] = it.effects.additions_sound->tick_chance;
                                            effects["additions_sound"] = std::move(additions_sound);
                                        }
                                        if (it.effects.music) {
                                            enbt::compound music;
                                            music["sound"] = it.effects.music->sound;
                                            music["min_delay"] = it.effects.music->min_delay;
                                            music["max_delay"] = it.effects.music->max_delay;
                                            music["replace_current_music"] = it.effects.music->replace_current_music;
                                            effects["music"] = std::move(music);
                                        }
                                        element["effects"] = std::move(effects);
                                    }
                                    return element;
                                }
                            ));
                        }
                        { // minecraft:chat_type
                            data.push_back(registry_data_serialize_entry(
                                "minecraft:chat_type",
                                registers::chatTypes,
                                [](registers::ChatType& it) {
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
                                }
                            ));
                        }
                        { // minecraft:damage_type
                            data.push_back(registry_data_serialize_entry(
                                "minecraft:damage_type",
                                registers::damageTypes,
                                [](registers::DamageType& it) { //element
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
                                }
                            ));
                        }
                        { // minecraft:dimension_type
                            data.push_back(registry_data_serialize_entry(
                                "minecraft:dimension_type",
                                registers::dimensionTypes,
                                [](registers::DimensionType& it) { //element
                                    enbt::compound element;
                                    if (std::holds_alternative<int32_t>(it.monster_spawn_light_level))
                                        element["monster_spawn_light_level"] = std::get<int32_t>(it.monster_spawn_light_level);
                                    else {
                                        enbt::compound distribution;
                                        auto& ddd = std::get<IntegerDistribution>(it.monster_spawn_light_level);
                                        distribution["type"] = ddd.type;
                                        distribution["value"] = ddd.value;
                                        element["monster_spawn_light_level"] = std::move(distribution);
                                    }
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
                                }
                            ));
                        }
                        { // minecraft:wolf_variant
                            data.push_back(registry_data_serialize_entry(
                                "minecraft:wolf_variant",
                                registers::wolfVariants,
                                [](registers::WolfVariant& it) { //element
                                    enbt::compound element;
                                    element["wild_texture"] = it.wild_texture;
                                    element["tame_texture"] = it.tame_texture;
                                    element["angry_texture"] = it.angry_texture;
                                    if (it.biomes.size() == 1) {
                                        element["biomes"] = it.biomes[0];
                                    } else {
                                        enbt::fixed_array biomes(it.biomes.size());
                                        it.biomes.for_each([&biomes](size_t index, const std::string& biome) {
                                            biomes.set(index, biome);
                                        });
                                        element["biomes"] = std::move(biomes);
                                    }
                                    return element;
                                }
                            ));
                        }
                    }
                    return Response::Answer(data);
                }

                Response resetChat() {
                    return Response::Answer({{0x06}});
                }

                Response removeResourcePacks() {
                    auto res = release_765::configuration::removeResourcePacks();
                    res.data[0].data[0] = 0x08;
                    return res;
                }

                Response removeResourcePack(const enbt::raw_uuid& pack_id) {
                    auto res = release_765::configuration::removeResourcePack(pack_id);
                    res.data[0].data[0] = 0x08;
                    return res;
                }

                Response addResourcePack(SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced) {
                    auto res = release_765::configuration::addResourcePack(client, pack_id, url, hash, forced);
                    res.data[0].data[0] = 0x09;
                    return res;
                }

                Response addResourcePackPrompted(SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt) {
                    auto res = release_765::configuration::addResourcePackPrompted(client, pack_id, url, hash, forced, prompt);
                    res.data[0].data[0] = 0x09;
                    return res;
                }

                Response storeCookie(const std::string& key, const list_array<uint8_t>& payload) {
                    if (payload.size() > 5120)
                        throw std::runtime_error("Payload size is too big");

                    list_array<uint8_t> packet;
                    packet.reserve(1 + 4 + key.size() + payload.size());
                    packet.push_back(0x0A);
                    WriteIdentifier(packet, key);
                    packet.push_back(payload);
                    return Response::Answer({std::move(packet)});
                }

                Response transfer(const std::string& host, int32_t port) {
                    list_array<uint8_t> packet;
                    packet.reserve(1 + 4 + host.size() + 4);
                    packet.push_back(0x0B);
                    WriteString(packet, host);
                    WriteVar<int32_t>(port, packet);
                    return Response::Answer({std::move(packet)});
                }

                Response setFeatureFlags(const list_array<std::string>& features) {
                    auto res = release_765::configuration::setFeatureFlags(features);
                    res.data[0].data[0] = 0x0C;
                    return res;
                }

                Response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries) {
                    auto res = release_765::configuration::updateTags(tags_entries);
                    res.data[0].data[0] = 0x0D;
                    return res;
                }

                Response knownPacks(const list_array<base_objects::packets::known_pack>& packs) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x0E);
                    WriteVar<int32_t>(packs.size(), packet);
                    for (const auto& pack : packs) {
                        WriteString(packet, pack.namespace_);
                        WriteString(packet, pack.id);
                        WriteString(packet, pack.version);
                    }
                    return Response::Answer({std::move(packet)});
                }
            }

            namespace play {
                Response bundleResponse(Response&& response) {
                    return release_765::play::bundleResponse(std::move(response));
                }

                Response spawnEntity(const base_objects::entity& entity, uint16_t protocol) {
                    if (protocol == UINT16_MAX)
                        protocol = 766;
                    return release_765::play::spawnEntity(entity, protocol);
                }

                Response spawnExperienceOrb(const base_objects::entity& entity, int16_t count) {
                    return release_765::play::spawnExperienceOrb(entity, count);
                }

                Response entityAnimation(const base_objects::entity& entity, uint8_t animation) {
                    return release_765::play::entityAnimation(entity, animation);
                }

                Response awardStatistics(const list_array<base_objects::packets::statistics>& statistics) {
                    return release_765::play::awardStatistics(statistics);
                }

                Response acknowledgeBlockChange(SharedClientData& client) {
                    return release_765::play::acknowledgeBlockChange(client);
                }

                Response setBlockDestroyStage(const base_objects::entity& entity, Position block, uint8_t stage) {
                    return release_765::play::setBlockDestroyStage(entity, block, stage);
                }

                Response blockEntityData(Position block, int32_t type, const enbt::value& data) {
                    return release_765::play::blockEntityData(block, type, data);
                }

                Response blockAction(Position block, int32_t action_id, int32_t param, int32_t block_type) {
                    return release_765::play::blockAction(block, action_id, param, block_type);
                }

                Response blockUpdate(Position block, int32_t block_type) {
                    return release_765::play::blockUpdate(block, block_type);
                }

                Response bossBarAdd(const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags) {
                    return release_765::play::bossBarAdd(id, title, health, color, division, flags);
                }

                Response bossBarRemove(const enbt::raw_uuid& id) {
                    return release_765::play::bossBarRemove(id);
                }

                Response bossBarUpdateHealth(const enbt::raw_uuid& id, float health) {
                    return release_765::play::bossBarUpdateHealth(id, health);
                }

                Response bossBarUpdateTitle(const enbt::raw_uuid& id, const Chat& title) {
                    return release_765::play::bossBarUpdateTitle(id, title);
                }

                Response bossBarUpdateStyle(const enbt::raw_uuid& id, int32_t color, int32_t division) {
                    return release_765::play::bossBarUpdateStyle(id, color, division);
                }

                Response bossBarUpdateFlags(const enbt::raw_uuid& id, uint8_t flags) {
                    return release_765::play::bossBarUpdateFlags(id, flags);
                }

                Response changeDifficulty(uint8_t difficulty, bool locked) {
                    return release_765::play::changeDifficulty(difficulty, locked);
                }

                Response chunkBatchFinished(int32_t count) {
                    return release_765::play::chunkBatchFinished(count);
                }

                Response chunkBatchStart() {
                    return release_765::play::chunkBatchStart();
                }

                Response chunkBiomes(list_array<base_objects::chunk::chunk_biomes>& chunk) {
                    return release_765::play::chunkBiomes(chunk);
                }

                Response clearTitles(bool reset) {
                    return release_765::play::clearTitles(reset);
                }

                Response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions) {
                    return release_765::play::commandSuggestionsResponse(transaction_id, start_pos, length, suggestions);
                }

                Response commands(int32_t root_id, const list_array<base_objects::packets::command_node>& nodes) {
                    return release_765::play::commands(root_id, nodes);
                }

                Response closeContainer(uint8_t container_id) {
                    return release_765::play::closeContainer(container_id);
                }

                Response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item) {
                    return release_765::play::setContainerContent(windows_id, state_id, slots, carried_item);
                }

                Response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value) {
                    return release_765::play::setContainerProperty(windows_id, property, value);
                }

                Response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item) {
                    return release_765::play::setContainerSlot(windows_id, state_id, slot, item);
                }

                Response cookieRequest(const std::string& key) {
                    list_array<uint8_t> packet;
                    packet.reserve(1 + 4 + key.size());
                    packet.push_back(0x16);
                    WriteIdentifier(packet, key);
                    return Response::Answer({std::move(packet)});
                }

                Response setCooldown(int32_t item_id, int32_t cooldown) {
                    auto res = release_765::play::setCooldown(item_id, cooldown);
                    res.data[0].data[0] = 0x17;
                    return res;
                }

                Response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions) {
                    auto res = release_765::play::chatSuggestionsResponse(action, count, suggestions);
                    res.data[0].data[0] = 0x18;
                    return res;
                }

                Response customPayload(const std::string& channel, const list_array<uint8_t>& data) {
                    auto res = release_765::play::customPayload(channel, data);
                    res.data[0].data[0] = 0x19;
                    return res;
                }

                Response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<calc::VECTOR> xyz) {
                    auto res = release_765::play::damageEvent(entity_id, source_type_id, source_cause_id, source_direct_id, xyz);
                    res.data[0].data[0] = 0x1A;
                    return res;
                }

                Response debugSample(const list_array<uint64_t>& sample, int32_t sample_type) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x1B);
                    WriteArray(packet, sample);
                    WriteVar<int32_t>(sample_type, packet);
                    return Response::Answer({std::move(packet)});
                }

                Response deleteMessageBySignature(uint8_t (&signature)[256]) {
                    auto res = release_765::play::deleteMessageBySignature(signature);
                    res.data[0].data[0] = 0x1C;
                    return res;
                }

                Response deleteMessageByID(int32_t message_id) {
                    auto res = release_765::play::deleteMessageByID(message_id);
                    res.data[0].data[0] = 0x1C;
                    return res;
                }

                Response kick(const Chat& reason) {
                    auto res = release_765::play::kick(reason);
                    res.data[0].data[0] = 0x1D;
                    return res;
                }

                Response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name) {
                    auto res = release_765::play::disguisedChatMessage(message, chat_type, sender, target_name);
                    res.data[0].data[0] = 0x1E;
                    return res;
                }

                Response entityEvent(int32_t entity_id, uint8_t entity_status) {
                    auto res = release_765::play::entityEvent(entity_id, entity_status);
                    res.data[0].data[0] = 0x1F;
                    return res;
                }

                Response explosion(
                    calc::VECTOR pos,
                    float strength,
                    const list_array<calc::XYZ<int8_t>>& affected_blocks,
                    calc::VECTOR player_motion,
                    int32_t block_interaction,
                    int32_t small_explosion_particle_id,
                    const base_objects::particle_data& small_explosion_particle_data,
                    int32_t large_explosion_particle_id,
                    const base_objects::particle_data& large_explosion_particle_data,
                    const std::string& sound_name,
                    std::optional<float> fixed_range
                ) {
                    auto res = release_765::play::explosion(pos, strength, affected_blocks, player_motion, block_interaction, small_explosion_particle_id, small_explosion_particle_data, large_explosion_particle_id, large_explosion_particle_data, sound_name, fixed_range);
                    res.data[0].data[0] = 0x20;
                    return res;
                }

                Response unloadChunk(int32_t x, int32_t z) {
                    auto res = release_765::play::unloadChunk(x, z);
                    res.data[0].data[0] = 0x21;
                    return res;
                }

                Response gameEvent(uint8_t event_id, float value) {
                    auto res = release_765::play::gameEvent(event_id, value);
                    res.data[0].data[0] = 0x22;
                    return res;
                }

                Response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id) {
                    auto res = release_765::play::openHorseWindow(window_id, slots, entity_id);
                    res.data[0].data[0] = 0x23;
                    return res;
                }

                Response hurtAnimation(int32_t entity_id, float yaw) {
                    auto res = release_765::play::hurtAnimation(entity_id, yaw);
                    res.data[0].data[0] = 0x24;
                    return res;
                }

                Response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time) {
                    auto res = release_765::play::initializeWorldBorder(x, z, old_diameter, new_diameter, speed_ms, portal_teleport_boundary, warning_blocks, warning_time);
                    res.data[0].data[0] = 0x25;
                    return res;
                }

                //internal use
                Response keepAlive(int64_t id) {
                    auto res = release_765::play::keepAlive(id);
                    res.data[0].data[0] = 0x26;
                    return res;
                }

                Response updateChunkDataWLights(
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
                    auto res = release_765::play::updateChunkDataWLights(chunk_x, chunk_z, heightmaps, data, sky_light_mask, block_light_mask, empty_skylight_mask, empty_block_light_mask, sky_light_arrays, block_light_arrays);
                    res.data[0].data[0] = 0x27;
                    return res;
                }

                Response worldEvent(int32_t event, Position pos, int32_t data, bool global) {
                    auto res = release_765::play::worldEvent(event, pos, data, global);
                    res.data[0].data[0] = 0x28;
                    return res;
                }

                Response particle(int32_t particle_id, bool long_distance, calc::VECTOR pos, calc::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data) {
                    auto res = release_765::play::particle(particle_id, long_distance, pos, offset, max_speed, count, data);
                    res.data[0].data[0] = 0x29;
                    return res;
                }

                Response updateLight(
                    int32_t chunk_x,
                    int32_t chunk_z,
                    const bit_list_array<>& sky_light_mask,
                    const bit_list_array<>& block_light_mask,
                    const bit_list_array<>& empty_skylight_mask,
                    const bit_list_array<>& empty_block_light_mask,
                    const list_array<std::vector<uint8_t>>& sky_light_arrays,
                    const list_array<std::vector<uint8_t>>& block_light_arrays
                ) {
                    auto res = release_765::play::updateLight(chunk_x, chunk_z, sky_light_mask, block_light_mask, empty_skylight_mask, empty_block_light_mask, sky_light_arrays, block_light_arrays);
                    res.data[0].data[0] = 0x2A;
                    return res;
                }

                Response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool enforces_secure_chat) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x2B);
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
                    packet.push_back(enforces_secure_chat);
                    return Response::Answer({std::move(packet)});
                }

                Response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t columns, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data) {
                    auto res = release_765::play::mapData(map_id, scale, locked, icons, columns, rows, x, z, data);
                    res.data[0].data[0] = 0x2C;
                    return res;
                }

                Response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade>& trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock) {
                    auto res = release_765::play::merchantOffers(window_id, trade_id, trades, level, experience, regular_villager, can_restock);
                    res.data[0].data[0] = 0x2D;
                    return res;
                }

                Response updateEntityPosition(int32_t entity_id, calc::XYZ<float> pos, bool on_ground) {
                    auto res = release_765::play::updateEntityPosition(entity_id, pos, on_ground);
                    res.data[0].data[0] = 0x2E;
                    return res;
                }

                Response updateEntityPositionAndRotation(int32_t entity_id, calc::XYZ<float> pos, calc::VECTOR rot, bool on_ground) {
                    auto res = release_765::play::updateEntityPositionAndRotation(entity_id, pos, rot, on_ground);
                    res.data[0].data[0] = 0x2F;
                    return res;
                }

                Response updateEntityRotation(int32_t entity_id, calc::VECTOR rot, bool on_ground) {
                    auto res = release_765::play::updateEntityRotation(entity_id, rot, on_ground);
                    res.data[0].data[0] = 0x30;
                    return res;
                }

                Response moveVehicle(calc::VECTOR pos, calc::VECTOR rot) {
                    auto res = release_765::play::moveVehicle(pos, rot);
                    res.data[0].data[0] = 0x31;
                    return res;
                }

                Response openBook(int32_t hand) {
                    auto res = release_765::play::openBook(hand);
                    res.data[0].data[0] = 0x32;
                    return res;
                }

                Response openScreen(int32_t window_id, int32_t type, const Chat& title) {
                    auto res = release_765::play::openScreen(window_id, type, title);
                    res.data[0].data[0] = 0x33;
                    return res;
                }

                Response openSignEditor(Position pos, bool is_front_text) {
                    auto res = release_765::play::openSignEditor(pos, is_front_text);
                    res.data[0].data[0] = 0x34;
                    return res;
                }

                Response ping(int32_t id) {
                    auto res = release_765::play::ping(id);
                    res.data[0].data[0] = 0x35;
                    return res;
                }

                Response pingResponse(int32_t id) {
                    auto res = release_765::play::pingResponse(id);
                    res.data[0].data[0] = 0x36;
                    return res;
                }

                Response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id) {
                    auto res = release_765::play::placeGhostRecipe(windows_id, recipe_id);
                    res.data[0].data[0] = 0x37;
                    return res;
                }

                Response playerAbilities(uint8_t flags, float flying_speed, float field_of_view) {
                    auto res = release_765::play::playerAbilities(flags, flying_speed, field_of_view);
                    res.data[0].data[0] = 0x38;
                    return res;
                }

                Response playerChatMessage(enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, const std::optional<enbt::value>& __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name) {
                    auto res = release_765::play::playerChatMessage(sender, index, signature, message, timestamp, salt, prev_messages, __UNDEFINED__FIELD__, filter_type, filtered_symbols_bitfield, chat_type, sender_name, target_name);
                    res.data[0].data[0] = 0x39;
                    return res;
                }

                Response endCombat(int32_t duration) {
                    auto res = release_765::play::endCombat(duration);
                    res.data[0].data[0] = 0x3A;
                    return res;
                }

                Response enterCombat() {
                    auto res = release_765::play::enterCombat();
                    res.data[0].data[0] = 0x3B;
                    return res;
                }

                Response combatDeath(int32_t player_id, const Chat& message) {
                    auto res = release_765::play::combatDeath(player_id, message);
                    res.data[0].data[0] = 0x3C;
                    return res;
                }

                Response playerInfoRemove(const list_array<enbt::raw_uuid>& players) {
                    auto res = release_765::play::playerInfoRemove(players);
                    res.data[0].data[0] = 0x3D;
                    return res;
                }

                Response playerInfoAdd(const list_array<base_objects::packets::player_actions_add>& add_players) {
                    auto res = release_765::play::playerInfoAdd(add_players);
                    res.data[0].data[0] = 0x3E;
                    return res;
                }

                Response playerInfoInitializeChat(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat) {
                    auto res = release_765::play::playerInfoInitializeChat(initialize_chat);
                    res.data[0].data[0] = 0x3E;
                    return res;
                }

                Response playerInfoUpdateGameMode(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode) {
                    auto res = release_765::play::playerInfoUpdateGameMode(update_game_mode);
                    res.data[0].data[0] = 0x3E;
                    return res;
                }

                Response playerInfoUpdateListed(const list_array<base_objects::packets::player_actions_update_listed>& update_listed) {
                    auto res = release_765::play::playerInfoUpdateListed(update_listed);
                    res.data[0].data[0] = 0x3E;
                    return res;
                }

                Response playerInfoUpdateLatency(const list_array<base_objects::packets::player_actions_update_latency>& update_latency) {
                    auto res = release_765::play::playerInfoUpdateLatency(update_latency);
                    res.data[0].data[0] = 0x3E;
                    return res;
                }

                Response playerInfoUpdateDisplayName(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name) {
                    auto res = release_765::play::playerInfoUpdateDisplayName(update_display_name);
                    res.data[0].data[0] = 0x3E;
                    return res;
                }

                Response lookAt(bool from_feet_or_eyes, calc::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id) {
                    auto res = release_765::play::lookAt(from_feet_or_eyes, target, entity_id);
                    res.data[0].data[0] = 0x3F;
                    return res;
                }

                Response synchronizePlayerPosition(calc::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id) {
                    auto res = release_765::play::synchronizePlayerPosition(pos, yaw, pitch, flags, teleport_id);
                    res.data[0].data[0] = 0x40;
                    return res;
                }

                Response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& displayed_recipe_ids, const list_array<std::string>& had_access_to_recipe_ids) {
                    auto res = release_765::play::initRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, displayed_recipe_ids, had_access_to_recipe_ids);
                    res.data[0].data[0] = 0x41;
                    return res;
                }

                Response addRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) {
                    auto res = release_765::play::addRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids);
                    res.data[0].data[0] = 0x41;
                    return res;
                }

                Response removeRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) {
                    auto res = release_765::play::removeRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids);
                    res.data[0].data[0] = 0x41;
                    return res;
                }

                Response removeEntities(const list_array<int32_t>& entity_ids) {
                    auto res = release_765::play::removeEntities(entity_ids);
                    res.data[0].data[0] = 0x42;
                    return res;
                }

                Response removeEntityEffect(int32_t entity_id, int32_t effect_id) {
                    auto res = release_765::play::removeEntityEffect(entity_id, effect_id);
                    res.data[0].data[0] = 0x43;
                    return res;
                }

                Response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name) {
                    auto res = release_765::play::resetScore(entity_name, objective_name);
                    res.data[0].data[0] = 0x44;
                    return res;
                }

                Response removeResourcePacks() {
                    auto res = release_765::play::removeResourcePacks();
                    res.data[0].data[0] = 0x45;
                    return res;
                }

                Response removeResourcePack(enbt::raw_uuid id) {
                    auto res = release_765::play::removeResourcePack(id);
                    res.data[0].data[0] = 0x45;
                    return res;
                }

                Response addResourcePack(enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt) {
                    auto res = release_765::play::addResourcePack(id, url, hash, forced, prompt);
                    res.data[0].data[0] = 0x46;
                    return res;
                }

                Response respawn(int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x47);
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
                    return Response::Answer({std::move(packet)});
                }

                Response setHeadRotation(int32_t entity_id, calc::VECTOR head_rotation) {
                    auto res = release_765::play::setHeadRotation(entity_id, head_rotation);
                    res.data[0].data[0] = 0x48;
                    return res;
                }

                Response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks) {
                    auto res = release_765::play::updateSectionBlocks(section_x, section_z, section_y, blocks);
                    res.data[0].data[0] = 0x49;
                    return res;
                }

                Response setAdvancementsTab(const std::optional<std::string>& tab_id) {
                    auto res = release_765::play::setAdvancementsTab(tab_id);
                    res.data[0].data[0] = 0x4A;
                    return res;
                }

                Response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool __ignored) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x4B);
                    packet.push_back(motd.ToTextComponent());
                    packet.push_back((bool)icon_png);
                    if (icon_png) {
                        WriteVar<int32_t>(icon_png->size(), packet);
                        packet.push_back(*icon_png);
                    }
                    return Response::Answer({std::move(packet)});
                }

                Response setActionBarText(const Chat& text) {
                    auto res = release_765::play::setActionBarText(text);
                    res.data[0].data[0] = 0x4C;
                    return res;
                }

                Response setBorderCenter(double x, double z) {
                    auto res = release_765::play::setBorderCenter(x, z);
                    res.data[0].data[0] = 0x4D;
                    return res;
                }

                Response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms) {
                    auto res = release_765::play::setBorderLerp(old_diameter, new_diameter, speed_ms);
                    res.data[0].data[0] = 0x4E;
                    return res;
                }

                Response setBorderSize(double diameter) {
                    auto res = release_765::play::setBorderSize(diameter);
                    res.data[0].data[0] = 0x4F;
                    return res;
                }

                Response setBorderWarningDelay(int32_t warning_delay) {
                    auto res = release_765::play::setBorderWarningDelay(warning_delay);
                    res.data[0].data[0] = 0x50;
                    return res;
                }

                Response setBorderWarningDistance(int32_t warning_distance) {
                    auto res = release_765::play::setBorderWarningDistance(warning_distance);
                    res.data[0].data[0] = 0x51;
                    return res;
                }

                Response setCamera(int32_t entity_id) {
                    auto res = release_765::play::setCamera(entity_id);
                    res.data[0].data[0] = 0x52;
                    return res;
                }

                Response setHeldItem(uint8_t slot) {
                    auto res = release_765::play::setHeldItem(slot);
                    res.data[0].data[0] = 0x53;
                    return res;
                }

                Response setCenterChunk(int32_t x, int32_t z) {
                    auto res = release_765::play::setCenterChunk(x, z);
                    res.data[0].data[0] = 0x54;
                    return res;
                }

                Response setRenderDistance(int32_t render_distance) {
                    auto res = release_765::play::setRenderDistance(render_distance);
                    res.data[0].data[0] = 0x55;
                    return res;
                }

                Response setDefaultSpawnPosition(Position pos, float angle) {
                    auto res = release_765::play::setDefaultSpawnPosition(pos, angle);
                    res.data[0].data[0] = 0x56;
                    return res;
                }

                Response displayObjective(int32_t position, const std::string& objective_name) {
                    auto res = release_765::play::displayObjective(position, objective_name);
                    res.data[0].data[0] = 0x57;
                    return res;
                }

                Response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata) {
                    auto res = release_765::play::setEntityMetadata(entity_id, metadata);
                    res.data[0].data[0] = 0x58;
                    return res;
                }

                Response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id) {
                    auto res = release_765::play::linkEntities(attached_entity_id, holder_entity_id);
                    res.data[0].data[0] = 0x59;
                    return res;
                }

                Response setEntityVelocity(int32_t entity_id, calc::VECTOR velocity) {
                    auto res = release_765::play::setEntityVelocity(entity_id, velocity);
                    res.data[0].data[0] = 0x5A;
                    return res;
                }

                Response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::slot& item) {
                    auto res = release_765::play::setEquipment(entity_id, slot, item);
                    res.data[0].data[0] = 0x5B;
                    return res;
                }

                Response setExperience(float experience_bar, int32_t level, int32_t total_experience) {
                    auto res = release_765::play::setExperience(experience_bar, level, total_experience);
                    res.data[0].data[0] = 0x5C;
                    return res;
                }

                Response setHealth(float health, int32_t food, float saturation) {
                    auto res = release_765::play::setHealth(health, food, saturation);
                    res.data[0].data[0] = 0x5D;
                    return res;
                }

                Response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type) {
                    auto res = release_765::play::updateObjectivesCreate(objective_name, display_name, render_type);
                    res.data[0].data[0] = 0x5E;
                    return res;
                }

                Response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
                    auto res = release_765::play::updateObjectivesCreateStyled(objective_name, display_name, render_type, style);
                    res.data[0].data[0] = 0x5E;
                    return res;
                }

                Response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
                    auto res = release_765::play::updateObjectivesCreateFixed(objective_name, display_name, render_type, content);
                    res.data[0].data[0] = 0x5E;
                    return res;
                }

                Response updateObjectivesRemove(const std::string& objective_name) {
                    auto res = release_765::play::updateObjectivesRemove(objective_name);
                    res.data[0].data[0] = 0x5E;
                    return res;
                }

                Response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type) {
                    auto res = release_765::play::updateObjectivesInfo(objective_name, display_name, render_type);
                    res.data[0].data[0] = 0x5E;
                    return res;
                }

                Response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
                    auto res = release_765::play::updateObjectivesInfoStyled(objective_name, display_name, render_type, style);
                    res.data[0].data[0] = 0x5E;
                    return res;
                }

                Response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
                    auto res = release_765::play::updateObjectivesInfoFixed(objective_name, display_name, render_type, content);
                    res.data[0].data[0] = 0x5E;
                    return res;
                }

                Response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers) {
                    auto res = release_765::play::setPassengers(vehicle_entity_id, passengers);
                    res.data[0].data[0] = 0x5F;
                    return res;
                }

                Response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities) {
                    auto res = release_765::play::updateTeamCreate(team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix, entities);
                    res.data[0].data[0] = 0x60;
                    return res;
                }

                Response updateTeamRemove(const std::string& team_name) {
                    auto res = release_765::play::updateTeamRemove(team_name);
                    res.data[0].data[0] = 0x60;
                    return res;
                }

                Response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix) {
                    auto res = release_765::play::updateTeamInfo(team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix);
                    res.data[0].data[0] = 0x60;
                    return res;
                }

                Response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities) {
                    auto res = release_765::play::updateTeamAddEntities(team_name, entities);
                    res.data[0].data[0] = 0x60;
                    return res;
                }

                Response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities) {
                    auto res = release_765::play::updateTeamRemoveEntities(team_name, entities);
                    res.data[0].data[0] = 0x60;
                    return res;
                }

                Response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name) {
                    auto res = release_765::play::setScore(entity_name, objective_name, value, display_name);
                    res.data[0].data[0] = 0x61;
                    return res;
                }

                Response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled) {
                    auto res = release_765::play::setScoreStyled(entity_name, objective_name, value, display_name, styled);
                    res.data[0].data[0] = 0x61;
                    return res;
                }

                Response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content) {
                    auto res = release_765::play::setScoreFixed(entity_name, objective_name, value, display_name, content);
                    res.data[0].data[0] = 0x61;
                    return res;
                }

                Response setSimulationDistance(int32_t distance) {
                    auto res = release_765::play::setSimulationDistance(distance);
                    res.data[0].data[0] = 0x62;
                    return res;
                }

                Response setSubtitleText(const Chat& text) {
                    auto res = release_765::play::setSubtitleText(text);
                    res.data[0].data[0] = 0x63;
                    return res;
                }

                Response updateTime(int64_t world_age, int64_t time_of_day) {
                    auto res = release_765::play::updateTime(world_age, time_of_day);
                    res.data[0].data[0] = 0x64;
                    return res;
                }

                Response setTitleText(const Chat& text) {
                    auto res = release_765::play::setTitleText(text);
                    res.data[0].data[0] = 0x65;
                    return res;
                }

                Response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out) {
                    auto res = release_765::play::setTitleAnimationTimes(fade_in, stay, fade_out);
                    res.data[0].data[0] = 0x66;
                    return res;
                }

                Response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
                    auto res = release_765::play::entitySoundEffect(sound_id, category, entity_id, volume, pitch, seed);
                    res.data[0].data[0] = 0x67;
                    return res;
                }

                Response entitySoundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
                    auto res = release_765::play::entitySoundEffectCustom(sound_id, range, category, entity_id, volume, pitch, seed);
                    res.data[0].data[0] = 0x67;
                    return res;
                }

                Response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
                    auto res = release_765::play::soundEffect(sound_id, category, x, y, z, volume, pitch, seed);
                    res.data[0].data[0] = 0x68;
                    return res;
                }

                Response soundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
                    auto res = release_765::play::soundEffectCustom(sound_id, range, category, x, y, z, volume, pitch, seed);
                    res.data[0].data[0] = 0x68;
                    return res;
                }

                Response startConfiguration() {
                    auto res = release_765::play::startConfiguration();
                    res.data[0].data[0] = 0x69;
                    return res;
                }

                Response stopSound(uint8_t flags) {
                    auto res = release_765::play::stopSound(flags);
                    res.data[0].data[0] = 0x6A;
                    return res;
                }

                Response stopSoundBySource(uint8_t flags, int32_t source) {
                    auto res = release_765::play::stopSoundBySource(flags, source);
                    res.data[0].data[0] = 0x6A;
                    return res;
                }

                Response stopSoundBySound(uint8_t flags, const std::string& sound) {
                    auto res = release_765::play::stopSoundBySound(flags, sound);
                    res.data[0].data[0] = 0x6A;
                    return res;
                }

                Response stopSoundBySourceAndSound(uint8_t flags, int32_t source, const std::string& sound) {
                    auto res = release_765::play::stopSoundBySourceAndSound(flags, source, sound);
                    res.data[0].data[0] = 0x6A;
                    return res;
                }

                Response storeCookie(const std::string& key, const list_array<uint8_t>& payload) {
                    if (payload.size() > 5120)
                        throw std::runtime_error("Payload size is too big");

                    list_array<uint8_t> packet;
                    packet.reserve(1 + 4 + key.size() + payload.size());
                    packet.push_back(0x6B);
                    WriteIdentifier(packet, key);
                    packet.push_back(payload);
                    return Response::Answer({std::move(packet)});
                }

                Response systemChatMessage(const Chat& message) {
                    auto res = release_765::play::systemChatMessage(message);
                    res.data[0].data[0] = 0x6C;
                    return res;
                }

                Response systemChatMessageOverlay(const Chat& message) {
                    auto res = release_765::play::systemChatMessageOverlay(message);
                    res.data[0].data[0] = 0x6C;
                    return res;
                }

                Response setTabListHeaderAndFooter(const Chat& header, const Chat& footer) {
                    auto res = release_765::play::setTabListHeaderAndFooter(header, footer);
                    res.data[0].data[0] = 0x6D;
                    return res;
                }

                Response tagQueryResponse(int32_t transaction_id, const enbt::value& nbt) {
                    auto res = release_765::play::tagQueryResponse(transaction_id, nbt);
                    res.data[0].data[0] = 0x6E;
                    return res;
                }

                Response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count) {
                    auto res = release_765::play::pickupItem(collected_entity_id, collector_entity_id, pickup_item_count);
                    res.data[0].data[0] = 0x6F;
                    return res;
                }

                Response teleportEntity(int32_t entity_id, calc::VECTOR pos, float yaw, float pitch, bool on_ground) {
                    auto res = release_765::play::teleportEntity(entity_id, pos, yaw, pitch, on_ground);
                    res.data[0].data[0] = 0x70;
                    return res;
                }

                Response setTickingState(float tick_rate, bool is_frozen) {
                    auto res = release_765::play::setTickingState(tick_rate, is_frozen);
                    res.data[0].data[0] = 0x71;
                    return res;
                }

                Response stepTick(int32_t step_count) {
                    auto res = release_765::play::stepTick(step_count);
                    res.data[0].data[0] = 0x72;
                    return res;
                }

                Response transfer(const std::string& host, int32_t port) {
                    list_array<uint8_t> packet;
                    packet.reserve(1 + 4 + host.size() + 4);
                    packet.push_back(0x73);
                    WriteString(packet, host);
                    WriteVar<int32_t>(port, packet);
                    return Response::Answer({std::move(packet)});
                }

                Response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping>& advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress>& progress_advancements) {
                    auto res = release_765::play::updateAdvancements(reset, advancement_mapping, remove_advancements, progress_advancements);
                    res.data[0].data[0] = 0x74;
                    return res;
                }

                Response updateAttributes__(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties, uint32_t protocol_version) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x75);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteVar<int32_t>(properties.size(), packet);
                    for (auto& [key, value, modifiers] : properties) {
                        auto& reg = registers::attributes.at(key);
                        auto id = reg.protocol.find(protocol_version);
                        if (id == reg.protocol.end())
                            continue;
                        WriteVar<int32_t>(id->second, packet);
                        WriteValue<double>(value, packet);
                        WriteVar<int32_t>(modifiers.size(), packet);
                        for (auto& [uuid, amount, operation] : modifiers) {
                            WriteUUID(uuid, packet);
                            WriteValue<double>(amount, packet);
                            packet.push_back(operation);
                        }
                    }
                    return Response::Answer({std::move(packet)});
                }

                Response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) {
                    return updateAttributes__(entity_id, properties);
                }

                Response entityEffect(int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, int8_t flags) {
                    list_array<uint8_t> packet;
                    packet.reserve(18);
                    packet.push_back(0x76);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteVar<int32_t>(effect_id, packet);
                    WriteVar<int32_t>(amplifier, packet);
                    WriteVar<int32_t>(duration, packet);
                    packet.push_back(flags);
                    return Response::Answer({std::move(packet)});
                }

                Response updateRecipes(const std::vector<base_objects::recipe>& recipes) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x77);
                    WriteVar<int32_t>(recipes.size(), packet);
                    for (auto& recipe : recipes) {
                        std::visit(
                            [&](auto&& item) {
                                using type = std::decay_t<decltype(item)>;
                                WriteIdentifier(packet, recipe.id);
                                WriteVar<int32_t>(base_objects::recipes::variant_data<type>::id, packet);
                                if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::crafting_shaped>) {
                                    WriteString(packet, item.group, 32767);
                                    WriteVar<int32_t>((int32_t)item.category, packet);
                                    WriteVar<int32_t>(item.width, packet);
                                    WriteVar<int32_t>(item.height, packet);
                                    for (auto& items : item.ingredients) {
                                        WriteVar<int32_t>(items.size(), packet);
                                        for (auto& ingredient : items)
                                            reader::WriteSlotItem(packet, ingredient);
                                    }
                                    reader::WriteSlotItem(packet, item.result);
                                    packet.push_back(item.show_notification);
                                } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::crafting_shapeless>) {
                                    WriteString(packet, item.group, 32767);
                                    WriteVar<int32_t>((int32_t)item.category, packet);
                                    WriteVar<int32_t>(item.ingredients.size(), packet);
                                    for (auto& items : item.ingredients) {
                                        WriteVar<int32_t>(items.size(), packet);
                                        for (auto& ingredient : items)
                                            reader::WriteSlotItem(packet, ingredient);
                                    }
                                    reader::WriteSlotItem(packet, item.result);
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
                                    WriteVar<int32_t>((int32_t)item.category, packet);
                                } else if constexpr (
                                    std::is_same_v<type, base_objects::recipes::minecraft::smelting>
                                    | std::is_same_v<type, base_objects::recipes::minecraft::blasting>
                                    | std::is_same_v<type, base_objects::recipes::minecraft::smoking>
                                    | std::is_same_v<type, base_objects::recipes::minecraft::campfire_cooking>
                                ) {
                                    WriteString(packet, item.group, 32767);
                                    WriteVar<int32_t>((int32_t)item.category, packet);
                                    WriteVar<int32_t>(item.ingredient.size(), packet);
                                    for (auto& slot : item.ingredient)
                                        reader::WriteSlotItem(packet, slot);
                                    reader::WriteSlotItem(packet, item.result);
                                    WriteValue<float>(item.experience, packet);
                                    WriteVar<int32_t>(item.cooking_time, packet);
                                } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::stonecutting>) {
                                    WriteString(packet, item.group, 32767);
                                    WriteVar<int32_t>(item.ingredient.size(), packet);
                                    for (auto& slot : item.ingredient)
                                        reader::WriteSlotItem(packet, slot);
                                    reader::WriteSlotItem(packet, item.result);
                                } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::smithing_transform>) {
                                    WriteVar<int32_t>(item._template.size(), packet);
                                    for (auto& slot : item._template)
                                        reader::WriteSlotItem(packet, slot);
                                    WriteVar<int32_t>(item.base.size(), packet);
                                    for (auto& slot : item.base)
                                        reader::WriteSlotItem(packet, slot);
                                    WriteVar<int32_t>(item.addition.size(), packet);
                                    for (auto& slot : item.addition)
                                        reader::WriteSlotItem(packet, slot);
                                    reader::WriteSlotItem(packet, item.result);
                                } else if constexpr (std::is_same_v<type, base_objects::recipes::minecraft::smithing_trim>) {
                                    WriteVar<int32_t>(item._template.size(), packet);
                                    for (auto& slot : item._template)
                                        reader::WriteSlotItem(packet, slot);
                                    WriteVar<int32_t>(item.base.size(), packet);
                                    for (auto& slot : item.base)
                                        reader::WriteSlotItem(packet, slot);
                                    WriteVar<int32_t>(item.addition.size(), packet);
                                    for (auto& slot : item.addition)
                                        reader::WriteSlotItem(packet, slot);
                                } else if constexpr (std::is_same_v<type, base_objects::recipes::custom>) {
                                    packet.push_back(item.data);
                                } else
                                    throw std::runtime_error("invalid recipe type");
                            },
                            recipe.data
                        );
                    }
                    return Response::Answer({std::move(packet)});
                }

                Response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings) {
                    auto res = release_765::play::updateTags(tag_mappings);
                    res.data[0].data[0] = 0x78;
                    return res;
                }

                Response projectilePower(int32_t entity_id, double power_x, double power_y, double power_z) {
                    list_array<uint8_t> packet;
                    packet.reserve(32);
                    packet.push_back(0x79);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteValue(power_x, packet);
                    WriteValue(power_y, packet);
                    WriteValue(power_z, packet);
                    return Response::Answer({std::move(packet)});
                }
            }
        }
    }
}
