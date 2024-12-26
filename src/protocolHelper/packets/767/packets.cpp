#include <src/protocolHelper/packets/766/packets.hpp>
#include <src/protocolHelper/packets/767/packets.hpp>
#include <src/protocolHelper/packets/767/writers_readers.hpp>
#include <src/protocolHelper/util.hpp>
#include <src/registers.hpp>

namespace copper_server::packets::release_767 {
    namespace login {
        base_objects::network::response login(int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data) {
            return release_766::login::login(plugin_message_id, chanel, data);
        }

        base_objects::network::response kick(const Chat& reason) {
            return release_766::login::kick(reason);
        }

        base_objects::network::response disableCompression() {
            return release_766::login::disableCompression();
        }

        base_objects::network::response setCompression(int32_t threshold) {
            return release_766::login::setCompression(threshold);
        }

        base_objects::network::response loginSuccess(base_objects::SharedClientData& client) {
            return release_766::login::loginSuccess(client);
        }

        base_objects::network::response encryptionRequest(const std::string& server_id, uint8_t (&verify_token)[4]) {
            return release_766::login::encryptionRequest(server_id, verify_token);
        }

        base_objects::network::response requestCookie(const std::string& key) {
            return release_766::login::requestCookie(key);
        }
    }

    namespace configuration {
        base_objects::network::response requestCookie(const std::string& key) {
            return release_766::configuration::requestCookie(key);
        }

        base_objects::network::response configuration(const std::string& chanel, const list_array<uint8_t>& data) {
            return release_766::configuration::configuration(chanel, data);
        }

        base_objects::network::response kick(const Chat& reason) {
            return release_766::configuration::kick(reason);
        }

        base_objects::network::response finish() {
            return release_766::configuration::finish();
        }

        base_objects::network::response keep_alive(int64_t keep_alive_packet) {
            return release_766::configuration::keep_alive(keep_alive_packet);
        }

        base_objects::network::response ping(int32_t excepted_pong) {
            return release_766::configuration::ping(excepted_pong);
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

        base_objects::network::response registry_data() {
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
                { // minecraft:painting_variant
                    data.push_back(registry_data_serialize_entry(
                        "minecraft:painting_variant",
                        registers::paintingVariants,
                        [](registers::PaintingVariant& it) { //element
                            enbt::compound element;
                            element["asset_id"] = it.asset_id;
                            element["height"] = it.height;
                            element["width"] = it.width;
                            return element;
                        }
                    ));
                }
            }
            return base_objects::network::response::answer(data);
        }

        base_objects::network::response resetChat() {
            return base_objects::network::response::answer({{0x06}});
        }

        base_objects::network::response removeResourcePacks() {
            return release_766::configuration::removeResourcePacks();
        }

        base_objects::network::response removeResourcePack(const enbt::raw_uuid& pack_id) {
            return release_766::configuration::removeResourcePack(pack_id);
        }

        base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced) {
            return release_766::configuration::addResourcePack(client, pack_id, url, hash, forced);
        }

        base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt) {
            return release_766::configuration::addResourcePackPrompted(client, pack_id, url, hash, forced, prompt);
        }

        base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload) {
            if (payload.size() > 5120)
                throw std::runtime_error("Payload size is too big");

            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + key.size() + payload.size());
            packet.push_back(0x0A);
            WriteIdentifier(packet, key);
            packet.push_back(payload);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response transfer(const std::string& host, int32_t port) {
            list_array<uint8_t> packet;
            packet.reserve(1 + 4 + host.size() + 4);
            packet.push_back(0x0B);
            WriteString(packet, host);
            WriteVar<int32_t>(port, packet);
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response setFeatureFlags(const list_array<std::string>& features) {
            return release_766::configuration::setFeatureFlags(features);
        }

        base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries) {
            return release_766::configuration::updateTags(tags_entries);
        }

        base_objects::network::response knownPacks(const list_array<base_objects::packets::known_pack>& packs) {
            list_array<uint8_t> packet;
            packet.push_back(0x0E);
            WriteVar<int32_t>(packs.size(), packet);
            for (const auto& pack : packs) {
                WriteString(packet, pack.namespace_);
                WriteString(packet, pack.id);
                WriteString(packet, pack.version);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response custom_report(const list_array<std::pair<std::string, std::string>>& values) {
            if (values.size() > 32)
                throw std::invalid_argument("Report cannot contain more than 32 entry's.");
            list_array<uint8_t> packet;
            packet.push_back(0x0F);
            WriteVar<int32_t>(values.size(), packet);
            for (auto&& [title, desc] : values) {
                WriteString(packet, title, 128);
                WriteString(packet, desc, 4096);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response server_links(const list_array<base_objects::packets::server_link>& links) {
            list_array<uint8_t> packet;
            packet.push_back(0x10);
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
    }

    namespace play {
        base_objects::network::response bundleResponse(base_objects::network::response&& response) {
            return release_766::play::bundleResponse(std::move(response));
        }

        base_objects::network::response spawnEntity(const base_objects::entity& entity, uint16_t protocol) {
            if (protocol == UINT16_MAX)
                protocol = 767;
            return release_766::play::spawnEntity(entity, protocol);
        }

        base_objects::network::response spawnExperienceOrb(const base_objects::entity& entity, int16_t count) {
            return release_766::play::spawnExperienceOrb(entity, count);
        }

        base_objects::network::response entityAnimation(const base_objects::entity& entity, uint8_t animation) {
            return release_766::play::entityAnimation(entity, animation);
        }

        base_objects::network::response awardStatistics(const list_array<base_objects::packets::statistics>& statistics) {
            return release_766::play::awardStatistics(statistics);
        }

        base_objects::network::response acknowledgeBlockChange(base_objects::SharedClientData& client) {
            return release_766::play::acknowledgeBlockChange(client);
        }

        base_objects::network::response setBlockDestroyStage(const base_objects::entity& entity, base_objects::position block, uint8_t stage) {
            return release_766::play::setBlockDestroyStage(entity, block, stage);
        }

        base_objects::network::response blockEntityData(base_objects::position block, int32_t type, const enbt::value& data) {
            return release_766::play::blockEntityData(block, type, data);
        }

        base_objects::network::response blockAction(base_objects::position block, int32_t action_id, int32_t param, int32_t block_type) {
            return release_766::play::blockAction(block, action_id, param, block_type);
        }

        base_objects::network::response blockUpdate(base_objects::position block, int32_t block_type) {
            return release_766::play::blockUpdate(block, block_type);
        }

        base_objects::network::response bossBarAdd(const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags) {
            return release_766::play::bossBarAdd(id, title, health, color, division, flags);
        }

        base_objects::network::response bossBarRemove(const enbt::raw_uuid& id) {
            return release_766::play::bossBarRemove(id);
        }

        base_objects::network::response bossBarUpdateHealth(const enbt::raw_uuid& id, float health) {
            return release_766::play::bossBarUpdateHealth(id, health);
        }

        base_objects::network::response bossBarUpdateTitle(const enbt::raw_uuid& id, const Chat& title) {
            return release_766::play::bossBarUpdateTitle(id, title);
        }

        base_objects::network::response bossBarUpdateStyle(const enbt::raw_uuid& id, int32_t color, int32_t division) {
            return release_766::play::bossBarUpdateStyle(id, color, division);
        }

        base_objects::network::response bossBarUpdateFlags(const enbt::raw_uuid& id, uint8_t flags) {
            return release_766::play::bossBarUpdateFlags(id, flags);
        }

        base_objects::network::response changeDifficulty(uint8_t difficulty, bool locked) {
            return release_766::play::changeDifficulty(difficulty, locked);
        }

        base_objects::network::response chunkBatchFinished(int32_t count) {
            return release_766::play::chunkBatchFinished(count);
        }

        base_objects::network::response chunkBatchStart() {
            return release_766::play::chunkBatchStart();
        }

        base_objects::network::response chunkBiomes(list_array<base_objects::chunk::chunk_biomes>& chunk) {
            return release_766::play::chunkBiomes(chunk);
        }

        base_objects::network::response clearTitles(bool reset) {
            return release_766::play::clearTitles(reset);
        }

        base_objects::network::response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions) {
            return release_766::play::commandSuggestionsResponse(transaction_id, start_pos, length, suggestions);
        }

        base_objects::network::response commands(int32_t root_id, const list_array<base_objects::packets::command_node>& nodes) {
            return release_766::play::commands(root_id, nodes);
        }

        base_objects::network::response closeContainer(uint8_t container_id) {
            return release_766::play::closeContainer(container_id);
        }

        base_objects::network::response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item) {
            return release_766::play::setContainerContent(windows_id, state_id, slots, carried_item);
        }

        base_objects::network::response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value) {
            return release_766::play::setContainerProperty(windows_id, property, value);
        }

        base_objects::network::response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item) {
            return release_766::play::setContainerSlot(windows_id, state_id, slot, item);
        }

        base_objects::network::response cookieRequest(const std::string& key) {
            return release_766::play::cookieRequest(key);
        }

        base_objects::network::response setCooldown(int32_t item_id, int32_t cooldown) {
            return release_766::play::setCooldown(item_id, cooldown);
        }

        base_objects::network::response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions) {
            return release_766::play::chatSuggestionsResponse(action, count, suggestions);
        }

        base_objects::network::response customPayload(const std::string& channel, const list_array<uint8_t>& data) {
            return release_766::play::customPayload(channel, data);
        }

        base_objects::network::response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<util::VECTOR> xyz) {
            return release_766::play::damageEvent(entity_id, source_type_id, source_cause_id, source_direct_id, xyz);
        }

        base_objects::network::response debugSample(const list_array<uint64_t>& sample, int32_t sample_type) {
            return release_766::play::debugSample(sample, sample_type);
        }

        base_objects::network::response deleteMessageBySignature(uint8_t (&signature)[256]) {
            return release_766::play::deleteMessageBySignature(signature);
        }

        base_objects::network::response deleteMessageByID(int32_t message_id) {
            return release_766::play::deleteMessageByID(message_id);
        }

        base_objects::network::response kick(const Chat& reason) {
            return release_766::play::kick(reason);
        }

        base_objects::network::response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name) {
            return release_766::play::disguisedChatMessage(message, chat_type, sender, target_name);
        }

        base_objects::network::response entityEvent(int32_t entity_id, uint8_t entity_status) {
            return release_766::play::entityEvent(entity_id, entity_status);
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
            return release_766::play::explosion(pos, strength, affected_blocks, player_motion, block_interaction, small_explosion_particle_id, small_explosion_particle_data, large_explosion_particle_id, large_explosion_particle_data, sound_name, fixed_range);
        }

        base_objects::network::response unloadChunk(int32_t x, int32_t z) {
            return release_766::play::unloadChunk(x, z);
        }

        base_objects::network::response gameEvent(uint8_t event_id, float value) {
            return release_766::play::gameEvent(event_id, value);
        }

        base_objects::network::response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id) {
            return release_766::play::openHorseWindow(window_id, slots, entity_id);
        }

        base_objects::network::response hurtAnimation(int32_t entity_id, float yaw) {
            return release_766::play::hurtAnimation(entity_id, yaw);
        }

        base_objects::network::response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time) {
            return release_766::play::initializeWorldBorder(x, z, old_diameter, new_diameter, speed_ms, portal_teleport_boundary, warning_blocks, warning_time);
        }

        //internal use
        base_objects::network::response keepAlive(int64_t id) {
            return release_766::play::keepAlive(id);
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
            return release_766::play::updateChunkDataWLights(chunk_x, chunk_z, heightmaps, data, sky_light_mask, block_light_mask, empty_skylight_mask, empty_block_light_mask, sky_light_arrays, block_light_arrays);
        }

        base_objects::network::response worldEvent(int32_t event, base_objects::position pos, int32_t data, bool global) {
            return release_766::play::worldEvent(event, pos, data, global);
        }

        base_objects::network::response particle(int32_t particle_id, bool long_distance, util::VECTOR pos, util::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data) {
            return release_766::play::particle(particle_id, long_distance, pos, offset, max_speed, count, data);
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
            return release_766::play::updateLight(chunk_x, chunk_z, sky_light_mask, block_light_mask, empty_skylight_mask, empty_block_light_mask, sky_light_arrays, block_light_arrays);
        }

        base_objects::network::response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool enforces_secure_chat) {
            return release_766::play::joinGame(entity_id, is_hardcore, dimension_names, max_players, view_distance, simulation_distance, reduced_debug_info, enable_respawn_screen, do_limited_crafting, current_dimension_type, dimension_name, hashed_seed, gamemode, prev_gamemode, is_debug, is_flat, death_location, portal_cooldown, enforces_secure_chat);
        }

        base_objects::network::response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t columns, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data) {
            return release_766::play::mapData(map_id, scale, locked, icons, columns, rows, x, z, data);
        }

        base_objects::network::response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade>& trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock) {
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
            return release_766::play::merchantOffers(window_id, trade_id, trades, level, experience, regular_villager, can_restock);
        }

        base_objects::network::response updateEntityPosition(int32_t entity_id, util::XYZ<float> pos, bool on_ground) {
            return release_766::play::updateEntityPosition(entity_id, pos, on_ground);
        }

        base_objects::network::response updateEntityPositionAndRotation(int32_t entity_id, util::XYZ<float> pos, util::VECTOR rot, bool on_ground) {
            return release_766::play::updateEntityPositionAndRotation(entity_id, pos, rot, on_ground);
        }

        base_objects::network::response updateEntityRotation(int32_t entity_id, util::VECTOR rot, bool on_ground) {
            return release_766::play::updateEntityRotation(entity_id, rot, on_ground);
        }

        base_objects::network::response moveVehicle(util::VECTOR pos, util::VECTOR rot) {
            return release_766::play::moveVehicle(pos, rot);
        }

        base_objects::network::response openBook(int32_t hand) {
            return release_766::play::openBook(hand);
        }

        base_objects::network::response openScreen(int32_t window_id, int32_t type, const Chat& title) {
            return release_766::play::openScreen(window_id, type, title);
        }

        base_objects::network::response openSignEditor(base_objects::position pos, bool is_front_text) {
            return release_766::play::openSignEditor(pos, is_front_text);
        }

        base_objects::network::response ping(int32_t id) {
            return release_766::play::ping(id);
        }

        base_objects::network::response pingResponse(int32_t id) {
            return release_766::play::pingResponse(id);
        }

        base_objects::network::response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id) {
            return release_766::play::placeGhostRecipe(windows_id, recipe_id);
        }

        base_objects::network::response playerAbilities(uint8_t flags, float flying_speed, float field_of_view) {
            return release_766::play::playerAbilities(flags, flying_speed, field_of_view);
        }

        base_objects::network::response playerChatMessage(enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, const std::optional<enbt::value>& __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name) {
            return release_766::play::playerChatMessage(sender, index, signature, message, timestamp, salt, prev_messages, __UNDEFINED__FIELD__, filter_type, filtered_symbols_bitfield, chat_type, sender_name, target_name);
        }

        base_objects::network::response endCombat(int32_t duration) {
            return release_766::play::endCombat(duration);
        }

        base_objects::network::response enterCombat() {
            return release_766::play::enterCombat();
        }

        base_objects::network::response combatDeath(int32_t player_id, const Chat& message) {
            return release_766::play::combatDeath(player_id, message);
        }

        base_objects::network::response playerInfoRemove(const list_array<enbt::raw_uuid>& players) {
            return release_766::play::playerInfoRemove(players);
        }

        base_objects::network::response playerInfoAdd(const list_array<base_objects::packets::player_actions_add>& add_players) {
            return release_766::play::playerInfoAdd(add_players);
        }

        base_objects::network::response playerInfoInitializeChat(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat) {
            return release_766::play::playerInfoInitializeChat(initialize_chat);
        }

        base_objects::network::response playerInfoUpdateGameMode(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode) {
            return release_766::play::playerInfoUpdateGameMode(update_game_mode);
        }

        base_objects::network::response playerInfoUpdateListed(const list_array<base_objects::packets::player_actions_update_listed>& update_listed) {
            return release_766::play::playerInfoUpdateListed(update_listed);
        }

        base_objects::network::response playerInfoUpdateLatency(const list_array<base_objects::packets::player_actions_update_latency>& update_latency) {
            return release_766::play::playerInfoUpdateLatency(update_latency);
        }

        base_objects::network::response playerInfoUpdateDisplayName(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name) {
            return release_766::play::playerInfoUpdateDisplayName(update_display_name);
        }

        base_objects::network::response lookAt(bool from_feet_or_eyes, util::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id) {
            return release_766::play::lookAt(from_feet_or_eyes, target, entity_id);
        }

        base_objects::network::response synchronizePlayerPosition(util::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id) {
            return release_766::play::synchronizePlayerPosition(pos, yaw, pitch, flags, teleport_id);
        }

        base_objects::network::response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& displayed_recipe_ids, const list_array<std::string>& had_access_to_recipe_ids) {
            return release_766::play::initRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, displayed_recipe_ids, had_access_to_recipe_ids);
        }

        base_objects::network::response addRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) {
            return release_766::play::addRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids);
        }

        base_objects::network::response removeRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) {
            return release_766::play::removeRecipeBook(crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids);
        }

        base_objects::network::response removeEntities(const list_array<int32_t>& entity_ids) {
            return release_766::play::removeEntities(entity_ids);
        }

        base_objects::network::response removeEntityEffect(int32_t entity_id, int32_t effect_id) {
            return release_766::play::removeEntityEffect(entity_id, effect_id);
        }

        base_objects::network::response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name) {
            return release_766::play::resetScore(entity_name, objective_name);
        }

        base_objects::network::response removeResourcePacks() {
            return release_766::play::removeResourcePacks();
        }

        base_objects::network::response removeResourcePack(enbt::raw_uuid id) {
            return release_766::play::removeResourcePack(id);
        }

        base_objects::network::response addResourcePack(enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt) {
            return release_766::play::addResourcePack(id, url, hash, forced, prompt);
        }

        base_objects::network::response respawn(int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata) {
            return release_766::play::respawn(dimension_type, dimension_name, hashed_seed, gamemode, previous_gamemode, is_debug, is_flat, death_location, portal_cooldown, keep_attributes, keep_metadata);
        }

        base_objects::network::response setHeadRotation(int32_t entity_id, util::VECTOR head_rotation) {
            return release_766::play::setHeadRotation(entity_id, head_rotation);
        }

        base_objects::network::response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks) {
            return release_766::play::updateSectionBlocks(section_x, section_z, section_y, blocks);
        }

        base_objects::network::response setAdvancementsTab(const std::optional<std::string>& tab_id) {
            return release_766::play::setAdvancementsTab(tab_id);
        }

        base_objects::network::response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool __ignored) {
            return release_766::play::serverData(motd, icon_png);
        }

        base_objects::network::response setActionBarText(const Chat& text) {
            return release_766::play::setActionBarText(text);
        }

        base_objects::network::response setBorderCenter(double x, double z) {
            return release_766::play::setBorderCenter(x, z);
        }

        base_objects::network::response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms) {
            return release_766::play::setBorderLerp(old_diameter, new_diameter, speed_ms);
        }

        base_objects::network::response setBorderSize(double diameter) {
            return release_766::play::setBorderSize(diameter);
        }

        base_objects::network::response setBorderWarningDelay(int32_t warning_delay) {
            return release_766::play::setBorderWarningDelay(warning_delay);
        }

        base_objects::network::response setBorderWarningDistance(int32_t warning_distance) {
            return release_766::play::setBorderWarningDistance(warning_distance);
        }

        base_objects::network::response setCamera(int32_t entity_id) {
            return release_766::play::setCamera(entity_id);
        }

        base_objects::network::response setHeldItem(uint8_t slot) {
            return release_766::play::setHeldItem(slot);
        }

        base_objects::network::response setCenterChunk(int32_t x, int32_t z) {
            return release_766::play::setCenterChunk(x, z);
        }

        base_objects::network::response setRenderDistance(int32_t render_distance) {
            return release_766::play::setRenderDistance(render_distance);
        }

        base_objects::network::response setDefaultSpawnPosition(base_objects::position pos, float angle) {
            return release_766::play::setDefaultSpawnPosition(pos, angle);
        }

        base_objects::network::response displayObjective(int32_t position, const std::string& objective_name) {
            return release_766::play::displayObjective(position, objective_name);
        }

        base_objects::network::response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata) {
            return release_766::play::setEntityMetadata(entity_id, metadata);
        }

        base_objects::network::response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id) {
            return release_766::play::linkEntities(attached_entity_id, holder_entity_id);
        }

        base_objects::network::response setEntityVelocity(int32_t entity_id, util::VECTOR velocity) {
            return release_766::play::setEntityVelocity(entity_id, velocity);
        }

        base_objects::network::response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::slot& item) {
            return release_766::play::setEquipment(entity_id, slot, item);
        }

        base_objects::network::response setExperience(float experience_bar, int32_t level, int32_t total_experience) {
            return release_766::play::setExperience(experience_bar, level, total_experience);
        }

        base_objects::network::response setHealth(float health, int32_t food, float saturation) {
            return release_766::play::setHealth(health, food, saturation);
        }

        base_objects::network::response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type) {
            return release_766::play::updateObjectivesCreate(objective_name, display_name, render_type);
        }

        base_objects::network::response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
            return release_766::play::updateObjectivesCreateStyled(objective_name, display_name, render_type, style);
        }

        base_objects::network::response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
            return release_766::play::updateObjectivesCreateFixed(objective_name, display_name, render_type, content);
        }

        base_objects::network::response updateObjectivesRemove(const std::string& objective_name) {
            return release_766::play::updateObjectivesRemove(objective_name);
        }

        base_objects::network::response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type) {
            return release_766::play::updateObjectivesInfo(objective_name, display_name, render_type);
        }

        base_objects::network::response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
            return release_766::play::updateObjectivesInfoStyled(objective_name, display_name, render_type, style);
        }

        base_objects::network::response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
            return release_766::play::updateObjectivesInfoFixed(objective_name, display_name, render_type, content);
        }

        base_objects::network::response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers) {
            return release_766::play::setPassengers(vehicle_entity_id, passengers);
        }

        base_objects::network::response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities) {
            return release_766::play::updateTeamCreate(team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix, entities);
        }

        base_objects::network::response updateTeamRemove(const std::string& team_name) {
            return release_766::play::updateTeamRemove(team_name);
        }

        base_objects::network::response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix) {
            return release_766::play::updateTeamInfo(team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix);
        }

        base_objects::network::response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities) {
            return release_766::play::updateTeamAddEntities(team_name, entities);
        }

        base_objects::network::response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities) {
            return release_766::play::updateTeamRemoveEntities(team_name, entities);
        }

        base_objects::network::response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name) {
            return release_766::play::setScore(entity_name, objective_name, value, display_name);
        }

        base_objects::network::response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled) {
            return release_766::play::setScoreStyled(entity_name, objective_name, value, display_name, styled);
        }

        base_objects::network::response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content) {
            return release_766::play::setScoreFixed(entity_name, objective_name, value, display_name, content);
        }

        base_objects::network::response setSimulationDistance(int32_t distance) {
            return release_766::play::setSimulationDistance(distance);
        }

        base_objects::network::response setSubtitleText(const Chat& text) {
            return release_766::play::setSubtitleText(text);
        }

        base_objects::network::response updateTime(int64_t world_age, int64_t time_of_day) {
            return release_766::play::updateTime(world_age, time_of_day);
        }

        base_objects::network::response setTitleText(const Chat& text) {
            return release_766::play::setTitleText(text);
        }

        base_objects::network::response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out) {
            return release_766::play::setTitleAnimationTimes(fade_in, stay, fade_out);
        }

        base_objects::network::response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
            return release_766::play::entitySoundEffect(sound_id, category, entity_id, volume, pitch, seed);
        }

        base_objects::network::response entitySoundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
            return release_766::play::entitySoundEffectCustom(sound_id, range, category, entity_id, volume, pitch, seed);
        }

        base_objects::network::response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
            return release_766::play::soundEffect(sound_id, category, x, y, z, volume, pitch, seed);
        }

        base_objects::network::response soundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
            return release_766::play::soundEffectCustom(sound_id, range, category, x, y, z, volume, pitch, seed);
        }

        base_objects::network::response startConfiguration() {
            return release_766::play::startConfiguration();
        }

        base_objects::network::response stopSound(uint8_t flags) {
            return release_766::play::stopSound(flags);
        }

        base_objects::network::response stopSoundBySource(uint8_t flags, int32_t source) {
            return release_766::play::stopSoundBySource(flags, source);
        }

        base_objects::network::response stopSoundBySound(uint8_t flags, const std::string& sound) {
            return release_766::play::stopSoundBySound(flags, sound);
        }

        base_objects::network::response stopSoundBySourceAndSound(uint8_t flags, int32_t source, const std::string& sound) {
            return release_766::play::stopSoundBySourceAndSound(flags, source, sound);
        }

        base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload) {
            return release_766::play::storeCookie(key, payload);
        }

        base_objects::network::response systemChatMessage(const Chat& message) {
            return release_766::play::systemChatMessage(message);
        }

        base_objects::network::response systemChatMessageOverlay(const Chat& message) {
            return release_766::play::systemChatMessageOverlay(message);
        }

        base_objects::network::response setTabListHeaderAndFooter(const Chat& header, const Chat& footer) {
            return release_766::play::setTabListHeaderAndFooter(header, footer);
        }

        base_objects::network::response tagQueryResponse(int32_t transaction_id, const enbt::value& nbt) {
            return release_766::play::tagQueryResponse(transaction_id, nbt);
        }

        base_objects::network::response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count) {
            return release_766::play::pickupItem(collected_entity_id, collector_entity_id, pickup_item_count);
        }

        base_objects::network::response teleportEntity(int32_t entity_id, util::VECTOR pos, float yaw, float pitch, bool on_ground) {
            return release_766::play::teleportEntity(entity_id, pos, yaw, pitch, on_ground);
        }

        base_objects::network::response setTickingState(float tick_rate, bool is_frozen) {
            return release_766::play::setTickingState(tick_rate, is_frozen);
        }

        base_objects::network::response stepTick(int32_t step_count) {
            return release_766::play::stepTick(step_count);
        }

        base_objects::network::response transfer(const std::string& host, int32_t port) {
            return release_766::play::transfer(host, port);
        }

        base_objects::network::response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping>& advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress>& progress_advancements) {
            return release_766::play::updateAdvancements(reset, advancement_mapping, remove_advancements, progress_advancements);
        }

        base_objects::network::response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) {
            return release_766::play::updateAttributes__(entity_id, properties, 767);
        }

        base_objects::network::response entityEffect(int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, int8_t flags) {
            return release_766::play::entityEffect(entity_id, effect_id, amplifier, duration, flags);
        }

        base_objects::network::response updateRecipes(const std::vector<base_objects::recipe>& recipes) {
            return release_766::play::updateRecipes(recipes);
        }

        base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings) {
            return release_766::play::updateTags(tag_mappings);
        }

        base_objects::network::response projectilePower(int32_t entity_id, double power_x, double power_y, double power_z) {
            return release_766::play::projectilePower(entity_id, power_x, power_y, power_z);
        }

        base_objects::network::response custom_report(const list_array<std::pair<std::string, std::string>>& values) {
            if (values.size() > 32)
                throw std::invalid_argument("Report cannot contain more than 32 entry's.");
            list_array<uint8_t> packet;
            packet.push_back(0x7A);
            WriteVar<int32_t>(values.size(), packet);
            for (auto&& [title, desc] : values) {
                WriteString(packet, title, 128);
                WriteString(packet, desc, 4096);
            }
            return base_objects::network::response::answer({std::move(packet)});
        }

        base_objects::network::response server_links(const list_array<base_objects::packets::server_link>& links) {
            list_array<uint8_t> packet;
            packet.push_back(0x7B);
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
    }
}
