#ifndef SRC_PROTOCOLHELPER_PACKETS_766_PACKETS
#define SRC_PROTOCOLHELPER_PACKETS_766_PACKETS
#include <src/base_objects/block.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/chunk.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/particle_data.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/recipe.hpp>

//packets for 1.20.5..., protocol 766
//changes between 765:
// encryption in offline mode is now optional instead of disabled
//
//# Packets changes:
//## Login
//Client bound@0x01
//   - append should_authenticate boolean
//Client bound@0x02
//   - append strict_error_handling boolean
//Client bound@0x05 new(Request cookie)
//  [Identifier] Cookie key
//Server bound@0x04 new(Cookie response)
//  [Identifier] Cookie key
//  [boolean] payload been registered
//  [var_int|optional] payload length
//  [payload|optional]
//
//
//## Configuration
//Client bound@0x00 new(Request cookie)
// [Identifier] Cookie key
//Client bound@0x01 <- @0x00
//Client bound@0x02 <- @0x01
//Client bound@0x03 <- @0x02
//Client bound@0x04 <- @0x03
//Client bound@0x05 <- @0x04
//Client bound@0x06 new (Reset chat)
//Client bound@0x07 modified <- @0x05
//   [Identifier] Registry ID
//   [var_int] Entry Count
//     {
//      [Identifier] Entry ID
//      [boolean] Has Data
//      [NBT] data, present if Has Data set
//     }
//    *Instead of sending whole pack of entries at once, send each of them separately.
//Client bound@0x08 <- @0x06
//Client bound@0x09 <- @0x07
//Client bound@0x0A new(Store cookie)
//  [Identifier] Cookie ID
//  [Byte...]  Payload, length calculated by packet size (max 5120 bytes)
//Client bound@0x0B new(transfer)
//  [Identifier] Host
//  [var_int] port
//Client bound@0x0C <- @0x08
//Client bound@0x0D <- @0x09
//Client bound@0x0E new(known packs)
//  [var_int] count
//     {
//      [String] namespace
//      [String] ID
//      [String] version
//     }
//  *blocks configuration, requires response from user
//Server bound@0x00 keeps
//Server bound@0x01 new(Cookie response)
//  [Identifier] Cookie key
//  [boolean] payload been registered
//  [var_int|optional] payload length
//  [payload|optional]
//Server bound@0x02 <- @0x01
//Server bound@0x03 <- @0x02
//Server bound@0x04 <- @0x03
//Server bound@0x05 <- @0x04
//Server bound@0x06 <- @0x05
//Server bound@0x07 new(known packs)
//  [var_int] count
//     {
//      [String] namespace
//      [String] ID
//      [String] version
//     }
//  *If the client specifies a pack in this packet, the server should omit its contained data from the Registry Data packet.
//
//


//## Play
//Updated enchanting table ids and name, sync with wiki
//Switched end raining and begin raining ids in event
//World events is not up to date in wiki.vg
//Changed sounds id in 0x28 (2005 Bonemeal particles -> 1505 Bonemeal particles)
//Removed sounds in 0x28 (1036 iron trap door opened, 1037 iron trap door closed)
//Added sounds in 0x28 (1044 Smithing table used,
//                      1045 Pointed dripstone landing,
//                      1046 Lava dripping on cauldron from dripstone,
//                      1047 Water dripping on cauldron from dripstone
//                      1048 Skeleton converts to stray
//                      1049 Crafter successfully crafts item
//                      1050 Crafter fails to craft item
//                      ---
//                      1504 Fluid drips from dripstone
//                      )
//
//
//Client bound@0x00 - @0x15 keeps
//Client bound@0x016 new(Request cookie)
// [Identifier] Cookie key
//Client bound@0x17 <- @0x16
//Client bound@0x18 <- @0x17
//Client bound@0x19 <- @0x18
//Client bound@0x1A <- @0x19
//Client bound@0x1B new(Debug Sample)
//  [var_int] samples size
//  [long array] type dependent samples(size form samples size)
//  [var_int] sample type
//Client bound@0x1C <- @0x1A
//Client bound@0x1D <- @0x1B
//Client bound@0x1E <- @0x1C
//Client bound@0x1F <- @0x1D
//Client bound@0x20 <- @0x1E
//Client bound@0x21 <- @0x1F
//Client bound@0x22 <- @0x20
//Client bound@0x23 <- @0x21
//Client bound@0x24 <- @0x22
//Client bound@0x25 <- @0x23
//Client bound@0x26 <- @0x24
//Client bound@0x27 <- @0x25
//Client bound@0x28 <- @0x26
//Client bound@0x29 <- @0x27
//Client bound@0x2A <- @0x28
//Client bound@0x2B <- @0x29 changed
// Dimension Type now accepts var_int, this is id from registry "minecraft:dimension_type"
// New field: Enforces secure chat [boolean]
//Client bound@0x2C <- @0x2A
//Client bound@0x2D <- @0x2B
//Client bound@0x2E <- @0x2C
//Client bound@0x2F <- @0x2D
//Client bound@0x30 <- @0x2E
//Client bound@0x31 <- @0x2F
//Client bound@0x32 <- @0x30
//Client bound@0x33 <- @0x31
//Client bound@0x34 <- @0x32
//Client bound@0x35 <- @0x33
//Client bound@0x36 <- @0x34
//Client bound@0x37 <- @0x35
//Client bound@0x38 <- @0x36
//Client bound@0x39 <- @0x37
//Client bound@0x3A <- @0x38
//Client bound@0x3B <- @0x39
//Client bound@0x3C <- @0x3A
//Client bound@0x3D <- @0x3B
//Client bound@0x3E <- @0x3C
//Client bound@0x3F <- @0x3D
//Client bound@0x40 <- @0x3E
//Client bound@0x41 <- @0x3F
//Client bound@0x42 <- @0x40
//Client bound@0x43 <- @0x41
//Client bound@0x44 <- @0x42
//Client bound@0x45 <- @0x43
//Client bound@0x46 <- @0x44
//Client bound@0x47 <- @0x45 changed
// Dimension Type now accepts var_int, this is id from registry "minecraft:dimension_type"
//Client bound@0x48 <- @0x46
//Client bound@0x49 <- @0x47
//Client bound@0x4A <- @0x48
//Client bound@0x4B <- @0x49 changed
// Number of bytes in the Icon array is now optional, only present if Has Icon is true
// Icon is now also optional, only present if Has Icon is true
// Field Enforces Secure Chat removed
//Client bound@0x4C <- @0x4A
//Client bound@0x4D <- @0x4B
//Client bound@0x4E <- @0x4C
//Client bound@0x4F <- @0x4D
//Client bound@0x50 <- @0x4E
//Client bound@0x51 <- @0x4F
//Client bound@0x52 <- @0x50
//Client bound@0x53 <- @0x51
//Client bound@0x54 <- @0x52
//Client bound@0x55 <- @0x53
//Client bound@0x56 <- @0x54
//Client bound@0x57 <- @0x55
//Client bound@0x58 <- @0x56
//Client bound@0x59 <- @0x57
//Client bound@0x5A <- @0x58
//Client bound@0x5B <- @0x59
//Client bound@0x5C <- @0x5A
//Client bound@0x5D <- @0x5B
//Client bound@0x5E <- @0x5C
//Client bound@0x5F <- @0x5D
//Client bound@0x60 <- @0x5E
//Client bound@0x61 <- @0x5F
//Client bound@0x62 <- @0x60
//Client bound@0x63 <- @0x61
//Client bound@0x64 <- @0x62
//Client bound@0x65 <- @0x63
//Client bound@0x66 <- @0x64
//Client bound@0x67 <- @0x65
//Client bound@0x68 <- @0x66
//Client bound@0x69 <- @0x67
//Client bound@0x6A <- @0x68
//Client bound@0x6B new (Store cookie)
//  [Identifier] Cookie ID
//  [Byte...]  Payload, length calculated by packet size(max 5120 bytes)
//Client bound@0x6C <- @0x69
//Client bound@0x6D <- @0x6A
//Client bound@0x6E <- @0x6B
//Client bound@0x6F <- @0x6C
//Client bound@0x70 <- @0x6D
//Client bound@0x71 <- @0x6E
//Client bound@0x72 <- @0x6F
//Client bound@0x73 new (transfer)
//  [String] Host
//  [var_int] port
//Client bound@0x74 <- @0x70
//Client bound@0x75 <- @0x71 changed
// Property Key now accepts var_int, known modifiers in wiki.vg
//Client bound@0x76 <- @0x72 changed
// Amplifier type changed to var_int
// Has factor data and factor codec fields removed
// New flag 0x08 blend
//Client bound@0x77 <- @0x73 changed
// Type renamed to Recipe ID
// And recipe id changed to Type ID, known types in wiki.vg
//Client bound@0x78 <- @0x74
//Client bound@0x79 new (Projectile Power)
//  [var_int] Entity ID
//  [double] Power X
//  [double] Power Y
//  [double] Power Z
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
    namespace base_objects {
        struct SharedClientData;
    }

    namespace packets::release_766 {
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

            base_objects::network::response resetChat();

            base_objects::network::response removeResourcePacks();
            base_objects::network::response removeResourcePack(const enbt::raw_uuid& pack_id);
            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced);
            base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt);

            base_objects::network::response storeCookie(const std::string& key, const list_array<uint8_t>& payload);
            base_objects::network::response transfer(const std::string& host, int32_t port);

            base_objects::network::response setFeatureFlags(const list_array<std::string>& features);

            base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries);

            base_objects::network::response knownPacks(const list_array<base_objects::packets::known_pack>& packs);
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

            base_objects::network::response updateAttributes__(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties, uint32_t protocol_version = 766);

            base_objects::network::response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties);

            base_objects::network::response entityEffect(int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, int8_t flags);

            base_objects::network::response updateRecipes(const std::vector<base_objects::recipe>& recipes);

            base_objects::network::response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings);


            base_objects::network::response projectilePower(int32_t entity_id, double power_x, double power_y, double power_z);
        }
    }
}
#endif /* SRC_PROTOCOLHELPER_PACKETS_766_RELEASE_PACKETS */