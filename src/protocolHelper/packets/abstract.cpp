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

        base_objects::network::response call(base_objects::SharedClientData& cl, Arguments... args) {
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
            base_objects::network::response login(base_objects::SharedClientData& client, int32_t plugin_message_id, const std::string& chanel, const list_array<uint8_t>& data) {
                return selectors::login::login.call(client, plugin_message_id, chanel, data);
            }

            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason) {
                return selectors::login::kick.call(client, reason);
            };

            base_objects::network::response disableCompression(base_objects::SharedClientData& client) {
                return selectors::login::disableCompression.call(client);
            };

            base_objects::network::response setCompression(base_objects::SharedClientData& client, int32_t threshold) {
                return selectors::login::setCompression.call(client, threshold);
            };

            base_objects::network::response requestCookie(base_objects::SharedClientData& client, const std::string& key) {
                return selectors::login::requestCookie.call(client, key);
            };

            base_objects::network::response loginSuccess(base_objects::SharedClientData& client) {
                return selectors::login::loginSuccess.call(client, client);
            };

            base_objects::network::response encryptionRequest(base_objects::SharedClientData& client, const std::string& server_id, uint8_t (&verify_token)[4]) {
                return selectors::login::encryptionRequest.call(client, server_id, verify_token);
            };
        }

        namespace configuration {
            base_objects::network::response requestCookie(base_objects::SharedClientData& client, const std::string& key) {
                return selectors::configuration::requestCookie.call(client, key);
            }

            base_objects::network::response configuration(base_objects::SharedClientData& client, const std::string& chanel, const list_array<uint8_t>& data) {
                return selectors::configuration::configuration.call(client, chanel, data);
            }

            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason) {
                return selectors::configuration::kick.call(client, reason);
            }

            base_objects::network::response finish(base_objects::SharedClientData& client) {
                return selectors::configuration::finish.call(client);
            }

            base_objects::network::response keep_alive(base_objects::SharedClientData& client, int64_t keep_alive_packet) {
                return selectors::configuration::keep_alive.call(client, keep_alive_packet);
            }

            base_objects::network::response ping(base_objects::SharedClientData& client, int32_t excepted_pong) {
                return selectors::configuration::ping.call(client, excepted_pong);
            }

            base_objects::network::response registry_data(base_objects::SharedClientData& client) {
                return selectors::configuration::registry_data.call(client);
            }

            base_objects::network::response resetChat(base_objects::SharedClientData& client) {
                return selectors::configuration::resetChat.call(client);
            }

            base_objects::network::response removeResourcePacks(base_objects::SharedClientData& client) {
                return selectors::configuration::removeResourcePacks.call(client);
            }

            base_objects::network::response removeResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id) {
                return selectors::configuration::removeResourcePack.call(client, pack_id);
            }

            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced) {
                return selectors::configuration::addResourcePack.call(client, client, pack_id, url, hash, forced);
            }

            base_objects::network::response addResourcePackPrompted(base_objects::SharedClientData& client, const enbt::raw_uuid& pack_id, const std::string& url, const std::string& hash, bool forced, const Chat& prompt) {
                return selectors::configuration::addResourcePackPrompted.call(client, client, pack_id, url, hash, forced, prompt);
            };

            base_objects::network::response storeCookie(base_objects::SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload) {
                return selectors::configuration::storeCookie.call(client, key, payload);
            }

            base_objects::network::response transfer(base_objects::SharedClientData& client, const std::string& host, int32_t port) {
                return selectors::configuration::transfer.call(client, host, port);
            }

            base_objects::network::response setFeatureFlags(base_objects::SharedClientData& client, const list_array<std::string>& features) {
                return selectors::configuration::setFeatureFlags.call(client, features);
            }

            base_objects::network::response updateTags(base_objects::SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tags_entries) {
                return selectors::configuration::updateTags.call(client, tags_entries);
            }

            base_objects::network::response knownPacks(base_objects::SharedClientData& client, const list_array<base_objects::packets::known_pack>& packs) {
                return selectors::configuration::knownPacks.call(client, packs);
            }

            base_objects::network::response custom_report(base_objects::SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values) {
                return selectors::configuration::custom_report.call(client, values);
            }

            base_objects::network::response server_links(base_objects::SharedClientData& client, const list_array<base_objects::packets::server_link>& links) {
                return selectors::configuration::server_links.call(client, links);
            }
        }

        namespace play {
            base_objects::network::response bundleResponse(base_objects::SharedClientData& client, base_objects::network::response&& response) {
                return selectors::play::bundleResponse.call(client, std::move(response));
            }

            base_objects::network::response spawnEntity(base_objects::SharedClientData& client, const base_objects::entity& entity, uint16_t protocol) {
                return selectors::play::spawnEntity.call(client, entity, 1);
            }

            base_objects::network::response spawnExperienceOrb(base_objects::SharedClientData& client, const base_objects::entity& entity, int16_t count) {
                return selectors::play::spawnExperienceOrb.call(client, entity, count);
            }

            base_objects::network::response entityAnimation(base_objects::SharedClientData& client, const base_objects::entity& entity, uint8_t animation) {
                return selectors::play::entityAnimation.call(client, entity, animation);
            }

            base_objects::network::response awardStatistics(base_objects::SharedClientData& client, const list_array<base_objects::packets::statistics>& statistics) {
                return selectors::play::awardStatistics.call(client, statistics);
            }

            base_objects::network::response acknowledgeBlockChange(base_objects::SharedClientData& client) {
                return selectors::play::acknowledgeBlockChange.call(client, client);
            }

            base_objects::network::response setBlockDestroyStage(base_objects::SharedClientData& client, const base_objects::entity& entity, base_objects::position block, uint8_t stage) {
                return selectors::play::setBlockDestroyStage.call(client, entity, block, stage);
            }

            base_objects::network::response blockEntityData(base_objects::SharedClientData& client, base_objects::position block, int32_t type, const enbt::value& data) {
                return selectors::play::blockEntityData.call(client, block, type, data);
            }

            base_objects::network::response blockAction(base_objects::SharedClientData& client, base_objects::position block, int32_t action_id, int32_t param, int32_t block_type) {
                return selectors::play::blockAction.call(client, block, action_id, param, block_type);
            }

            base_objects::network::response blockUpdate(base_objects::SharedClientData& client, base_objects::position block, int32_t block_type) {
                return selectors::play::blockUpdate.call(client, block, block_type);
            }

            base_objects::network::response bossBarAdd(base_objects::SharedClientData& client, const enbt::raw_uuid& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags) {
                return selectors::play::bossBarAdd.call(client, id, title, health, color, division, flags);
            }

            base_objects::network::response bossBarRemove(base_objects::SharedClientData& client, const enbt::raw_uuid& id) {
                return selectors::play::bossBarRemove.call(client, id);
            }

            base_objects::network::response bossBarUpdateHealth(base_objects::SharedClientData& client, const enbt::raw_uuid& id, float health) {
                return selectors::play::bossBarUpdateHealth.call(client, id, health);
            }

            base_objects::network::response bossBarUpdateTitle(base_objects::SharedClientData& client, const enbt::raw_uuid& id, const Chat& title) {
                return selectors::play::bossBarUpdateTitle.call(client, id, title);
            }

            base_objects::network::response bossBarUpdateStyle(base_objects::SharedClientData& client, const enbt::raw_uuid& id, int32_t color, int32_t division) {
                return selectors::play::bossBarUpdateStyle.call(client, id, color, division);
            }

            base_objects::network::response bossBarUpdateFlags(base_objects::SharedClientData& client, const enbt::raw_uuid& id, uint8_t flags) {
                return selectors::play::bossBarUpdateFlags.call(client, id, flags);
            }

            base_objects::network::response changeDifficulty(base_objects::SharedClientData& client, uint8_t difficulty, bool locked) {
                return selectors::play::changeDifficulty.call(client, difficulty, locked);
            }

            base_objects::network::response chunkBatchFinished(base_objects::SharedClientData& client, int32_t count) {
                return selectors::play::chunkBatchFinished.call(client, count);
            }

            base_objects::network::response chunkBatchStart(base_objects::SharedClientData& client) {
                return selectors::play::chunkBatchStart.call(client);
            }

            base_objects::network::response chunkBiomes(base_objects::SharedClientData& client, list_array<base_objects::chunk::chunk_biomes>& chunk) {
                return selectors::play::chunkBiomes.call(client, chunk);
            }

            base_objects::network::response clearTitles(base_objects::SharedClientData& client, bool reset) {
                return selectors::play::clearTitles.call(client, reset);
            }

            base_objects::network::response commandSuggestionsResponse(base_objects::SharedClientData& client, int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions) {
                return selectors::play::commandSuggestionsResponse.call(client, transaction_id, start_pos, length, suggestions);
            }

            base_objects::network::response commands(base_objects::SharedClientData& client, int32_t root_id, const list_array<base_objects::packets::command_node>& nodes) {
                return selectors::play::commands.call(client, root_id, nodes);
            }

            base_objects::network::response closeContainer(base_objects::SharedClientData& client, uint8_t container_id) {
                return selectors::play::closeContainer.call(client, container_id);
            }

            base_objects::network::response setContainerContent(base_objects::SharedClientData& client, uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item) {
                return selectors::play::setContainerContent.call(client, windows_id, state_id, slots, carried_item);
            }

            base_objects::network::response setContainerProperty(base_objects::SharedClientData& client, uint8_t windows_id, uint16_t property, uint16_t value) {
                return selectors::play::setContainerProperty.call(client, windows_id, property, value);
            }

            base_objects::network::response setContainerSlot(base_objects::SharedClientData& client, uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item) {
                return selectors::play::setContainerSlot.call(client, windows_id, state_id, slot, item);
            }

            base_objects::network::response cookieRequest(base_objects::SharedClientData& client, const std::string& key) {
                return selectors::play::cookieRequest.call(client, key);
            }

            base_objects::network::response setCooldown(base_objects::SharedClientData& client, int32_t item_id, int32_t cooldown) {
                return selectors::play::setCooldown.call(client, item_id, cooldown);
            }

            base_objects::network::response chatSuggestionsResponse(base_objects::SharedClientData& client, int32_t action, int32_t count, const list_array<std::string>& suggestions) {
                return selectors::play::chatSuggestionsResponse.call(client, action, count, suggestions);
            }

            base_objects::network::response customPayload(base_objects::SharedClientData& client, const std::string& channel, const list_array<uint8_t>& data) {
                return selectors::play::customPayload.call(client, channel, data);
            }

            base_objects::network::response damageEvent(base_objects::SharedClientData& client, int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<util::VECTOR> xyz) {
                return selectors::play::damageEvent.call(client, entity_id, source_type_id, source_cause_id, source_direct_id, xyz);
            }

            base_objects::network::response debugSample(base_objects::SharedClientData& client, const list_array<uint64_t>& sample, int32_t sample_type) {
                return selectors::play::debugSample.call(client, sample, sample_type);
            }

            base_objects::network::response deleteMessageBySignature(base_objects::SharedClientData& client, uint8_t (&signature)[256]) {
                return selectors::play::deleteMessageBySignature.call(client, signature);
            }

            base_objects::network::response deleteMessageByID(base_objects::SharedClientData& client, int32_t message_id) {
                return selectors::play::deleteMessageByID.call(client, message_id);
            }

            base_objects::network::response kick(base_objects::SharedClientData& client, const Chat& reason) {
                return selectors::play::kick.call(client, reason);
            }

            base_objects::network::response disguisedChatMessage(base_objects::SharedClientData& client, const Chat& message, int32_t chat_type, const Chat& sender, const std::optional<Chat>& target_name) {
                return selectors::play::disguisedChatMessage.call(client, message, chat_type, sender, target_name);
            }

            base_objects::network::response entityEvent(base_objects::SharedClientData& client, int32_t entity_id, uint8_t entity_status) {
                return selectors::play::entityEvent.call(client, entity_id, entity_status);
            }

            base_objects::network::response explosion(base_objects::SharedClientData& client, util::VECTOR pos, float strength, const list_array<util::XYZ<int8_t>>& affected_blocks, util::VECTOR player_motion, int32_t block_interaction, int32_t small_explosion_particle_id, const base_objects::particle_data& small_explosion_particle_data, int32_t large_explosion_particle_id, const base_objects::particle_data& large_explosion_particle_data, const std::string& sound_name, std::optional<float> fixed_range) {
                return selectors::play::explosion.call(client, pos, strength, affected_blocks, player_motion, block_interaction, small_explosion_particle_id, small_explosion_particle_data, large_explosion_particle_id, large_explosion_particle_data, sound_name, fixed_range);
            }

            base_objects::network::response unloadChunk(base_objects::SharedClientData& client, int32_t x, int32_t z) {
                return selectors::play::unloadChunk.call(client, x, z);
            }

            base_objects::network::response gameEvent(base_objects::SharedClientData& client, uint8_t event_id, float value) {
                return selectors::play::gameEvent.call(client, event_id, value);
            }

            base_objects::network::response openHorseWindow(base_objects::SharedClientData& client, uint8_t window_id, int32_t slots, int32_t entity_id) {
                return selectors::play::openHorseWindow.call(client, window_id, slots, entity_id);
            }

            base_objects::network::response hurtAnimation(base_objects::SharedClientData& client, int32_t entity_id, float yaw) {
                return selectors::play::hurtAnimation.call(client, entity_id, yaw);
            }

            base_objects::network::response initializeWorldBorder(base_objects::SharedClientData& client, double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time) {
                return selectors::play::initializeWorldBorder.call(client, x, z, old_diameter, new_diameter, speed_ms, portal_teleport_boundary, warning_blocks, warning_time);
            }

            base_objects::network::response keepAlive(base_objects::SharedClientData& client, int64_t id) {
                return selectors::play::keepAlive.call(client, id);
            }

            base_objects::network::response updateChunkDataWLights(base_objects::SharedClientData& client, int32_t chunk_x, int32_t chunk_z, const NBT& heightmaps, const std::vector<uint8_t> data, const bit_list_array<>& sky_light_mask, const bit_list_array<>& block_light_mask, const bit_list_array<>& empty_skylight_mask, const bit_list_array<>& empty_block_light_mask, const list_array<std::vector<uint8_t>> sky_light_arrays, const list_array<std::vector<uint8_t>> block_light_arrays) {
                return selectors::play::updateChunkDataWLights.call(client, chunk_x, chunk_z, heightmaps, data, sky_light_mask, block_light_mask, block_light_mask, empty_skylight_mask, sky_light_arrays, block_light_arrays);
            }

            base_objects::network::response worldEvent(base_objects::SharedClientData& client, int32_t event, base_objects::position pos, int32_t data, bool global) {
                return selectors::play::worldEvent.call(client, event, pos, data, global);
            }

            base_objects::network::response particle(base_objects::SharedClientData& client, int32_t particle_id, bool long_distance, util::VECTOR pos, util::XYZ<float> offset, float max_speed, int32_t count, const list_array<uint8_t>& data) {
                return selectors::play::particle.call(client, particle_id, long_distance, pos, offset, max_speed, count, data);
            }

            base_objects::network::response updateLight(base_objects::SharedClientData& client, int32_t chunk_x, int32_t chunk_z, const bit_list_array<>& sky_light_mask, const bit_list_array<>& block_light_mask, const bit_list_array<>& empty_skylight_mask, const bit_list_array<>& empty_block_light_mask, const list_array<std::vector<uint8_t>> sky_light_arrays, const list_array<std::vector<uint8_t>> block_light_arrays) {
                return selectors::play::updateLight.call(client, chunk_x, chunk_z, sky_light_mask, block_light_mask, empty_skylight_mask, empty_block_light_mask, sky_light_arrays, block_light_arrays);
            }

            base_objects::network::response joinGame(base_objects::SharedClientData& client, int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, int32_t current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, std::optional<base_objects::packets::death_location_data> death_location, int32_t portal_cooldown, bool enforces_secure_chat) {
                return selectors::play::joinGame.call(client, entity_id, is_hardcore, dimension_names, max_players, view_distance, simulation_distance, reduced_debug_info, enable_respawn_screen, do_limited_crafting, current_dimension_type, dimension_name, hashed_seed, gamemode, prev_gamemode, is_debug, is_flat, death_location, portal_cooldown, enforces_secure_chat);
            }

            base_objects::network::response mapData(base_objects::SharedClientData& client, int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t column, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data) {
                return selectors::play::mapData.call(client, map_id, scale, locked, icons, column, rows, x, z, data);
            }

            base_objects::network::response merchantOffers(base_objects::SharedClientData& client, int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade> trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock) {
                return selectors::play::merchantOffers.call(client, window_id, trade_id, trades, level, experience, regular_villager, can_restock);
            }

            base_objects::network::response updateEntityPosition(base_objects::SharedClientData& client, int32_t entity_id, util::XYZ<float> pos, bool on_ground) {
                return selectors::play::updateEntityPosition.call(client, entity_id, pos, on_ground);
            }

            base_objects::network::response updateEntityPositionAndRotation(base_objects::SharedClientData& client, int32_t entity_id, util::XYZ<float> pos, util::VECTOR rot, bool on_ground) {
                return selectors::play::updateEntityPositionAndRotation.call(client, entity_id, pos, rot, on_ground);
            }

            base_objects::network::response updateEntityRotation(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR rot, bool on_ground) {
                return selectors::play::updateEntityRotation.call(client, entity_id, rot, on_ground);
            }

            base_objects::network::response moveVehicle(base_objects::SharedClientData& client, util::VECTOR pos, util::VECTOR rot) {
                return selectors::play::moveVehicle.call(client, pos, rot);
            }

            base_objects::network::response openBook(base_objects::SharedClientData& client, int32_t hand) {
                return selectors::play::openBook.call(client, hand);
            }

            base_objects::network::response openScreen(base_objects::SharedClientData& client, int32_t window_id, int32_t type, const Chat& title) {
                return selectors::play::openScreen.call(client, window_id, type, title);
            }

            base_objects::network::response openSignEditor(base_objects::SharedClientData& client, base_objects::position pos, bool is_front_text) {
                return selectors::play::openSignEditor.call(client, pos, is_front_text);
            }

            base_objects::network::response ping(base_objects::SharedClientData& client, int32_t id) {
                return selectors::play::ping.call(client, id);
            }

            base_objects::network::response pingResponse(base_objects::SharedClientData& client, int32_t id) {
                return selectors::play::pingResponse.call(client, id);
            }

            base_objects::network::response placeGhostRecipe(base_objects::SharedClientData& client, int32_t windows_id, const std::string& recipe_id) {
                return selectors::play::placeGhostRecipe.call(client, windows_id, recipe_id);
            }

            base_objects::network::response playerAbilities(base_objects::SharedClientData& client, uint8_t flags, float flying_speed, float field_of_view) {
                return selectors::play::playerAbilities.call(client, flags, flying_speed, field_of_view);
            }

            base_objects::network::response playerChatMessage(base_objects::SharedClientData& client, enbt::raw_uuid sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, std::optional<enbt::value> __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name) {
                return selectors::play::playerChatMessage.call(client, sender, index, signature, message, timestamp, salt, prev_messages, __UNDEFINED__FIELD__, filter_type, filtered_symbols_bitfield, chat_type, sender_name, target_name);
            }

            base_objects::network::response endCombat(base_objects::SharedClientData& client, int32_t duration) {
                return selectors::play::endCombat.call(client, duration);
            }

            base_objects::network::response enterCombat(base_objects::SharedClientData& client) {
                return selectors::play::enterCombat.call(client);
            }

            base_objects::network::response combatDeath(base_objects::SharedClientData& client, int32_t player_id, const Chat& message) {
                return selectors::play::combatDeath.call(client, player_id, message);
            }

            base_objects::network::response playerInfoRemove(base_objects::SharedClientData& client, const list_array<enbt::raw_uuid>& players) {
                return selectors::play::playerInfoRemove.call(client, players);
            }

            base_objects::network::response playerInfoAdd(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_add>& add_players) {
                return selectors::play::playerInfoAdd.call(client, add_players);
            }

            base_objects::network::response playerInfoInitializeChat(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat) {
                return selectors::play::playerInfoInitializeChat.call(client, initialize_chat);
            }

            base_objects::network::response playerInfoUpdateGameMode(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode) {
                return selectors::play::playerInfoUpdateGameMode.call(client, update_game_mode);
            }

            base_objects::network::response playerInfoUpdateListed(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_listed>& update_listed) {
                return selectors::play::playerInfoUpdateListed.call(client, update_listed);
            }

            base_objects::network::response playerInfoUpdateLatency(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_latency>& update_latency) {
                return selectors::play::playerInfoUpdateLatency.call(client, update_latency);
            }

            base_objects::network::response playerInfoUpdateDisplayName(base_objects::SharedClientData& client, const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name) {
                return selectors::play::playerInfoUpdateDisplayName.call(client, update_display_name);
            }

            base_objects::network::response lookAt(base_objects::SharedClientData& client, bool from_feet_or_eyes, util::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id) {
                return selectors::play::lookAt.call(client, from_feet_or_eyes, target, entity_id);
            }

            base_objects::network::response synchronizePlayerPosition(base_objects::SharedClientData& client, util::VECTOR pos, float yaw, float pitch, uint8_t flags) {
                auto id = client.packets_state.teleport_id_sequence++;
                client.packets_state.pending_teleport_ids.push_back(id);
                return selectors::play::synchronizePlayerPosition.call(client, pos, yaw, pitch, flags, id);
            }

            base_objects::network::response initRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> displayed_recipe_ids, list_array<std::string> had_access_to_recipe_ids) {
                return selectors::play::initRecipeBook.call(client, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, displayed_recipe_ids, had_access_to_recipe_ids);
            }

            base_objects::network::response addRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids) {
                return selectors::play::addRecipeBook.call(client, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids);
            }

            base_objects::network::response removeRecipeBook(base_objects::SharedClientData& client, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids) {
                return selectors::play::removeRecipeBook.call(client, crafting_recipe_book_open, crafting_recipe_book_filter_active, smelting_recipe_book_open, smelting_recipe_book_filter_active, blast_furnace_recipe_book_open, blast_furnace_recipe_book_filter_active, smoker_recipe_book_open, smoker_recipe_book_filter_active, recipe_ids);
            }

            base_objects::network::response removeEntities(base_objects::SharedClientData& client, const list_array<int32_t>& entity_ids) {
                return selectors::play::removeEntities.call(client, entity_ids);
            }

            base_objects::network::response removeEntityEffect(base_objects::SharedClientData& client, int32_t entity_id, int32_t effect_id) {
                return selectors::play::removeEntityEffect.call(client, entity_id, effect_id);
            }

            base_objects::network::response resetScore(base_objects::SharedClientData& client, const std::string& entity_name, const std::optional<std::string>& objective_name) {
                return selectors::play::resetScore.call(client, entity_name, objective_name);
            }

            base_objects::network::response removeResourcePacks(base_objects::SharedClientData& client) {
                return selectors::play::removeResourcePacks.call(client);
            }

            base_objects::network::response removeResourcePack(base_objects::SharedClientData& client, enbt::raw_uuid id) {
                return selectors::play::removeResourcePack.call(client, id);
            }

            base_objects::network::response addResourcePack(base_objects::SharedClientData& client, enbt::raw_uuid id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt) {
                return selectors::play::addResourcePack.call(client, id, url, hash, forced, prompt);
            }

            base_objects::network::response respawn(base_objects::SharedClientData& client, int32_t dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata) {
                return selectors::play::respawn.call(client, dimension_type, dimension_name, hashed_seed, gamemode, previous_gamemode, is_debug, is_flat, death_location, portal_cooldown, keep_attributes, keep_metadata);
            }

            base_objects::network::response setHeadRotation(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR head_rotation) {
                return selectors::play::setHeadRotation.call(client, entity_id, head_rotation);
            }

            base_objects::network::response updateSectionBlocks(base_objects::SharedClientData& client, int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks) {
                return selectors::play::updateSectionBlocks.call(client, section_x, section_z, section_y, blocks);
            }

            base_objects::network::response setAdvancementsTab(base_objects::SharedClientData& client, const std::optional<std::string>& tab_id) {
                return selectors::play::setAdvancementsTab.call(client, tab_id);
            }

            base_objects::network::response serverData(base_objects::SharedClientData& client, const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool __ignored) {
                return selectors::play::serverData.call(client, motd, icon_png, false);
            }

            base_objects::network::response setActionBarText(base_objects::SharedClientData& client, const Chat& text) {
                return selectors::play::setActionBarText.call(client, text);
            }

            base_objects::network::response setBorderCenter(base_objects::SharedClientData& client, double x, double z) {
                return selectors::play::setBorderCenter.call(client, x, z);
            }

            base_objects::network::response setBorderLerp(base_objects::SharedClientData& client, double old_diameter, double new_diameter, int64_t speed_ms) {
                return selectors::play::setBorderLerp.call(client, old_diameter, new_diameter, speed_ms);
            }

            base_objects::network::response setBorderSize(base_objects::SharedClientData& client, double diameter) {
                return selectors::play::setBorderSize.call(client, diameter);
            }

            base_objects::network::response setBorderWarningDelay(base_objects::SharedClientData& client, int32_t warning_delay) {
                return selectors::play::setBorderWarningDelay.call(client, warning_delay);
            }

            base_objects::network::response setBorderWarningDistance(base_objects::SharedClientData& client, int32_t warning_distance) {
                return selectors::play::setBorderWarningDistance.call(client, warning_distance);
            }

            base_objects::network::response setCamera(base_objects::SharedClientData& client, int32_t entity_id) {
                return selectors::play::setCamera.call(client, entity_id);
            }

            base_objects::network::response setHeldItem(base_objects::SharedClientData& client, uint8_t slot) {
                return selectors::play::setHeldItem.call(client, slot);
            }

            base_objects::network::response setCenterChunk(base_objects::SharedClientData& client, int32_t x, int32_t z) {
                return selectors::play::setCenterChunk.call(client, x, z);
            }

            base_objects::network::response setRenderDistance(base_objects::SharedClientData& client, int32_t render_distance) {
                return selectors::play::setRenderDistance.call(client, render_distance);
            }

            base_objects::network::response setDefaultSpawnPosition(base_objects::SharedClientData& client, base_objects::position pos, float angle) {
                return selectors::play::setDefaultSpawnPosition.call(client, pos, angle);
            }

            base_objects::network::response displayObjective(base_objects::SharedClientData& client, int32_t position, const std::string& objective_name) {
                return selectors::play::displayObjective.call(client, position, objective_name);
            }

            base_objects::network::response setEntityMetadata(base_objects::SharedClientData& client, int32_t entity_id, const list_array<uint8_t>& metadata) {
                return selectors::play::setEntityMetadata.call(client, entity_id, metadata);
            }

            base_objects::network::response linkEntities(base_objects::SharedClientData& client, int32_t attached_entity_id, int32_t holder_entity_id) {
                return selectors::play::linkEntities.call(client, attached_entity_id, holder_entity_id);
            }

            base_objects::network::response setEntityVelocity(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR velocity) {
                return selectors::play::setEntityVelocity.call(client, entity_id, velocity);
            }

            base_objects::network::response setEquipment(base_objects::SharedClientData& client, int32_t entity_id, uint8_t slot, const base_objects::slot& item) {
                return selectors::play::setEquipment.call(client, entity_id, slot, item);
            }

            base_objects::network::response setExperience(base_objects::SharedClientData& client, float experience_bar, int32_t level, int32_t total_experience) {
                return selectors::play::setExperience.call(client, experience_bar, level, total_experience);
            }

            base_objects::network::response setHealth(base_objects::SharedClientData& client, float health, int32_t food, float saturation) {
                return selectors::play::setHealth.call(client, health, food, saturation);
            }

            base_objects::network::response updateObjectivesCreate(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type) {
                return selectors::play::updateObjectivesCreate.call(client, objective_name, display_name, render_type);
            }

            base_objects::network::response updateObjectivesCreateStyled(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
                return selectors::play::updateObjectivesCreateStyled.call(client, objective_name, display_name, render_type, style);
            }

            base_objects::network::response updateObjectivesCreateFixed(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
                return selectors::play::updateObjectivesCreateFixed.call(client, objective_name, display_name, render_type, content);
            }

            base_objects::network::response updateObjectivesRemove(base_objects::SharedClientData& client, const std::string& objective_name) {
                return selectors::play::updateObjectivesRemove.call(client, objective_name);
            }

            base_objects::network::response updateObjectivesInfo(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type) {
                return selectors::play::updateObjectivesInfo.call(client, objective_name, display_name, render_type);
            }

            base_objects::network::response updateObjectivesInfoStyled(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const enbt::value& style) {
                return selectors::play::updateObjectivesInfoStyled.call(client, objective_name, display_name, render_type, style);
            }

            base_objects::network::response updateObjectivesInfoFixed(base_objects::SharedClientData& client, const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
                return selectors::play::updateObjectivesInfoFixed.call(client, objective_name, display_name, render_type, content);
            }

            base_objects::network::response setPassengers(base_objects::SharedClientData& client, int32_t vehicle_entity_id, const list_array<int32_t>& passengers) {
                return selectors::play::setPassengers.call(client, vehicle_entity_id, passengers);
            }

            base_objects::network::response updateTeamCreate(base_objects::SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities) {
                return selectors::play::updateTeamCreate.call(client, team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix, entities);
            }

            base_objects::network::response updateTeamRemove(base_objects::SharedClientData& client, const std::string& team_name) {
                return selectors::play::updateTeamRemove.call(client, team_name);
            }

            base_objects::network::response updateTeamInfo(base_objects::SharedClientData& client, const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix) {
                return selectors::play::updateTeamInfo.call(client, team_name, display_name, allow_fire_co_teamer, see_invisible_co_teamer, name_tag_visibility, collision_rule, team_color, prefix, suffix);
            }

            base_objects::network::response updateTeamAddEntities(base_objects::SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities) {
                return selectors::play::updateTeamAddEntities.call(client, team_name, entities);
            }

            base_objects::network::response updateTeamRemoveEntities(base_objects::SharedClientData& client, const std::string& team_name, const list_array<std::string>& entities) {
                return selectors::play::updateTeamRemoveEntities.call(client, team_name, entities);
            }

            base_objects::network::response setScore(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name) {
                return selectors::play::setScore.call(client, entity_name, objective_name, value, display_name);
            }

            base_objects::network::response setScoreStyled(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const enbt::value& styled) {
                return selectors::play::setScoreStyled.call(client, entity_name, objective_name, value, display_name, styled);
            }

            base_objects::network::response setScoreFixed(base_objects::SharedClientData& client, const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content) {
                return selectors::play::setScoreFixed.call(client, entity_name, objective_name, value, display_name, content);
            }

            base_objects::network::response setSimulationDistance(base_objects::SharedClientData& client, int32_t distance) {
                return selectors::play::setSimulationDistance.call(client, distance);
            }

            base_objects::network::response setSubtitleText(base_objects::SharedClientData& client, const Chat& text) {
                return selectors::play::setSubtitleText.call(client, text);
            }

            base_objects::network::response updateTime(base_objects::SharedClientData& client, int64_t world_age, int64_t time_of_day) {
                return selectors::play::updateTime.call(client, world_age, time_of_day);
            }

            base_objects::network::response setTitleText(base_objects::SharedClientData& client, const Chat& text) {
                return selectors::play::setTitleText.call(client, text);
            }

            base_objects::network::response setTitleAnimationTimes(base_objects::SharedClientData& client, int32_t fade_in, int32_t stay, int32_t fade_out) {
                return selectors::play::setTitleAnimationTimes.call(client, fade_in, stay, fade_out);
            }

            base_objects::network::response entitySoundEffect(base_objects::SharedClientData& client, uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
                return selectors::play::entitySoundEffect.call(client, sound_id, category, entity_id, volume, pitch, seed);
            }

            base_objects::network::response entitySoundEffectCustom(base_objects::SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
                return selectors::play::entitySoundEffectCustom.call(client, sound_id, range, category, entity_id, volume, pitch, seed);
            }

            base_objects::network::response soundEffect(base_objects::SharedClientData& client, uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
                return selectors::play::soundEffect.call(client, sound_id, category, x, y, z, volume, pitch, seed);
            }

            base_objects::network::response soundEffectCustom(base_objects::SharedClientData& client, const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
                return selectors::play::soundEffectCustom.call(client, sound_id, range, category, x, y, z, volume, pitch, seed);
            }

            base_objects::network::response startConfiguration(base_objects::SharedClientData& client) {
                return selectors::play::startConfiguration.call(client);
            }

            base_objects::network::response stopSound(base_objects::SharedClientData& client, uint8_t flags) {
                return selectors::play::stopSound.call(client, flags);
            }

            base_objects::network::response stopSoundBySource(base_objects::SharedClientData& client, uint8_t flags, int32_t source) {
                return selectors::play::stopSoundBySource.call(client, flags, source);
            }

            base_objects::network::response stopSoundBySound(base_objects::SharedClientData& client, uint8_t flags, const std::string& sound) {
                return selectors::play::stopSoundBySound.call(client, flags, sound);
            }

            base_objects::network::response stopSoundBySourceAndSound(base_objects::SharedClientData& client, uint8_t flags, int32_t source, const std::string& sound) {
                return selectors::play::stopSoundBySourceAndSound.call(client, flags, source, sound);
            }

            base_objects::network::response storeCookie(base_objects::SharedClientData& client, const std::string& key, const list_array<uint8_t>& payload) {
                return selectors::play::storeCookie.call(client, key, payload);
            }

            base_objects::network::response systemChatMessage(base_objects::SharedClientData& client, const Chat& message) {
                return selectors::play::systemChatMessage.call(client, message);
            }

            base_objects::network::response systemChatMessageOverlay(base_objects::SharedClientData& client, const Chat& message) {
                return selectors::play::systemChatMessageOverlay.call(client, message);
            }

            base_objects::network::response setTabListHeaderAndFooter(base_objects::SharedClientData& client, const Chat& header, const Chat& footer) {
                return selectors::play::setTabListHeaderAndFooter.call(client, header, footer);
            }

            base_objects::network::response tagQueryResponse(base_objects::SharedClientData& client, int32_t transaction_id, const enbt::value& nbt) {
                return selectors::play::tagQueryResponse.call(client, transaction_id, nbt);
            }

            base_objects::network::response pickupItem(base_objects::SharedClientData& client, int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count) {
                return selectors::play::pickupItem.call(client, collected_entity_id, collector_entity_id, pickup_item_count);
            }

            base_objects::network::response teleportEntity(base_objects::SharedClientData& client, int32_t entity_id, util::VECTOR pos, float yaw, float pitch, bool on_ground) {
                return selectors::play::teleportEntity.call(client, entity_id, pos, yaw, pitch, on_ground);
            }

            base_objects::network::response setTickingState(base_objects::SharedClientData& client, float tick_rate, bool is_frozen) {
                return selectors::play::setTickingState.call(client, tick_rate, is_frozen);
            }

            base_objects::network::response stepTick(base_objects::SharedClientData& client, int32_t step_count) {
                return selectors::play::stepTick.call(client, step_count);
            }

            base_objects::network::response transfer(base_objects::SharedClientData& client, const std::string& host, int32_t port) {
                return selectors::play::transfer.call(client, host, port);
            }

            base_objects::network::response updateAdvancements(base_objects::SharedClientData& client, bool reset, const list_array<base_objects::packets::advancements_maping> advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress> progress_advancements) {
                return selectors::play::updateAdvancements.call(client, reset, advancement_mapping, remove_advancements, progress_advancements);
            }

            base_objects::network::response updateAttributes(base_objects::SharedClientData& client, int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) {
                return selectors::play::updateAttributes.call(client, entity_id, properties);
            }

            base_objects::network::response entityEffect(base_objects::SharedClientData& client, int32_t entity_id, int32_t effect_id, int32_t amplifier, int32_t duration, int8_t flags) {
                return selectors::play::entityEffect.call(client, entity_id, effect_id, amplifier, duration, flags);
            }

            base_objects::network::response updateRecipes(base_objects::SharedClientData& client, const std::vector<base_objects::recipe>& recipes) {
                return selectors::play::updateRecipes.call(client, recipes);
            }

            base_objects::network::response updateTags(base_objects::SharedClientData& client, const list_array<base_objects::packets::tag_mapping>& tag_mappings) {
                return selectors::play::updateTags.call(client, tag_mappings);
            }

            base_objects::network::response projectilePower(base_objects::SharedClientData& client, int32_t entity_id, double power_x, double power_y, double power_z) {
                return selectors::play::projectilePower.call(client, entity_id, power_x, power_y, power_z);
            }

            base_objects::network::response custom_report(base_objects::SharedClientData& client, const list_array<std::pair<std::string, std::string>>& values) {
                return selectors::play::custom_report.call(client, values);
            }

            base_objects::network::response server_links(base_objects::SharedClientData& client, const list_array<base_objects::packets::server_link>& links) {
                return selectors::play::server_links.call(client, links);
            }
        }
    }
}