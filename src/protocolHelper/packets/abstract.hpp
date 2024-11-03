

#include "../../base_objects/block.hpp"
#include "../../base_objects/chat.hpp"
#include "../../base_objects/chunk.hpp"
#include "../../base_objects/entity.hpp"
#include "../../base_objects/packets.hpp"
#include "../../base_objects/particle_data.hpp"
#include "../../base_objects/position.hpp"
#include "../../base_objects/recipe.hpp"
#include <array>

namespace crafted_craft {
    namespace packets {
        namespace login {
            Response login(SharedClientData& client, int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data);

            Response kick(SharedClientData& client, const Chat& reason);

            Response disableCompression(SharedClientData& client);

            Response setCompression(SharedClientData& client, int32_t threshold);

            Response requestCookie(SharedClientData& client, const std::string& key);

            Response loginSuccess(SharedClientData& client);

            Response encryptionRequest(SharedClientData& client, const std::string& server_id, uint8_t (&verify_token)[4]);
        }

        namespace configuration {
            Response requestCookie(SharedClientData& client, const std::string& key);

            Response configuration(SharedClientData& client, const std::string& chanel, const list_array<uint8_t>& data);

            Response kick(SharedClientData& client, const Chat& reason);

            Response finish(SharedClientData& client);

            Response keep_alive(SharedClientData& client, int64_t keep_alive_packet);

            Response ping(SharedClientData& client, int32_t excepted_pong);

            Response registry_data(SharedClientData& client);

            Response resetChat(SharedClientData& client);

            Response removeResourcePacks(SharedClientData& client);

            Response removeResourcePack(SharedClientData& client, const enbt::raw_uuid& pack_id);

            Response addResourcePack(SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced);

            Response addResourcePackPrompted(SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt);

            Response storeCookie(SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload);

            Response transfer(SharedClientData& client, const std::string& host, int32_t port);

            Response setFeatureFlags(SharedClientData& client, const list_array<std::string>& features);

            Response updateTags(SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tags_entries);

            Response knownPacks(SharedClientData& client, const list_array<base_objects::packets::known_pack>& packs);

            Response custom_report(SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values);

            Response server_links(SharedClientData& client, const list_array<base_objects::packets::server_link>& links);
        }

        namespace play {
            Response bundleResponse(SharedClientData& client, Response&& response);

            Response spawnEntity(SharedClientData& client, const base_objects::entity& entity, uint16_t protocol);

            Response spawnExperienceOrb(SharedClientData& client, const base_objects::entity& entity, int16_t count);

            Response entityAnimation(SharedClientData& client, const base_objects::entity& entity, uint8_t animation);

            Response awardStatistics(SharedClientData& client, const list_array<base_objects::packets::statistics>& statistics);

            Response acknowledgeBlockChange(SharedClientData& client);

            Response setBlockDestroyStage(SharedClientData& client, const base_objects::entity& entity, Position block, uint8_t stage);

            Response blockEntityData(SharedClientData& client, Position block, int32_t type, const enbt::value& data);

            Response blockAction(SharedClientData& client, Position block, int32_t action_id, int32_t param, int32_t block_type);

            Response blockUpdate(SharedClientData& client, Position block, int32_t block_type);

            Response bossBarAdd(SharedClientData& client, const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags);

            Response bossBarRemove(SharedClientData& client, const enbt::raw_uuid& id);

            Response bossBarUpdateHealth(SharedClientData& client, const enbt::raw_uuid& id, float health);

            Response bossBarUpdateTitle(SharedClientData& client, const enbt::raw_uuid& id, const Chat& title);

            Response bossBarUpdateStyle(SharedClientData& client, const enbt::raw_uuid& id, int32_t color, int32_t division);

            Response bossBarUpdateFlags(SharedClientData& client, const enbt::raw_uuid& id, uint8_t flags);

            Response changeDifficulty(SharedClientData& client, uint8_t difficulty, bool locked);

            Response chunkBatchFinished(SharedClientData& client, int32_t count);

            Response chunkBatchStart(SharedClientData& client);

            Response chunkBiomes(SharedClientData& client, list_array<base_objects::chunk::chunk_biomes>& chunk);

            Response clearTitles(SharedClientData& client, bool reset);

            Response commandSuggestionsResponse(SharedClientData& client, int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions);

            Response commands(SharedClientData& client, int32_t root_id, const list_array<base_objects::packets::command_node>& nodes);

            Response closeContainer(SharedClientData& client, uint8_t container_id);

            Response setContainerContent(SharedClientData& client, uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item);

            Response setContainerProperty(SharedClientData& client, uint8_t windows_id, uint16_t property, uint16_t value);

            Response setContainerSlot(SharedClientData& client, uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item);

            Response cookieRequest(SharedClientData& client, const std::string& key);

            Response setCooldown(SharedClientData& client, int32_t item_id, int32_t cooldown);

            Response chatSuggestionsResponse(SharedClientData& client, int32_t action, int32_t count, const list_array<std::string>& suggestions);

            Response customPayload(SharedClientData& client, const std::string& channel, const list_array<uint8_t>& data);

            Response damageEvent(SharedClientData& client, int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<calc::VECTOR> xyz);

            Response debugSample(SharedClientData& client, const list_array<uint64_t>& sample, int32_t sample_type);

            Response deleteMessageBySignature(SharedClientData& client, uint8_t (&signature)[256]);

            Response deleteMessageByID(SharedClientData& client, int32_t message_id);

            Response kick(SharedClientData& client, const Chat& reason);

            Response disguisedChatMessage(SharedClientData& client, const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name);

            Response entityEvent(SharedClientData& client, int32_t entity_id, uint8_t entity_status);

            Response explosion(SharedClientData& client, calc::VECTOR pos, float strength, const list_array<calc::XYZ<int8_t>>& affected_blocks, calc::VECTOR player_motion, int32_t block_interaction, int32_t small_explosion_particle_id, const base_objects::particle_data& small_explosion_particle_data, int32_t large_explosion_particle_id, const base_objects::particle_data& large_explosion_particle_data, const std::string& sound_name, std::optional<float> fixed_range);

            Response unloadChunk(SharedClientData& client, int32_t x, int32_t z);

            Response gameEvent(SharedClientData& client, uint8_t event_id, float value);

            Response openHorseWindow(SharedClientData& client, uint8_t window_id, int32_t slots, int32_t entity_id);

            Response hurtAnimation(SharedClientData& client, int32_t entity_id, float yaw);

            Response initializeWorldBorder(SharedClientData& client, double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time);

            Response keepAlive(SharedClientData& client, int64_t id);

            Response updateChunkDataWLights(SharedClientData& client, int32_t chunk_x, int32_t chunk_z, const NBT& heightmaps, const std::vector<uint8_t> data, const bit_list_array<>& sky_light_mask, const bit_list_array<>& block_light_mask, const bit_list_array<>& empty_skylight_mask, const bit_list_array<>& empty_block_light_mask, const list_array<std::vector<uint8_t>> sky_light_arrays, const list_array<std::vector<uint8_t>> block_light_arrays);

            Response worldEvent(SharedClientData& client, int32_t event, Position pos, int32_t data, bool global);

            Response particle(SharedClientData& client, int32_t particle_id, bool long_distance, calc::VECTOR pos, calc::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data);

            Response updateLight(SharedClientData& client, int32_t chunk_x, int32_t chunk_z, const bit_list_array<>& sky_light_mask, const bit_list_array<>& block_light_mask, const bit_list_array<>& empty_skylight_mask, const bit_list_array<>& empty_block_light_mask, const list_array<std::vector<uint8_t>> sky_light_arrays, const list_array<std::vector<uint8_t>> block_light_arrays);

            Response joinGame(SharedClientData& client, int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, std::optional<base_objects::packets::death_location_data> death_location, int32_t portal_cooldown, bool enforces_secure_chat);

            Response mapData(SharedClientData& client, int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t column, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data);

            Response merchantOffers(SharedClientData& client, int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade> trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock);

            Response updateEntityPosition(SharedClientData& client, int32_t entity_id, calc::XYZ<float> pos, bool on_ground);

            Response updateEntityPositionAndRotation(SharedClientData& client, int32_t entity_id, calc::XYZ<float> pos, calc::VECTOR rot, bool on_ground);

            Response updateEntityRotation(SharedClientData& client, int32_t entity_id, calc::VECTOR rot, bool on_ground);

            Response moveVehicle(SharedClientData& client, calc::VECTOR pos, calc::VECTOR rot);

            Response openBook(SharedClientData& client, int32_t hand);

            Response openScreen(SharedClientData& client, int32_t window_id, int32_t type, const Chat& title);

            Response openSignEditor(SharedClientData& client, Position pos, bool is_front_text);

            Response ping(SharedClientData& client, int32_t id);

            Response pingResponse(SharedClientData& client, int32_t id);

            Response placeGhostRecipe(SharedClientData& client, int32_t windows_id, const std::string& recipe_id);

            Response playerAbilities(SharedClientData& client, uint8_t flags, float flying_speed, float field_of_view);

            Response playerChatMessage(SharedClientData& client, enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, std::optional<enbt::value> __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name);

            Response endCombat(SharedClientData& client, int32_t duration);

            Response enterCombat(SharedClientData& client);

            Response combatDeath(SharedClientData& client, int32_t player_id, const Chat& message);

            Response playerInfoRemove(SharedClientData& client, const list_array<enbt::raw_uuid>& players);

            Response playerInfoAdd(SharedClientData& client, const list_array<base_objects::packets::player_actions_add>& add_players);

            Response playerInfoInitializeChat(SharedClientData& client, const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat);

            Response playerInfoUpdateGameMode(SharedClientData& client, const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode);

            Response playerInfoUpdateListed(SharedClientData& client, const list_array<base_objects::packets::player_actions_update_listed>& update_listed);

            Response playerInfoUpdateLatency(SharedClientData& client, const list_array<base_objects::packets::player_actions_update_latency>& update_latency);

            Response playerInfoUpdateDisplayName(SharedClientData& client, const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name);

            Response lookAt(SharedClientData& client, bool from_feet_or_eyes, calc::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id);

            Response synchronizePlayerPosition(SharedClientData& client, calc::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id);

            Response initRecipeBook(SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> displayed_recipe_ids, list_array<std::string> had_access_to_recipe_ids);

            Response addRecipeBook(SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids);

            Response removeRecipeBook(SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids);

            Response removeEntities(SharedClientData& client, const list_array<int32_t>& entity_ids);

            Response removeEntityEffect(SharedClientData& client, int32_t entity_id, int32_t effect_id);

            Response resetScore(SharedClientData& client, const std::string& entity_name, const std::optional<std::string>& objective_name);

            Response removeResourcePacks(SharedClientData& client);

            Response removeResourcePack(SharedClientData& client, enbt::raw_uuid id);

            Response addResourcePack(SharedClientData& client, enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt);

            Response respawn(SharedClientData& client, int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata);

            Response setHeadRotation(SharedClientData& client, int32_t entity_id, calc::VECTOR head_rotation);

            Response updateSectionBlocks(SharedClientData& client, int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks);

            Response setAdvancementsTab(SharedClientData& client, const std::optional<std::string>& tab_id);

            Response serverData(SharedClientData& client, const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool __ignored);

            Response setActionBarText(SharedClientData& client, const Chat& text);

            Response setBorderCenter(SharedClientData& client, double x, double z);

            Response setBorderLerp(SharedClientData& client, double old_diameter, double new_diameter, int64_t speed_ms);

            Response setBorderSize(SharedClientData& client, double diameter);

            Response setBorderWarningDelay(SharedClientData& client, int32_t warning_delay);

            Response setBorderWarningDistance(SharedClientData& client, int32_t warning_distance);

            Response setCamera(SharedClientData& client, int32_t entity_id);

            Response setHeldItem(SharedClientData& client, uint8_t slot);

            Response setCenterChunk(SharedClientData& client, int32_t x, int32_t z);

            Response setRenderDistance(SharedClientData& client, int32_t render_distance);

            Response setDefaultSpawnPosition(SharedClientData& client, Position pos, float angle);

            Response displayObjective(SharedClientData& client, int32_t position, const std::string& objective_name);

            Response setEntityMetadata(SharedClientData& client, int32_t entity_id, const list_array<uint8_t>& metadata);

            Response linkEntities(SharedClientData& client, int32_t attached_entity_id, int32_t holder_entity_id);

            Response setEntityVelocity(SharedClientData& client, int32_t entity_id, calc::VECTOR velocity);

            Response setEquipment(SharedClientData& client, int32_t entity_id, uint8_t slot, const base_objects::slot& item);

            Response setExperience(SharedClientData& client, float experience_bar, int32_t level, int32_t total_experience);

            Response setHealth(SharedClientData& client, float health, int32_t food, float saturation);

            Response updateObjectivesCreate(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type);

            Response updateObjectivesCreateStyled(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style);

            Response updateObjectivesCreateFixed(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);

            Response updateObjectivesRemove(SharedClientData& client, const std::string& objective_name);

            Response updateObjectivesInfo(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type);

            Response updateObjectivesInfoStyled(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style);

            Response updateObjectivesInfoFixed(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);

            Response setPassengers(SharedClientData& client, int32_t vehicle_entity_id, const list_array<int32_t>& passengers);

            Response updateTeamCreate(SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities);

            Response updateTeamRemove(SharedClientData& client, const std::string& team_name);

            Response updateTeamInfo(SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix);

            Response updateTeamAddEntities(SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities);

            Response updateTeamRemoveEntities(SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities);

            Response setScore(SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name);

            Response setScoreStyled(SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled);

            Response setScoreFixed(SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content);

            Response setSimulationDistance(SharedClientData& client, int32_t distance);

            Response setSubtitleText(SharedClientData& client, const Chat& text);

            Response updateTime(SharedClientData& client, int64_t world_age, int64_t time_of_day);

            Response setTitleText(SharedClientData& client, const Chat& text);

            Response setTitleAnimationTimes(SharedClientData& client, int32_t fade_in, int32_t stay, int32_t fade_out);

            Response entitySoundEffect(SharedClientData& client, uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);

            Response entitySoundEffectCustom(SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);

            Response soundEffect(SharedClientData& client, uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);

            Response soundEffectCustom(SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);

            Response startConfiguration(SharedClientData& client);

            Response stopSound(SharedClientData& client, uint8_t flags);

            Response stopSoundBySource(SharedClientData& client, uint8_t flags, int32_t source);

            Response stopSoundBySound(SharedClientData& client, uint8_t flags, const std::string& sound);

            Response stopSoundBySourceAndSound(SharedClientData& client, uint8_t flags, int32_t source, const std::string& sound);

            Response storeCookie(SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload);

            Response systemChatMessage(SharedClientData& client, const Chat& message);

            Response systemChatMessageOverlay(SharedClientData& client, const Chat& message);

            Response setTabListHeaderAndFooter(SharedClientData& client, const Chat& header, const Chat& footer);

            Response tagQueryResponse(SharedClientData& client, int32_t transaction_id, const enbt::value& nbt);

            Response pickupItem(SharedClientData& client, int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count);

            Response teleportEntity(SharedClientData& client, int32_t entity_id, calc::VECTOR pos, float yaw, float pitch, bool on_ground);

            Response setTickingState(SharedClientData& client, float tick_rate, bool is_frozen);

            Response stepTick(SharedClientData& client, int32_t step_count);

            Response transfer(SharedClientData& client, const std::string& host, int32_t port);

            Response updateAdvancements(SharedClientData& client, bool reset, const list_array<base_objects::packets::advancements_maping> advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress> progress_advancements);

            Response updateAttributes(SharedClientData& client, int32_t entity_id, const list_array<base_objects::packets::attributes>& properties);

            Response entityEffect(SharedClientData& client, int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, int8_t flags);

            Response updateRecipes(SharedClientData& client, const std::vector<base_objects::recipe>& recipes);

            Response updateTags(SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tag_mappings);

            Response projectilePower(SharedClientData& client, int32_t entity_id, double power_x, double power_y, double power_z);

            Response custom_report(SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values);

            Response server_links(SharedClientData& client, const list_array<base_objects::packets::server_link>& links);
        }
    }
}