#ifndef SRC_PROTOCOLHELPER_PACKETS_765_RELEASE_PACKETS
#define SRC_PROTOCOLHELPER_PACKETS_765_RELEASE_PACKETS
#include "../../../ClientHandleHelper.hpp"
#include "../../../base_objects/chat.hpp"
#include "../../../base_objects/chunk.hpp"
#include "../../../base_objects/entity.hpp"
#include "../../../base_objects/packets.hpp"
#include "../../../base_objects/position.hpp"

//packets for 1.20.4, protocol 765
namespace crafted_craft {
    namespace packets {
        namespace release_765 {
            namespace login {
                TCPclient::Response kick(const Chat& reason);
                TCPclient::Response disableCompression();
                TCPclient::Response setCompression(int32_t threshold);
            }

            namespace configuration {
                TCPclient::Response configuration(const std::string& chanel, const list_array<uint8_t>& data);

                TCPclient::Response kick(const Chat& reason);

                TCPclient::Response removeResourcePacks();

                TCPclient::Response removeResourcePack(const ENBT::UUID& pack_id);

                TCPclient::Response addResourcePack(const ENBT::UUID& pack_id, const std::string& url, const std::string& hash, bool forced);

                TCPclient::Response addResourcePack(const ENBT::UUID& pack_id, const std::string& url, const std::string& hash, bool forced, Chat prompt);

                TCPclient::Response setFeatureFlags(const list_array<std::string>& features);

                TCPclient::Response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries);
            }

            namespace play {
                TCPclient::Response bundleResponse(TCPclient::Response&& response);

                TCPclient::Response spawnEntity(SharedClientData& client, const Entity& entity);
                TCPclient::Response spawnExperienceOrb(SharedClientData& client, const Entity& entity, int16_t count);
                TCPclient::Response entityAnimation(SharedClientData& client, const Entity& entity, uint8_t animation);
                TCPclient::Response awardStatistics(const list_array<base_objects::packets::statistics>& statistics);
                TCPclient::Response acknowledgeBlockChange(SharedClientData& client);

                TCPclient::Response setBlockDestroyStage(SharedClientData& client, const Entity& entity, Position block, uint8_t stage);

                TCPclient::Response blockEntityData(Position block, int32_t type, const ENBT& data);
                //block_type is from "minecraft:block" registry, not a block state.
                TCPclient::Response blockAction(Position block, int32_t action_id, int32_t param, int32_t block_type);
                //block_type is from "minecraft:block" registry, not a block state.
                TCPclient::Response blockUpdate(Position block, int32_t block_type);
                TCPclient::Response bossBarAdd(const ENBT::UUID& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags);
                TCPclient::Response bossBarRemove(const ENBT::UUID& id);
                TCPclient::Response bossBarUpdateHealth(const ENBT::UUID& id, float health);
                TCPclient::Response bossBarUpdateTitle(const ENBT::UUID& id, const Chat& title);
                TCPclient::Response bossBarUpdateStyle(const ENBT::UUID& id, int32_t color, int32_t division);
                TCPclient::Response bossBarUpdateFlags(const ENBT::UUID& id, uint8_t flags);
                TCPclient::Response changeDifficulty(uint8_t difficulty, bool locked);
                TCPclient::Response chunkBatchFinished(int32_t count);
                TCPclient::Response chunkBatchStart();

                TCPclient::Response chunkBiomes(list_array<base_objects::chunk::chunk_biomes>& chunk);
                TCPclient::Response clearTitles(bool reset);

                TCPclient::Response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions);
                TCPclient::Response commands(int32_t root_id, const list_array<base_objects::packets::command_node>& nodes);
                TCPclient::Response closeContainer(uint8_t container_id);
                TCPclient::Response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::packets::slot>& slots, base_objects::packets::slot carried_item);

                TCPclient::Response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value);

                TCPclient::Response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, base_objects::packets::slot item);

                TCPclient::Response setCooldown(int32_t item_id, int32_t cooldown);

                //UNUSED by Notchian client
                TCPclient::Response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions);

                TCPclient::Response customPayload(const std::string& channel, const list_array<uint8_t>& data);

                TCPclient::Response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<calc::VECTOR> xyz);

                TCPclient::Response deleteMessage(uint8_t signature[256]);

                TCPclient::Response deleteMessage(int32_t message_id);

                TCPclient::Response kick(const Chat& reason);

                TCPclient::Response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, std::optional<Chat> target_name);

                TCPclient::Response entityEvent(int32_t entity_id, uint8_t entity_status);

                //TODO: particles
                //TCPclient::Response explosion(calc::VECTOR pos, float strength, list_array<calc::XYZ<int8_t>> affected_blocks, calc::VECTOR player_motion, int32_t block_interaction, int32_t small_explosion_particle_id, ) {
                //    list_array<uint8_t> packet;
                //    packet.reserve(1 + 4 * 4 + 4 + 4 * 3 * affected_blocks.size() + 4 * 3);
                //    packet.push_back(0x1E);
                //    WriteValue<float>(pos.x, packet);
                //    WriteValue<float>(pos.y, packet);
                //    WriteValue<float>(pos.z, packet);
                //    WriteValue<float>(strength, packet);
                //    WriteVar<int32_t>(affected_blocks.size(), packet);
                //    for (auto& it : affected_blocks) {
                //        WriteValue<int8_t>(it.x, packet);
                //        WriteValue<int8_t>(it.y, packet);
                //        WriteValue<int8_t>(it.z, packet);
                //    }
                //    WriteValue<float>(player_motion.x, packet);
                //    WriteValue<float>(player_motion.y, packet);
                //    WriteValue<float>(player_motion.z, packet);
                //    return TCPclient::Response::Answer({packet});
                //}

                TCPclient::Response unloadChunk(int32_t x, int32_t z);
                TCPclient::Response gameEvent(uint8_t event_id, float value);
                TCPclient::Response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id);

                TCPclient::Response hurtAnimation(int32_t entity_id, float yaw);

                TCPclient::Response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time);
                //internal use
                TCPclient::Response keepAlive(int64_t id);
                //TODO:
                //TCPclient::Response updateChunkDataWLights(...){}

                TCPclient::Response worldEvent(int32_t event, Position pos, int32_t data, bool global);
                TCPclient::Response particle(int32_t particle_id, bool long_distance, calc::VECTOR pos, calc::XYZ<float> offset, float max_speed, int32_t count, list_array<uint8_t> data);
                //TODO:
                //TCPclient::Response updateLight(int32_t chunk_x, int32_t chunk_z, list_array<base_objects::chunk::chunk_light> light) {
                //    list_array<uint8_t> packet;
                //    packet.reserve(1 + 4 * 2 + 4 * light.size());
                //    packet.push_back(0x28);
                //    WriteVar<int32_t>(chunk_x, packet);
                //    WriteVar<int32_t>(chunk_z, packet);
                //    WriteVar<int32_t>(light.size(), packet);
                //    for (auto& it : light) {
                //        WriteVar<int32_t>(it.y, packet);
                //        WriteVar<int32_t>(it.sky_light, packet);
                //        WriteVar<int32_t>(it.block_light, packet);
                //    }
                //    return TCPclient::Response::Answer({packet});
                //}

                TCPclient::Response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, const std::string& current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, std::optional<base_objects::packets::death_location_data> death_location, int32_t portal_cooldown);
                TCPclient::Response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons = {}, uint8_t columns = 0, uint8_t rows = 0, uint8_t x = 0, uint8_t z = 0, const list_array<uint8_t>& data = {});
                TCPclient::Response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade> trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock);
                TCPclient::Response updateEntityPosition(int32_t entity_id, calc::XYZ<float> pos, bool on_ground);
                TCPclient::Response updateEntityPositionAndRotation(int32_t entity_id, calc::XYZ<float> pos, calc::VECTOR rot, bool on_ground);
                TCPclient::Response updateEntityRotation(int32_t entity_id, calc::VECTOR rot, bool on_ground);
                TCPclient::Response moveVehicle(calc::VECTOR pos, calc::VECTOR rot);
                TCPclient::Response openBook(int32_t hand);
                TCPclient::Response openScreen(int32_t window_id, int32_t type, const Chat& title);
                TCPclient::Response openSignEditor(Position pos, bool is_front_text);
                TCPclient::Response ping(int32_t id);
                TCPclient::Response pingResponse(int32_t id);
                TCPclient::Response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id);
                TCPclient::Response playerAbilities(uint8_t flags, float flying_speed, float field_of_view);
                TCPclient::Response playerChatMessage(ENBT::UUID sender, int32_t index, std::optional<std::array<uint8_t, 256>> signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::pair<int32_t, ENBT::UUID>>& prev_messages, std::optional<ENBT> __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name);
                //UNUSED by Notchian client
                TCPclient::Response endCombat(int32_t duration);
                //UNUSED by Notchian client
                TCPclient::Response enterCombat();
                TCPclient::Response combatDeath(int32_t player_id, const Chat& message);
                TCPclient::Response playerInfoRemove(const list_array<ENBT::UUID>& players);
                TCPclient::Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_add>& add_players);
                TCPclient::Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat);
                TCPclient::Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode);
                TCPclient::Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_update_listed>& update_listed);
                TCPclient::Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_update_latency>& update_latency);
                TCPclient::Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name);
                TCPclient::Response lookAt(bool from_feet_or_eyes, calc::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id);
                TCPclient::Response synchronizePlayerPosition(calc::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id);
                TCPclient::Response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> displayed_recipe_ids, list_array<std::string> had_access_to_recipe_ids);
                TCPclient::Response updateRecipeBook(bool add_remove, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids);
                TCPclient::Response removeEntities(const list_array<int32_t>& entity_ids);
                TCPclient::Response removeEntityEffect(int32_t entity_id, int32_t effect_id);
                TCPclient::Response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name);
                TCPclient::Response removeResourcePacks();
                TCPclient::Response removeResourcePack(ENBT::UUID id);
                TCPclient::Response addResourcePack(ENBT::UUID id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt);
                TCPclient::Response respawn(const std::string& dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata);
                TCPclient::Response setHeadRotation(int32_t entity_id, calc::VECTOR head_rotation);

                TCPclient::Response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<CompressedBlockState>& blocks);

                TCPclient::Response setAdvancementsTab(const std::optional<std::string>& tab_id);
                TCPclient::Response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool secure_chat);
                TCPclient::Response setActionBarText(const Chat& text);
                TCPclient::Response setBorderCenter(double x, double z);
                TCPclient::Response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms);
                TCPclient::Response setBorderSize(double diameter);
                TCPclient::Response setBorderWarningDelay(int32_t warning_delay);
                TCPclient::Response setBorderWarningDistance(int32_t warning_distance);
                TCPclient::Response setCamera(int32_t entity_id);
                TCPclient::Response setHeldItem(uint8_t slot);
                TCPclient::Response setCenterChunk(int32_t x, int32_t z);
                TCPclient::Response setRenderDistance(int32_t render_distance);
                TCPclient::Response setDefaultSpawnPosition(Position pos, float angle);
                TCPclient::Response displayObjective(int32_t position, const std::string& objective_name);
                TCPclient::Response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata);
                TCPclient::Response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id);
                TCPclient::Response setEntityVelocity(int32_t entity_id, calc::VECTOR velocity);

                TCPclient::Response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::packets::slot& item);

                TCPclient::Response setExperience(float experience_bar, int32_t level, int32_t total_experience);

                TCPclient::Response setHealth(float health, int32_t food, float saturation);

                TCPclient::Response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type);
                TCPclient::Response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const ENBT& style);
                TCPclient::Response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);
                TCPclient::Response updateObjectivesRemove(const std::string& objective_name);
                TCPclient::Response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type);
                TCPclient::Response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const ENBT& style);
                TCPclient::Response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);

                TCPclient::Response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers);


                TCPclient::Response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities);
                TCPclient::Response updateTeamRemove(const std::string& team_name);
                TCPclient::Response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix);
                TCPclient::Response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities);
                TCPclient::Response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities);


                TCPclient::Response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name);
                TCPclient::Response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const ENBT& styled);
                TCPclient::Response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content);

                TCPclient::Response setSimulationDistance(float distance);
                TCPclient::Response setSubtitleText(const Chat& text);
                TCPclient::Response updateTime(int64_t world_age, int64_t time_of_day);
                TCPclient::Response setTitleText(const Chat& text);
                TCPclient::Response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out);


                TCPclient::Response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);
                TCPclient::Response entitySoundEffect(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);

                TCPclient::Response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);
                TCPclient::Response soundEffect(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);


                TCPclient::Response startConfiguration();
                TCPclient::Response stopSound(uint8_t flags);
                TCPclient::Response stopSound(uint8_t flags, int32_t source);
                TCPclient::Response stopSound(uint8_t flags, const std::string& sound);
                TCPclient::Response stopSound(uint8_t flags, int32_t source, const std::string& sound);
                TCPclient::Response systemChatMessage(const Chat& message);
                TCPclient::Response systemChatMessageOverlay(const Chat& message);
                TCPclient::Response setTabListHeaderAndFooter(const Chat& header, const Chat& footer);
                TCPclient::Response tagQueryResponse(int32_t transaction_id, const ENBT& nbt);
                TCPclient::Response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count);
                TCPclient::Response teleportEntity(int32_t entity_id, calc::VECTOR pos, float yaw, float pitch, bool on_ground);
                TCPclient::Response setTickingState(float tick_rate, bool is_frozen);
                TCPclient::Response stepTick(int32_t step_count);
                TCPclient::Response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping> advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress> progress_advancements);
                TCPclient::Response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties);

                TCPclient::Response entityEffect(int32_t entity_id, int32_t effect_id, int8_t amplifier, int32_t duration, int8_t flags, std::optional<ENBT> factor_codec);
                //TODO:
                //TCPclient::Response updateRecipes()

                TCPclient::Response updateTags(bool reset, const list_array<base_objects::packets::tag_mapping>& tag_mappings, const list_array<std::string>& remove_tags);
            }
        }
    }
}

#endif /* SRC_PROTOCOLHELPER_PACKETS_765_RELEASE_PACKETS */
