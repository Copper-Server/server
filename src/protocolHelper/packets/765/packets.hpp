#ifndef SRC_PROTOCOLHELPER_PACKETS_765_PACKETS
#define SRC_PROTOCOLHELPER_PACKETS_765_PACKETS
#include "../../../ClientHandleHelper.hpp"
#include "../../../base_objects/chat.hpp"
#include "../../../base_objects/chunk.hpp"
#include "../../../base_objects/entity.hpp"
#include "../../../base_objects/packets.hpp"
#include "../../../base_objects/particle_data.hpp"
#include "../../../base_objects/position.hpp"
#include "../../../base_objects/recipe.hpp"

//packets for 1.20.4, protocol 765
namespace crafted_craft {
    namespace packets {
        namespace release_765 {
            namespace login {
                Response login(int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data);

                Response kick(const Chat& reason);
                Response disableCompression();
                Response setCompression(int32_t threshold);

                Response loginSuccess(SharedClientData& client);
                Response encryptionRequest(const std::string& server_id, uint8_t (&verify_token)[4]);
            }

            namespace configuration {
                Response configuration(const std::string& chanel, const list_array<uint8_t>& data);

                Response kick(const Chat& reason);

                Response finish();

                Response keep_alive(int64_t keep_alive_packet);

                Response ping(int32_t excepted_pong);

                Response registry_data();

                Response removeResourcePacks();

                Response removeResourcePack(const ENBT::UUID& pack_id);

                Response addResourcePack(SharedClientData& client, const ENBT::UUID& pack_id, const std::string& url, const std::string& hash, bool forced);

                Response addResourcePackPrompted(SharedClientData& client, const ENBT::UUID& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt);

                Response setFeatureFlags(const list_array<std::string>& features);

                Response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries);
            }

            namespace play {
                Response bundleResponse(Response&& response);

                Response spawnEntity(const base_objects::entity& entity, uint16_t protocol = UINT16_MAX);
                Response spawnExperienceOrb(const base_objects::entity& entity, int16_t count);
                Response entityAnimation(const base_objects::entity& entity, uint8_t animation);
                Response awardStatistics(const list_array<base_objects::packets::statistics>& statistics);
                Response acknowledgeBlockChange(SharedClientData& client);

                Response setBlockDestroyStage(const base_objects::entity& entity, Position block, uint8_t stage);

                Response blockEntityData(Position block, int32_t type, const ENBT& data);
                //block_type is from "minecraft:block" registry, not a block state.
                Response blockAction(Position block, int32_t action_id, int32_t param, int32_t block_type);
                //block_type is from "minecraft:block" registry, not a block state.
                Response blockUpdate(Position block, int32_t block_type);
                Response bossBarAdd(const ENBT::UUID& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags);
                Response bossBarRemove(const ENBT::UUID& id);
                Response bossBarUpdateHealth(const ENBT::UUID& id, float health);
                Response bossBarUpdateTitle(const ENBT::UUID& id, const Chat& title);
                Response bossBarUpdateStyle(const ENBT::UUID& id, int32_t color, int32_t division);
                Response bossBarUpdateFlags(const ENBT::UUID& id, uint8_t flags);
                Response changeDifficulty(uint8_t difficulty, bool locked);
                Response chunkBatchFinished(int32_t count);
                Response chunkBatchStart();

                Response chunkBiomes(list_array<base_objects::chunk::chunk_biomes>& chunk);
                Response clearTitles(bool reset);

                Response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions);
                Response commands(int32_t root_id, const list_array<base_objects::packets::command_node>& nodes);
                Response closeContainer(uint8_t container_id);
                Response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item);

                Response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value);

                Response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item);

                Response setCooldown(int32_t item_id, int32_t cooldown);

                //UNUSED by Notchian client
                Response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions);

                Response customPayload(const std::string& channel, const list_array<uint8_t>& data);

                Response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<calc::VECTOR> xyz);

                Response deleteMessageBySignature(uint8_t (&signature)[256]);

                Response deleteMessageByID(int32_t message_id);

                Response kick(const Chat& reason);

                Response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name);

                Response entityEvent(int32_t entity_id, uint8_t entity_status);


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
                );

                Response unloadChunk(int32_t x, int32_t z);
                Response gameEvent(uint8_t event_id, float value);
                Response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id);

                Response hurtAnimation(int32_t entity_id, float yaw);

                Response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time);
                //internal use
                Response keepAlive(int64_t id);

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
                );

                Response worldEvent(int32_t event, Position pos, int32_t data, bool global);
                Response particle(int32_t particle_id, bool long_distance, calc::VECTOR pos, calc::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data);

                Response updateLight(
                    int32_t chunk_x,
                    int32_t chunk_z,
                    const bit_list_array<>& sky_light_mask,
                    const bit_list_array<>& block_light_mask,
                    const bit_list_array<>& empty_skylight_mask,
                    const bit_list_array<>& empty_block_light_mask,
                    const list_array<std::vector<uint8_t>>& sky_light_arrays,
                    const list_array<std::vector<uint8_t>>& block_light_arrays
                );

                Response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool __unused);
                Response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons = {}, uint8_t columns = 0, uint8_t rows = 0, uint8_t x = 0, uint8_t z = 0, const list_array<uint8_t>& data = {});
                Response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade>& trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock);
                Response updateEntityPosition(int32_t entity_id, calc::XYZ<float> pos, bool on_ground);
                Response updateEntityPositionAndRotation(int32_t entity_id, calc::XYZ<float> pos, calc::VECTOR rot, bool on_ground);
                Response updateEntityRotation(int32_t entity_id, calc::VECTOR rot, bool on_ground);
                Response moveVehicle(calc::VECTOR pos, calc::VECTOR rot);
                Response openBook(int32_t hand);
                Response openScreen(int32_t window_id, int32_t type, const Chat& title);
                Response openSignEditor(Position pos, bool is_front_text);
                Response ping(int32_t id);
                Response pingResponse(int32_t id);
                Response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id);
                Response playerAbilities(uint8_t flags, float flying_speed, float field_of_view);
                Response playerChatMessage(ENBT::UUID sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, const std::optional<ENBT>& __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name);
                //UNUSED by Notchian client
                Response endCombat(int32_t duration);
                //UNUSED by Notchian client
                Response enterCombat();
                Response combatDeath(int32_t player_id, const Chat& message);
                Response playerInfoRemove(const list_array<ENBT::UUID>& players);
                Response playerInfoAdd(const list_array<base_objects::packets::player_actions_add>& add_players);
                Response playerInfoInitializeChat(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat);
                Response playerInfoUpdateGameMode(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode);
                Response playerInfoUpdateListed(const list_array<base_objects::packets::player_actions_update_listed>& update_listed);
                Response playerInfoUpdateLatency(const list_array<base_objects::packets::player_actions_update_latency>& update_latency);
                Response playerInfoUpdateDisplayName(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name);
                Response lookAt(bool from_feet_or_eyes, calc::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id);
                Response synchronizePlayerPosition(calc::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id);
                Response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& displayed_recipe_ids, const list_array<std::string>& had_access_to_recipe_ids);
                Response addRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids);
                Response removeRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids);
                Response removeEntities(const list_array<int32_t>& entity_ids);
                Response removeEntityEffect(int32_t entity_id, int32_t effect_id);
                Response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name);
                Response removeResourcePacks();
                Response removeResourcePack(ENBT::UUID id);
                Response addResourcePack(ENBT::UUID id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt);
                Response respawn(int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata);

                Response setHeadRotation(int32_t entity_id, calc::VECTOR head_rotation);

                Response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks);

                Response setAdvancementsTab(const std::optional<std::string>& tab_id);
                Response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool secure_chat);
                Response setActionBarText(const Chat& text);
                Response setBorderCenter(double x, double z);
                Response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms);
                Response setBorderSize(double diameter);
                Response setBorderWarningDelay(int32_t warning_delay);
                Response setBorderWarningDistance(int32_t warning_distance);
                Response setCamera(int32_t entity_id);
                Response setHeldItem(uint8_t slot);
                Response setCenterChunk(int32_t x, int32_t z);
                Response setRenderDistance(int32_t render_distance);
                Response setDefaultSpawnPosition(Position pos, float angle);
                Response displayObjective(int32_t position, const std::string& objective_name);
                Response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata);
                Response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id);
                Response setEntityVelocity(int32_t entity_id, calc::VECTOR velocity);

                Response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::slot& item);

                Response setExperience(float experience_bar, int32_t level, int32_t total_experience);

                Response setHealth(float health, int32_t food, float saturation);

                Response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type);
                Response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const ENBT& style);
                Response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);
                Response updateObjectivesRemove(const std::string& objective_name);
                Response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type);
                Response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const ENBT& style);
                Response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);

                Response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers);


                Response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities);
                Response updateTeamRemove(const std::string& team_name);
                Response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix);
                Response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities);
                Response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities);


                Response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name);
                Response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const ENBT& styled);
                Response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content);

                Response setSimulationDistance(int32_t distance);
                Response setSubtitleText(const Chat& text);
                Response updateTime(int64_t world_age, int64_t time_of_day);
                Response setTitleText(const Chat& text);
                Response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out);


                Response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);
                Response entitySoundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);

                Response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);
                Response soundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);


                Response startConfiguration();
                Response stopSound(uint8_t flags);
                Response stopSoundBySource(uint8_t flags, int32_t source);
                Response stopSoundBySound(uint8_t flags, const std::string& sound);
                Response stopSoundBySourceAndSound(uint8_t flags, int32_t source, const std::string& sound);
                Response systemChatMessage(const Chat& message);
                Response systemChatMessageOverlay(const Chat& message);
                Response setTabListHeaderAndFooter(const Chat& header, const Chat& footer);
                Response tagQueryResponse(int32_t transaction_id, const ENBT& nbt);
                Response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count);
                Response teleportEntity(int32_t entity_id, calc::VECTOR pos, float yaw, float pitch, bool on_ground);
                Response setTickingState(float tick_rate, bool is_frozen);
                Response stepTick(int32_t step_count);
                Response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping>& advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress>& progress_advancements);
                Response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties);

                Response entityEffect(int32_t entity_id, int32_t effect_id, int8_t amplifier, int32_t duration, int8_t flags, const std::optional<ENBT>& factor_codec);
                Response entityEffect(int32_t entity_id, int32_t effect_id, int8_t amplifier, int32_t duration, int8_t flags);

                Response updateRecipes(const std::vector<base_objects::recipe>& recipes);

                Response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings);
            }
        }
    }
}

#endif /* SRC_PROTOCOLHELPER_PACKETS_765_PACKETS */
