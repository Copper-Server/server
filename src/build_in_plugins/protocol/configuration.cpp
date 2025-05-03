#include <src/api/configuration.hpp>
#include <src/api/protocol.hpp>
#include <src/base_objects/network/tcp/accept_packet_registry.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets/abstract.hpp>
#include <src/protocolHelper/util.hpp>
#include <src/registers.hpp>

namespace copper_server::build_in_plugins::protocol::play_770 {
    namespace configuration {
        void client_settings(base_objects::network::tcp::session* session, ArrayStream& packet) {
            auto& shared_data = session->sharedData();
            shared_data.locale = packet.read_string(16);
            shared_data.view_distance = packet.read();
            shared_data.chat_mode = (base_objects::SharedClientData::ChatMode)packet.read_var<int32_t>();
            shared_data.enable_chat_colors = packet.read();
            shared_data.skin_parts.mask = packet.read();
            shared_data.main_hand = (base_objects::SharedClientData::MainHand)packet.read_var<int32_t>();
            shared_data.enable_filtering = packet.read();
            shared_data.allow_server_listings = packet.read();
            shared_data.particle_status = (base_objects::SharedClientData::ParticleStatus)packet.read_var<int32_t>();
        }

        void cookie_response(base_objects::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::cookie_response data;
            data.key = packet.read_identifier();
            if (packet.read())
                data.payload = packet.read_array<uint8_t>(5120);
            api::protocol::on_cookie_response.async_notify({data, *session, session->sharedDataRef()});
            if (auto plugin = pluginManagement.get_bind_cookies(PluginManagement::registration_on::configuration, data.key); plugin) {
                if (auto response = plugin->OnConfigurationCookie(plugin, data.key, data.payload ? *data.payload : list_array<uint8_t>{}, session->sharedDataRef()); response)
                    session->sharedData().sendPacket(std::move(*response));
            }
        }

        void plugin_message(base_objects::network::tcp::session* session, ArrayStream& packet) {
            std::string channel = packet.read_string(32769);
            auto it = pluginManagement.get_bind_plugin(PluginManagement::registration_on::configuration, channel);
            if (it != nullptr)
                if (auto res = it->OnConfigurationHandle(it, channel, packet.read_left(32767).to_vector(), session->sharedDataRef()); res)
                    session->sharedData().sendPacket(std::move(*res));
        }

        void configuration_complete(base_objects::network::tcp::session* session, ArrayStream& packet) {
            auto& shared_data = session->sharedData();
            if (shared_data.packets_state.load_state != base_objects::SharedClientData::packets_state_t::configuration_load_state_t::done) {
                shared_data.sendPacket(packets::configuration::kick(session->sharedData(), "Configuration is not finished."));
                return;
            }
            if (!session->sharedData().packets_state.pending_resource_packs.empty()) {
                shared_data.sendPacket(packets::configuration::kick(session->sharedData(), "You are not downloaded all requested packs."));
                return;
            }
            shared_data.packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::play;
            shared_data.switchToHandler(client_handler::abstract::createhandle_play(session));
        }

        void keep_alive(base_objects::network::tcp::session* session, ArrayStream& packet) {
            int64_t keep_alive_packet_response = packet.read_value<int64_t>();
            session->sharedData().gotKeepAlive(keep_alive_packet_response);
        }

        void pong(base_objects::network::tcp::session* session, ArrayStream& packet) {
            auto& shared_data = session->sharedData();
            int32_t pong = packet.read_value<int32_t>();
            if (pong == shared_data.packets_state.excepted_pong)
                shared_data.packets_state.excepted_pong = 0;
            else {
                shared_data.sendPacket(packets::configuration::kick(session->sharedData(), "Invalid pong"));
                return;
            }
            session->sharedData().ping = std::chrono::duration_cast<std::chrono::milliseconds>(shared_data.packets_state.pong_timer - std::chrono::system_clock::now());
        }

        void registry_resource_pack(base_objects::network::tcp::session* session, ArrayStream& packet) {
            enbt::raw_uuid id = packet.read_uuid();
            int32_t result = packet.read_var<int32_t>();
            auto res = session->sharedData().packets_state.pending_resource_packs.find(id);
            if (res != session->sharedData().packets_state.pending_resource_packs.end()) {
                switch (result) {
                case 0:
                case 3:
                    session->sharedData().packets_state.active_resource_packs.insert(id);
                    session->sharedData().packets_state.pending_resource_packs.erase(res);
                    break;
                default:
                    if (res->second.required)
                        session->sharedData().sendPacket(packets::configuration::kick(session->sharedData(), "Resource pack is required"));
                    else
                        session->sharedData().packets_state.pending_resource_packs.erase(res);
                }
            }
        }

        void known_packs(base_objects::network::tcp::session* session, ArrayStream& packet) {
            int32_t len = packet.read_var<int32_t>();
            list_array<base_objects::data_packs::known_pack> packs;
            for (int32_t i = 0; i < len; i++) {
                std::string name = packet.read_string(32769);
                std::string id = packet.read_string(32769);
                std::string version = packet.read_string(32769);

                packs.emplace_back(std::move(name), std::move(id), std::move(version));
            }
            session->sharedData().sendPacket(
                packets::configuration::registry_data(session->sharedData())
            );

            {
                base_objects::packets::tag_mapping block;
                block.registry = "minecraft:block";
                for (auto& [id, values] : registers::tags.at("block").at("minecraft")) {
                    block.tags.push_back(
                        {.tag_name = id,
                         .entires = values.convert_fn(
                             [&session](auto& it) -> int32_t {
                                 return base_objects::block::get_block_id(it, session->protocol_version);
                             }
                         )}
                    );
                }

                base_objects::packets::tag_mapping item;
                item.registry = "minecraft:item";
                for (auto& [id, values] : registers::tags.at("item").at("minecraft")) {
                    item.tags.push_back(
                        {.tag_name = id,
                         .entires = values.convert_fn(
                             [&session](auto& it) -> int32_t {
                                 return base_objects::item_id_t(it).to_protocol(session->protocol_version);
                             }
                         )}
                    );
                }
                base_objects::packets::tag_mapping fluid;
                fluid.registry = "minecraft:fluid";
                for (auto& [id, values] : registers::tags.at("fluid").at("minecraft")) {
                    fluid.tags.push_back(
                        {.tag_name = id,
                         .entires = registers::convert_reg_pro_id("minecraft:fluid", values, session->protocol_version)
                        }
                    );
                }
                base_objects::packets::tag_mapping worldgen_biome;
                worldgen_biome.registry = "minecraft:worldgen/biome";
                for (auto& [id, values] : registers::tags.at("worldgen/biome").at("minecraft")) {
                    worldgen_biome.tags.push_back(
                        {.tag_name = id,
                         .entires = values.convert_fn([&session](auto& it) -> int32_t {
                             return registers::biomes.at(it).id;
                         })
                        }
                    );
                }
                base_objects::packets::tag_mapping entity_type;
                entity_type.registry = "minecraft:entity_type";
                for (auto& [id, values] : registers::tags.at("entity_type").at("minecraft")) {
                    entity_type.tags.push_back(
                        {.tag_name = id,
                         .entires = registers::convert_reg_pro_id("minecraft:entity_type", values, session->protocol_version)
                        }
                    );
                }

                base_objects::packets::tag_mapping game_event;
                game_event.registry = "minecraft:game_event";
                for (auto& [id, values] : registers::tags.at("game_event").at("minecraft")) {
                    game_event.tags.push_back(
                        {.tag_name = id,
                         .entires = registers::convert_reg_pro_id("minecraft:game_event", values, session->protocol_version)
                        }
                    );
                }


                session->sharedData().sendPacket(
                    packets::configuration::updateTags(
                        session->sharedData(),
                        {
                            std::move(block),
                            std::move(item),
                            std::move(fluid),
                            std::move(entity_type),
                            std::move(game_event),
                            std::move(worldgen_biome),
                        }
                    )
                );
            }

            pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::configuration, [session, &packs](PluginRegistrationPtr plugin) {
                if (auto res = plugin->OnConfiguration_gotKnownPacks(session->sharedDataRef(), packs); res)
                    session->sharedData().sendPacket(std::move(*res));
            });

            session->sharedData().packets_state.load_state = base_objects::SharedClientData::packets_state_t::configuration_load_state_t::await_processing;
        }
    }

    class ProtocolSupport_configuration : public PluginAutoRegister<"protocol_support_for_configuration_state_universal", ProtocolSupport_configuration> {
    public:
        void OnRegister(const PluginRegistrationPtr& self) override {
            base_objects::network::tcp::packet_registry.serverbound.configuration.register_seq(
                770,
                {
                    {"client_settings", configuration::client_settings},
                    {"cookie_response", configuration::cookie_response},
                    {"plugin_message", configuration::plugin_message},
                    {"configuration_complete", configuration::configuration_complete},
                    {"keep_alive", configuration::keep_alive},
                    {"pong", configuration::pong},
                    {"registry_resource_pack", configuration::registry_resource_pack},
                    {"known_packs", configuration::known_packs},
                }
            );
        }
    };
}