#ifndef SRC_PROTOCOLHELPER_PACKETS_767_PACKETS
#define SRC_PROTOCOLHELPER_PACKETS_767_PACKETS
#include <src/base_objects/block.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/chunk.hpp>
#include <src/base_objects/data_packs/known_pack.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/particle_data.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/recipe.hpp>

//packets for 1.21.0 - 1.21.1, protocol 767
//changes between 766:
// Now client requires to get packet knownPacks with pack "minecraft:core" with version "1.21" also must be send before registry_data
//
namespace copper_server {
    namespace base_objects {
        struct SharedClientData;
    }

    namespace packets::release_767 {

        namespace login {
            base_objects::network::response login(int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data);

            base_objects::network::response kick(const Chat& reason);
            base_objects::network::response disableCompression();
            base_objects::network::response setCompression(int32_t threshold);
            base_objects::network::response requestCookie(const std::string& key);

            base_objects::network::response loginSuccess(base_objects::SharedClientData& client);
            base_objects::network::response encryptionRequest(const std::string& server_id, uint8_t (&verify_token)[4]);
        }

        namespace configuration {
            base_objects::network::response requestCookie(const std::string& key);
            base_objects::network::response configuration(const std::string& chanel, const list_array<uint8_t>& data);

            base_objects::network::response kick(const Chat& reason);

            base_objects::network::response finish();

            base_objects::network::response keep_alive(int64_t keep_alive_packet);

            base_objects::network::response ping(int32_t excepted_pong);

            base_objects::network::response registry_data();
            base_objects::network::response registry_item(const std::string& registry_id, list_array<std::pair<std::string, enbt::compound>>& entries);

            base_objects::network::response resetChat();

            base_objects::network::response removeResourcePacks();
            base_objects::network::response removeResourcePack(const enbt::raw_uuid& pack_id);
            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced);
            base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt);

            base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload);
            base_objects::network::response transfer(const std::string& host, int32_t port);

            base_objects::network::response setFeatureFlags(const list_array<std::string>& features);

            base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries);

            base_objects::network::response knownPacks(const list_array<base_objects::data_packs::known_pack>& packs);

            //max 32 items
            //  {
            //  title max 128 bytes
            //  desc max 4096 bytes
            //  }
            base_objects::network::response custom_report(const list_array<std::pair<std::string, std::string>>& values);

            base_objects::network::response server_links(const list_array<base_objects::packets::server_link>& links);
        }

        namespace play {
            base_objects::network::response bundleResponse(base_objects::network::response&& response);

            base_objects::network::response spawnEntity(const base_objects::entity& entity, uint16_t protocol = -1);
            base_objects::network::response spawnExperienceOrb(const base_objects::entity& entity, int16_t count);
            base_objects::network::response entityAnimation(const base_objects::entity& entity, uint8_t animation);
            base_objects::network::response awardStatistics(const list_array<base_objects::packets::statistics>& statistics);
            base_objects::network::response acknowledgeBlockChange(base_objects::SharedClientData& client);

            base_objects::network::response setBlockDestroyStage(const base_objects::entity& entity, base_objects::position block, uint8_t stage);

            base_objects::network::response blockEntityData(base_objects::position block, int32_t type, const enbt::value& data);
            //block_type is from "minecraft:block" registry, not a block state.
            base_objects::network::response blockAction(base_objects::position block, int32_t action_id, int32_t param, int32_t block_type);
            //block_type is from "minecraft:block" registry, not a block state.
            base_objects::network::response blockUpdate(base_objects::position block, int32_t block_type);
            base_objects::network::response bossBarAdd(const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags);
            base_objects::network::response bossBarRemove(const enbt::raw_uuid& id);
            base_objects::network::response bossBarUpdateHealth(const enbt::raw_uuid& id, float health);
            base_objects::network::response bossBarUpdateTitle(const enbt::raw_uuid& id, const Chat& title);
            base_objects::network::response bossBarUpdateStyle(const enbt::raw_uuid& id, int32_t color, int32_t division);


            base_objects::network::response bossBarUpdateFlags(const enbt::raw_uuid& id, uint8_t flags);
            base_objects::network::response changeDifficulty(uint8_t difficulty, bool locked);
            base_objects::network::response chunkBatchFinished(int32_t count);
            base_objects::network::response chunkBatchStart();

            base_objects::network::response chunkBiomes(list_array<base_objects::chunk::chunk_biomes>& chunk);
            base_objects::network::response clearTitles(bool reset);

            base_objects::network::response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions);
            base_objects::network::response commands(int32_t root_id, const list_array<base_objects::packets::command_node>& nodes);
            base_objects::network::response closeContainer(uint8_t container_id);
            base_objects::network::response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item);

            base_objects::network::response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value);

            base_objects::network::response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item);

            base_objects::network::response cookieRequest(const std::string& key);


            base_objects::network::response setCooldown(int32_t item_id, int32_t cooldown);

            //UNUSED by Notchian client
            base_objects::network::response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions);

            base_objects::network::response customPayload(const std::string& channel, const list_array<uint8_t>& data);

            base_objects::network::response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<util::VECTOR> xyz);

            base_objects::network::response debugSample(const list_array<uint64_t>& sample, int32_t sample_type);

            base_objects::network::response deleteMessageBySignature(uint8_t (&signature)[256]);

            base_objects::network::response deleteMessageByID(int32_t message_id);

            base_objects::network::response kick(const Chat& reason);

            base_objects::network::response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name);

            base_objects::network::response entityEvent(int32_t entity_id, uint8_t entity_status);


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
            );

            base_objects::network::response unloadChunk(int32_t x, int32_t z);
            base_objects::network::response gameEvent(uint8_t event_id, float value);
            base_objects::network::response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id);

            base_objects::network::response hurtAnimation(int32_t entity_id, float yaw);

            base_objects::network::response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time);
            //internal use
            base_objects::network::response keepAlive(int64_t id);

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
            );

            base_objects::network::response worldEvent(int32_t event, base_objects::position pos, int32_t data, bool global);
            base_objects::network::response particle(int32_t particle_id, bool long_distance, util::VECTOR pos, util::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data);

            base_objects::network::response updateLight(
                int32_t chunk_x,
                int32_t chunk_z,
                const bit_list_array<>& sky_light_mask,
                const bit_list_array<>& block_light_mask,
                const bit_list_array<>& empty_skylight_mask,
                const bit_list_array<>& empty_block_light_mask,
                const list_array<std::vector<uint8_t>>& sky_light_arrays,
                const list_array<std::vector<uint8_t>>& block_light_arrays
            );

            base_objects::network::response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool enforces_secure_chat);
            base_objects::network::response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons = {}, uint8_t columns = 0, uint8_t rows = 0, uint8_t x = 0, uint8_t z = 0, const list_array<uint8_t>& data = {});
            base_objects::network::response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade>& trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock);
            base_objects::network::response updateEntityPosition(int32_t entity_id, util::XYZ<float> pos, bool on_ground);
            base_objects::network::response updateEntityPositionAndRotation(int32_t entity_id, util::XYZ<float> pos, util::VECTOR rot, bool on_ground);
            base_objects::network::response updateEntityRotation(int32_t entity_id, util::VECTOR rot, bool on_ground);
            base_objects::network::response moveVehicle(util::VECTOR pos, util::VECTOR rot);
            base_objects::network::response openBook(int32_t hand);
            base_objects::network::response openScreen(int32_t window_id, int32_t type, const Chat& title);
            base_objects::network::response openSignEditor(base_objects::position pos, bool is_front_text);
            base_objects::network::response ping(int32_t id);
            base_objects::network::response pingResponse(int32_t id);
            base_objects::network::response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id);
            base_objects::network::response playerAbilities(uint8_t flags, float flying_speed, float field_of_view);
            base_objects::network::response playerChatMessage(enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, const std::optional<enbt::value>& unsigned_content, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name);
            //UNUSED by Notchian client
            base_objects::network::response endCombat(int32_t duration);
            //UNUSED by Notchian client
            base_objects::network::response enterCombat();
            base_objects::network::response combatDeath(int32_t player_id, const Chat& message);
            base_objects::network::response playerInfoRemove(const list_array<enbt::raw_uuid>& players);
            base_objects::network::response playerInfoAdd(const list_array<base_objects::packets::player_actions_add>& add_players);
            base_objects::network::response playerInfoInitializeChat(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat);
            base_objects::network::response playerInfoUpdateGameMode(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode);
            base_objects::network::response playerInfoUpdateListed(const list_array<base_objects::packets::player_actions_update_listed>& update_listed);
            base_objects::network::response playerInfoUpdateLatency(const list_array<base_objects::packets::player_actions_update_latency>& update_latency);
            base_objects::network::response playerInfoUpdateDisplayName(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name);
            base_objects::network::response lookAt(bool from_feet_or_eyes, util::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id);
            base_objects::network::response synchronizePlayerPosition(util::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id);
            base_objects::network::response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& displayed_recipe_ids, const list_array<std::string>& had_access_to_recipe_ids);
            base_objects::network::response addRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids);
            base_objects::network::response removeRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, const list_array<std::string>& recipe_ids);
            base_objects::network::response updateRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active);
            base_objects::network::response removeEntities(const list_array<int32_t>& entity_ids);
            base_objects::network::response removeEntityEffect(int32_t entity_id, int32_t effect_id);
            base_objects::network::response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name);
            base_objects::network::response removeResourcePacks();
            base_objects::network::response removeResourcePack(enbt::raw_uuid id);
            base_objects::network::response addResourcePack(enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt);
            base_objects::network::response respawn(int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata);
            base_objects::network::response setHeadRotation(int32_t entity_id, util::VECTOR head_rotation);

            base_objects::network::response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks);

            base_objects::network::response setAdvancementsTab(const std::optional<std::string>& tab_id);
            base_objects::network::response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool __ignored = false);
            base_objects::network::response setActionBarText(const Chat& text);
            base_objects::network::response setBorderCenter(double x, double z);
            base_objects::network::response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms);
            base_objects::network::response setBorderSize(double diameter);
            base_objects::network::response setBorderWarningDelay(int32_t warning_delay);
            base_objects::network::response setBorderWarningDistance(int32_t warning_distance);
            base_objects::network::response setCamera(int32_t entity_id);
            base_objects::network::response setHeldSlot(int32_t slot);
            base_objects::network::response setCenterChunk(int32_t x, int32_t z);
            base_objects::network::response setRenderDistance(int32_t render_distance);
            base_objects::network::response setDefaultSpawnPosition(base_objects::position pos, float angle);
            base_objects::network::response displayObjective(int32_t position, const std::string& objective_name);
            base_objects::network::response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata);
            base_objects::network::response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id);
            base_objects::network::response setEntityVelocity(int32_t entity_id, util::VECTOR velocity);

            base_objects::network::response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::slot& item);

            base_objects::network::response setExperience(float experience_bar, int32_t level, int32_t total_experience);

            base_objects::network::response setHealth(float health, int32_t food, float saturation);

            base_objects::network::response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type);
            base_objects::network::response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style);
            base_objects::network::response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);
            base_objects::network::response updateObjectivesRemove(const std::string& objective_name);
            base_objects::network::response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type);
            base_objects::network::response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style);
            base_objects::network::response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content);

            base_objects::network::response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers);


            base_objects::network::response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities);
            base_objects::network::response updateTeamRemove(const std::string& team_name);
            base_objects::network::response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix);
            base_objects::network::response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities);
            base_objects::network::response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities);


            base_objects::network::response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name);
            base_objects::network::response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled);
            base_objects::network::response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content);

            base_objects::network::response setSimulationDistance(int32_t distance);
            base_objects::network::response setSubtitleText(const Chat& text);
            base_objects::network::response updateTime(int64_t world_age, int64_t time_of_day);
            base_objects::network::response setTitleText(const Chat& text);
            base_objects::network::response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out);


            base_objects::network::response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);
            base_objects::network::response entitySoundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed);

            base_objects::network::response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);
            base_objects::network::response soundEffectCustom(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed);


            base_objects::network::response startConfiguration();
            base_objects::network::response stopSound(uint8_t flags);
            base_objects::network::response stopSoundBySource(uint8_t flags, int32_t source);
            base_objects::network::response stopSoundBySound(uint8_t flags, const std::string& sound);
            base_objects::network::response stopSoundBySourceAndSound(uint8_t flags, int32_t source, const std::string& sound);

            base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload);

            base_objects::network::response systemChatMessage(const Chat& message);
            base_objects::network::response systemChatMessageOverlay(const Chat& message);
            base_objects::network::response setTabListHeaderAndFooter(const Chat& header, const Chat& footer);
            base_objects::network::response tagQueryResponse(int32_t transaction_id, const enbt::value& nbt);
            base_objects::network::response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count);
            base_objects::network::response teleportEntity(int32_t entity_id, util::VECTOR pos, float yaw, float pitch, bool on_ground);
            base_objects::network::response setTickingState(float tick_rate, bool is_frozen);
            base_objects::network::response stepTick(int32_t step_count);

            base_objects::network::response transfer(const std::string& host, int32_t port);

            base_objects::network::response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping>& advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress>& progress_advancements);
            base_objects::network::response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties);

            base_objects::network::response entityEffect(int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, int8_t flags);

            base_objects::network::response updateRecipes(const std::vector<base_objects::recipe>& recipes);

            base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings);

            base_objects::network::response projectilePower(int32_t entity_id, double power_x, double power_y, double power_z);

            base_objects::network::response custom_report(const list_array<std::pair<std::string, std::string>>& values);

            base_objects::network::response server_links(const list_array<base_objects::packets::server_link>& links);
        }
    }
}
#endif /* SRC_PROTOCOLHELPER_PACKETS_767_PACKETS */