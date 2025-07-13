#ifndef SRC_API_PACKETS
#define SRC_API_PACKETS
#include <array>
#include <src/base_objects/block.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/data_packs/known_pack.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/entity/animation.hpp>
#include <src/base_objects/entity/event.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/packets/effect_flags.hpp>
#include <src/base_objects/particle_data.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/recipe.hpp>

namespace copper_server {
    namespace base_objects {
        struct SharedClientData;
        class command_manager;
    }

    namespace storage {
        class chunk_data;
    }

    namespace api::packets {
        namespace login {
            base_objects::network::response login(base_objects::SharedClientData& client, int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data);
            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason);
            base_objects::network::response disableCompression(base_objects::SharedClientData& client);
            base_objects::network::response setCompression(base_objects::SharedClientData& client, int32_t threshold);
            base_objects::network::response requestCookie(base_objects::SharedClientData& client, const std::string& key);
            base_objects::network::response loginSuccess(base_objects::SharedClientData& client);
            base_objects::network::response encryptionRequest(base_objects::SharedClientData& client, const std::string& server_id, uint8_t (&verify_token)[4]);
        }

        namespace configuration {
            base_objects::network::response requestCookie(base_objects::SharedClientData& client, const std::string& key);
            base_objects::network::response configuration(base_objects::SharedClientData& client, const std::string& chanel, const list_array<uint8_t>& data);
            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason);
            base_objects::network::response finish(base_objects::SharedClientData& client);
            base_objects::network::response keep_alive(base_objects::SharedClientData& client, int64_t keep_alive_packet);
            base_objects::network::response ping(base_objects::SharedClientData& client, int32_t excepted_pong);
            base_objects::network::response registry_data(base_objects::SharedClientData& client);
            base_objects::network::response resetChat(base_objects::SharedClientData& client);
            base_objects::network::response removeResourcePacks(base_objects::SharedClientData& client);
            base_objects::network::response removeResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id);
            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced);
            base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt);
            base_objects::network::response storeCookie(base_objects::SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload);
            base_objects::network::response transfer(base_objects::SharedClientData& client, const std::string& host, int32_t port);
            base_objects::network::response setFeatureFlags(base_objects::SharedClientData& client, const list_array<std::string>& features);
            base_objects::network::response updateTags(base_objects::SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tags_entries);
            base_objects::network::response knownPacks(base_objects::SharedClientData& client, const list_array<base_objects::data_packs::known_pack>& packs);
            base_objects::network::response custom_report(base_objects::SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values);
            base_objects::network::response server_links(base_objects::SharedClientData& client, const list_array<base_objects::packets::server_link>& links);
        }

        namespace play {
            base_objects::network::response bundleResponse(base_objects::SharedClientData& client, base_objects::network::response&& response);
            base_objects::network::response spawnEntity(base_objects::SharedClientData& client, const base_objects::entity& entity);
            base_objects::network::response entityAnimation(base_objects::SharedClientData& client, const base_objects::entity& entity, base_objects::entity_animation animation);
            base_objects::network::response awardStatistics(base_objects::SharedClientData& client, const list_array<base_objects::packets::statistics>& statistics);
            base_objects::network::response acknowledgeBlockChange(base_objects::SharedClientData& client);
            base_objects::network::response setBlockDestroyStage(base_objects::SharedClientData& client, const base_objects::entity& entity, base_objects::position pos, uint8_t stage);
            base_objects::network::response blockEntityData(base_objects::SharedClientData& client, base_objects::position pos, const base_objects::block& block_type, const enbt::value& data);
            base_objects::network::response blockAction(base_objects::SharedClientData& client, base_objects::position pos, int32_t action_id, int32_t param, const base_objects::block& block_type);
            base_objects::network::response blockUpdate(base_objects::SharedClientData& client, base_objects::position pos, const base_objects::block& block_type);
            base_objects::network::response bossBarAdd(base_objects::SharedClientData& client, const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags);
            base_objects::network::response bossBarRemove(base_objects::SharedClientData& client, const enbt::raw_uuid& id);
            base_objects::network::response bossBarUpdateHealth(base_objects::SharedClientData& client, const enbt::raw_uuid& id, float health);
            base_objects::network::response bossBarUpdateTitle(base_objects::SharedClientData& client, const enbt::raw_uuid& id, const Chat& title);
            base_objects::network::response bossBarUpdateStyle(base_objects::SharedClientData& client, const enbt::raw_uuid& id, int32_t color, int32_t division);
            base_objects::network::response bossBarUpdateFlags(base_objects::SharedClientData& client, const enbt::raw_uuid& id, uint8_t flags);
            base_objects::network::response changeDifficulty(base_objects::SharedClientData& client, uint8_t difficulty, bool locked);
            base_objects::network::response chunkBatchFinished(base_objects::SharedClientData& client, int32_t count);
            base_objects::network::response chunkBatchStart(base_objects::SharedClientData& client);
            base_objects::network::response chunkBiomes(base_objects::SharedClientData& client, const list_array<storage::chunk_data*>& chunk);
            base_objects::network::response clearTitles(base_objects::SharedClientData& client, bool reset);
            base_objects::network::response commandSuggestionsResponse(base_objects::SharedClientData& client, int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions);
            base_objects::network::response commands(base_objects::SharedClientData& client, const base_objects::command_manager& compiled_graph);
            base_objects::network::response closeContainer(base_objects::SharedClientData& client, uint8_t container_id);
            base_objects::network::response setContainerContent(base_objects::SharedClientData& client, uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item);
            base_objects::network::response setContainerProperty(base_objects::SharedClientData& client, uint8_t windows_id, uint16_t property, uint16_t value);
            base_objects::network::response setContainerSlot(base_objects::SharedClientData& client, uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item);
            base_objects::network::response cookieRequest(base_objects::SharedClientData& client, const std::string& key);
            base_objects::network::response setCooldown(base_objects::SharedClientData& client, int32_t item_id, int32_t cooldown);
            base_objects::network::response chatSuggestionsResponse(base_objects::SharedClientData& client, int32_t action, int32_t count, const list_array<std::string>& suggestions);
            base_objects::network::response customPayload(base_objects::SharedClientData& client, const std::string& channel, const list_array<uint8_t>& data);
            base_objects::network::response damageEvent(base_objects::SharedClientData& client, int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<util::VECTOR> xyz);
            base_objects::network::response debugSample(base_objects::SharedClientData& client, const list_array<uint64_t>& sample, int32_t sample_type);
            base_objects::network::response deleteMessageBySignature(base_objects::SharedClientData& client, uint8_t (&signature)[256]);
            base_objects::network::response deleteMessageByID(base_objects::SharedClientData& client, int32_t message_id);
            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason);
            base_objects::network::response disguisedChatMessage(base_objects::SharedClientData& client, const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name);
            base_objects::network::response entityEvent(base_objects::SharedClientData& client, int32_t entity_id, base_objects::entity_event entity_status);
            base_objects::network::response explosion(base_objects::SharedClientData& client, util::VECTOR pos, float strength, const list_array<util::XYZ<int8_t>>& affected_blocks, util::VECTOR player_motion, int32_t block_interaction, int32_t small_explosion_particle_id, const base_objects::particle_data& small_explosion_particle_data, int32_t large_explosion_particle_id, const base_objects::particle_data& large_explosion_particle_data, const std::string& sound_name, std::optional<float> fixed_range);
            base_objects::network::response unloadChunk(base_objects::SharedClientData& client, int32_t x, int32_t z);
            base_objects::network::response gameEvent(base_objects::SharedClientData& client, base_objects::packets::game_event_id event_id, float value);
            base_objects::network::response openHorseWindow(base_objects::SharedClientData& client, uint8_t window_id, int32_t slots, int32_t entity_id);
            base_objects::network::response hurtAnimation(base_objects::SharedClientData& client, int32_t entity_id, float yaw);
            base_objects::network::response initializeWorldBorder(base_objects::SharedClientData& client, double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time);
            base_objects::network::response keepAlive(base_objects::SharedClientData& client, int64_t id);
            base_objects::network::response updateChunkDataWLights(base_objects::SharedClientData& client, const storage::chunk_data& chunk);
            base_objects::network::response worldEvent(base_objects::SharedClientData& client, base_objects::packets::world_event_id event, base_objects::position pos, int32_t data, bool global);
            base_objects::network::response particle(base_objects::SharedClientData& client, int32_t particle_id, bool long_distance, util::VECTOR pos, util::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data);
            base_objects::network::response updateLight(base_objects::SharedClientData& client, const storage::chunk_data& chunk);
            base_objects::network::response joinGame(base_objects::SharedClientData& client, int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, std::optional<base_objects::packets::death_location_data> death_location, int32_t portal_cooldown, int32_t sea_level, bool enforces_secure_chat);
            base_objects::network::response mapData(base_objects::SharedClientData& client, int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t column, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data);
            base_objects::network::response merchantOffers(base_objects::SharedClientData& client, int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade> trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock);
            base_objects::network::response updateEntityPosition(base_objects::SharedClientData& client, int32_t entity_id, util::XYZ<float> pos, bool on_ground);
            base_objects::network::response updateEntityPositionAndRotation(base_objects::SharedClientData& client, int32_t entity_id, util::XYZ<float> pos, util::ANGLE_DEG rot, bool on_ground);
            base_objects::network::response updateEntityRotation(base_objects::SharedClientData& client, int32_t entity_id, util::ANGLE_DEG rot, bool on_ground);
            base_objects::network::response moveVehicle(base_objects::SharedClientData& client, util::VECTOR pos, util::ANGLE_DEG rot);
            base_objects::network::response openBook(base_objects::SharedClientData& client, int32_t hand);
            base_objects::network::response openScreen(base_objects::SharedClientData& client, int32_t window_id, int32_t type, const Chat& title);
            base_objects::network::response openSignEditor(base_objects::SharedClientData& client, base_objects::position pos, bool is_front_text);
            base_objects::network::response ping(base_objects::SharedClientData& client, int32_t id);
            base_objects::network::response pingResponse(base_objects::SharedClientData& client, int32_t id);
            base_objects::network::response placeGhostRecipe(base_objects::SharedClientData& client, int32_t windows_id, const std::string& recipe_id);
            base_objects::network::response playerAbilities(base_objects::SharedClientData& client, uint8_t flags, float flying_speed, float field_of_view);
            base_objects::network::response playerChatMessage(base_objects::SharedClientData& client, enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, std::optional<enbt::value> unsigned_content, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name);
            base_objects::network::response endCombat(base_objects::SharedClientData& client, int32_t duration);
            base_objects::network::response enterCombat(base_objects::SharedClientData& client);
            base_objects::network::response combatDeath(base_objects::SharedClientData& client, int32_t player_id, const Chat& message);
            base_objects::network::response playerInfoRemove(base_objects::SharedClientData& client, const list_array<enbt::raw_uuid>& players);
            base_objects::network::response playerInfoAdd(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_add>& add_players);
            base_objects::network::response playerInfoInitializeChat(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat);
            base_objects::network::response playerInfoUpdateGameMode(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode);
            base_objects::network::response playerInfoUpdateListed(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_listed>& update_listed);
            base_objects::network::response playerInfoUpdateLatency(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_latency>& update_latency);
            base_objects::network::response playerInfoUpdateDisplayName(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name);
            base_objects::network::response lookAt(base_objects::SharedClientData& client, bool from_feet_or_eyes, util::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id);
            base_objects::network::response synchronizePlayerPosition(base_objects::SharedClientData& client, util::VECTOR pos, util::VECTOR velocity, float yaw, float pitch, int flags);
            base_objects::network::response initRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> displayed_recipe_ids, list_array<std::string> had_access_to_recipe_ids);
            base_objects::network::response addRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids);
            base_objects::network::response removeRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids);
            base_objects::network::response removeEntities(base_objects::SharedClientData& client, const list_array<int32_t>& entity_ids);
            base_objects::network::response removeEntityEffect(base_objects::SharedClientData& client, int32_t entity_id, int32_t effect_id);
            base_objects::network::response resetScore(base_objects::SharedClientData& client, const std::string& entity_name, const std::optional<std::string>& objective_name);
            base_objects::network::response removeResourcePacks(base_objects::SharedClientData& client);
            base_objects::network::response removeResourcePack(base_objects::SharedClientData& client, enbt::raw_uuid id);
            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt);
            base_objects::network::response respawn(base_objects::SharedClientData& client, int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata);
            base_objects::network::response setHeadRotation(base_objects::SharedClientData& client, int32_t entity_id, util::ANGLE_DEG head_rotation);
            base_objects::network::response updateSectionBlocks(base_objects::SharedClientData& client, int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks);
            base_objects::network::response setAdvancementsTab(base_objects::SharedClientData& client, const std::optional<std::string>& tab_id);
            base_objects::network::response serverData(base_objects::SharedClientData& client, const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png);
            base_objects::network::response setActionBarText(base_objects::SharedClientData& client, const Chat& text);
            base_objects::network::response setBorderCenter(base_objects::SharedClientData& client, double x, double z);
            base_objects::network::response setBorderLerp(base_objects::SharedClientData& client, double old_diameter, double new_diameter, int64_t speed_ms);
            base_objects::network::response setBorderSize(base_objects::SharedClientData& client, double diameter);
            base_objects::network::response setBorderWarningDelay(base_objects::SharedClientData& client, int32_t warning_delay);
            base_objects::network::response setBorderWarningDistance(base_objects::SharedClientData& client, int32_t warning_distance);
            base_objects::network::response setCamera(base_objects::SharedClientData& client, int32_t entity_id);
            base_objects::network::response setHeldSlot(base_objects::SharedClientData& client, int32_t slot);
            base_objects::network::response setCenterChunk(base_objects::SharedClientData& client, int32_t x, int32_t z);
            base_objects::network::response setRenderDistance(base_objects::SharedClientData& client, int32_t render_distance);
            base_objects::network::response setDefaultSpawnPosition(base_objects::SharedClientData& client, base_objects::position pos, float angle);
            base_objects::network::response displayObjective(base_objects::SharedClientData& client, int32_t position, const std::string& objective_name);
            base_objects::network::response setEntityMetadata(base_objects::SharedClientData& client, int32_t entity_id, const list_array<uint8_t>& metadata);
            base_objects::network::response linkEntities(base_objects::SharedClientData& client, int32_t attached_entity_id, int32_t holder_entity_id);
            base_objects::network::response setEntityMotion(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR velocity);
            base_objects::network::response setEquipment(base_objects::SharedClientData& client, int32_t entity_id, uint8_t slot, const base_objects::slot& item);
            base_objects::network::response setExperience(base_objects::SharedClientData& client, float experience_bar, int32_t level, int32_t total_experience);
            base_objects::network::response setHealth(base_objects::SharedClientData& client, float health, int32_t food, float saturation);
            base_objects::network::response updateObjectivesCreate(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type);
            base_objects::network::response updateObjectivesCreateStyled(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style);
            base_objects::network::response updateObjectivesCreateFixed(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);
            base_objects::network::response updateObjectivesRemove(base_objects::SharedClientData& client, const std::string& objective_name);
            base_objects::network::response updateObjectivesInfo(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type);
            base_objects::network::response updateObjectivesInfoStyled(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style);
            base_objects::network::response updateObjectivesInfoFixed(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);
            base_objects::network::response setPassengers(base_objects::SharedClientData& client, int32_t vehicle_entity_id, const list_array<int32_t>& passengers);
            base_objects::network::response updateTeamCreate(base_objects::SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities);
            base_objects::network::response updateTeamRemove(base_objects::SharedClientData& client, const std::string& team_name);
            base_objects::network::response updateTeamInfo(base_objects::SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix);
            base_objects::network::response updateTeamAddEntities(base_objects::SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities);
            base_objects::network::response updateTeamRemoveEntities(base_objects::SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities);
            base_objects::network::response setScore(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name);
            base_objects::network::response setScoreStyled(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled);
            base_objects::network::response setScoreFixed(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content);
            base_objects::network::response setSimulationDistance(base_objects::SharedClientData& client, int32_t distance);
            base_objects::network::response setSubtitleText(base_objects::SharedClientData& client, const Chat& text);
            base_objects::network::response updateTime(base_objects::SharedClientData& client, int64_t world_age, int64_t time_of_day, bool increase_time);
            base_objects::network::response setTitleText(base_objects::SharedClientData& client, const Chat& text);
            base_objects::network::response setTitleAnimationTimes(base_objects::SharedClientData& client, int32_t fade_in, int32_t stay, int32_t fade_out);
            base_objects::network::response entitySoundEffect(base_objects::SharedClientData& client, uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);
            base_objects::network::response entitySoundEffectCustom(base_objects::SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);
            base_objects::network::response soundEffect(base_objects::SharedClientData& client, uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);
            base_objects::network::response soundEffectCustom(base_objects::SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);
            base_objects::network::response startConfiguration(base_objects::SharedClientData& client);
            base_objects::network::response stopSound(base_objects::SharedClientData& client, uint8_t flags);
            base_objects::network::response stopSoundBySource(base_objects::SharedClientData& client, uint8_t flags, int32_t source);
            base_objects::network::response stopSoundBySound(base_objects::SharedClientData& client, uint8_t flags, const std::string& sound);
            base_objects::network::response stopSoundBySourceAndSound(base_objects::SharedClientData& client, uint8_t flags, int32_t source, const std::string& sound);
            base_objects::network::response storeCookie(base_objects::SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload);
            base_objects::network::response systemChatMessage(base_objects::SharedClientData& client, const Chat& message);
            base_objects::network::response systemChatMessageOverlay(base_objects::SharedClientData& client, const Chat& message);
            base_objects::network::response setTabListHeaderAndFooter(base_objects::SharedClientData& client, const Chat& header, const Chat& footer);
            base_objects::network::response tagQueryResponse(base_objects::SharedClientData& client, int32_t transaction_id, const enbt::value& nbt);
            base_objects::network::response pickupItem(base_objects::SharedClientData& client, int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count);
            base_objects::network::response teleportEntity(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR pos, float yaw, float pitch, bool on_ground);

            base_objects::network::response test_instance_block_status(base_objects::SharedClientData& client, const Chat& status, std::optional<util::VECTOR> size);

            base_objects::network::response setTickingState(base_objects::SharedClientData& client, float tick_rate, bool is_frozen);
            base_objects::network::response stepTick(base_objects::SharedClientData& client, int32_t step_count);
            base_objects::network::response transfer(base_objects::SharedClientData& client, const std::string& host, int32_t port);
            base_objects::network::response updateAdvancements(base_objects::SharedClientData& client, bool reset, const list_array<base_objects::packets::advancements_maping> advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress> progress_advancements);
            base_objects::network::response updateAttributes(base_objects::SharedClientData& client, int32_t entity_id, const list_array<base_objects::packets::attributes>& properties);
            base_objects::network::response entityEffect(base_objects::SharedClientData& client, int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, base_objects::packets::effect_flags flags);
            base_objects::network::response updateRecipes(base_objects::SharedClientData& client, const std::vector<base_objects::recipe>& recipes);
            base_objects::network::response updateTags(base_objects::SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tag_mappings);
            base_objects::network::response projectilePower(base_objects::SharedClientData& client, int32_t entity_id, double power_x, double power_y, double power_z);
            base_objects::network::response custom_report(base_objects::SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values);
            base_objects::network::response server_links(base_objects::SharedClientData& client, const list_array<base_objects::packets::server_link>& links);
        }

        namespace registry {
            class login_functions {
            public:
                virtual ~login_functions() {}

                virtual base_objects::network::response login(int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data) = 0;
                virtual base_objects::network::response kick(const Chat& reason) = 0;
                virtual base_objects::network::response disableCompression() = 0;
                virtual base_objects::network::response setCompression(int32_t threshold) = 0;
                virtual base_objects::network::response requestCookie(const std::string& key) = 0;
                virtual base_objects::network::response loginSuccess(base_objects::SharedClientData& client) = 0;
                virtual base_objects::network::response encryptionRequest(const std::string& server_id, uint8_t (&verify_token)[4]) = 0;
            };

            class configuration_functions {
            public:
                virtual ~configuration_functions() {}

                virtual base_objects::network::response requestCookie(const std::string& key) = 0;
                virtual base_objects::network::response configuration(const std::string& chanel, const list_array<uint8_t>& data) = 0;
                virtual base_objects::network::response kick(const Chat& reason) = 0;
                virtual base_objects::network::response finish() = 0;
                virtual base_objects::network::response keep_alive(int64_t keep_alive_packet) = 0;
                virtual base_objects::network::response ping(int32_t excepted_pong) = 0;
                virtual base_objects::network::response registry_data() = 0;
                virtual base_objects::network::response registry_item(const std::string& registry_id, list_array<std::pair<std::string, enbt::compound>>& entries) = 0;
                virtual base_objects::network::response resetChat() = 0;
                virtual base_objects::network::response removeResourcePacks() = 0;
                virtual base_objects::network::response removeResourcePack(const enbt::raw_uuid& pack_id) = 0;
                virtual base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced) = 0;
                virtual base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt) = 0;
                virtual base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload) = 0;
                virtual base_objects::network::response transfer(const std::string& host, int32_t port) = 0;
                virtual base_objects::network::response setFeatureFlags(const list_array<std::string>& features) = 0;
                virtual base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries) = 0;
                virtual base_objects::network::response knownPacks(const list_array<base_objects::data_packs::known_pack>& packs) = 0;
                virtual base_objects::network::response custom_report(const list_array<std::pair<std::string, std::string>>& values) = 0;
                virtual base_objects::network::response server_links(const list_array<base_objects::packets::server_link>& links) = 0;
            };

            class play_functions {
            public:
                virtual ~play_functions() {}

                virtual base_objects::network::response bundleResponse(base_objects::network::response&& response) = 0;
                virtual base_objects::network::response spawnEntity(const base_objects::entity& entity) = 0;
                virtual base_objects::network::response entityAnimation(const base_objects::entity& entity, base_objects::entity_animation animation) = 0;
                virtual base_objects::network::response awardStatistics(const list_array<base_objects::packets::statistics>& statistics) = 0;
                virtual base_objects::network::response acknowledgeBlockChange(base_objects::SharedClientData& client) = 0;
                virtual base_objects::network::response setBlockDestroyStage(const base_objects::entity& entity, base_objects::position pos, uint8_t stage) = 0;
                virtual base_objects::network::response blockEntityData(base_objects::position pos, const base_objects::block& block_type, const enbt::value& data) = 0;
                virtual base_objects::network::response blockAction(base_objects::position pos, int32_t action_id, int32_t param, const base_objects::block& block_type) = 0;
                virtual base_objects::network::response blockUpdate(base_objects::position pos, const base_objects::block& block_type) = 0;
                virtual base_objects::network::response bossBarAdd(const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags) = 0;
                virtual base_objects::network::response bossBarRemove(const enbt::raw_uuid& id) = 0;
                virtual base_objects::network::response bossBarUpdateHealth(const enbt::raw_uuid& id, float health) = 0;
                virtual base_objects::network::response bossBarUpdateTitle(const enbt::raw_uuid& id, const Chat& title) = 0;
                virtual base_objects::network::response bossBarUpdateStyle(const enbt::raw_uuid& id, int32_t color, int32_t division) = 0;
                virtual base_objects::network::response bossBarUpdateFlags(const enbt::raw_uuid& id, uint8_t flags) = 0;
                virtual base_objects::network::response changeDifficulty(uint8_t difficulty, bool locked) = 0;
                virtual base_objects::network::response chunkBatchFinished(int32_t count) = 0;
                virtual base_objects::network::response chunkBatchStart() = 0;
                virtual base_objects::network::response chunkBiomes(const list_array<storage::chunk_data*>& chunk) = 0;
                virtual base_objects::network::response clearTitles(bool reset) = 0;
                virtual base_objects::network::response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions) = 0;
                virtual base_objects::network::response commands(const base_objects::command_manager& compiled_graph) = 0;
                virtual base_objects::network::response closeContainer(uint8_t container_id) = 0;
                virtual base_objects::network::response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item) = 0;
                virtual base_objects::network::response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value) = 0;
                virtual base_objects::network::response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item) = 0;
                virtual base_objects::network::response cookieRequest(const std::string& key) = 0;
                virtual base_objects::network::response setCooldown(int32_t item_id, int32_t cooldown) = 0;
                virtual base_objects::network::response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions) = 0;
                virtual base_objects::network::response customPayload(const std::string& channel, const list_array<uint8_t>& data) = 0;
                virtual base_objects::network::response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<util::VECTOR> xyz) = 0;
                virtual base_objects::network::response debugSample(const list_array<uint64_t>& sample, int32_t sample_type) = 0;
                virtual base_objects::network::response deleteMessageBySignature(uint8_t (&signature)[256]) = 0;
                virtual base_objects::network::response deleteMessageByID(int32_t message_id) = 0;
                virtual base_objects::network::response kick(const Chat& reason) = 0;
                virtual base_objects::network::response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name) = 0;
                virtual base_objects::network::response entityEvent(int32_t entity_id, base_objects::entity_event entity_status) = 0;
                virtual base_objects::network::response teleportEntityEX(int32_t entity_id, util::VECTOR pos, util::VECTOR velocity, float yaw, float pitch, bool on_ground, base_objects::packets::teleport_flags flags) = 0;
                virtual base_objects::network::response explosion(
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
                ) = 0;
                virtual base_objects::network::response unloadChunk(int32_t x, int32_t z) = 0;
                virtual base_objects::network::response gameEvent(base_objects::packets::game_event_id event_id, float value) = 0;
                virtual base_objects::network::response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id) = 0;
                virtual base_objects::network::response hurtAnimation(int32_t entity_id, float yaw) = 0;
                virtual base_objects::network::response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time) = 0;
                virtual base_objects::network::response keepAlive(int64_t id) = 0;
                virtual base_objects::network::response updateChunkDataWLights(const storage::chunk_data& chunk) = 0;
                virtual base_objects::network::response worldEvent(base_objects::packets::world_event_id event, base_objects::position pos, int32_t data, bool global) = 0;
                virtual base_objects::network::response particle(int32_t particle_id, bool long_distance, util::VECTOR pos, util::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data) = 0;
                virtual base_objects::network::response updateLight(const storage::chunk_data& chunk) = 0;
                virtual base_objects::network::response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, int32_t sea_level, bool enforces_secure_chat) = 0;
                virtual base_objects::network::response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons = {}, uint8_t columns = 0, uint8_t rows = 0, uint8_t x = 0, uint8_t z = 0, const list_array<uint8_t>& data = {}) = 0;
                virtual base_objects::network::response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade>& trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock) = 0;
                virtual base_objects::network::response updateEntityPosition(int32_t entity_id, util::XYZ<float> pos, bool on_ground) = 0;
                virtual base_objects::network::response updateEntityPositionAndRotation(int32_t entity_id, util::XYZ<float> pos, util::ANGLE_DEG rot, bool on_ground) = 0;
                virtual base_objects::network::response moveMinecartAlongTrack(int32_t entity_id, list_array<base_objects::packets::minecart_state>& states) = 0;
                virtual base_objects::network::response updateEntityRotation(int32_t entity_id, util::ANGLE_DEG rot, bool on_ground) = 0;
                virtual base_objects::network::response moveVehicle(util::VECTOR pos, util::ANGLE_DEG rot) = 0;
                virtual base_objects::network::response openBook(int32_t hand) = 0;
                virtual base_objects::network::response openScreen(int32_t window_id, int32_t type, const Chat& title) = 0;
                virtual base_objects::network::response openSignEditor(base_objects::position pos, bool is_front_text) = 0;
                virtual base_objects::network::response ping(int32_t id) = 0;
                virtual base_objects::network::response pingResponse(int32_t id) = 0;
                virtual base_objects::network::response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id) = 0;
                virtual base_objects::network::response playerAbilities(uint8_t flags, float flying_speed, float field_of_view) = 0;
                virtual base_objects::network::response playerChatMessage(enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, const std::optional<enbt::value>& unsigned_content, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name) = 0;
                virtual base_objects::network::response endCombat(int32_t duration) = 0;
                virtual base_objects::network::response enterCombat() = 0;
                virtual base_objects::network::response combatDeath(int32_t player_id, const Chat& message) = 0;
                virtual base_objects::network::response playerInfoRemove(const list_array<enbt::raw_uuid>& players) = 0;
                virtual base_objects::network::response playerInfoAdd(const list_array<base_objects::packets::player_actions_add>& add_players) = 0;
                virtual base_objects::network::response playerInfoInitializeChat(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat) = 0;
                virtual base_objects::network::response playerInfoUpdateGameMode(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode) = 0;
                virtual base_objects::network::response playerInfoUpdateListed(const list_array<base_objects::packets::player_actions_update_listed>& update_listed) = 0;
                virtual base_objects::network::response playerInfoUpdateLatency(const list_array<base_objects::packets::player_actions_update_latency>& update_latency) = 0;
                virtual base_objects::network::response playerInfoUpdateDisplayName(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name) = 0;
                virtual base_objects::network::response lookAt(bool from_feet_or_eyes, util::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id) = 0;
                virtual base_objects::network::response synchronizePlayerPosition(util::VECTOR pos, util::VECTOR velocity, float yaw, float pitch, int flags, int32_t teleport_id) = 0;
                virtual base_objects::network::response synchronizePlayerRotation(float yaw, float pitch) = 0;
                virtual base_objects::network::response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& displayed_recipe_ids, const list_array<std::string>& had_access_to_recipe_ids) = 0;
                virtual base_objects::network::response addRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) = 0;
                virtual base_objects::network::response removeRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids) = 0;
                virtual base_objects::network::response updateRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active) = 0;
                virtual base_objects::network::response RecipeBookAdd(const list_array<base_objects::recipe>& recipes) = 0;
                virtual base_objects::network::response RecipeBookRemove(const list_array<uint32_t>& recipe_ids) = 0;
                virtual base_objects::network::response RecipeBookSettings(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active) = 0;
                virtual base_objects::network::response removeEntities(const list_array<int32_t>& entity_ids) = 0;
                virtual base_objects::network::response removeEntityEffect(int32_t entity_id, int32_t effect_id) = 0;
                virtual base_objects::network::response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name) = 0;
                virtual base_objects::network::response removeResourcePacks() = 0;
                virtual base_objects::network::response removeResourcePack(enbt::raw_uuid id) = 0;
                virtual base_objects::network::response addResourcePack(enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt) = 0;
                virtual base_objects::network::response respawn(int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata) = 0;
                virtual base_objects::network::response setHeadRotation(int32_t entity_id, util::ANGLE_DEG head_rotation) = 0;
                virtual base_objects::network::response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks) = 0;
                virtual base_objects::network::response setAdvancementsTab(const std::optional<std::string>& tab_id) = 0;
                virtual base_objects::network::response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png) = 0;
                virtual base_objects::network::response setActionBarText(const Chat& text) = 0;
                virtual base_objects::network::response setBorderCenter(double x, double z) = 0;
                virtual base_objects::network::response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms) = 0;
                virtual base_objects::network::response setBorderSize(double diameter) = 0;
                virtual base_objects::network::response setBorderWarningDelay(int32_t warning_delay) = 0;
                virtual base_objects::network::response setBorderWarningDistance(int32_t warning_distance) = 0;
                virtual base_objects::network::response setCamera(int32_t entity_id) = 0;
                virtual base_objects::network::response setHeldSlot(int32_t slot) = 0;
                virtual base_objects::network::response setCenterChunk(int32_t x, int32_t z) = 0;
                virtual base_objects::network::response setRenderDistance(int32_t render_distance) = 0;
                virtual base_objects::network::response setCursorItem(const base_objects::slot& item) = 0;
                virtual base_objects::network::response setDefaultSpawnPosition(base_objects::position pos, float angle) = 0;
                virtual base_objects::network::response displayObjective(int32_t position, const std::string& objective_name) = 0;
                virtual base_objects::network::response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata) = 0;
                virtual base_objects::network::response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id) = 0;
                virtual base_objects::network::response setEntityMotion(int32_t entity_id, util::VECTOR velocity) = 0;
                virtual base_objects::network::response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::slot& item) = 0;
                virtual base_objects::network::response setExperience(float experience_bar, int32_t level, int32_t total_experience) = 0;
                virtual base_objects::network::response setHealth(float health, int32_t food, float saturation) = 0;
                virtual base_objects::network::response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type) = 0;
                virtual base_objects::network::response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) = 0;
                virtual base_objects::network::response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) = 0;
                virtual base_objects::network::response updateObjectivesRemove(const std::string& objective_name) = 0;
                virtual base_objects::network::response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type) = 0;
                virtual base_objects::network::response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) = 0;
                virtual base_objects::network::response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) = 0;
                virtual base_objects::network::response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers) = 0;
                virtual base_objects::network::response setPlayerInventory(int32_t slot, const base_objects::slot& item) = 0;
                virtual base_objects::network::response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities) = 0;
                virtual base_objects::network::response updateTeamRemove(const std::string& team_name) = 0;
                virtual base_objects::network::response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix) = 0;
                virtual base_objects::network::response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities) = 0;
                virtual base_objects::network::response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities) = 0;
                virtual base_objects::network::response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name) = 0;
                virtual base_objects::network::response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled) = 0;
                virtual base_objects::network::response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content) = 0;
                virtual base_objects::network::response setSimulationDistance(int32_t distance) = 0;
                virtual base_objects::network::response setSubtitleText(const Chat& text) = 0;
                virtual base_objects::network::response updateTime(int64_t world_age, int64_t time_of_day, bool increase_time) = 0;
                virtual base_objects::network::response setTitleText(const Chat& text) = 0;
                virtual base_objects::network::response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out) = 0;
                virtual base_objects::network::response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) = 0;
                virtual base_objects::network::response entitySoundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) = 0;
                virtual base_objects::network::response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) = 0;
                virtual base_objects::network::response soundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) = 0;
                virtual base_objects::network::response startConfiguration() = 0;
                virtual base_objects::network::response stopSound(uint8_t flags) = 0;
                virtual base_objects::network::response stopSoundBySource(uint8_t flags, int32_t source) = 0;
                virtual base_objects::network::response stopSoundBySound(uint8_t flags, const std::string& sound) = 0;
                virtual base_objects::network::response stopSoundBySourceAndSound(uint8_t flags, int32_t source, const std::string& sound) = 0;
                virtual base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload) = 0;
                virtual base_objects::network::response systemChatMessage(const Chat& message) = 0;
                virtual base_objects::network::response systemChatMessageOverlay(const Chat& message) = 0;
                virtual base_objects::network::response setTabListHeaderAndFooter(const Chat& header, const Chat& footer) = 0;
                virtual base_objects::network::response tagQueryResponse(int32_t transaction_id, const enbt::value& nbt) = 0;
                virtual base_objects::network::response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count) = 0;
                virtual base_objects::network::response teleportEntity(int32_t entity_id, util::VECTOR pos, float yaw, float pitch, bool on_ground) = 0;
                virtual base_objects::network::response teleportEntityVelocity(int32_t entity_id, util::VECTOR pos, util::VECTOR velocity, float yaw, float pitch, bool on_ground) = 0;
                virtual base_objects::network::response test_instance_block_status(const Chat& status, std::optional<util::VECTOR> size) = 0;
                virtual base_objects::network::response setTickingState(float tick_rate, bool is_frozen) = 0;
                virtual base_objects::network::response stepTick(int32_t step_count) = 0;
                virtual base_objects::network::response transfer(const std::string& host, int32_t port) = 0;
                virtual base_objects::network::response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping>& advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress>& progress_advancements) = 0;
                virtual base_objects::network::response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) = 0;
                virtual base_objects::network::response entityEffect(int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, base_objects::packets::effect_flags flags) = 0;
                virtual base_objects::network::response updateRecipes(const std::vector<base_objects::recipe>& recipes) = 0;
                virtual base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings) = 0;
                virtual base_objects::network::response projectilePower(int32_t entity_id, double power_x, double power_y, double power_z) = 0;
                virtual base_objects::network::response custom_report(const list_array<std::pair<std::string, std::string>>& values) = 0;
                virtual base_objects::network::response server_links(const list_array<base_objects::packets::server_link>& links) = 0;
            };

            struct protocol_registry_entry {
                login_functions* login;
                configuration_functions* configuration;
                play_functions* play;
                //called on remove, if nullptr then just called `delete on everything`
                void (*functions_destructor)(protocol_registry_entry&) = nullptr;
            };

            bool contains_protocol(int32_t number);
            size_t protocols_count();
            void register_protocol(int32_t number, const protocol_registry_entry&);
            void erase_protocol(int32_t number);
        }
    }
}

#endif /* SRC_API_PACKETS */
