#include <src/base_objects/shared_client_data.hpp>
#include <src/protocolHelper/packets/765/packets.hpp>
#include <src/protocolHelper/packets/766/packets.hpp>
#include <src/protocolHelper/packets/767/packets.hpp>
#include <src/protocolHelper/packets/abstract.hpp>

namespace copper_server {

    template <class fn>
    class function_selector;

    template <class Return, class... Arguments>
    class function_selector<Return(Arguments...)> {
        using fn = Return (*)(Arguments...);
        std::unordered_map<uint32_t, fn> functions;

    public:
        function_selector(std::initializer_list<std::pair<const uint32_t, fn>> list)
            : functions(list) {}

        Response call(SharedClientData& cl, Arguments... args) {
            return functions.at(cl.packets_state.protocol_version)(std::forward<Arguments>(args)...);
        }
    };

#define create_function_selector(_namespace, name)                   \
    function_selector<decltype(release_767::_namespace::name)> name{ \
        {765, release_765::_namespace::name},                        \
        {766, release_766::_namespace::name},                        \
        {767, release_767::_namespace::name},                        \
    };

#define create_function_selector_001(_namespace, name)               \
    function_selector<decltype(release_767::_namespace::name)> name{ \
        {767, release_767::_namespace::name},                        \
    };
#define create_function_selector_011(_namespace, name)               \
    function_selector<decltype(release_767::_namespace::name)> name{ \
        {766, release_766::_namespace::name},                        \
        {767, release_767::_namespace::name},                        \
    };

    namespace packets {
        namespace selectors {

            namespace login {
                create_function_selector(login, login);
                create_function_selector(login, kick);
                create_function_selector(login, disableCompression);
                create_function_selector(login, setCompression);
                create_function_selector_011(login, requestCookie);

                create_function_selector(login, loginSuccess);
                create_function_selector(login, encryptionRequest);
            }

            namespace configuration {
                create_function_selector_011(configuration, requestCookie);
                create_function_selector(configuration, configuration);

                create_function_selector(configuration, kick);

                create_function_selector(configuration, finish);

                create_function_selector(configuration, keep_alive);

                create_function_selector(configuration, ping);

                create_function_selector(configuration, registry_data);

                create_function_selector_011(configuration, resetChat);

                create_function_selector(configuration, removeResourcePacks);
                create_function_selector(configuration, removeResourcePack);
                create_function_selector(configuration, addResourcePack);
                create_function_selector(configuration, addResourcePackPrompted);

                create_function_selector_011(configuration, storeCookie);
                create_function_selector_011(configuration, transfer);

                create_function_selector(configuration, setFeatureFlags);

                create_function_selector(configuration, updateTags);

                create_function_selector_011(configuration, knownPacks);


                create_function_selector_001(configuration, custom_report);

                create_function_selector_001(configuration, server_links);
            }

            namespace play {
                create_function_selector(play, bundleResponse);

                create_function_selector(play, spawnEntity);
                create_function_selector(play, spawnExperienceOrb);
                create_function_selector(play, entityAnimation);
                create_function_selector(play, awardStatistics);
                create_function_selector(play, acknowledgeBlockChange);

                create_function_selector(play, setBlockDestroyStage);

                create_function_selector(play, blockEntityData);

                create_function_selector(play, blockAction);

                create_function_selector(play, blockUpdate);
                create_function_selector(play, bossBarAdd);
                create_function_selector(play, bossBarRemove);
                create_function_selector(play, bossBarUpdateHealth);
                create_function_selector(play, bossBarUpdateTitle);
                create_function_selector(play, bossBarUpdateStyle);


                create_function_selector(play, bossBarUpdateFlags);
                create_function_selector(play, changeDifficulty);
                create_function_selector(play, chunkBatchFinished);
                create_function_selector(play, chunkBatchStart);

                create_function_selector(play, chunkBiomes);
                create_function_selector(play, clearTitles);

                create_function_selector(play, commandSuggestionsResponse);
                create_function_selector(play, commands);
                create_function_selector(play, closeContainer);
                create_function_selector(play, setContainerContent);

                create_function_selector(play, setContainerProperty);

                create_function_selector(play, setContainerSlot);

                create_function_selector_011(play, cookieRequest);


                create_function_selector(play, setCooldown);


                create_function_selector(play, chatSuggestionsResponse);

                create_function_selector(play, customPayload);

                create_function_selector(play, damageEvent);

                create_function_selector_011(play, debugSample);

                create_function_selector(play, deleteMessageBySignature);

                create_function_selector(play, deleteMessageByID);

                create_function_selector(play, kick);

                create_function_selector(play, disguisedChatMessage);

                create_function_selector(play, entityEvent);


                create_function_selector(play, explosion);

                create_function_selector(play, unloadChunk);
                create_function_selector(play, gameEvent);
                create_function_selector(play, openHorseWindow);

                create_function_selector(play, hurtAnimation);

                create_function_selector(play, initializeWorldBorder);

                create_function_selector(play, keepAlive);
                create_function_selector(play, updateChunkDataWLights);

                create_function_selector(play, worldEvent);
                create_function_selector(play, particle);

                create_function_selector(play, updateLight);

                create_function_selector(play, joinGame);
                create_function_selector(play, mapData);
                create_function_selector(play, merchantOffers);
                create_function_selector(play, updateEntityPosition);
                create_function_selector(play, updateEntityPositionAndRotation);
                create_function_selector(play, updateEntityRotation);
                create_function_selector(play, moveVehicle);
                create_function_selector(play, openBook);
                create_function_selector(play, openScreen);
                create_function_selector(play, openSignEditor);
                create_function_selector(play, ping);
                create_function_selector(play, pingResponse);
                create_function_selector(play, placeGhostRecipe);
                create_function_selector(play, playerAbilities);
                create_function_selector(play, playerChatMessage);

                create_function_selector(play, endCombat);

                create_function_selector(play, enterCombat);
                create_function_selector(play, combatDeath);
                create_function_selector(play, playerInfoRemove);
                create_function_selector(play, playerInfoAdd);
                create_function_selector(play, playerInfoInitializeChat);
                create_function_selector(play, playerInfoUpdateGameMode);
                create_function_selector(play, playerInfoUpdateListed);
                create_function_selector(play, playerInfoUpdateLatency);
                create_function_selector(play, playerInfoUpdateDisplayName);
                create_function_selector(play, lookAt);
                create_function_selector(play, synchronizePlayerPosition);
                create_function_selector(play, initRecipeBook);
                create_function_selector(play, addRecipeBook);
                create_function_selector(play, removeRecipeBook);
                create_function_selector(play, removeEntities);
                create_function_selector(play, removeEntityEffect);
                create_function_selector(play, resetScore);
                create_function_selector(play, removeResourcePacks);
                create_function_selector(play, removeResourcePack);
                create_function_selector(play, addResourcePack);
                create_function_selector(play, respawn);
                create_function_selector(play, setHeadRotation);

                create_function_selector(play, updateSectionBlocks);

                create_function_selector(play, setAdvancementsTab);
                create_function_selector(play, serverData);
                create_function_selector(play, setActionBarText);
                create_function_selector(play, setBorderCenter);
                create_function_selector(play, setBorderLerp);
                create_function_selector(play, setBorderSize);
                create_function_selector(play, setBorderWarningDelay);
                create_function_selector(play, setBorderWarningDistance);
                create_function_selector(play, setCamera);
                create_function_selector(play, setHeldItem);
                create_function_selector(play, setCenterChunk);
                create_function_selector(play, setRenderDistance);
                create_function_selector(play, setDefaultSpawnPosition);
                create_function_selector(play, displayObjective);
                create_function_selector(play, setEntityMetadata);
                create_function_selector(play, linkEntities);
                create_function_selector(play, setEntityVelocity);

                create_function_selector(play, setEquipment);

                create_function_selector(play, setExperience);

                create_function_selector(play, setHealth);

                create_function_selector(play, updateObjectivesCreate);
                create_function_selector(play, updateObjectivesCreateStyled);
                create_function_selector(play, updateObjectivesCreateFixed);
                create_function_selector(play, updateObjectivesRemove);
                create_function_selector(play, updateObjectivesInfo);
                create_function_selector(play, updateObjectivesInfoStyled);
                create_function_selector(play, updateObjectivesInfoFixed);

                create_function_selector(play, setPassengers);


                create_function_selector(play, updateTeamCreate);
                create_function_selector(play, updateTeamRemove);
                create_function_selector(play, updateTeamInfo);
                create_function_selector(play, updateTeamAddEntities);
                create_function_selector(play, updateTeamRemoveEntities);


                create_function_selector(play, setScore);
                create_function_selector(play, setScoreStyled);
                create_function_selector(play, setScoreFixed);

                create_function_selector(play, setSimulationDistance);
                create_function_selector(play, setSubtitleText);
                create_function_selector(play, updateTime);
                create_function_selector(play, setTitleText);
                create_function_selector(play, setTitleAnimationTimes);


                create_function_selector(play, entitySoundEffect);
                create_function_selector(play, entitySoundEffectCustom);

                create_function_selector(play, soundEffect);
                create_function_selector(play, soundEffectCustom);


                create_function_selector(play, startConfiguration);
                create_function_selector(play, stopSound);
                create_function_selector(play, stopSoundBySource);
                create_function_selector(play, stopSoundBySound);
                create_function_selector(play, stopSoundBySourceAndSound);

                create_function_selector_011(play, storeCookie);

                create_function_selector(play, systemChatMessage);
                create_function_selector(play, systemChatMessageOverlay);
                create_function_selector(play, setTabListHeaderAndFooter);
                create_function_selector(play, tagQueryResponse);
                create_function_selector(play, pickupItem);
                create_function_selector(play, teleportEntity);
                create_function_selector(play, setTickingState);
                create_function_selector(play, stepTick);

                create_function_selector_011(play, transfer);

                create_function_selector(play, updateAdvancements);
                create_function_selector(play, updateAttributes);

                create_function_selector_011(play, entityEffect);

                create_function_selector(play, updateRecipes);

                create_function_selector(play, updateTags);

                create_function_selector_011(play, projectilePower);

                create_function_selector_001(play, custom_report);
                create_function_selector_001(play, server_links);
            }
        }

        namespace login {
            Response login(SharedClientData& client, int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data) {
                return selectors::login::login.call(client, plugin_message_id, chanel, data);
            }

            Response kick(SharedClientData& client, const Chat& reason) {
                return selectors::login::kick.call(client, reason);
            };

            Response disableCompression(SharedClientData& client) {
                return selectors::login::disableCompression.call(client);
            };

            Response setCompression(SharedClientData& client, int32_t threshold) {
                return selectors::login::setCompression.call(client, threshold);
            };

            Response requestCookie(SharedClientData& client, const std::string& key) {
                return selectors::login::requestCookie.call(client, key);
            };

            Response loginSuccess(SharedClientData& client) {
                return selectors::login::loginSuccess.call(client, client);
            };

            Response encryptionRequest(SharedClientData& client, const std::string& server_id, uint8_t (&verify_token)[4]) {
                return selectors::login::encryptionRequest.call(client, server_id, verify_token);
            };


        }

        namespace configuration {
            Response requestCookie(SharedClientData& client, const std::string& key) {
                return selectors::configuration::requestCookie.call(client, key);
            }

            Response configuration(SharedClientData& client, const std::string& chanel, const list_array<uint8_t>& data) {
                return selectors::configuration::configuration.call(client, chanel, data);
            }

            Response kick(SharedClientData& client, const Chat& reason) {
                return selectors::configuration::kick.call(client, reason);
            }

            Response finish(SharedClientData& client) {
                return selectors::configuration::finish.call(client);
            }

            Response keep_alive(SharedClientData& client, int64_t keep_alive_packet) {
                return selectors::configuration::keep_alive.call(client, keep_alive_packet);
            }

            Response ping(SharedClientData& client, int32_t excepted_pong) {
                return selectors::configuration::ping.call(client, excepted_pong);
            }

            Response registry_data(SharedClientData& client) {
                return selectors::configuration::registry_data.call(client);
            }

            Response resetChat(SharedClientData& client) {
                return selectors::configuration::resetChat.call(client);
            }

            Response removeResourcePacks(SharedClientData& client) {
                return selectors::configuration::removeResourcePacks.call(client);
            }

            Response removeResourcePack(SharedClientData& client, const enbt::raw_uuid& pack_id) {
                return selectors::configuration::removeResourcePack.call(client, pack_id);
            }

            Response addResourcePack(SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced) {
                return selectors::configuration::addResourcePack.call(client, client, pack_id, url, hash, forced);
            }

            Response addResourcePackPrompted(SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt) {
                return selectors::configuration::addResourcePackPrompted.call(client, client, pack_id, url, hash, forced, prompt);
            };

            Response storeCookie(SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload) {
                return selectors::configuration::storeCookie.call(client, key, payload);
            }

            Response transfer(SharedClientData& client, const std::string& host, int32_t port) {
                return selectors::configuration::transfer.call(client, host, port);
            }

            Response setFeatureFlags(SharedClientData& client, const list_array<std::string>& features) {
                return selectors::configuration::setFeatureFlags.call(client, features);
            }

            Response updateTags(SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tags_entries) {
                return selectors::configuration::updateTags.call(client, tags_entries);
            }

            Response knownPacks(SharedClientData& client, const list_array<base_objects::packets::known_pack>& packs) {
                return selectors::configuration::knownPacks.call(client, packs);
            }

            Response custom_report(SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values) {
                return selectors::configuration::custom_report.call(client, values);
            }

            Response server_links(SharedClientData& client, const list_array<base_objects::packets::server_link>& links) {
                return selectors::configuration::server_links.call(client, links);
            }
        }

        namespace play {
            Response bundleResponse(SharedClientData& client, Response&& response) {
                return selectors::play::bundleResponse.call(client, std::move(response));
            }

            Response spawnEntity(SharedClientData& client, const base_objects::entity& entity, uint16_t protocol) {
                return selectors::play::spawnEntity.call(client, entity, 1);
            }

            Response spawnExperienceOrb(SharedClientData& client, const base_objects::entity& entity, int16_t count) {
                return selectors::play::spawnExperienceOrb.call(client, entity, count);
            }

            Response entityAnimation(SharedClientData& client, const base_objects::entity& entity, uint8_t animation) {
                return selectors::play::entityAnimation.call(client, entity, animation);
            }

            Response awardStatistics(SharedClientData& client, const list_array<base_objects::packets::statistics>& statistics) {
                return selectors::play::awardStatistics.call(client, statistics);
            }

            Response acknowledgeBlockChange(SharedClientData& client) {
                return selectors::play::acknowledgeBlockChange.call(client, client);
            }

            Response setBlockDestroyStage(SharedClientData& client, const base_objects::entity& entity, Position block, uint8_t stage) {
                return selectors::play::setBlockDestroyStage.call(client, entity, block, stage);
            }

            Response blockEntityData(SharedClientData& client, Position block, int32_t type, const enbt::value& data) {
                return selectors::play::blockEntityData.call(client, block, type, data);
            }

            Response blockAction(SharedClientData& client, Position block, int32_t action_id, int32_t param, int32_t block_type) {
                return selectors::play::blockAction.call(client, block, action_id, param, block_type);
            }

            Response blockUpdate(SharedClientData& client, Position block, int32_t block_type) {
                return selectors::play::blockUpdate.call(client, block, block_type);
            }

            Response bossBarAdd(SharedClientData& client, const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags) {
                return selectors::play::bossBarAdd.call(client, id, title, health, color, division, flags);
            }

            Response bossBarRemove(SharedClientData& client, const enbt::raw_uuid& id) {
                return selectors::play::bossBarRemove.call(client, id);
            }

            Response bossBarUpdateHealth(SharedClientData& client, const enbt::raw_uuid& id, float health) {
                return selectors::play::bossBarUpdateHealth.call(client, id, health);
            }

            Response bossBarUpdateTitle(SharedClientData& client, const enbt::raw_uuid& id, const Chat& title) {
                return selectors::play::bossBarUpdateTitle.call(client, id, title);
            }

            Response bossBarUpdateStyle(SharedClientData& client, const enbt::raw_uuid& id, int32_t color, int32_t division) {
                return selectors::play::bossBarUpdateStyle.call(client, id, color, division);
            }

            Response bossBarUpdateFlags(SharedClientData& client, const enbt::raw_uuid& id, uint8_t flags) {
                return selectors::play::bossBarUpdateFlags.call(client, id, flags);
            }

            Response changeDifficulty(SharedClientData& client, uint8_t difficulty, bool locked) {
                return selectors::play::changeDifficulty.call(client, difficulty, locked);
            }

            Response chunkBatchFinished(SharedClientData& client, int32_t count) {
                return selectors::play::chunkBatchFinished.call(client, count);
            }

            Response chunkBatchStart(SharedClientData& client) {
                return selectors::play::chunkBatchStart.call(client);
            }

            Response chunkBiomes(SharedClientData& client, list_array<base_objects::chunk::chunk_biomes>& chunk) {
                return selectors::play::chunkBiomes.call(client, chunk);
            }

            Response clearTitles(SharedClientData& client, bool reset) {
                return selectors::play::clearTitles.call(client, reset);
            }

            Response commandSuggestionsResponse(SharedClientData& client, int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions) {
                return selectors::play::commandSuggestionsResponse.call(client, transaction_id, start_pos, length, suggestions);
            }

            Response commands(SharedClientData& client, int32_t root_id, const list_array<base_objects::packets::command_node>& nodes) {
                return selectors::play::commands.call(client, root_id, nodes);
            }

            Response closeContainer(SharedClientData& client, uint8_t container_id) {
                return selectors::play::closeContainer.call(client, container_id);
            }

            Response setContainerContent(SharedClientData& client, uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item) {
                return selectors::play::setContainerContent.call(client, windows_id, state_id, slots, carried_item);
            }

            Response setContainerProperty(SharedClientData& client, uint8_t windows_id, uint16_t property, uint16_t value) {
                return selectors::play::setContainerProperty.call(client, windows_id, property, value);
            }

            Response setContainerSlot(SharedClientData& client, uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item) {
                return selectors::play::setContainerSlot.call(client, windows_id, state_id, slot, item);
            }

            Response cookieRequest(SharedClientData& client, const std::string& key) {
                return selectors::play::cookieRequest.call(client, key);
            }

            Response setCooldown(SharedClientData& client, int32_t item_id, int32_t cooldown) {
                return selectors::play::setCooldown.call(client, item_id, cooldown);
            }

            Response chatSuggestionsResponse(SharedClientData& client, int32_t action, int32_t count, const list_array<std::string>& suggestions) {
                return selectors::play::chatSuggestionsResponse.call(client, action, count, suggestions);
            }

            Response customPayload(SharedClientData& client, const std::string& channel, const list_array<uint8_t>& data) {
                return selectors::play::customPayload.call(client, channel, data);
            }

            Response damageEvent(SharedClientData& client, int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<calc::VECTOR> xyz) {
                return selectors::play::damageEvent.call(client, entity_id, source_type_id, source_cause_id, source_direct_id, xyz);
            }

            Response debugSample(SharedClientData& client, const list_array<uint64_t>& sample, int32_t sample_type) {
                return selectors::play::debugSample.call(client, sample, sample_type);
            }

            Response deleteMessageBySignature(SharedClientData& client, uint8_t (&signature)[256]) {
                return selectors::play::deleteMessageBySignature.call(client, signature);
            }

            Response deleteMessageByID(SharedClientData& client, int32_t message_id) {
                return selectors::play::deleteMessageByID.call(client, message_id);
            }

            Response kick(SharedClientData& client, const Chat& reason) {
                return selectors::play::kick.call(client, reason);
            }

            Response disguisedChatMessage(SharedClientData& client, const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name) {
                return selectors::play::disguisedChatMessage.call(client, message, chat_type, sender, target_name);
            }

            Response entityEvent(SharedClientData& client, int32_t entity_id, uint8_t entity_status) {
                return selectors::play::entityEvent.call(client, entity_id, entity_status);
            }

            Response explosion(SharedClientData& client, calc::VECTOR pos, float strength, const list_array<calc::XYZ<int8_t>>& affected_blocks, calc::VECTOR player_motion, int32_t block_interaction, int32_t small_explosion_particle_id, const base_objects::particle_data& small_explosion_particle_data, int32_t large_explosion_particle_id, const base_objects::particle_data& large_explosion_particle_data, const std::string& sound_name, std::optional<float> fixed_range) {
                return selectors::play::explosion.call(client, pos, strength, affected_blocks, player_motion, block_interaction, small_explosion_particle_id, small_explosion_particle_data, large_explosion_particle_id, large_explosion_particle_data, sound_name, fixed_range);
            }

            Response unloadChunk(SharedClientData& client, int32_t x, int32_t z) {
                return selectors::play::unloadChunk.call(client, x, z);
            }

            Response gameEvent(SharedClientData& client, uint8_t event_id, float value) {
                return selectors::play::gameEvent.call(client, event_id, value);
            }

            Response openHorseWindow(SharedClientData& client, uint8_t window_id, int32_t slots, int32_t entity_id) {
                return selectors::play::openHorseWindow.call(client, window_id, slots, entity_id);
            }

            Response hurtAnimation(SharedClientData& client, int32_t entity_id, float yaw) {
                return selectors::play::hurtAnimation.call(client, entity_id, yaw);
            }

            Response initializeWorldBorder(SharedClientData& client, double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time) {
                return selectors::play::initializeWorldBorder.call(client, x, z, old_diameter, new_diameter, speed_ms, portal_teleport_boundary, warning_blocks, warning_time);
            }

            Response keepAlive(SharedClientData& client, int64_t id) {
                return selectors::play::keepAlive.call(client, id);
            }

            Response updateChunkDataWLights(SharedClientData& client, int32_t chunk_x, int32_t chunk_z, const NBT& heightmaps, const std::vector<uint8_t> data, const bit_list_array<>& sky_light_mask, const bit_list_array<>& block_light_mask, const bit_list_array<>& empty_skylight_mask, const bit_list_array<>& empty_block_light_mask, const list_array<std::vector<uint8_t>> sky_light_arrays, const list_array<std::vector<uint8_t>> block_light_arrays) {
                return selectors::play::updateChunkDataWLights.call(client, chunk_x, chunk_z, heightmaps, data, sky_light_mask, block_light_mask, block_light_mask, empty_skylight_mask, sky_light_arrays, block_light_arrays);
            }

            Response worldEvent(SharedClientData& client, int32_t event, Position pos, int32_t data, bool global) {
                return selectors::play::worldEvent.call(client, event, pos, data, global);
            }

            Response particle(SharedClientData& client, int32_t particle_id, bool long_distance, calc::VECTOR pos, calc::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data) {
                return selectors::play::particle.call(client, particle_id, long_distance, pos, offset, max_speed, count, data);
            }

            Response updateLight(SharedClientData& client, int32_t chunk_x, int32_t chunk_z, const bit_list_array<>& sky_light_mask, const bit_list_array<>& block_light_mask, const bit_list_array<>& empty_skylight_mask, const bit_list_array<>& empty_block_light_mask, const list_array<std::vector<uint8_t>> sky_light_arrays, const list_array<std::vector<uint8_t>> block_light_arrays) {
                return selectors::play::updateLight.call(client, chunk_x, chunk_z, sky_light_mask, block_light_mask, empty_skylight_mask, empty_block_light_mask, sky_light_arrays, block_light_arrays);
            }

            Response joinGame(SharedClientData& client, int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, std::optional<base_objects::packets::death_location_data> death_location, int32_t portal_cooldown, bool enforces_secure_chat) {
                return selectors::play::joinGame.call(client, entity_id, is_hardcore, dimension_names, max_players, view_distance, simulation_distance, reduced_debug_info, enable_respawn_screen, do_limited_crafting, current_dimension_type, dimension_name, hashed_seed, gamemode, prev_gamemode, is_debug, is_flat, death_location, portal_cooldown, enforces_secure_chat);
            }

            Response mapData(SharedClientData& client, int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t column, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data) {
                return selectors::play::mapData.call(client, map_id, scale, locked, icons, column, rows, x, z, data);
            }

            Response merchantOffers(SharedClientData& client, int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade> trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock) {
                return selectors::play::merchantOffers.call(client, window_id, trade_id, trades, level, experience, regular_villager, can_restock);
            }

            Response updateEntityPosition(SharedClientData& client, int32_t entity_id, calc::XYZ<float> pos, bool on_ground) {
                return selectors::play::updateEntityPosition.call(client, entity_id, pos, on_ground);
            }

            Response updateEntityPositionAndRotation(SharedClientData& client, int32_t entity_id, calc::XYZ<float> pos, calc::VECTOR rot, bool on_ground) {
                return selectors::play::updateEntityPositionAndRotation.call(client, entity_id, pos, rot, on_ground);
            }

            Response updateEntityRotation(SharedClientData& client, int32_t entity_id, calc::VECTOR rot, bool on_ground) {
                return selectors::play::updateEntityRotation.call(client, entity_id, rot, on_ground);
            }

            Response moveVehicle(SharedClientData& client, calc::VECTOR pos, calc::VECTOR rot) {
                return selectors::play::moveVehicle.call(client, pos, rot);
            }

            Response openBook(SharedClientData& client, int32_t hand) {
                return selectors::play::openBook.call(client, hand);
            }

            Response openScreen(SharedClientData& client, int32_t window_id, int32_t type, const Chat& title) {
                return selectors::play::openScreen.call(client, window_id, type, title);
            }

            Response openSignEditor(SharedClientData& client, Position pos, bool is_front_text) {
                return selectors::play::openSignEditor.call(client, pos, is_front_text);
            }

            Response ping(SharedClientData& client, int32_t id) {
                return selectors::play::ping.call(client, id);
            }

            Response pingResponse(SharedClientData& client, int32_t id) {
                return selectors::play::pingResponse.call(client, id);
            }

            Response placeGhostRecipe(SharedClientData& client, int32_t windows_id, const std::string& recipe_id) {
                return selectors::play::placeGhostRecipe.call(client, windows_id, recipe_id);
            }

            Response playerAbilities(SharedClientData& client, uint8_t flags, float flying_speed, float field_of_view) {
                return selectors::play::playerAbilities.call(client, flags, flying_speed, field_of_view);
            }

            Response playerChatMessage(SharedClientData& client, enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, std::optional<enbt::value> __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name) {
                return selectors::play::playerChatMessage.call(client, sender, index, signature, message, timestamp, salt, prev_messages, __UNDEFINED__FIELD__, filter_type, filtered_symbols_bitfield, chat_type, sender_name, target_name);
            }

            Response endCombat(SharedClientData& client, int32_t duration) {
                return selectors::play::endCombat.call(client, duration);
            }

            Response enterCombat(SharedClientData& client) {
                return selectors::play::enterCombat.call(client);
            }

            Response combatDeath(SharedClientData& client, int32_t player_id, const Chat& message) {
                return selectors::play::combatDeath.call(client, player_id, message);
            }

            Response playerInfoRemove(SharedClientData& client, const list_array<enbt::raw_uuid>& players) {
                return selectors::play::playerInfoRemove.call(client, players);
            }

            Response playerInfoAdd(SharedClientData& client, const list_array<base_objects::packets::player_actions_add>& add_players) {
                return selectors::play::playerInfoAdd.call(client, add_players);
            }

            Response playerInfoInitializeChat(SharedClientData& client, const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat) {
                return selectors::play::playerInfoInitializeChat.call(client, initialize_chat);
            }

            Response playerInfoUpdateGameMode(SharedClientData& client, const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode) {
                return selectors::play::playerInfoUpdateGameMode.call(client, update_game_mode);
            }

            Response playerInfoUpdateListed(SharedClientData& client, const list_array<base_objects::packets::player_actions_update_listed>& update_listed) {
                return selectors::play::playerInfoUpdateListed.call(client, update_listed);
            }

            Response playerInfoUpdateLatency(SharedClientData& client, const list_array<base_objects::packets::player_actions_update_latency>& update_latency) {
                return selectors::play::playerInfoUpdateLatency.call(client, update_latency);
            }

            Response playerInfoUpdateDisplayName(SharedClientData& client, const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name) {
                return selectors::play::playerInfoUpdateDisplayName.call(client, update_display_name);
            }

            Response lookAt(SharedClientData& client, bool from_feet_or_eyes, calc::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id) {
                return selectors::play::lookAt.call(client, from_feet_or_eyes, target, entity_id);
            }

            Response synchronizePlayerPosition(SharedClientData& client, calc::VECTOR pos, float yaw, float pitch, uint8_t flags) {
                auto id = client.packets_state.teleport_id_sequence++;
                client.packets_state.pending_teleport_ids.push_back(id);
                return selectors::play::synchronizePlayerPosition.call(client, pos, yaw, pitch, flags, id);
            }

            Response initRecipeBook(SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> displayed_recipe_ids, list_array<std::string> had_access_to_recipe_ids) {
                return selectors::play::initRecipeBook.call(client, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, displayed_recipe_ids, had_access_to_recipe_ids);
            }

            Response addRecipeBook(SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids) {
                return selectors::play::addRecipeBook.call(client, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids);
            }

            Response removeRecipeBook(SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids) {
                return selectors::play::removeRecipeBook.call(client, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids);
            }

            Response removeEntities(SharedClientData& client, const list_array<int32_t>& entity_ids) {
                return selectors::play::removeEntities.call(client, entity_ids);
            }

            Response removeEntityEffect(SharedClientData& client, int32_t entity_id, int32_t effect_id) {
                return selectors::play::removeEntityEffect.call(client, entity_id, effect_id);
            }

            Response resetScore(SharedClientData& client, const std::string& entity_name, const std::optional<std::string>& objective_name) {
                return selectors::play::resetScore.call(client, entity_name, objective_name);
            }

            Response removeResourcePacks(SharedClientData& client) {
                return selectors::play::removeResourcePacks.call(client);
            }

            Response removeResourcePack(SharedClientData& client, enbt::raw_uuid id) {
                return selectors::play::removeResourcePack.call(client, id);
            }

            Response addResourcePack(SharedClientData& client, enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt) {
                return selectors::play::addResourcePack.call(client, id, url, hash, forced, prompt);
            }

            Response respawn(SharedClientData& client, int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata) {
                return selectors::play::respawn.call(client, dimension_type, dimension_name, hashed_seed, gamemode, previous_gamemode, is_debug, is_flat, death_location, portal_cooldown, keep_attributes, keep_metadata);
            }

            Response setHeadRotation(SharedClientData& client, int32_t entity_id, calc::VECTOR head_rotation) {
                return selectors::play::setHeadRotation.call(client, entity_id, head_rotation);
            }

            Response updateSectionBlocks(SharedClientData& client, int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks) {
                return selectors::play::updateSectionBlocks.call(client, section_x, section_z, section_y, blocks);
            }

            Response setAdvancementsTab(SharedClientData& client, const std::optional<std::string>& tab_id) {
                return selectors::play::setAdvancementsTab.call(client, tab_id);
            }

            Response serverData(SharedClientData& client, const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool __ignored) {
                return selectors::play::serverData.call(client, motd, icon_png, false);
            }

            Response setActionBarText(SharedClientData& client, const Chat& text) {
                return selectors::play::setActionBarText.call(client, text);
            }

            Response setBorderCenter(SharedClientData& client, double x, double z) {
                return selectors::play::setBorderCenter.call(client, x, z);
            }

            Response setBorderLerp(SharedClientData& client, double old_diameter, double new_diameter, int64_t speed_ms) {
                return selectors::play::setBorderLerp.call(client, old_diameter, new_diameter, speed_ms);
            }

            Response setBorderSize(SharedClientData& client, double diameter) {
                return selectors::play::setBorderSize.call(client, diameter);
            }

            Response setBorderWarningDelay(SharedClientData& client, int32_t warning_delay) {
                return selectors::play::setBorderWarningDelay.call(client, warning_delay);
            }

            Response setBorderWarningDistance(SharedClientData& client, int32_t warning_distance) {
                return selectors::play::setBorderWarningDistance.call(client, warning_distance);
            }

            Response setCamera(SharedClientData& client, int32_t entity_id) {
                return selectors::play::setCamera.call(client, entity_id);
            }

            Response setHeldItem(SharedClientData& client, uint8_t slot) {
                return selectors::play::setHeldItem.call(client, slot);
            }

            Response setCenterChunk(SharedClientData& client, int32_t x, int32_t z) {
                return selectors::play::setCenterChunk.call(client, x, z);
            }

            Response setRenderDistance(SharedClientData& client, int32_t render_distance) {
                return selectors::play::setRenderDistance.call(client, render_distance);
            }

            Response setDefaultSpawnPosition(SharedClientData& client, Position pos, float angle) {
                return selectors::play::setDefaultSpawnPosition.call(client, pos, angle);
            }

            Response displayObjective(SharedClientData& client, int32_t position, const std::string& objective_name) {
                return selectors::play::displayObjective.call(client, position, objective_name);
            }

            Response setEntityMetadata(SharedClientData& client, int32_t entity_id, const list_array<uint8_t>& metadata) {
                return selectors::play::setEntityMetadata.call(client, entity_id, metadata);
            }

            Response linkEntities(SharedClientData& client, int32_t attached_entity_id, int32_t holder_entity_id) {
                return selectors::play::linkEntities.call(client, attached_entity_id, holder_entity_id);
            }

            Response setEntityVelocity(SharedClientData& client, int32_t entity_id, calc::VECTOR velocity) {
                return selectors::play::setEntityVelocity.call(client, entity_id, velocity);
            }

            Response setEquipment(SharedClientData& client, int32_t entity_id, uint8_t slot, const base_objects::slot& item) {
                return selectors::play::setEquipment.call(client, entity_id, slot, item);
            }

            Response setExperience(SharedClientData& client, float experience_bar, int32_t level, int32_t total_experience) {
                return selectors::play::setExperience.call(client, experience_bar, level, total_experience);
            }

            Response setHealth(SharedClientData& client, float health, int32_t food, float saturation) {
                return selectors::play::setHealth.call(client, health, food, saturation);
            }

            Response updateObjectivesCreate(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type) {
                return selectors::play::updateObjectivesCreate.call(client, objective_name, display_name, render_type);
            }

            Response updateObjectivesCreateStyled(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
                return selectors::play::updateObjectivesCreateStyled.call(client, objective_name, display_name, render_type, style);
            }

            Response updateObjectivesCreateFixed(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
                return selectors::play::updateObjectivesCreateFixed.call(client, objective_name, display_name, render_type, content);
            }

            Response updateObjectivesRemove(SharedClientData& client, const std::string& objective_name) {
                return selectors::play::updateObjectivesRemove.call(client, objective_name);
            }

            Response updateObjectivesInfo(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type) {
                return selectors::play::updateObjectivesInfo.call(client, objective_name, display_name, render_type);
            }

            Response updateObjectivesInfoStyled(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
                return selectors::play::updateObjectivesInfoStyled.call(client, objective_name, display_name, render_type, style);
            }

            Response updateObjectivesInfoFixed(SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
                return selectors::play::updateObjectivesInfoFixed.call(client, objective_name, display_name, render_type, content);
            }

            Response setPassengers(SharedClientData& client, int32_t vehicle_entity_id, const list_array<int32_t>& passengers) {
                return selectors::play::setPassengers.call(client, vehicle_entity_id, passengers);
            }

            Response updateTeamCreate(SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities) {
                return selectors::play::updateTeamCreate.call(client, team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix, entities);
            }

            Response updateTeamRemove(SharedClientData& client, const std::string& team_name) {
                return selectors::play::updateTeamRemove.call(client, team_name);
            }

            Response updateTeamInfo(SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix) {
                return selectors::play::updateTeamInfo.call(client, team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix);
            }

            Response updateTeamAddEntities(SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities) {
                return selectors::play::updateTeamAddEntities.call(client, team_name, entities);
            }

            Response updateTeamRemoveEntities(SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities) {
                return selectors::play::updateTeamRemoveEntities.call(client, team_name, entities);
            }

            Response setScore(SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name) {
                return selectors::play::setScore.call(client, entity_name, objective_name, value, display_name);
            }

            Response setScoreStyled(SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled) {
                return selectors::play::setScoreStyled.call(client, entity_name, objective_name, value, display_name, styled);
            }

            Response setScoreFixed(SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content) {
                return selectors::play::setScoreFixed.call(client, entity_name, objective_name, value, display_name, content);
            }

            Response setSimulationDistance(SharedClientData& client, int32_t distance) {
                return selectors::play::setSimulationDistance.call(client, distance);
            }

            Response setSubtitleText(SharedClientData& client, const Chat& text) {
                return selectors::play::setSubtitleText.call(client, text);
            }

            Response updateTime(SharedClientData& client, int64_t world_age, int64_t time_of_day) {
                return selectors::play::updateTime.call(client, world_age, time_of_day);
            }

            Response setTitleText(SharedClientData& client, const Chat& text) {
                return selectors::play::setTitleText.call(client, text);
            }

            Response setTitleAnimationTimes(SharedClientData& client, int32_t fade_in, int32_t stay, int32_t fade_out) {
                return selectors::play::setTitleAnimationTimes.call(client, fade_in, stay, fade_out);
            }

            Response entitySoundEffect(SharedClientData& client, uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
                return selectors::play::entitySoundEffect.call(client, sound_id, category, entity_id, volume, pitch, seed);
            }

            Response entitySoundEffectCustom(SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
                return selectors::play::entitySoundEffectCustom.call(client, sound_id, range, category, entity_id, volume, pitch, seed);
            }

            Response soundEffect(SharedClientData& client, uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
                return selectors::play::soundEffect.call(client, sound_id, category, x, y, z, volume, pitch, seed);
            }

            Response soundEffectCustom(SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
                return selectors::play::soundEffectCustom.call(client, sound_id, range, category, x, y, z, volume, pitch, seed);
            }

            Response startConfiguration(SharedClientData& client) {
                return selectors::play::startConfiguration.call(client);
            }

            Response stopSound(SharedClientData& client, uint8_t flags) {
                return selectors::play::stopSound.call(client, flags);
            }

            Response stopSoundBySource(SharedClientData& client, uint8_t flags, int32_t source) {
                return selectors::play::stopSoundBySource.call(client, flags, source);
            }

            Response stopSoundBySound(SharedClientData& client, uint8_t flags, const std::string& sound) {
                return selectors::play::stopSoundBySound.call(client, flags, sound);
            }

            Response stopSoundBySourceAndSound(SharedClientData& client, uint8_t flags, int32_t source, const std::string& sound) {
                return selectors::play::stopSoundBySourceAndSound.call(client, flags, source, sound);
            }

            Response storeCookie(SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload) {
                return selectors::play::storeCookie.call(client, key, payload);
            }

            Response systemChatMessage(SharedClientData& client, const Chat& message) {
                return selectors::play::systemChatMessage.call(client, message);
            }

            Response systemChatMessageOverlay(SharedClientData& client, const Chat& message) {
                return selectors::play::systemChatMessageOverlay.call(client, message);
            }

            Response setTabListHeaderAndFooter(SharedClientData& client, const Chat& header, const Chat& footer) {
                return selectors::play::setTabListHeaderAndFooter.call(client, header, footer);
            }

            Response tagQueryResponse(SharedClientData& client, int32_t transaction_id, const enbt::value& nbt) {
                return selectors::play::tagQueryResponse.call(client, transaction_id, nbt);
            }

            Response pickupItem(SharedClientData& client, int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count) {
                return selectors::play::pickupItem.call(client, collected_entity_id, collector_entity_id, pickup_item_count);
            }

            Response teleportEntity(SharedClientData& client, int32_t entity_id, calc::VECTOR pos, float yaw, float pitch, bool on_ground) {
                return selectors::play::teleportEntity.call(client, entity_id, pos, yaw, pitch, on_ground);
            }

            Response setTickingState(SharedClientData& client, float tick_rate, bool is_frozen) {
                return selectors::play::setTickingState.call(client, tick_rate, is_frozen);
            }

            Response stepTick(SharedClientData& client, int32_t step_count) {
                return selectors::play::stepTick.call(client, step_count);
            }

            Response transfer(SharedClientData& client, const std::string& host, int32_t port) {
                return selectors::play::transfer.call(client, host, port);
            }

            Response updateAdvancements(SharedClientData& client, bool reset, const list_array<base_objects::packets::advancements_maping> advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress> progress_advancements) {
                return selectors::play::updateAdvancements.call(client, reset, advancement_mapping, remove_advancements, progress_advancements);
            }

            Response updateAttributes(SharedClientData& client, int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) {
                return selectors::play::updateAttributes.call(client, entity_id, properties);
            }

            Response entityEffect(SharedClientData& client, int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, int8_t flags) {
                return selectors::play::entityEffect.call(client, entity_id, effect_id, amplifier, duration, flags);
            }

            Response updateRecipes(SharedClientData& client, const std::vector<base_objects::recipe>& recipes) {
                return selectors::play::updateRecipes.call(client, recipes);
            }

            Response updateTags(SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tag_mappings) {
                return selectors::play::updateTags.call(client, tag_mappings);
            }

            Response projectilePower(SharedClientData& client, int32_t entity_id, double power_x, double power_y, double power_z) {
                return selectors::play::projectilePower.call(client, entity_id, power_x, power_y, power_z);
            }

            Response custom_report(SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values) {
                return selectors::play::custom_report.call(client, values);
            }

            Response server_links(SharedClientData& client, const list_array<base_objects::packets::server_link>& links) {
                return selectors::play::server_links.call(client, links);
            }
        }
    }
}