#include "packets.hpp"
#include "../../util.hpp"

namespace crafted_craft {
    namespace packets {
        namespace release_765 {
            namespace login {
                Response kick(const Chat& reason) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x00);
                    WriteIdentifier(packet, reason.ToStr());
                    return Response::Disconnect({packet});
                }

                Response disableCompression() {
                    return Response::Answer({{0x03, 0}});
                }

                Response setCompression(int32_t threshold) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x00);
                    WriteVar<int32_t>(threshold, packet);
                    return Response::EnableCompressAnswer(packet, threshold);
                }
            }

            namespace configuration {
                Response configuration(const std::string& chanel, const list_array<uint8_t>& data) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x00);
                    WriteIdentifier(packet, chanel);
                    packet.push_back(data);
                    return Response::Answer({packet});
                }

                Response kick(const Chat& reason) {
                    list_array<uint8_t> packet = reason.ToTextComponent();
                    packet.push_front(0x01); //disconnect
                    return Response::Disconnect({packet});
                }

                Response removeResourcePacks() {
                    return Response::Answer({{0x06, false}});
                }

                Response removeResourcePack(const ENBT::UUID& pack_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(2 + 16);
                    packet.push_back(0x06);
                    packet.push_back(true);
                    WriteUUID(pack_id, packet);
                    return Response::Answer({packet});
                }

                Response addResourcePack(const ENBT::UUID& pack_id, const std::string& url, const std::string& hash, bool forced) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(2 + 16 + 2 + url.size() + 2 + hash.size());
                    packet.push_back(0x07);
                    WriteUUID(pack_id, packet);
                    WriteString(packet, url, 32767);
                    WriteString(packet, hash, 40);
                    packet.push_back(forced);
                    packet.push_back(false);
                    return Response::Answer({packet});
                }

                Response addResourcePack(const ENBT::UUID& pack_id, const std::string& url, const std::string& hash, bool forced, Chat prompt) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(2 + 16 + 2 + url.size() + 2 + hash.size());
                    packet.push_back(0x07);
                    WriteUUID(pack_id, packet);
                    WriteString(packet, url, 32767);
                    WriteString(packet, hash, 40);
                    packet.push_back(forced);
                    packet.push_back(true);
                    packet.push_back(prompt.ToTextComponent());
                    return Response::Answer({packet});
                }

                Response setFeatureFlags(const list_array<std::string>& features) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x08);
                    WriteVar<int16_t>(features.size(), packet);
                    for (auto& it : features)
                        WriteIdentifier(packet, it);
                    return Response::Answer({packet});
                }

                Response updateTags(const list_array<base_objects::packets::tag_mapping>& tags_entries) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x09);
                    WriteVar<int32_t>(tags_entries.size(), packet);
                    for (auto& [registry, tags] : tags_entries) {
                        WriteIdentifier(packet, registry);
                        WriteVar<int32_t>(tags.size(), packet);
                        for (auto& it : tags) {
                            WriteIdentifier(packet, it.tag_name);
                            WriteVar<int32_t>(it.entires.size(), packet);
                            for (auto& entry : it.entires)
                                WriteVar<int32_t>(entry, packet);
                        }
                    }
                    return Response::Answer({packet});
                }
            }

            namespace play {
                Response bundleResponse(Response&& response) {
                    if (response.do_disconnect)
                        return std::move(response);
                    else {
                        response.data.remove_if([](auto& it) {
                            if (it.data.size() == 1)
                                return it.data[0] == 0x00;
                            else
                                return false;
                        });
                        if (response.data.empty())
                            return Response::Empty();
                        else if (response.data.size() == 1)
                            return std::move(response);

                        Response answer(Response::Answer({{0x00}}));
                        answer.data.push_back(response.data);
                        answer.data.push_back({0x00});
                        return answer;
                    }
                }

                Response spawnEntity(TCPsession& client, const Entity& entity) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 * 3 + 3 * 2 + 1 + 16 + 4 + 3 * 2);
                    packet.push_back(0x01);
                    WriteVar<int32_t>(client.serverData().entity_ids_map.get_id(entity.id), packet);
                    WriteUUID(entity.id, packet);
                    WriteVar<int32_t>(entity.entity_id, packet);
                    WriteValue<double>(entity.position.x, packet);
                    WriteValue<double>(entity.position.y, packet);
                    WriteValue<double>(entity.position.z, packet);
                    auto res = calc::to_yaw_pitch(entity.rotation);
                    WriteValue<int8_t>(res.x, packet);
                    WriteValue<int8_t>(res.y, packet);
                    res = calc::to_yaw_pitch(entity.head_rotation);
                    WriteValue<int8_t>(res.y, packet);
                    WriteVar<int32_t>(entity.data, packet);
                    auto velocity = calc::minecraft::packets::velocity(entity.motion);
                    WriteValue<int16_t>(velocity.x, packet);
                    WriteValue<int16_t>(velocity.y, packet);
                    WriteValue<int16_t>(velocity.z, packet);
                    return Response::Answer({packet});
                }

                Response spawnExperienceOrb(TCPsession& client, const Entity& entity, int16_t count) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 * 3 + 2);
                    packet.push_back(0x02);
                    WriteVar<int32_t>(client.serverData().entity_ids_map.get_id(entity.id), packet);
                    WriteValue<double>(entity.position.x, packet);
                    WriteValue<double>(entity.position.y, packet);
                    WriteValue<double>(entity.position.z, packet);
                    WriteValue<int16_t>(count, packet);
                    return Response::Answer({packet});
                }

                Response entityAnimation(TCPsession& client, const Entity& entity, uint8_t animation) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1);
                    packet.push_back(0x03);
                    WriteVar<int32_t>(client.serverData().entity_ids_map.get_id(entity.id), packet);
                    packet.push_back(animation);
                    return Response::Answer({packet});
                }

                Response awardStatistics(const list_array<base_objects::packets::statistics>& statistics) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 3 * statistics.size());
                    packet.push_back(0x04);
                    WriteVar<int32_t>(statistics.size(), packet);
                    for (auto& [category_id, statistic_id, value] : statistics) {
                        WriteVar<int32_t>(category_id, packet);
                        WriteVar<int32_t>(statistic_id, packet);
                        WriteVar<int32_t>(value, packet);
                    }
                    return Response::Answer({packet});
                }

                Response acknowledgeBlockChange(SharedClientData& client) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(5);
                    packet.push_back(0x05);
                    WriteVar<int32_t>(client.packets_state.current_block_sequence_id, packet);
                    return Response::Answer({packet});
                }

                Response setBlockDestroyStage(TCPsession& client, const Entity& entity, Position block, uint8_t stage) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 8 + 1);
                    packet.push_back(0x06);
                    WriteVar<int32_t>(client.serverData().entity_ids_map.get_id(entity.id), packet);
                    WriteValue<uint64_t>(block.raw, packet);
                    packet.push_back(stage);
                    return Response::Answer({packet});
                }

                Response blockEntityData(Position block, int32_t type, const ENBT& data) {

                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 + 4);
                    packet.push_back(0x07);
                    WriteValue<uint64_t>(block.raw, packet);
                    WriteVar<int32_t>(type, packet);
                    packet.push_back(NBT(data).get_as_normal());
                    return Response::Answer({packet});
                }

                //block_type is from "minecraft:block" registry, not a block state.
                Response blockAction(Position block, int32_t action_id, int32_t param, int32_t block_type) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 + 4 * 3);
                    packet.push_back(0x08);
                    WriteValue<uint64_t>(block.raw, packet);
                    WriteVar<int32_t>(action_id, packet);
                    WriteVar<int32_t>(param, packet);
                    WriteVar<int32_t>(block_type, packet);
                    return Response::Answer({packet});
                }

                //block_type is from "minecraft:block" registry, not a block state.
                Response blockUpdate(Position block, int32_t block_type) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 + 4);
                    packet.push_back(0x09);
                    WriteValue<uint64_t>(block.raw, packet);
                    WriteVar<int32_t>(block_type, packet);
                    return Response::Answer({packet});
                }

                Response bossBarAdd(const ENBT::UUID& id, const Chat& title, float health, int32_t color, int32_t division, uint8_t flags) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 16 + 2 + title.ToStr().size() + 4 * 4);
                    packet.push_back(0x0A);
                    WriteUUID(id, packet);
                    packet.push_back(0);
                    WriteString(packet, title.ToStr(), 32767);
                    WriteValue<float>(health, packet);
                    WriteVar<int32_t>(color, packet);
                    WriteVar<int32_t>(division, packet);
                    WriteValue<uint8_t>(flags, packet);
                    return Response::Answer({packet});
                }

                Response bossBarRemove(const ENBT::UUID& id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 16 + 1);
                    packet.push_back(0x0A);
                    WriteUUID(id, packet);
                    packet.push_back(0);
                    return Response::Answer({packet});
                }

                Response bossBarUpdateHealth(const ENBT::UUID& id, float health) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 16 + 4);
                    packet.push_back(0x0A);
                    WriteUUID(id, packet);
                    packet.push_back(2);
                    WriteValue<float>(health, packet);
                    return Response::Answer({packet});
                }

                Response bossBarUpdateTitle(const ENBT::UUID& id, const Chat& title) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 16 + 2 + title.ToStr().size());
                    packet.push_back(0x0A);
                    WriteUUID(id, packet);
                    packet.push_back(3);
                    WriteString(packet, title.ToStr(), 32767);
                    return Response::Answer({packet});
                }

                Response bossBarUpdateStyle(const ENBT::UUID& id, int32_t color, int32_t division) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 16 + 4 * 2);
                    packet.push_back(0x0A);
                    WriteUUID(id, packet);
                    packet.push_back(4);
                    WriteVar<int32_t>(color, packet);
                    WriteVar<int32_t>(division, packet);
                    return Response::Answer({packet});
                }

                Response bossBarUpdateFlags(const ENBT::UUID& id, uint8_t flags) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 16 + 4);
                    packet.push_back(0x0A);
                    WriteUUID(id, packet);
                    packet.push_back(5);
                    WriteValue<uint8_t>(flags, packet);
                    return Response::Answer({packet});
                }

                Response changeDifficulty(uint8_t difficulty, bool locked) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 1);
                    packet.push_back(0x0B);
                    packet.push_back(difficulty);
                    packet.push_back(locked);
                    return Response::Answer({packet});
                }

                Response chunkBatchFinished(int32_t count) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x0C);
                    WriteVar<int32_t>(count, packet);
                    return Response::Answer({packet});
                }

                Response chunkBatchStart() {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1);
                    packet.push_back(0x0D);
                    return Response::Answer({packet});
                }

                Response chunkBiomes(list_array<base_objects::chunk::chunk_biomes>& chunk) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x0E);
                    WriteVar<int32_t>(chunk.size(), packet);
                    for (auto& section : chunk) {
                        WriteVar<int32_t>(section.z, packet);
                        WriteVar<int32_t>(section.x, packet);
                        WriteVar<int32_t>(section.biomes.size(), packet);
                        list_array<uint8_t> biomes;
                        for (auto& biome : section.biomes)
                            biomes.push_back(biome.serialize());

                        packet.push_back(biomes);
                    }
                    return Response::Answer({packet});
                }

                Response clearTitles(bool reset) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1);
                    packet.push_back(0x0F);
                    packet.push_back(reset);
                    return Response::Answer({packet});
                }

                Response commandSuggestionsResponse(int32_t transaction_id, int32_t start_pos, int32_t length, const list_array<base_objects::packets::command_suggestion>& suggestions) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 4);
                    packet.push_back(0x10);
                    WriteVar<int32_t>(transaction_id, packet);
                    WriteVar<int32_t>(start_pos, packet);
                    WriteVar<int32_t>(length, packet);
                    WriteVar<int32_t>(suggestions.size(), packet);
                    for (auto& [suggestion, tooltip] : suggestions) {
                        WriteString(packet, suggestion, 32767);
                        packet.push_back(!tooltip);
                        if (tooltip)
                            packet.push_back(tooltip->ToTextComponent());
                    }
                    return Response::Answer({packet});
                }

                Response commands(int32_t root_id, const list_array<base_objects::packets::command_node>& nodes) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x11);
                    WriteVar<int32_t>(nodes.size(), packet);
                    for (auto& node : nodes) {
                        WriteValue(node.flags.raw, packet);
                        WriteVar<int32_t>(node.children.size(), packet);
                        for (auto& child : node.children)
                            WriteVar<int32_t>(child, packet);
                        if (node.flags.has_redirect)
                            WriteVar<int32_t>(node.redirect_node.value(), packet);
                        if (node.flags.node_type == base_objects::packets::command_node::node_type::literal || node.flags.node_type == base_objects::packets::command_node::node_type::argument)
                            WriteIdentifier(packet, node.name.value());
                        if (node.flags.has_suggestion)
                            WriteIdentifier(packet, node.suggestion_type.value());
                        if (node.flags.node_type == base_objects::packets::command_node::node_type::argument) {
                            WriteVar<int32_t>((int32_t)node.parser_id.value(), packet);
                            if (node.properties) {
                                auto& properties = *node.properties;
                                if (properties.flags)
                                    packet.push_back(*properties.flags);
                                if (properties.min)
                                    std::visit([&packet](auto&& arg) { WriteValue(arg, packet); }, *properties.min);
                                if (properties.max)
                                    std::visit([&packet](auto&& arg) { WriteValue(arg, packet); }, *properties.max);
                                if (properties.registry)
                                    WriteIdentifier(packet, *properties.registry);
                            }
                        }
                        if (node.flags.has_suggestion)
                            WriteIdentifier(packet, node.suggestion_type.value());
                    }
                    WriteVar<int32_t>(root_id, packet);
                    return Response::Answer({packet});
                }

                Response closeContainer(uint8_t container_id) {
                    return Response::Answer({{0x12, container_id}});
                }

                Response setContainerContent(uint8_t windows_id, int32_t state_id, const list_array<base_objects::slot>& slots, const base_objects::slot& carried_item) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 4 + 2 * slots.size());
                    packet.push_back(0x13);
                    packet.push_back(windows_id);
                    WriteVar<int32_t>(state_id, packet);
                    WriteVar<int32_t>(slots.size(), packet);
                    for (auto& it : slots)
                        WriteSlot(packet, it);

                    WriteSlot(packet, carried_item);
                    return Response::Answer({packet});
                }

                Response setContainerProperty(uint8_t windows_id, uint16_t property, uint16_t value) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 2 * 2);
                    packet.push_back(0x14);
                    packet.push_back(windows_id);
                    WriteValue<uint16_t>(property, packet);
                    WriteValue<uint16_t>(value, packet);
                    return Response::Answer({packet});
                }

                Response setContainerSlot(uint8_t windows_id, int32_t state_id, int16_t slot, const base_objects::slot& item) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 4 + 2 + (bool)item);
                    packet.push_back(0x15);
                    packet.push_back(windows_id);
                    WriteVar<int32_t>(state_id, packet);
                    WriteValue<int16_t>(slot, packet);

                    WriteSlot(packet, item);
                    return Response::Answer({packet});
                }

                Response setCooldown(int32_t item_id, int32_t cooldown) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 2);
                    packet.push_back(0x16);
                    WriteVar<int32_t>(item_id, packet);
                    WriteVar<int32_t>(cooldown, packet);
                    return Response::Answer({packet});
                }

                //UNUSED by Notchian client
                Response chatSuggestionsResponse(int32_t action, int32_t count, const list_array<std::string>& suggestions) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 2);
                    packet.push_back(0x17);
                    WriteVar<int32_t>(action, packet);
                    WriteVar<int32_t>(count, packet);
                    for (auto& it : suggestions)
                        WriteString(packet, it, 32767);
                    return Response::Answer({packet});
                }

                Response customPayload(const std::string& channel, const list_array<uint8_t>& data) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + channel.size() + 2);
                    packet.push_back(0x18);
                    WriteString(packet, channel, 32767);
                    packet.push_back(data);
                    return Response::Answer({packet});
                }

                Response damageEvent(int32_t entity_id, int32_t source_type_id, int32_t source_cause_id, int32_t source_direct_id, std::optional<calc::VECTOR> xyz) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 4 + 1 + (3 * 4) * (bool)xyz);
                    packet.push_back(0x19);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteVar<int32_t>(source_type_id, packet);
                    WriteVar<int32_t>(source_cause_id, packet);
                    WriteVar<int32_t>(source_direct_id, packet);
                    packet.push_back((bool)xyz);
                    if (xyz) {
                        WriteValue<float>(xyz->x, packet);
                        WriteValue<float>(xyz->y, packet);
                        WriteValue<float>(xyz->z, packet);
                    }
                    return Response::Answer({packet});
                }

                Response deleteMessage(uint8_t signature[256]) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(12 + 256);
                    packet.push_back(0x1A);
                    packet.push_back(0);
                    packet.push_back(signature, 256);
                    return Response::Answer({packet});
                }

                Response deleteMessage(int32_t message_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(12 + 4);
                    packet.push_back(0x1A);
                    if (message_id == -1)
                        throw std::runtime_error("message id can't be -1");
                    WriteVar<int32_t>(message_id + 1, packet);
                    return Response::Answer({packet});
                }

                Response kick(const Chat& reason) {
                    return Response::Disconnect({{0x1B, reason.ToTextComponent()}});
                }

                Response disguisedChatMessage(const Chat& message, int32_t chat_type, const Chat& sender, std::optional<Chat> target_name) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 2 + 2 + 1 + (bool)target_name * 2 + (bool)target_name * target_name->ToStr().size());
                    packet.push_back(0x1C);
                    packet.push_back(message.ToTextComponent());
                    WriteVar<int32_t>(chat_type, packet);
                    packet.push_back(sender.ToTextComponent());
                    packet.push_back((bool)target_name);
                    if (target_name)
                        packet.push_back(target_name->ToTextComponent());

                    return Response::Answer({packet});
                }

                Response entityEvent(int32_t entity_id, uint8_t entity_status) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1);
                    packet.push_back(0x1D);
                    WriteValue<int32_t>(entity_id, packet);
                    packet.push_back(entity_status);
                    return Response::Answer({packet});
                }

                //TODO: particles
                //Response explosion(calc::VECTOR pos, float strength, list_array<calc::XYZ<int8_t>> affected_blocks, calc::VECTOR player_motion, int32_t block_interaction, int32_t small_explosion_particle_id, ) {
                //    list_array<uint8_t> packet;
                //    packet.reserve_push_back(1 + 4 * 4 + 4 + 4 * 3 * affected_blocks.size() + 4 * 3);
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
                //    return Response::Answer({packet});
                //}

                Response unloadChunk(int32_t x, int32_t z) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 2);
                    packet.push_back(0x1F);
                    WriteValue<int32_t>(z, packet);
                    WriteValue<int32_t>(x, packet);
                    return Response::Answer({packet});
                }

                Response gameEvent(uint8_t event_id, float value) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 4);
                    packet.push_back(0x20);
                    packet.push_back(event_id);
                    WriteValue<float>(value, packet);
                    return Response::Answer({packet});
                }

                Response openHorseWindow(uint8_t window_id, int32_t slots, int32_t entity_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 2 + 16);
                    packet.push_back(0x21);
                    packet.push_back(window_id);
                    WriteVar<int32_t>(slots, packet);
                    WriteValue<int32_t>(entity_id, packet);
                    return Response::Answer({packet});
                }

                Response hurtAnimation(int32_t entity_id, float yaw) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4);
                    packet.push_back(0x22);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteValue<float>(yaw, packet);
                    return Response::Answer({packet});
                }

                Response initializeWorldBorder(double x, double z, double old_diameter, double new_diameter, int64_t speed_ms, int32_t portal_teleport_boundary, int32_t warning_blocks, int32_t warning_time) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 * 2 + 8 * 2 + 8 + 8 + 8 + 4 * 3);
                    packet.push_back(0x23);
                    WriteValue<double>(x, packet);
                    WriteValue<double>(z, packet);
                    WriteValue<double>(old_diameter, packet);
                    WriteValue<double>(new_diameter, packet);
                    WriteVar<int64_t>(speed_ms, packet);
                    WriteVar<int32_t>(portal_teleport_boundary, packet);
                    WriteVar<int32_t>(warning_blocks, packet);
                    WriteVar<int32_t>(warning_time, packet);
                    return Response::Answer({packet});
                }

                //internal use
                Response keepAlive(int64_t id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8);
                    packet.push_back(0x24);
                    WriteValue<int64_t>(id, packet);
                    return Response::Answer({packet});
                }

                //TODO
                //Response updateChunkDataWLights(...){}

                Response worldEvent(int32_t event, Position pos, int32_t data, bool global) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 8 + 4 + 1);
                    packet.push_back(0x26);
                    WriteVar<int32_t>(event, packet);
                    WriteValue<uint64_t>(pos.raw, packet);
                    WriteVar<int32_t>(data, packet);
                    packet.push_back(global);
                    return Response::Answer({packet});
                }

                Response particle(int32_t particle_id, bool long_distance, calc::VECTOR pos, calc::XYZ<float> offset, float max_speed, int32_t count, list_array<uint8_t> data) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1 + 4 * 3 + 4 * 3 + 4 + 4 + 4 * data.size());
                    packet.push_back(0x27);
                    WriteVar<int32_t>(particle_id, packet);
                    packet.push_back(long_distance);
                    WriteValue<double>(pos.x, packet);
                    WriteValue<double>(pos.y, packet);
                    WriteValue<double>(pos.z, packet);
                    WriteValue<float>(offset.x, packet);
                    WriteValue<float>(offset.y, packet);
                    WriteValue<float>(offset.z, packet);
                    WriteValue<float>(max_speed, packet);
                    WriteVar<int32_t>(count, packet);
                    packet.push_back(data);
                    return Response::Answer({packet});
                }

                //TODO
                //Response updateLight(int32_t chunk_x, int32_t chunk_z, list_array<base_objects::chunk::chunk_light> light) {
                //    list_array<uint8_t> packet;
                //    packet.reserve_push_back(1 + 4 * 2 + 4 * light.size());
                //    packet.push_back(0x28);
                //    WriteVar<int32_t>(chunk_x, packet);
                //    WriteVar<int32_t>(chunk_z, packet);
                //    WriteVar<int32_t>(light.size(), packet);
                //    for (auto& it : light) {
                //        WriteVar<int32_t>(it.y, packet);
                //        WriteVar<int32_t>(it.sky_light, packet);
                //        WriteVar<int32_t>(it.block_light, packet);
                //    }
                //    return Response::Answer({packet});
                //}

                Response joinGame(int32_t entity_id, bool is_hardcore, const list_array<std::string>& dimension_names, int32_t max_players, int32_t view_distance, int32_t simulation_distance, bool reduced_debug_info, bool enable_respawn_screen, bool do_limited_crafting, const std::string& current_dimension_type, const std::string& dimension_name, int64_t hashed_seed, uint8_t gamemode, int8_t prev_gamemode, bool is_debug, bool is_flat, std::optional<base_objects::packets::death_location_data> death_location, int32_t portal_cooldown) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1 + 4 + 1 + 4 + 1 + 4);
                    packet.push_back(0x29);
                    WriteVar<int32_t>(entity_id, packet);
                    packet.push_back(is_hardcore);
                    WriteVar<int32_t>(dimension_names.size(), packet);
                    for (auto& it : dimension_names)
                        WriteIdentifier(packet, it);

                    WriteVar<int32_t>(max_players, packet);
                    WriteVar<int32_t>(view_distance, packet);
                    WriteVar<int32_t>(simulation_distance, packet);
                    packet.push_back(reduced_debug_info);
                    packet.push_back(enable_respawn_screen);
                    packet.push_back(do_limited_crafting);
                    WriteIdentifier(packet, current_dimension_type);
                    WriteIdentifier(packet, dimension_name);
                    WriteValue<int64_t>(hashed_seed, packet);
                    packet.push_back(gamemode);
                    packet.push_back(prev_gamemode);
                    packet.push_back(is_debug);
                    packet.push_back(is_flat);
                    packet.push_back((bool)death_location);
                    if (death_location) {
                        WriteValue(death_location->position.raw, packet);
                        WriteIdentifier(packet, death_location->dimension);
                    }
                    WriteVar<int32_t>(portal_cooldown, packet);
                    return Response::Answer({packet});
                }

                Response mapData(int32_t map_id, uint8_t scale, bool locked, const list_array<base_objects::packets::map_icon>& icons, uint8_t columns, uint8_t rows, uint8_t x, uint8_t z, const list_array<uint8_t>& data) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1 + 1 + 4 * 2 + 1 + 1 + 1 + 1 + 2 * icons.size() + 4 * 4 + data.size());
                    packet.push_back(0x2A);
                    WriteVar<int32_t>(map_id, packet);
                    packet.push_back(scale);
                    packet.push_back(locked);
                    packet.push_back(!icons.empty());
                    if (!icons.empty()) {
                        WriteVar<int32_t>(icons.size(), packet);
                        for (auto& [name, type, x, z, direction] : icons) {
                            WriteVar<int32_t>(type, packet);
                            packet.push_back(x);
                            packet.push_back(z);
                            packet.push_back(direction);
                            packet.push_back((bool)name);
                            if (name)
                                packet.push_back(name->ToTextComponent());
                        }
                    }
                    packet.push_back(columns);
                    if (columns) {
                        packet.push_back(rows);
                        packet.push_back(x);
                        packet.push_back(z);
                        WriteVar<int32_t>(data.size(), packet);
                        packet.push_back(data);
                    }
                    return Response::Answer({packet});
                }

                Response merchantOffers(int32_t window_id, int32_t trade_id, const list_array<base_objects::packets::trade> trades, int32_t level, int32_t experience, bool regular_villager, bool can_restock) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 + 4 * 2 + 1 + 1 + 1 + 1 + 1 + 2 * trades.size());
                    packet.push_back(0x2B);
                    packet.push_back(window_id);
                    packet.push_back(trade_id);
                    WriteVar<int32_t>(trades.size(), packet);
                    for (auto& [input1, output, input2, number_of_trade_uses, max_number_of_trades, xp, spec_price, price_multiplier, demand, trade_disabled] : trades) {
                        WriteSlot(packet, input1);
                        WriteSlot(packet, output);
                        WriteSlot(packet, input2);
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
                    return Response::Answer({packet});
                }

                Response updateEntityPosition(int32_t entity_id, calc::XYZ<float> pos, bool on_ground) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * 3 + 1);
                    packet.push_back(0x2C);
                    WriteVar<int32_t>(entity_id, packet);
                    auto res = calc::minecraft::packets::delta_move(pos);
                    WriteValue<int16_t>(res.x, packet);
                    WriteValue<int16_t>(res.y, packet);
                    WriteValue<int16_t>(res.z, packet);
                    packet.push_back(on_ground);
                    return Response::Answer({packet});
                }

                Response updateEntityPositionAndRotation(int32_t entity_id, calc::XYZ<float> pos, calc::VECTOR rot, bool on_ground) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * 3 + 4 * 3 + 1);
                    packet.push_back(0x2D);
                    WriteVar<int32_t>(entity_id, packet);
                    auto res = calc::minecraft::packets::delta_move(pos);
                    WriteValue<int16_t>(res.x, packet);
                    WriteValue<int16_t>(res.y, packet);
                    WriteValue<int16_t>(res.z, packet);
                    auto res2 = calc::to_yaw_pitch_256(rot);
                    WriteValue<uint8_t>(res2.x, packet);
                    WriteValue<uint8_t>(res2.y, packet);
                    packet.push_back(on_ground);
                    return Response::Answer({packet});
                }

                Response updateEntityRotation(int32_t entity_id, calc::VECTOR rot, bool on_ground) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * 2 + 1);
                    packet.push_back(0x2E);
                    WriteVar<int32_t>(entity_id, packet);
                    auto res = calc::to_yaw_pitch_256(rot);
                    WriteValue<uint8_t>(res.x, packet);
                    WriteValue<uint8_t>(res.y, packet);
                    packet.push_back(on_ground);
                    return Response::Answer({packet});
                }

                Response moveVehicle(calc::VECTOR pos, calc::VECTOR rot) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 3 + 4 * 3);
                    packet.push_back(0x2F);
                    WriteValue<double>(pos.x, packet);
                    WriteValue<double>(pos.y, packet);
                    WriteValue<double>(pos.z, packet);
                    auto res = calc::to_yaw_pitch(rot);
                    WriteValue<float>(res.x, packet);
                    WriteValue<float>(res.y, packet);
                    return Response::Answer({packet});
                }

                Response openBook(int32_t hand) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x30);
                    WriteVar<int32_t>(hand, packet);
                    return Response::Answer({packet});
                }

                Response openScreen(int32_t window_id, int32_t type, const Chat& title) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 2 + title.ToStr().size());
                    packet.push_back(0x31);
                    packet.push_back(window_id);
                    WriteVar<int32_t>(type, packet);
                    packet.push_back(title.ToTextComponent());
                    return Response::Answer({packet});
                }

                Response openSignEditor(Position pos, bool is_front_text) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 + 1);
                    packet.push_back(0x32);
                    WriteValue<uint64_t>(pos.raw, packet);
                    packet.push_back(is_front_text);
                    return Response::Answer({packet});
                }

                Response ping(int32_t id) {
                    if (id == -1)
                        throw std::runtime_error("ping id can't be -1");
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x33);
                    WriteVar<int32_t>(id, packet);
                    return Response::Answer({packet});
                }

                Response pingResponse(int32_t id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x34);
                    WriteVar<int32_t>(id, packet);
                    return Response::Answer({packet});
                }

                Response placeGhostRecipe(int32_t windows_id, const std::string& recipe_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + recipe_id.size());
                    packet.push_back(0x35);
                    packet.push_back(windows_id);
                    WriteIdentifier(packet, recipe_id);
                    return Response::Answer({packet});
                }

                Response playerAbilities(uint8_t flags, float flying_speed, float field_of_view) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 4 * 2);
                    packet.push_back(0x36);
                    packet.push_back(flags);
                    WriteValue<float>(flying_speed, packet);
                    WriteValue<float>(field_of_view, packet);
                    return Response::Answer({packet});
                }

                Response playerChatMessage(ENBT::UUID sender, int32_t index, const std::optional<std::array<uint8_t, 256>>& signature, const std::string& message, int64_t timestamp, int64_t salt, const list_array<std::array<uint8_t, 256>>& prev_messages, std::optional<ENBT> __UNDEFINED__FIELD__, int32_t filter_type, const list_array<uint8_t>& filtered_symbols_bitfield, int32_t chat_type, const Chat& sender_name, const std::optional<Chat>& target_name) {
                    if (prev_messages.size() > 20)
                        throw std::runtime_error("too many prev messages");
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 16 + 4 + 1 + message.size() + 8 + 8 + 4 * prev_messages.size() + 1 + 1 + 1 + 2 * 4 + 1 + (bool)target_name * 2);
                    packet.push_back(0x37);
                    WriteUUID(sender, packet);
                    WriteVar<int32_t>(index, packet);
                    packet.push_back((bool)signature);
                    if (signature)
                        packet.push_back(signature->data(), signature->size());

                    WriteString(packet, message, 256);
                    WriteValue<int64_t>(timestamp, packet);
                    WriteValue<int64_t>(salt, packet);
                    WriteVar<int32_t>(prev_messages.size(), packet);
                    for (auto& msg_signature : prev_messages) {
                        WriteVar<int32_t>(0, packet);
                        packet.push_back(msg_signature.data(), msg_signature.size());
                    }
                    packet.push_back((bool)__UNDEFINED__FIELD__);
                    if (__UNDEFINED__FIELD__)
                        packet.push_back(NBT(*__UNDEFINED__FIELD__).get_as_normal());

                    WriteVar<int32_t>(filter_type, packet);
                    if (filter_type == 2) { //PARTIALLY_FILTERED
                        size_t need_bytes_for_message = (message.size() + 7) / 8;
                        if (filtered_symbols_bitfield.size() != need_bytes_for_message)
                            throw std::runtime_error("invalid filtered symbols bitfield size");
                        packet.push_back(filtered_symbols_bitfield);
                    }

                    WriteVar<int32_t>(chat_type, packet);
                    packet.push_back(sender_name.ToTextComponent());
                    packet.push_back((bool)target_name);
                    if (target_name)
                        packet.push_back(target_name->ToTextComponent());

                    return Response::Answer({packet});
                }

                //UNUSED by Notchian client
                Response endCombat(int32_t duration) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x38);
                    WriteVar<int32_t>(duration, packet);
                    return Response::Answer({packet});
                }

                //UNUSED by Notchian client
                Response enterCombat() {
                    return Response::Answer({{0x39}});
                }

                Response combatDeath(int32_t player_id, const Chat& message) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x3A);
                    WriteVar<int32_t>(player_id, packet);
                    packet.push_back(message.ToTextComponent());
                    return Response::Answer({packet});
                }

                Response playerInfoRemove(const list_array<ENBT::UUID>& players) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + players.size() * sizeof(ENBT::UUID));
                    packet.push_back(0x3B);
                    WriteVar<int32_t>(players.size(), packet);
                    for (auto& it : players)
                        WriteUUID(it, packet);
                    return Response::Answer({packet});
                }

                Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_add>& add_players) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x3C);
                    packet.push_back(0x01);
                    WriteVar<int32_t>(add_players.size(), packet);
                    for (auto& [uuid, name, properties] : add_players) {
                        WriteUUID(uuid, packet);
                        WriteString(packet, name, 16);
                        WriteVar<int32_t>(properties.size(), packet);
                        for (auto& [name, value, signature] : properties) {
                            WriteString(packet, name, 32767);
                            WriteString(packet, value, 32767);
                            packet.push_back((bool)signature);
                            if (signature)
                                packet.push_back((uint8_t*)signature->data(), signature->size());
                        }
                    }
                    return Response::Answer({packet});
                }

                Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_initialize_chat>& initialize_chat) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x3C);
                    packet.push_back(0x02);
                    WriteVar<int32_t>(initialize_chat.size(), packet);
                    for (auto& [uuid, chat_session_id, public_key_expiry_time, public_key, public_key_signature] : initialize_chat) {
                        if (public_key.size() > 512)
                            throw std::runtime_error("public key too long");
                        if (public_key_signature.size() > 4096)
                            throw std::runtime_error("public key signature too long");
                        WriteUUID(uuid, packet);
                        packet.push_back((bool)chat_session_id);
                        if (chat_session_id)
                            WriteUUID(*chat_session_id, packet);
                        WriteValue<int64_t>(public_key_expiry_time, packet);
                        WriteVar<int32_t>(public_key.size(), packet);
                        packet.push_back(public_key);
                        WriteVar<int32_t>(public_key_signature.size(), packet);
                        packet.push_back(public_key_signature);
                    }
                    return Response::Answer({packet});
                }

                Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_update_gamemode>& update_game_mode) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x3C);
                    packet.push_back(0x04);
                    WriteVar<int32_t>(update_game_mode.size(), packet);
                    for (auto& [uuid, game_mode] : update_game_mode) {
                        WriteUUID(uuid, packet);
                        packet.push_back(game_mode);
                    }
                    return Response::Answer({packet});
                }

                Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_update_listed>& update_listed) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x3C);
                    packet.push_back(0x08);
                    WriteVar<int32_t>(update_listed.size(), packet);
                    for (auto& [uuid, listed] : update_listed) {
                        WriteUUID(uuid, packet);
                        packet.push_back(listed);
                    }
                    return Response::Answer({packet});
                }

                Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_update_latency>& update_latency) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x3C);
                    packet.push_back(0x10);
                    WriteVar<int32_t>(update_latency.size(), packet);
                    for (auto& [uuid, latency] : update_latency) {
                        WriteUUID(uuid, packet);
                        WriteVar<int32_t>(latency, packet);
                    }
                    return Response::Answer({packet});
                }

                Response playerInfoUpdate(const list_array<base_objects::packets::player_actions_update_display_name>& update_display_name) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x3C);
                    packet.push_back(0x20);
                    WriteVar<int32_t>(update_display_name.size(), packet);
                    for (auto& [uuid, display_name] : update_display_name) {
                        WriteUUID(uuid, packet);
                        packet.push_back((bool)display_name);
                        if (display_name)
                            packet.push_back(display_name->ToTextComponent());
                    }
                    return Response::Answer({packet});
                }

                Response lookAt(bool from_feet_or_eyes, calc::VECTOR target, std::optional<std::pair<int32_t, bool>> entity_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 4 * 3 + (bool)entity_id * 4 * 2);
                    packet.push_back(0x3D);
                    packet.push_back(from_feet_or_eyes);
                    WriteValue<float>(target.x, packet);
                    WriteValue<float>(target.y, packet);
                    WriteValue<float>(target.z, packet);
                    packet.push_back((bool)entity_id);
                    if (entity_id) {
                        WriteVar<int32_t>(entity_id->first, packet); //entity id
                        packet.push_back(entity_id->second);         // to entity foot or eyes
                    }
                    return Response::Answer({packet});
                }

                Response synchronizePlayerPosition(calc::VECTOR pos, float yaw, float pitch, uint8_t flags, int32_t teleport_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 3 + 4 * 2 + 1);
                    packet.push_back(0x3E);
                    WriteValue<double>(pos.x, packet);
                    WriteValue<double>(pos.y, packet);
                    WriteValue<double>(pos.z, packet);
                    WriteValue<float>(yaw, packet);
                    WriteValue<float>(pitch, packet);
                    packet.push_back(flags);
                    WriteVar<int32_t>(teleport_id, packet);
                    return Response::Answer({packet});
                }

                Response initRecipeBook(bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> displayed_recipe_ids, list_array<std::string> had_access_to_recipe_ids) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 2 * displayed_recipe_ids.size() + 2 * had_access_to_recipe_ids.size());
                    packet.push_back(0x3F);
                    packet.push_back(0);
                    packet.push_back(crafting_recipe_book_open);
                    packet.push_back(crafting_recipe_book_filter_active);
                    packet.push_back(smelting_recipe_book_open);
                    packet.push_back(smelting_recipe_book_filter_active);
                    packet.push_back(blast_furnace_recipe_book_open);
                    packet.push_back(blast_furnace_recipe_book_filter_active);
                    packet.push_back(smoker_recipe_book_open);
                    packet.push_back(smoker_recipe_book_filter_active);
                    WriteVar<int32_t>(displayed_recipe_ids.size(), packet);
                    for (auto& it : displayed_recipe_ids)
                        WriteIdentifier(packet, it);
                    WriteVar<int32_t>(had_access_to_recipe_ids.size(), packet);
                    for (auto& it : had_access_to_recipe_ids)
                        WriteIdentifier(packet, it);
                    return Response::Answer({packet});
                }

                Response updateRecipeBook(bool add_remove, bool crafting_recipe_book_open, bool crafting_recipe_book_filter_active, bool smelting_recipe_book_open, bool smelting_recipe_book_filter_active, bool blast_furnace_recipe_book_open, bool blast_furnace_recipe_book_filter_active, bool smoker_recipe_book_open, bool smoker_recipe_book_filter_active, list_array<std::string> recipe_ids) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 2 * recipe_ids.size());
                    packet.push_back(0x3F);
                    WriteVar<int32_t>(1 + add_remove, packet);
                    packet.push_back(crafting_recipe_book_open);
                    packet.push_back(crafting_recipe_book_filter_active);
                    packet.push_back(smelting_recipe_book_open);
                    packet.push_back(smelting_recipe_book_filter_active);
                    packet.push_back(blast_furnace_recipe_book_open);
                    packet.push_back(blast_furnace_recipe_book_filter_active);
                    packet.push_back(smoker_recipe_book_open);
                    packet.push_back(smoker_recipe_book_filter_active);
                    WriteVar<int32_t>(recipe_ids.size(), packet);
                    for (auto& it : recipe_ids)
                        WriteIdentifier(packet, it);
                    return Response::Answer({packet});
                }

                Response removeEntities(const list_array<int32_t>& entity_ids) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * entity_ids.size());
                    packet.push_back(0x40);
                    WriteVar<int32_t>(entity_ids.size(), packet);
                    for (auto& it : entity_ids)
                        WriteVar<int32_t>(it, packet);
                    return Response::Answer({packet});
                }

                Response removeEntityEffect(int32_t entity_id, int32_t effect_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 2);
                    packet.push_back(0x41);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteVar<int32_t>(effect_id, packet);
                    return Response::Answer({packet});
                }

                Response resetScore(const std::string& entity_name, const std::optional<std::string>& objective_name) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + entity_name.size() + (bool)objective_name * objective_name->size());
                    packet.push_back(0x42);
                    WriteString(packet, entity_name, 32767);
                    packet.push_back((bool)objective_name);
                    if (objective_name)
                        WriteString(packet, *objective_name, 32767);
                    return Response::Answer({packet});
                }

                Response removeResourcePacks() {
                    return Response::Answer({{0x43, false}});
                }

                Response removeResourcePack(ENBT::UUID id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 16);
                    packet.push_back(0x43);
                    packet.push_back(true);
                    WriteUUID(id, packet);
                    return Response::Answer({packet});
                }

                Response addResourcePack(ENBT::UUID id, const std::string& url, const std::string& hash, bool forced, const std::optional<Chat>& prompt) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 16 + url.size() + hash.size() + 1);
                    packet.push_back(0x44);
                    WriteUUID(id, packet);
                    WriteString(packet, url, 32767);
                    WriteString(packet, hash, 40);
                    packet.push_back(forced);
                    packet.push_back((bool)prompt);
                    if (prompt)
                        packet.push_back(prompt->ToTextComponent());
                    return Response::Answer({packet});
                }

                Response respawn(const std::string& dimension_type, const std::string& dimension_name, long hashed_seed, uint8_t gamemode, uint8_t previous_gamemode, bool is_debug, bool is_flat, const std::optional<base_objects::packets::death_location_data>& death_location, int32_t portal_cooldown, bool keep_attributes, bool keep_metadata) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + dimension_type.size() + dimension_name.size() + 8 + 1 + 1 + 1 + 1 + 4 + 1 + 1);
                    packet.push_back(0x45);
                    WriteIdentifier(packet, dimension_type);
                    WriteIdentifier(packet, dimension_name);
                    WriteValue<int64_t>(hashed_seed, packet);
                    packet.push_back(gamemode);
                    packet.push_back(previous_gamemode);
                    packet.push_back(is_debug);
                    packet.push_back(is_flat);
                    packet.push_back((bool)death_location);
                    if (death_location) {
                        WriteValue(death_location->position.raw, packet);
                        WriteIdentifier(packet, death_location->dimension);
                    }
                    WriteVar<int32_t>(portal_cooldown, packet);
                    uint8_t data_kept = keep_attributes;
                    data_kept |= keep_metadata << 1;
                    packet.push_back(data_kept);
                    return Response::Answer({packet});
                }

                Response setHeadRotation(int32_t entity_id, calc::VECTOR head_rotation) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * 2);
                    packet.push_back(0x46);
                    WriteVar<int32_t>(entity_id, packet);
                    auto res = calc::to_yaw_pitch_256(head_rotation);
                    packet.push_back(res.x);
                    packet.push_back(res.y);
                    return Response::Answer({packet});
                }

                Response updateSectionBlocks(int32_t section_x, int32_t section_z, int32_t section_y, const list_array<base_objects::compressed_block_state>& blocks) {
                    list_array<uint8_t> packet;
                    int64_t section_pos = ((section_x & 0x3FFFFF) << 42) | ((section_z & 0x3FFFFF) << 20) | (section_y & 0xFFFFF);
                    packet.reserve_push_back(1 + 8 + 5 * blocks.size() * 6);
                    packet.push_back(0x47);
                    WriteValue<int64_t>(section_pos, packet);
                    WriteVar<int32_t>(blocks.size(), packet);
                    for (auto& it : blocks)
                        WriteVar<int64_t>(it.value, packet);
                    return Response::Answer({packet});
                }

                Response setAdvancementsTab(const std::optional<std::string>& tab_id) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x48);
                    packet.push_back((bool)tab_id);
                    if (tab_id)
                        WriteIdentifier(packet, *tab_id);
                    return Response::Answer({packet});
                }

                Response serverData(const Chat& motd, const std::optional<list_array<uint8_t>>& icon_png, bool secure_chat) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x49);
                    packet.push_back(motd.ToTextComponent());
                    packet.push_back((bool)icon_png);
                    if (icon_png) {
                        WriteVar<int32_t>(icon_png->size(), packet);
                        packet.push_back(*icon_png);
                    }
                    packet.push_back(secure_chat);
                    return Response::Answer({packet});
                }

                Response setActionBarText(const Chat& text) {
                    return Response::Answer({{0x4A, text.ToTextComponent()}});
                }

                Response setBorderCenter(double x, double z) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 * 2);
                    packet.push_back(0x4B);
                    WriteValue<double>(x, packet);
                    WriteValue<double>(z, packet);
                    return Response::Answer({packet});
                }

                Response setBorderLerp(double old_diameter, double new_diameter, int64_t speed_ms) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 * 2 + 8);
                    packet.push_back(0x4C);
                    WriteValue<double>(old_diameter, packet);
                    WriteValue<double>(new_diameter, packet);
                    WriteVar<int64_t>(speed_ms, packet);
                    return Response::Answer({packet});
                }

                Response setBorderSize(double diameter) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8);
                    packet.push_back(0x4D);
                    WriteValue<double>(diameter, packet);
                    return Response::Answer({packet});
                }

                Response setBorderWarningDelay(int32_t warning_delay) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x4E);
                    WriteVar<int32_t>(warning_delay, packet);
                    return Response::Answer({packet});
                }

                Response setBorderWarningDistance(int32_t warning_distance) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x4F);
                    WriteVar<int32_t>(warning_distance, packet);
                    return Response::Answer({packet});
                }

                Response setCamera(int32_t entity_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x50);
                    WriteVar<int32_t>(entity_id, packet);
                    return Response::Answer({packet});
                }

                Response setHeldItem(uint8_t slot) {
                    if (slot > 8)
                        throw std::runtime_error("invalid slot");
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x51);
                    packet.push_back(slot);
                    return Response::Answer({packet});
                }

                Response setCenterChunk(int32_t x, int32_t z) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 2);
                    packet.push_back(0x52);
                    WriteVar<int32_t>(x, packet);
                    WriteVar<int32_t>(z, packet);
                    return Response::Answer({packet});
                }

                Response setRenderDistance(int32_t render_distance) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x53);
                    WriteVar<int32_t>(render_distance, packet);
                    return Response::Answer({packet});
                }

                Response setDefaultSpawnPosition(Position pos, float angle) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 + 1 + 8);
                    packet.push_back(0x54);
                    WriteValue(pos.raw, packet);
                    WriteValue<float>(angle, packet);
                    return Response::Answer({packet});
                }

                Response displayObjective(int32_t position, const std::string& objective_name) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + objective_name.size());
                    packet.push_back(0x55);
                    packet.push_back(position);
                    WriteString(packet, objective_name, 32767);
                    return Response::Answer({packet});
                }

                Response setEntityMetadata(int32_t entity_id, const list_array<uint8_t>& metadata) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + metadata.size());
                    packet.push_back(0x56);
                    WriteVar<int32_t>(entity_id, packet);
                    packet.push_back(metadata);
                    return Response::Answer({packet});
                }

                Response linkEntities(int32_t attached_entity_id, int32_t holder_entity_id) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 2);
                    packet.push_back(0x57);
                    WriteValue<int32_t>(attached_entity_id, packet);
                    WriteValue<int32_t>(holder_entity_id, packet);
                    return Response::Answer({packet});
                }

                Response setEntityVelocity(int32_t entity_id, calc::VECTOR velocity) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * 3);
                    packet.push_back(0x58);
                    WriteVar<int32_t>(entity_id, packet);
                    auto res = calc::minecraft::packets::velocity(velocity);
                    WriteValue<int16_t>(res.x, packet);
                    WriteValue<int16_t>(res.y, packet);
                    WriteValue<int16_t>(res.z, packet);
                    return Response::Answer({packet});
                }

                Response setEquipment(int32_t entity_id, uint8_t slot, const base_objects::slot& item) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1);
                    packet.push_back(0x59);
                    WriteVar<int32_t>(entity_id, packet);
                    packet.push_back(slot);
                    WriteSlot(packet, item);
                    return Response::Answer({packet});
                }

                Response setExperience(float experience_bar, int32_t level, int32_t total_experience) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * 2);
                    packet.push_back(0x5A);
                    WriteValue<float>(experience_bar, packet);
                    WriteVar<int32_t>(level, packet);
                    WriteVar<int32_t>(total_experience, packet);
                    return Response::Answer({packet});
                }

                Response setHealth(float health, int32_t food, float saturation) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 + 4);
                    packet.push_back(0x5B);
                    WriteValue<float>(health, packet);
                    WriteVar<int32_t>(food, packet);
                    WriteValue<float>(saturation, packet);
                    return Response::Answer({packet});
                }

                Response updateObjectivesCreate(const std::string& objective_name, const Chat& display_name, int32_t render_type) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + objective_name.size() + 1 + 1 + 1);
                    packet.push_back(0x5C);
                    WriteString(packet, objective_name, 32767);
                    packet.push_back(0);
                    packet.push_back(display_name.ToTextComponent());
                    packet.push_back(render_type);
                    packet.push_back(0);
                    return Response::Answer({packet});
                }

                Response updateObjectivesCreateStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const ENBT& style) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + objective_name.size() + 1 + 1 + 1);
                    packet.push_back(0x5C);
                    WriteString(packet, objective_name, 32767);
                    packet.push_back(0);
                    packet.push_back(display_name.ToTextComponent());
                    packet.push_back(render_type);
                    packet.push_back(true);
                    packet.push_back(1);
                    packet.push_back(NBT(style).get_as_network());
                    return Response::Answer({packet});
                }

                Response updateObjectivesCreateFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + objective_name.size() + 1 + 1 + 1);
                    packet.push_back(0x5C);
                    WriteString(packet, objective_name, 32767);
                    packet.push_back(0);
                    packet.push_back(display_name.ToTextComponent());
                    packet.push_back(render_type);
                    packet.push_back(true);
                    packet.push_back(2);
                    packet.push_back(content.ToTextComponent());
                    return Response::Answer({packet});
                }

                Response updateObjectivesRemove(const std::string& objective_name) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + objective_name.size());
                    packet.push_back(0x5C);
                    WriteString(packet, objective_name, 32767);
                    packet.push_back(1);
                    return Response::Answer({packet});
                }

                Response updateObjectivesInfo(const std::string& objective_name, const Chat& display_name, int32_t render_type) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + objective_name.size() + 1 + 1 + 1);
                    packet.push_back(0x5C);
                    WriteString(packet, objective_name, 32767);
                    packet.push_back(2);
                    packet.push_back(display_name.ToTextComponent());
                    packet.push_back(render_type);
                    packet.push_back(false);
                    return Response::Answer({packet});
                }

                Response updateObjectivesInfoStyled(const std::string& objective_name, const Chat& display_name, int32_t render_type, const ENBT& style) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + objective_name.size() + 1 + 1 + 1);
                    packet.push_back(0x5C);
                    WriteString(packet, objective_name, 32767);
                    packet.push_back(2);
                    packet.push_back(display_name.ToTextComponent());
                    packet.push_back(render_type);
                    packet.push_back(true);
                    packet.push_back(1);
                    packet.push_back(NBT(style).get_as_network());
                    return Response::Answer({packet});
                }

                Response updateObjectivesInfoFixed(const std::string& objective_name, const Chat& display_name, int32_t render_type, const Chat& content) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + objective_name.size() + 1 + 1 + 1);
                    packet.push_back(0x5C);
                    WriteString(packet, objective_name, 32767);
                    packet.push_back(2);
                    packet.push_back(display_name.ToTextComponent());
                    packet.push_back(render_type);
                    packet.push_back(true);
                    packet.push_back(2);
                    packet.push_back(content.ToTextComponent());
                    return Response::Answer({packet});
                }

                Response setPassengers(int32_t vehicle_entity_id, const list_array<int32_t>& passengers) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * passengers.size());
                    packet.push_back(0x5D);
                    WriteVar<int32_t>(vehicle_entity_id, packet);
                    WriteVar<int32_t>(passengers.size(), packet);
                    for (auto& it : passengers)
                        WriteVar<int32_t>(it, packet);
                    return Response::Answer({packet});
                }

                Response updateTeamCreate(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix, const list_array<std::string>& entities) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + team_name.size() + name_tag_visibility.size() + collision_rule.size() + 4 * 3 + 1 + 4);
                    packet.push_back(0x5E);
                    WriteString(packet, team_name, 32767);
                    packet.push_back(0);
                    packet.push_back(display_name.ToTextComponent());
                    packet.push_back(int8_t(allow_fire_co_teamer) & (int8_t(see_invisible_co_teamer) << 1));
                    WriteString(packet, name_tag_visibility, 40);
                    WriteString(packet, collision_rule, 40);
                    WriteVar<int32_t>(team_color, packet);
                    packet.push_back(prefix.ToTextComponent());
                    packet.push_back(suffix.ToTextComponent());
                    WriteVar<int32_t>(entities.size(), packet);
                    for (auto& it : entities)
                        WriteString(packet, it, 32767);
                    return Response::Answer({packet});
                }

                Response updateTeamRemove(const std::string& team_name) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + team_name.size());
                    packet.push_back(0x5E);
                    WriteString(packet, team_name, 32767);
                    packet.push_back(1);
                    return Response::Answer({packet});
                }

                Response updateTeamInfo(const std::string& team_name, const Chat& display_name, bool allow_fire_co_teamer, bool see_invisible_co_teamer, const std::string& name_tag_visibility, const std::string& collision_rule, int32_t team_color, const Chat& prefix, const Chat& suffix) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + team_name.size() + name_tag_visibility.size() + collision_rule.size() + 4 * 3);
                    packet.push_back(0x5E);
                    WriteString(packet, team_name, 32767);
                    packet.push_back(2);
                    packet.push_back(display_name.ToTextComponent());
                    packet.push_back(int8_t(allow_fire_co_teamer) & (int8_t(see_invisible_co_teamer) << 1));
                    WriteString(packet, name_tag_visibility, 40);
                    WriteString(packet, collision_rule, 40);
                    WriteVar<int32_t>(team_color, packet);
                    packet.push_back(prefix.ToTextComponent());
                    packet.push_back(suffix.ToTextComponent());
                    return Response::Answer({packet});
                }

                Response updateTeamAddEntities(const std::string& team_name, const list_array<std::string>& entities) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + team_name.size() + 4);
                    packet.push_back(0x5E);
                    WriteString(packet, team_name, 32767);
                    packet.push_back(3);
                    WriteVar<int32_t>(entities.size(), packet);
                    for (auto& it : entities)
                        WriteString(packet, it, 32767);
                    return Response::Answer({packet});
                }

                Response updateTeamRemoveEntities(const std::string& team_name, const list_array<std::string>& entities) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + team_name.size() + 4);
                    packet.push_back(0x5E);
                    WriteString(packet, team_name, 32767);
                    packet.push_back(4);
                    WriteVar<int32_t>(entities.size(), packet);
                    for (auto& it : entities)
                        WriteString(packet, it, 32767);
                    return Response::Answer({packet});
                }

                Response setScore(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + entity_name.size() + objective_name.size() + 4 + 1 + 1);
                    packet.push_back(0x5F);
                    WriteString(packet, entity_name, 32767);
                    WriteString(packet, objective_name, 32767);
                    WriteVar<int32_t>(value, packet);
                    packet.push_back((bool)display_name);
                    if (display_name)
                        packet.push_back(display_name->ToTextComponent());
                    packet.push_back(0);
                    return Response::Answer({packet});
                }

                Response setScoreStyled(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, const ENBT& styled) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + entity_name.size() + objective_name.size() + 4 + 1 + 1);
                    packet.push_back(0x5F);
                    WriteString(packet, entity_name, 32767);
                    WriteString(packet, objective_name, 32767);
                    WriteVar<int32_t>(value, packet);
                    packet.push_back((bool)display_name);
                    if (display_name)
                        packet.push_back(display_name->ToTextComponent());
                    packet.push_back(1);
                    packet.push_back(NBT(styled).get_as_network());
                    return Response::Answer({packet});
                }

                Response setScoreFixed(const std::string& entity_name, const std::string& objective_name, int32_t value, const std::optional<Chat>& display_name, Chat content) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + entity_name.size() + objective_name.size() + 4 + 1 + 1);
                    packet.push_back(0x5F);
                    WriteString(packet, entity_name, 32767);
                    WriteString(packet, objective_name, 32767);
                    WriteVar<int32_t>(value, packet);
                    packet.push_back((bool)display_name);
                    if (display_name)
                        packet.push_back(display_name->ToTextComponent());
                    packet.push_back(2);
                    packet.push_back(content.ToTextComponent());
                    return Response::Answer({packet});
                }

                Response setSimulationDistance(float distance) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x60);
                    WriteValue<float>(distance, packet);
                    return Response::Answer({packet});
                }

                Response setSubtitleText(const Chat& text) {
                    return Response::Answer({{0x61, text.ToTextComponent()}});
                }

                Response updateTime(int64_t world_age, int64_t time_of_day) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 8 * 2);
                    packet.push_back(0x62);
                    WriteVar<int64_t>(world_age, packet);
                    WriteVar<int64_t>(time_of_day, packet);
                    return Response::Answer({packet});
                }

                Response setTitleText(const Chat& text) {
                    return Response::Answer({{0x63, text.ToTextComponent()}});
                }

                Response setTitleAnimationTimes(int32_t fade_in, int32_t stay, int32_t fade_out) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 3);
                    packet.push_back(0x64);
                    WriteValue<int32_t>(fade_in, packet);
                    WriteValue<int32_t>(stay, packet);
                    WriteValue<int32_t>(fade_out, packet);
                    return Response::Answer({packet});
                }

                Response entitySoundEffect(uint32_t sound_id, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
                    if (volume < 0 || volume > 1)
                        throw std::runtime_error("invalid volume value");
                    if (pitch < 0.5 || pitch > 2.0)
                        throw std::runtime_error("invalid pitch value");

                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 3 + 4 * 2 + 8);
                    packet.push_back(0x65);
                    WriteVar<int32_t>(sound_id + 1, packet);
                    WriteVar<int32_t>(category, packet);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteValue<float>(volume, packet);
                    WriteValue<float>(pitch, packet);
                    WriteValue<int64_t>(seed, packet);
                    return Response::Answer({packet});
                }

                Response entitySoundEffect(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t entity_id, float volume, float pitch, int64_t seed) {
                    if (volume < 0 || volume > 1)
                        throw std::runtime_error("invalid volume value");
                    if (pitch < 0.5 || pitch > 2.0)
                        throw std::runtime_error("invalid pitch value");

                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 3 + 4 * 2 + 8);
                    packet.push_back(0x65);
                    packet.push_back(0);
                    WriteIdentifier(packet, sound_id);
                    packet.push_back((bool)range);
                    if (range)
                        WriteValue<float>(*range, packet);
                    WriteVar<int32_t>(category, packet);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteValue<float>(volume, packet);
                    WriteValue<float>(pitch, packet);
                    WriteValue<int64_t>(seed, packet);
                    return Response::Answer({packet});
                }

                Response soundEffect(uint32_t sound_id, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
                    if (volume < 0 || volume > 1)
                        throw std::runtime_error("invalid volume value");
                    if (pitch < 0.5 || pitch > 2.0)
                        throw std::runtime_error("invalid pitch value");

                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 3 + 4 * 3 + 4 * 2 + 8);
                    packet.push_back(0x66);
                    WriteVar<int32_t>(sound_id + 1, packet);
                    WriteVar<int32_t>(category, packet);
                    WriteValue<int32_t>(x, packet);
                    WriteValue<int32_t>(y, packet);
                    WriteValue<int32_t>(z, packet);
                    WriteValue<float>(volume, packet);
                    WriteValue<float>(pitch, packet);
                    WriteValue<int64_t>(seed, packet);
                    return Response::Answer({packet});
                }

                Response soundEffect(const std::string& sound_id, std::optional<float> range, int32_t category, int32_t x, int32_t y, int32_t z, float volume, float pitch, int64_t seed) {
                    if (volume < 0 || volume > 1)
                        throw std::runtime_error("invalid volume value");
                    if (pitch < 0.5 || pitch > 2.0)
                        throw std::runtime_error("invalid pitch value");

                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 3 + 4 * 3 + 4 * 2 + 8);
                    packet.push_back(0x66);
                    packet.push_back(0);
                    WriteIdentifier(packet, sound_id);
                    packet.push_back((bool)range);
                    if (range)
                        WriteValue<float>(*range, packet);
                    WriteVar<int32_t>(category, packet);
                    WriteValue<int32_t>(x, packet);
                    WriteValue<int32_t>(y, packet);
                    WriteValue<int32_t>(z, packet);
                    WriteValue<float>(volume, packet);
                    WriteValue<float>(pitch, packet);
                    WriteValue<int64_t>(seed, packet);
                    return Response::Answer({packet});
                }

                //internal
                Response startConfiguration() {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1);
                    packet.push_back(0x67);
                    return Response::Answer({packet});
                }

                Response stopSound(uint8_t flags) {
                    if (flags & 0x01 || flags & 0x02)
                        throw std::runtime_error("invalid params for this flag");
                    return Response::Answer({{0x68, flags}});
                }

                Response stopSound(uint8_t flags, int32_t source) {
                    if (flags & 0x02)
                        throw std::runtime_error("invalid params for this flag");
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 4);
                    packet.push_back(0x68);
                    packet.push_back(flags);
                    WriteVar<int32_t>(source, packet);
                    return Response::Answer({packet});
                }

                Response stopSound(uint8_t flags, const std::string& sound) {
                    if (flags & 0x01)
                        throw std::runtime_error("invalid params for this flag");
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + sound.size());
                    packet.push_back(0x68);
                    packet.push_back(flags);
                    WriteIdentifier(packet, sound);
                    return Response::Answer({packet});
                }

                Response stopSound(uint8_t flags, int32_t source, const std::string& sound) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 1 + 4 + sound.size());
                    packet.push_back(0x68);
                    packet.push_back(flags);
                    WriteVar<int32_t>(source, packet);
                    WriteIdentifier(packet, sound);
                    return Response::Answer({packet});
                }

                Response systemChatMessage(const Chat& message) {
                    return Response::Answer({{0x69, message.ToTextComponent(), false}});
                }

                Response systemChatMessageOverlay(const Chat& message) {
                    return Response::Answer({{0x69, message.ToTextComponent(), true}});
                }

                Response setTabListHeaderAndFooter(const Chat& header, const Chat& footer) {
                    return Response::Answer({{0x6A, header.ToTextComponent(), footer.ToTextComponent()}});
                }

                Response tagQueryResponse(int32_t transaction_id, const ENBT& nbt) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x6B);
                    WriteVar<int32_t>(transaction_id, packet);
                    packet.push_back(NBT(nbt).get_as_network());
                    return Response::Answer({packet});
                }

                Response pickupItem(int32_t collected_entity_id, int32_t collector_entity_id, int32_t pickup_item_count) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 * 3);
                    packet.push_back(0x6C);
                    WriteVar<int32_t>(collected_entity_id, packet);
                    WriteVar<int32_t>(collector_entity_id, packet);
                    WriteVar<int32_t>(pickup_item_count, packet);
                    return Response::Answer({packet});
                }

                Response teleportEntity(int32_t entity_id, calc::VECTOR pos, float yaw, float pitch, bool on_ground) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 4 * 3 + 4 * 2 + 1);
                    packet.push_back(0x6D);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteValue<double>(pos.x, packet);
                    WriteValue<double>(pos.y, packet);
                    WriteValue<double>(pos.z, packet);
                    WriteValue<float>(yaw, packet);
                    WriteValue<float>(pitch, packet);
                    packet.push_back(on_ground);
                    return Response::Answer({packet});
                }

                Response setTickingState(float tick_rate, bool is_frozen) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1);
                    packet.push_back(0x6E);
                    WriteValue<float>(tick_rate, packet);
                    packet.push_back(is_frozen);
                    return Response::Answer({packet});
                }

                Response stepTick(int32_t step_count) {
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4);
                    packet.push_back(0x6F);
                    WriteVar<int32_t>(step_count, packet);
                    return Response::Answer({packet});
                }

                Response updateAdvancements(bool reset, const list_array<base_objects::packets::advancements_maping> advancement_mapping, const list_array<std::string>& remove_advancements, const list_array<base_objects::packets::advancement_progress> progress_advancements) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x70);
                    packet.push_back(reset);
                    WriteVar<int32_t>(advancement_mapping.size(), packet);
                    for (auto& item : advancement_mapping) {
                        WriteIdentifier(packet, item.key);
                        packet.push_back((bool)item.parent);
                        if (item.parent)
                            WriteIdentifier(packet, *item.parent);
                        packet.push_back((bool)item.display);
                        if (item.display) {
                            auto display = *item.display;
                            packet.push_back(display.title.ToTextComponent());
                            packet.push_back(display.description.ToTextComponent());
                            WriteSlot(packet, display.icon);
                            WriteVar<int32_t>(display.frame_type, packet);
                            //automaticaly set background_texture flag(0x01) to true if it has value
                            WriteValue(display.flags & (int32_t)display.background_texture.has_value(), packet);
                            if (display.background_texture)
                                WriteIdentifier(packet, *display.background_texture);
                            WriteValue(display.x, packet);
                            WriteValue(display.y, packet);
                        }
                    }
                    WriteVar<int32_t>(remove_advancements.size(), packet);
                    for (auto& it : remove_advancements)
                        WriteIdentifier(packet, it);
                    WriteVar<int32_t>(progress_advancements.size(), packet);
                    for (auto& [advancement, progress] : progress_advancements) {
                        WriteIdentifier(packet, advancement);
                        WriteVar<int32_t>(progress.size(), packet);
                        for (auto& [criterion, criterion_progress] : progress) {
                            WriteIdentifier(packet, criterion);
                            packet.push_back(criterion_progress.achieved);
                            if (criterion_progress.achieved)
                                WriteValue<int64_t>(criterion_progress.date, packet);
                        }
                    }
                    return Response::Answer({packet});
                }

                Response updateAttributes(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x71);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteVar<int32_t>(properties.size(), packet);
                    for (auto& [key, value, modifiers] : properties) {
                        WriteString(packet, key, 32767);
                        WriteValue<double>(value, packet);
                        WriteVar<int32_t>(modifiers.size(), packet);
                        for (auto& [uuid, amount, operation] : modifiers) {
                            WriteUUID(uuid, packet);
                            WriteValue<double>(amount, packet);
                            packet.push_back(operation);
                        }
                    }
                    return Response::Answer({packet});
                }

                Response entityEffect(int32_t entity_id, int32_t effect_id, int8_t amplifier, int32_t duration, int8_t flags, std::optional<ENBT> factor_codec) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x72);
                    WriteVar<int32_t>(entity_id, packet);
                    WriteVar<int32_t>(effect_id, packet);
                    packet.push_back(amplifier);
                    WriteVar<int32_t>(duration, packet);
                    packet.push_back(flags);
                    packet.push_back((bool)factor_codec);
                    if (factor_codec)
                        packet.push_back(NBT(*factor_codec).get_as_network());
                    return Response::Answer({packet});
                }

                //TODO:
                //Response updateRecipes()

                Response updateTags(const list_array<base_objects::packets::tag_mapping>& tag_mappings) {
                    list_array<uint8_t> packet;
                    packet.push_back(0x74);
                    WriteVar<int32_t>(tag_mappings.size(), packet);
                    for (auto& [registry, values] : tag_mappings) {
                        WriteIdentifier(packet, registry);
                        WriteVar<int32_t>(values.size(), packet);
                        for (auto& [tag, entries] : values) {
                            WriteIdentifier(packet, tag);
                            WriteVar<int32_t>(entries.size(), packet);
                            for (auto& entry : entries)
                                WriteVar<int32_t>(entry, packet);
                        }
                    }
                    return Response::Answer({packet});
                }
            }
        }
    }
}