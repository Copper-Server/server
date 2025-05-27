#include <library/fast_task.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/api/packets.hpp>

namespace copper_server {

#define call_function_selector(state, name, ...) registry::entrys.get([&](auto& entry) { return entry.at(client.packets_state.protocol_version).state->name(__VA_ARGS__); });

    namespace api::packets {
        namespace registry {

            fast_task::protected_value<std::unordered_map<int32_t, protocol_registry_entry>> entrys;

            bool contains_protocol(int32_t number) {
                return entrys.get([&number](auto& entrys) {
                    return entrys.contains(number);
                });
            }

            size_t protocols_count() {
                return entrys.get([](auto& entrys) {
                    return entrys.size();
                });
            }

            void register_protocol(int32_t id, const protocol_registry_entry& entry) {
                entrys.set([id, &entry](auto& entrys) {
                    if (!entrys.try_emplace(id, entry).second)
                        throw std::runtime_error("Protocol " + std::to_string(id) + " already registered");
                });
            }

            void erase_protocol(int32_t number) {
                protocol_registry_entry entry;
                bool call = false;
                entrys.set([number, &entry, &call](auto& entrys) {
                    auto it = entrys.find(number);
                    if (it != entrys.end()) {
                        entry = it->second;
                        call = true;
                        entrys.erase(it);
                    }
                });
                if (call) {
                    if (entry.functions_destructor)
                        entry.functions_destructor(entry);
                    else {
                        delete entry.login;
                        delete entry.configuration;
                        delete entry.play;
                    }
                }
            }
        }

        namespace login {
            base_objects::network::response login(base_objects::SharedClientData& client, int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data) {
                return call_function_selector(login, login, plugin_message_id, chanel, data);
            }

            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason) {
                return call_function_selector(login, kick, reason)
            };

            base_objects::network::response disableCompression(base_objects::SharedClientData& client) {
                return call_function_selector(login, disableCompression)
            };

            base_objects::network::response setCompression(base_objects::SharedClientData& client, int32_t threshold) {
                return call_function_selector(login, setCompression, threshold)
            };

            base_objects::network::response requestCookie(base_objects::SharedClientData& client, const std::string& key) {
                return call_function_selector(login, requestCookie, key)
            };

            base_objects::network::response loginSuccess(base_objects::SharedClientData& client) {
                return call_function_selector(login, loginSuccess, client)
            };

            base_objects::network::response encryptionRequest(base_objects::SharedClientData& client, const std::string& server_id, uint8_t (&verify_token)[4]) {
                return call_function_selector(login, encryptionRequest, server_id, verify_token)
            };
        }

        namespace configuration {
            base_objects::network::response requestCookie(base_objects::SharedClientData& client, const std::string& key){
                return call_function_selector(configuration, requestCookie, key)
            }

            base_objects::network::response configuration(base_objects::SharedClientData& client, const std::string& chanel, const list_array<uint8_t>& data){
                return call_function_selector(configuration, configuration, chanel, data)
            }

            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason){
                return call_function_selector(configuration, kick, reason)
            }

            base_objects::network::response finish(base_objects::SharedClientData& client){
                return call_function_selector(configuration, finish)
            }

            base_objects::network::response keep_alive(base_objects::SharedClientData& client, int64_t keep_alive_packet){
                return call_function_selector(configuration, keep_alive, keep_alive_packet)
            }

            base_objects::network::response ping(base_objects::SharedClientData& client, int32_t excepted_pong){
                return call_function_selector(configuration, ping, excepted_pong)
            }

            base_objects::network::response registry_data(base_objects::SharedClientData& client){
                return call_function_selector(configuration, registry_data, )
            }

            base_objects::network::response resetChat(base_objects::SharedClientData& client){
                return call_function_selector(configuration, resetChat, )
            }

            base_objects::network::response removeResourcePacks(base_objects::SharedClientData& client){
                return call_function_selector(configuration, removeResourcePacks, )
            }

            base_objects::network::response removeResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id){
                return call_function_selector(configuration, removeResourcePack, pack_id)
            }

            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced){
                return call_function_selector(configuration, addResourcePack, client, pack_id, url, hash, forced)
            }

            base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt) {
                return call_function_selector(configuration, addResourcePackPrompted, client, pack_id, url, hash, forced, prompt)
            };

            base_objects::network::response storeCookie(base_objects::SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload){
                return call_function_selector(configuration, storeCookie, key, payload)
            }

            base_objects::network::response transfer(base_objects::SharedClientData& client, const std::string& host, int32_t port){
                return call_function_selector(configuration, transfer, host, port)
            }

            base_objects::network::response setFeatureFlags(base_objects::SharedClientData& client, const list_array<std::string>& features){
                return call_function_selector(configuration, setFeatureFlags, features)
            }

            base_objects::network::response updateTags(base_objects::SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tags_entries){
                return call_function_selector(configuration, updateTags, tags_entries)
            }

            base_objects::network::response knownPacks(base_objects::SharedClientData& client, const list_array<base_objects::data_packs::known_pack>& packs){
                return call_function_selector(configuration, knownPacks, packs)
            }

            base_objects::network::response custom_report(base_objects::SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values){
                return call_function_selector(configuration, custom_report, values)
            }

            base_objects::network::response server_links(base_objects::SharedClientData& client, const list_array<base_objects::packets::server_link>& links) {
                return call_function_selector(configuration, server_links, links)
            }
        }

        namespace play {
            base_objects::network::response bundleResponse(base_objects::SharedClientData& client, base_objects::network::response&& response){
                return call_function_selector(play, bundleResponse, std::move(response))
            }

            base_objects::network::response spawnEntity(base_objects::SharedClientData& client, const base_objects::entity& entity, uint16_t protocol){
                return call_function_selector(play, spawnEntity, entity, 1)
            }

            base_objects::network::response entityAnimation(base_objects::SharedClientData& client, const base_objects::entity& entity, base_objects::entity_animation animation){
                return call_function_selector(play, entityAnimation, entity, animation)
            }

            base_objects::network::response awardStatistics(base_objects::SharedClientData& client, const list_array<base_objects::packets::statistics>& statistics){
                return call_function_selector(play, awardStatistics, statistics)
            }

            base_objects::network::response acknowledgeBlockChange(base_objects::SharedClientData& client){
                return call_function_selector(play, acknowledgeBlockChange, client)
            }

            base_objects::network::response setBlockDestroyStage(base_objects::SharedClientData& client, const base_objects::entity& entity, base_objects::position block, uint8_t stage){
                return call_function_selector(play, setBlockDestroyStage, entity, block, stage)
            }

            base_objects::network::response blockEntityData(base_objects::SharedClientData& client, base_objects::position block, int32_t type, const enbt::value& data){
                return call_function_selector(play, blockEntityData, block, type, data)
            }

            base_objects::network::response blockAction(base_objects::SharedClientData& client, base_objects::position block, int32_t action_id, int32_t param, int32_t block_type){
                return call_function_selector(play, blockAction, block, action_id, param, block_type)
            }

            base_objects::network::response blockUpdate(base_objects::SharedClientData& client, base_objects::position block, int32_t block_type){
                return call_function_selector(play, blockUpdate, block, block_type)
            }

            base_objects::network::response bossBarAdd(base_objects::SharedClientData& client, const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags){
                return call_function_selector(play, bossBarAdd, id, title, health, color, division, flags)
            }

            base_objects::network::response bossBarRemove(base_objects::SharedClientData& client, const enbt::raw_uuid& id){
                return call_function_selector(play, bossBarRemove, id)
            }

            base_objects::network::response bossBarUpdateHealth(base_objects::SharedClientData& client, const enbt::raw_uuid& id, float health){
                return call_function_selector(play, bossBarUpdateHealth, id, health)
            }

            base_objects::network::response bossBarUpdateTitle(base_objects::SharedClientData& client, const enbt::raw_uuid& id, const Chat& title){
                return call_function_selector(play, bossBarUpdateTitle, id, title)
            }

            base_objects::network::response bossBarUpdateStyle(base_objects::SharedClientData& client, const enbt::raw_uuid& id, int32_t color, int32_t division){
                return call_function_selector(play, bossBarUpdateStyle, id, color, division)
            }

            base_objects::network::response bossBarUpdateFlags(base_objects::SharedClientData& client, const enbt::raw_uuid& id, uint8_t flags){
                return call_function_selector(play, bossBarUpdateFlags, id, flags)
            }

            base_objects::network::response changeDifficulty(base_objects::SharedClientData& client, uint8_t difficulty, bool locked){
                return call_function_selector(play, changeDifficulty, difficulty, locked)
            }

            base_objects::network::response chunkBatchFinished(base_objects::SharedClientData& client, int32_t count){
                return call_function_selector(play, chunkBatchFinished, count)
            }

            base_objects::network::response chunkBatchStart(base_objects::SharedClientData& client){
                return call_function_selector(play, chunkBatchStart)
            }

            base_objects::network::response chunkBiomes(base_objects::SharedClientData& client, list_array<base_objects::chunk::chunk_biomes>& chunk){
                return call_function_selector(play, chunkBiomes, chunk)
            }

            base_objects::network::response clearTitles(base_objects::SharedClientData& client, bool reset){
                return call_function_selector(play, clearTitles, reset)
            }

            base_objects::network::response commandSuggestionsResponse(base_objects::SharedClientData& client, int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions){
                return call_function_selector(play, commandSuggestionsResponse, transaction_id, start_pos, length, suggestions)
            }

            base_objects::network::response commands(base_objects::SharedClientData& client, int32_t root_id, const list_array<uint8_t>& nodes){
                return call_function_selector(play, commands, root_id, nodes)
            }

            base_objects::network::response closeContainer(base_objects::SharedClientData& client, uint8_t container_id){
                return call_function_selector(play, closeContainer, container_id)
            }

            base_objects::network::response setContainerContent(base_objects::SharedClientData& client, uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item){
                return call_function_selector(play, setContainerContent, windows_id, state_id, slots, carried_item)
            }

            base_objects::network::response setContainerProperty(base_objects::SharedClientData& client, uint8_t windows_id, uint16_t property, uint16_t value){
                return call_function_selector(play, setContainerProperty, windows_id, property, value)
            }

            base_objects::network::response setContainerSlot(base_objects::SharedClientData& client, uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item){
                return call_function_selector(play, setContainerSlot, windows_id, state_id, slot, item)
            }

            base_objects::network::response cookieRequest(base_objects::SharedClientData& client, const std::string& key){
                return call_function_selector(play, cookieRequest, key)
            }

            base_objects::network::response setCooldown(base_objects::SharedClientData& client, int32_t item_id, int32_t cooldown){
                return call_function_selector(play, setCooldown, item_id, cooldown)
            }

            base_objects::network::response chatSuggestionsResponse(base_objects::SharedClientData& client, int32_t action, int32_t count, const list_array<std::string>& suggestions){
                return call_function_selector(play, chatSuggestionsResponse, action, count, suggestions)
            }

            base_objects::network::response customPayload(base_objects::SharedClientData& client, const std::string& channel, const list_array<uint8_t>& data){
                return call_function_selector(play, customPayload, channel, data)
            }

            base_objects::network::response damageEvent(base_objects::SharedClientData& client, int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<util::VECTOR> xyz){
                return call_function_selector(play, damageEvent, entity_id, source_type_id, source_cause_id, source_direct_id, xyz)
            }

            base_objects::network::response debugSample(base_objects::SharedClientData& client, const list_array<uint64_t>& sample, int32_t sample_type){
                return call_function_selector(play, debugSample, sample, sample_type)
            }

            base_objects::network::response deleteMessageBySignature(base_objects::SharedClientData& client, uint8_t (&signature)[256]){
                return call_function_selector(play, deleteMessageBySignature, signature)
            }

            base_objects::network::response deleteMessageByID(base_objects::SharedClientData& client, int32_t message_id){
                return call_function_selector(play, deleteMessageByID, message_id)
            }

            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason){
                return call_function_selector(play, kick, reason)
            }

            base_objects::network::response disguisedChatMessage(base_objects::SharedClientData& client, const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name){
                return call_function_selector(play, disguisedChatMessage, message, chat_type, sender, target_name)
            }

            base_objects::network::response entityEvent(base_objects::SharedClientData& client, int32_t entity_id, base_objects::entity_event entity_status){
                return call_function_selector(play, entityEvent, entity_id, entity_status)
            }

            base_objects::network::response explosion(base_objects::SharedClientData& client, util::VECTOR pos, float strength, const list_array<util::XYZ<int8_t>>& affected_blocks, util::VECTOR player_motion, int32_t block_interaction, int32_t small_explosion_particle_id, const base_objects::particle_data& small_explosion_particle_data, int32_t large_explosion_particle_id, const base_objects::particle_data& large_explosion_particle_data, const std::string& sound_name, std::optional<float> fixed_range){
                return call_function_selector(play, explosion, pos, strength, affected_blocks, player_motion, block_interaction, small_explosion_particle_id, small_explosion_particle_data, large_explosion_particle_id, large_explosion_particle_data, sound_name, fixed_range)
            }

            base_objects::network::response unloadChunk(base_objects::SharedClientData& client, int32_t x, int32_t z){
                return call_function_selector(play, unloadChunk, x, z)
            }

            base_objects::network::response gameEvent(base_objects::SharedClientData& client, base_objects::packets::game_event_id event_id, float value){
                return call_function_selector(play, gameEvent, event_id, value)
            }

            base_objects::network::response openHorseWindow(base_objects::SharedClientData& client, uint8_t window_id, int32_t slots, int32_t entity_id){
                return call_function_selector(play, openHorseWindow, window_id, slots, entity_id)
            }

            base_objects::network::response hurtAnimation(base_objects::SharedClientData& client, int32_t entity_id, float yaw){
                return call_function_selector(play, hurtAnimation, entity_id, yaw)
            }

            base_objects::network::response initializeWorldBorder(base_objects::SharedClientData& client, double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time){
                return call_function_selector(play, initializeWorldBorder, x, z, old_diameter, new_diameter, speed_ms, portal_teleport_boundary, warning_blocks, warning_time)
            }

            base_objects::network::response keepAlive(base_objects::SharedClientData& client, int64_t id){
                return call_function_selector(play, keepAlive, id)
            }

            base_objects::network::response updateChunkDataWLights(base_objects::SharedClientData& client, int32_t chunk_x, int32_t chunk_z, const NBT& heightmaps, const std::vector<uint8_t> data, const bit_list_array<>& sky_light_mask, const bit_list_array<>& block_light_mask, const bit_list_array<>& empty_skylight_mask, const bit_list_array<>& empty_block_light_mask, const list_array<std::vector<uint8_t>> sky_light_arrays, const list_array<std::vector<uint8_t>> block_light_arrays){
                return call_function_selector(play, updateChunkDataWLights, chunk_x, chunk_z, heightmaps, data, sky_light_mask, block_light_mask, block_light_mask, empty_skylight_mask, sky_light_arrays, block_light_arrays)
            }

            base_objects::network::response worldEvent(base_objects::SharedClientData& client, int32_t event, base_objects::position pos, int32_t data, bool global){
                return call_function_selector(play, worldEvent, event, pos, data, global)
            }

            base_objects::network::response particle(base_objects::SharedClientData& client, int32_t particle_id, bool long_distance, util::VECTOR pos, util::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data){
                return call_function_selector(play, particle, particle_id, long_distance, pos, offset, max_speed, count, data)
            }

            base_objects::network::response updateLight(base_objects::SharedClientData& client, int32_t chunk_x, int32_t chunk_z, const bit_list_array<>& sky_light_mask, const bit_list_array<>& block_light_mask, const bit_list_array<>& empty_skylight_mask, const bit_list_array<>& empty_block_light_mask, const list_array<std::vector<uint8_t>> sky_light_arrays, const list_array<std::vector<uint8_t>> block_light_arrays){
                return call_function_selector(play, updateLight, chunk_x, chunk_z, sky_light_mask, block_light_mask, empty_skylight_mask, empty_block_light_mask, sky_light_arrays, block_light_arrays)
            }

            base_objects::network::response joinGame(base_objects::SharedClientData& client, int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, std::optional<base_objects::packets::death_location_data> death_location, int32_t portal_cooldown, bool enforces_secure_chat){
                return call_function_selector(play, joinGame, entity_id, is_hardcore, dimension_names, max_players, view_distance, simulation_distance, reduced_debug_info, enable_respawn_screen, do_limited_crafting, current_dimension_type, dimension_name, hashed_seed, gamemode, prev_gamemode, is_debug, is_flat, death_location, portal_cooldown, enforces_secure_chat)
            }

            base_objects::network::response mapData(base_objects::SharedClientData& client, int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t column, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data){
                return call_function_selector(play, mapData, map_id, scale, locked, icons, column, rows, x, z, data)
            }

            base_objects::network::response merchantOffers(base_objects::SharedClientData& client, int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade> trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock){
                return call_function_selector(play, merchantOffers, window_id, trade_id, trades, level, experience, regular_villager, can_restock)
            }

            base_objects::network::response updateEntityPosition(base_objects::SharedClientData& client, int32_t entity_id, util::XYZ<float> pos, bool on_ground){
                return call_function_selector(play, updateEntityPosition, entity_id, pos, on_ground)
            }

            base_objects::network::response updateEntityPositionAndRotation(base_objects::SharedClientData& client, int32_t entity_id, util::XYZ<float> pos, util::VECTOR rot, bool on_ground){
                return call_function_selector(play, updateEntityPositionAndRotation, entity_id, pos, rot, on_ground)
            }

            base_objects::network::response updateEntityRotation(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR rot, bool on_ground){
                return call_function_selector(play, updateEntityRotation, entity_id, rot, on_ground)
            }

            base_objects::network::response moveVehicle(base_objects::SharedClientData& client, util::VECTOR pos, util::VECTOR rot){
                return call_function_selector(play, moveVehicle, pos, rot)
            }

            base_objects::network::response openBook(base_objects::SharedClientData& client, int32_t hand){
                return call_function_selector(play, openBook, hand)
            }

            base_objects::network::response openScreen(base_objects::SharedClientData& client, int32_t window_id, int32_t type, const Chat& title){
                return call_function_selector(play, openScreen, window_id, type, title)
            }

            base_objects::network::response openSignEditor(base_objects::SharedClientData& client, base_objects::position pos, bool is_front_text){
                return call_function_selector(play, openSignEditor, pos, is_front_text)
            }

            base_objects::network::response ping(base_objects::SharedClientData& client, int32_t id){
                return call_function_selector(play, ping, id)
            }

            base_objects::network::response pingResponse(base_objects::SharedClientData& client, int32_t id){
                return call_function_selector(play, pingResponse, id)
            }

            base_objects::network::response placeGhostRecipe(base_objects::SharedClientData& client, int32_t windows_id, const std::string& recipe_id){
                return call_function_selector(play, placeGhostRecipe, windows_id, recipe_id)
            }

            base_objects::network::response playerAbilities(base_objects::SharedClientData& client, uint8_t flags, float flying_speed, float field_of_view){
                return call_function_selector(play, playerAbilities, flags, flying_speed, field_of_view)
            }

            base_objects::network::response playerChatMessage(base_objects::SharedClientData& client, enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, std::optional<enbt::value> unsigned_content, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name){
                return call_function_selector(play, playerChatMessage, sender, index, signature, message, timestamp, salt, prev_messages, unsigned_content, filter_type, filtered_symbols_bitfield, chat_type, sender_name, target_name)
            }

            base_objects::network::response endCombat(base_objects::SharedClientData& client, int32_t duration){
                return call_function_selector(play, endCombat, duration)
            }

            base_objects::network::response enterCombat(base_objects::SharedClientData& client){
                return call_function_selector(play, enterCombat)
            }

            base_objects::network::response combatDeath(base_objects::SharedClientData& client, int32_t player_id, const Chat& message){
                return call_function_selector(play, combatDeath, player_id, message)
            }

            base_objects::network::response playerInfoRemove(base_objects::SharedClientData& client, const list_array<enbt::raw_uuid>& players){
                return call_function_selector(play, playerInfoRemove, players)
            }

            base_objects::network::response playerInfoAdd(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_add>& add_players){
                return call_function_selector(play, playerInfoAdd, add_players)
            }

            base_objects::network::response playerInfoInitializeChat(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat){
                return call_function_selector(play, playerInfoInitializeChat, initialize_chat)
            }

            base_objects::network::response playerInfoUpdateGameMode(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode){
                return call_function_selector(play, playerInfoUpdateGameMode, update_game_mode)
            }

            base_objects::network::response playerInfoUpdateListed(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_listed>& update_listed){
                return call_function_selector(play, playerInfoUpdateListed, update_listed)
            }

            base_objects::network::response playerInfoUpdateLatency(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_latency>& update_latency){
                return call_function_selector(play, playerInfoUpdateLatency, update_latency)
            }

            base_objects::network::response playerInfoUpdateDisplayName(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name){
                return call_function_selector(play, playerInfoUpdateDisplayName, update_display_name)
            }

            base_objects::network::response lookAt(base_objects::SharedClientData& client, bool from_feet_or_eyes, util::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id){
                return call_function_selector(play, lookAt, from_feet_or_eyes, target, entity_id)
            }

            base_objects::network::response synchronizePlayerPosition(base_objects::SharedClientData& client, util::VECTOR pos, float yaw, float pitch, uint8_t flags) {
                auto id = client.packets_state.play_data->teleport_id_sequence++;
                client.packets_state.play_data->pending_teleport_ids.push_back(id);
                return call_function_selector(play, synchronizePlayerPosition, pos, yaw, pitch, flags, id)
            }

            base_objects::network::response initRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> displayed_recipe_ids, list_array<std::string> had_access_to_recipe_ids){
                return call_function_selector(play, initRecipeBook, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, displayed_recipe_ids, had_access_to_recipe_ids)
            }

            base_objects::network::response addRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids){
                return call_function_selector(play, addRecipeBook, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids)
            }

            base_objects::network::response removeRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids){
                return call_function_selector(play, removeRecipeBook, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids)
            }

            base_objects::network::response removeEntities(base_objects::SharedClientData& client, const list_array<int32_t>& entity_ids){
                return call_function_selector(play, removeEntities, entity_ids)
            }

            base_objects::network::response removeEntityEffect(base_objects::SharedClientData& client, int32_t entity_id, int32_t effect_id){
                return call_function_selector(play, removeEntityEffect, entity_id, effect_id)
            }

            base_objects::network::response resetScore(base_objects::SharedClientData& client, const std::string& entity_name, const std::optional<std::string>& objective_name){
                return call_function_selector(play, resetScore, entity_name, objective_name)
            }

            base_objects::network::response removeResourcePacks(base_objects::SharedClientData& client){
                return call_function_selector(play, removeResourcePacks)
            }

            base_objects::network::response removeResourcePack(base_objects::SharedClientData& client, enbt::raw_uuid id){
                return call_function_selector(play, removeResourcePack, id)
            }

            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt){
                return call_function_selector(play, addResourcePack, id, url, hash, forced, prompt)
            }

            base_objects::network::response respawn(base_objects::SharedClientData& client, int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata){
                return call_function_selector(play, respawn, dimension_type, dimension_name, hashed_seed, gamemode, previous_gamemode, is_debug, is_flat, death_location, portal_cooldown, keep_attributes, keep_metadata)
            }

            base_objects::network::response setHeadRotation(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR head_rotation){
                return call_function_selector(play, setHeadRotation, entity_id, head_rotation)
            }

            base_objects::network::response updateSectionBlocks(base_objects::SharedClientData& client, int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks){
                return call_function_selector(play, updateSectionBlocks, section_x, section_z, section_y, blocks)
            }

            base_objects::network::response setAdvancementsTab(base_objects::SharedClientData& client, const std::optional<std::string>& tab_id){
                return call_function_selector(play, setAdvancementsTab, tab_id)
            }

            base_objects::network::response serverData(base_objects::SharedClientData& client, const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png){
                return call_function_selector(play, serverData, motd, icon_png)
            }

            base_objects::network::response setActionBarText(base_objects::SharedClientData& client, const Chat& text){
                return call_function_selector(play, setActionBarText, text)
            }

            base_objects::network::response setBorderCenter(base_objects::SharedClientData& client, double x, double z){
                return call_function_selector(play, setBorderCenter, x, z)
            }

            base_objects::network::response setBorderLerp(base_objects::SharedClientData& client, double old_diameter, double new_diameter, int64_t speed_ms){
                return call_function_selector(play, setBorderLerp, old_diameter, new_diameter, speed_ms)
            }

            base_objects::network::response setBorderSize(base_objects::SharedClientData& client, double diameter){
                return call_function_selector(play, setBorderSize, diameter)
            }

            base_objects::network::response setBorderWarningDelay(base_objects::SharedClientData& client, int32_t warning_delay){
                return call_function_selector(play, setBorderWarningDelay, warning_delay)
            }

            base_objects::network::response setBorderWarningDistance(base_objects::SharedClientData& client, int32_t warning_distance){
                return call_function_selector(play, setBorderWarningDistance, warning_distance)
            }

            base_objects::network::response setCamera(base_objects::SharedClientData& client, int32_t entity_id){
                return call_function_selector(play, setCamera, entity_id)
            }

            base_objects::network::response setHeldSlot(base_objects::SharedClientData& client, int32_t slot){
                return call_function_selector(play, setHeldSlot, slot)
            }

            base_objects::network::response setCenterChunk(base_objects::SharedClientData& client, int32_t x, int32_t z){
                return call_function_selector(play, setCenterChunk, x, z)
            }

            base_objects::network::response setRenderDistance(base_objects::SharedClientData& client, int32_t render_distance){
                return call_function_selector(play, setRenderDistance, render_distance)
            }

            base_objects::network::response setDefaultSpawnPosition(base_objects::SharedClientData& client, base_objects::position pos, float angle){
                return call_function_selector(play, setDefaultSpawnPosition, pos, angle)
            }

            base_objects::network::response displayObjective(base_objects::SharedClientData& client, int32_t position, const std::string& objective_name){
                return call_function_selector(play, displayObjective, position, objective_name)
            }

            base_objects::network::response setEntityMetadata(base_objects::SharedClientData& client, int32_t entity_id, const list_array<uint8_t>& metadata){
                return call_function_selector(play, setEntityMetadata, entity_id, metadata)
            }

            base_objects::network::response linkEntities(base_objects::SharedClientData& client, int32_t attached_entity_id, int32_t holder_entity_id){
                return call_function_selector(play, linkEntities, attached_entity_id, holder_entity_id)
            }

            base_objects::network::response setEntityMotion(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR velocity){
                return call_function_selector(play, setEntityMotion, entity_id, velocity)
            }

            base_objects::network::response setEquipment(base_objects::SharedClientData& client, int32_t entity_id, uint8_t slot, const base_objects::slot& item){
                return call_function_selector(play, setEquipment, entity_id, slot, item)
            }

            base_objects::network::response setExperience(base_objects::SharedClientData& client, float experience_bar, int32_t level, int32_t total_experience){
                return call_function_selector(play, setExperience, experience_bar, level, total_experience)
            }

            base_objects::network::response setHealth(base_objects::SharedClientData& client, float health, int32_t food, float saturation){
                return call_function_selector(play, setHealth, health, food, saturation)
            }

            base_objects::network::response updateObjectivesCreate(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type){
                return call_function_selector(play, updateObjectivesCreate, objective_name, display_name, render_type)
            }

            base_objects::network::response updateObjectivesCreateStyled(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style){
                return call_function_selector(play, updateObjectivesCreateStyled, objective_name, display_name, render_type, style)
            }

            base_objects::network::response updateObjectivesCreateFixed(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content){
                return call_function_selector(play, updateObjectivesCreateFixed, objective_name, display_name, render_type, content)
            }

            base_objects::network::response updateObjectivesRemove(base_objects::SharedClientData& client, const std::string& objective_name){
                return call_function_selector(play, updateObjectivesRemove, objective_name)
            }

            base_objects::network::response updateObjectivesInfo(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type){
                return call_function_selector(play, updateObjectivesInfo, objective_name, display_name, render_type)
            }

            base_objects::network::response updateObjectivesInfoStyled(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style){
                return call_function_selector(play, updateObjectivesInfoStyled, objective_name, display_name, render_type, style)
            }

            base_objects::network::response updateObjectivesInfoFixed(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content){
                return call_function_selector(play, updateObjectivesInfoFixed, objective_name, display_name, render_type, content)
            }

            base_objects::network::response setPassengers(base_objects::SharedClientData& client, int32_t vehicle_entity_id, const list_array<int32_t>& passengers){
                return call_function_selector(play, setPassengers, vehicle_entity_id, passengers)
            }

            base_objects::network::response updateTeamCreate(base_objects::SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities){
                return call_function_selector(play, updateTeamCreate, team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix, entities)
            }

            base_objects::network::response updateTeamRemove(base_objects::SharedClientData& client, const std::string& team_name){
                return call_function_selector(play, updateTeamRemove, team_name)
            }

            base_objects::network::response updateTeamInfo(base_objects::SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix){
                return call_function_selector(play, updateTeamInfo, team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix)
            }

            base_objects::network::response updateTeamAddEntities(base_objects::SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities){
                return call_function_selector(play, updateTeamAddEntities, team_name, entities)
            }

            base_objects::network::response updateTeamRemoveEntities(base_objects::SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities){
                return call_function_selector(play, updateTeamRemoveEntities, team_name, entities)
            }

            base_objects::network::response setScore(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name){
                return call_function_selector(play, setScore, entity_name, objective_name, value, display_name)
            }

            base_objects::network::response setScoreStyled(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled){
                return call_function_selector(play, setScoreStyled, entity_name, objective_name, value, display_name, styled)
            }

            base_objects::network::response setScoreFixed(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content){
                return call_function_selector(play, setScoreFixed, entity_name, objective_name, value, display_name, content)
            }

            base_objects::network::response setSimulationDistance(base_objects::SharedClientData& client, int32_t distance){
                return call_function_selector(play, setSimulationDistance, distance)
            }

            base_objects::network::response setSubtitleText(base_objects::SharedClientData& client, const Chat& text){
                return call_function_selector(play, setSubtitleText, text)
            }

            base_objects::network::response updateTime(base_objects::SharedClientData& client, int64_t world_age, int64_t time_of_day, bool increase_time){
                return call_function_selector(play, updateTime, world_age, time_of_day, increase_time)
            }

            base_objects::network::response setTitleText(base_objects::SharedClientData& client, const Chat& text){
                return call_function_selector(play, setTitleText, text)
            }

            base_objects::network::response setTitleAnimationTimes(base_objects::SharedClientData& client, int32_t fade_in, int32_t stay, int32_t fade_out){
                return call_function_selector(play, setTitleAnimationTimes, fade_in, stay, fade_out)
            }

            base_objects::network::response entitySoundEffect(base_objects::SharedClientData& client, uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed){
                return call_function_selector(play, entitySoundEffect, sound_id, category, entity_id, volume, pitch, seed)
            }

            base_objects::network::response entitySoundEffectCustom(base_objects::SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed){
                return call_function_selector(play, entitySoundEffectCustom, sound_id, range, category, entity_id, volume, pitch, seed)
            }

            base_objects::network::response soundEffect(base_objects::SharedClientData& client, uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed){
                return call_function_selector(play, soundEffect, sound_id, category, x, y, z, volume, pitch, seed)
            }

            base_objects::network::response soundEffectCustom(base_objects::SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed){
                return call_function_selector(play, soundEffectCustom, sound_id, range, category, x, y, z, volume, pitch, seed)
            }

            base_objects::network::response startConfiguration(base_objects::SharedClientData& client){
                return call_function_selector(play, startConfiguration)
            }

            base_objects::network::response stopSound(base_objects::SharedClientData& client, uint8_t flags){
                return call_function_selector(play, stopSound, flags)
            }

            base_objects::network::response stopSoundBySource(base_objects::SharedClientData& client, uint8_t flags, int32_t source){
                return call_function_selector(play, stopSoundBySource, flags, source)
            }

            base_objects::network::response stopSoundBySound(base_objects::SharedClientData& client, uint8_t flags, const std::string& sound){
                return call_function_selector(play, stopSoundBySound, flags, sound)
            }

            base_objects::network::response stopSoundBySourceAndSound(base_objects::SharedClientData& client, uint8_t flags, int32_t source, const std::string& sound){
                return call_function_selector(play, stopSoundBySourceAndSound, flags, source, sound)
            }

            base_objects::network::response storeCookie(base_objects::SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload){
                return call_function_selector(play, storeCookie, key, payload)
            }

            base_objects::network::response systemChatMessage(base_objects::SharedClientData& client, const Chat& message){
                return call_function_selector(play, systemChatMessage, message)
            }

            base_objects::network::response systemChatMessageOverlay(base_objects::SharedClientData& client, const Chat& message){
                return call_function_selector(play, systemChatMessageOverlay, message)
            }

            base_objects::network::response setTabListHeaderAndFooter(base_objects::SharedClientData& client, const Chat& header, const Chat& footer){
                return call_function_selector(play, setTabListHeaderAndFooter, header, footer)
            }

            base_objects::network::response tagQueryResponse(base_objects::SharedClientData& client, int32_t transaction_id, const enbt::value& nbt){
                return call_function_selector(play, tagQueryResponse, transaction_id, nbt)
            }

            base_objects::network::response pickupItem(base_objects::SharedClientData& client, int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count){
                return call_function_selector(play, pickupItem, collected_entity_id, collector_entity_id, pickup_item_count)
            }

            base_objects::network::response teleportEntity(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR pos, float yaw, float pitch, bool on_ground){
                return call_function_selector(play, teleportEntity, entity_id, pos, yaw, pitch, on_ground)
            }

            base_objects::network::response test_instance_block_status(base_objects::SharedClientData& client, const Chat& status, std::optional<util::VECTOR> size){
                return call_function_selector(play, test_instance_block_status, status, size)
            }


            base_objects::network::response setTickingState(base_objects::SharedClientData& client, float tick_rate, bool is_frozen){
                return call_function_selector(play, setTickingState, tick_rate, is_frozen)
            }

            base_objects::network::response stepTick(base_objects::SharedClientData& client, int32_t step_count){
                return call_function_selector(play, stepTick, step_count)
            }

            base_objects::network::response transfer(base_objects::SharedClientData& client, const std::string& host, int32_t port){
                return call_function_selector(play, transfer, host, port)
            }

            base_objects::network::response updateAdvancements(base_objects::SharedClientData& client, bool reset, const list_array<base_objects::packets::advancements_maping> advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress> progress_advancements){
                return call_function_selector(play, updateAdvancements, reset, advancement_mapping, remove_advancements, progress_advancements)
            }

            base_objects::network::response updateAttributes(base_objects::SharedClientData& client, int32_t entity_id, const list_array<base_objects::packets::attributes>& properties){
                return call_function_selector(play, updateAttributes, entity_id, properties)
            }

            base_objects::network::response entityEffect(base_objects::SharedClientData& client, int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, base_objects::packets::effect_flags flags){
                return call_function_selector(play, entityEffect, entity_id, effect_id, amplifier, duration, flags)
            }

            base_objects::network::response updateRecipes(base_objects::SharedClientData& client, const std::vector<base_objects::recipe>& recipes){
                return call_function_selector(play, updateRecipes, recipes)
            }

            base_objects::network::response updateTags(base_objects::SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tag_mappings){
                return call_function_selector(play, updateTags, tag_mappings)
            }

            base_objects::network::response projectilePower(base_objects::SharedClientData& client, int32_t entity_id, double power_x, double power_y, double power_z){
                return call_function_selector(play, projectilePower, entity_id, power_x, power_y, power_z)
            }

            base_objects::network::response custom_report(base_objects::SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values){
                return call_function_selector(play, custom_report, values)
            }

            base_objects::network::response server_links(base_objects::SharedClientData& client, const list_array<base_objects::packets::server_link>& links) {
                return call_function_selector(play, server_links, links)
            }
        }
    }
}