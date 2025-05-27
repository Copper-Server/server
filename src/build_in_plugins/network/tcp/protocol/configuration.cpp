#include <src/api/configuration.hpp>
#include <src/api/packets.hpp>
#include <src/api/protocol.hpp>
#include <src/api/tags.hpp>
#include <src/base_objects/network/tcp/accept_packet_registry.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/build_in_plugins/network/tcp/client_handler/abstract.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>
#include <src/plugin/main.hpp>
#include <src/registers.hpp>

namespace copper_server::build_in_plugins::network::tcp::protocol {
    namespace configuration {
        void client_settings(api::network::tcp::session* session, ArrayStream& packet) {
            auto& shared_data = session->shared_data();
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

        void cookie_response(api::network::tcp::session* session, ArrayStream& packet) {
            api::protocol::data::cookie_response data;
            data.key = packet.read_identifier();
            if (packet.read())
                data.payload = packet.read_array<uint8_t>(5120);
            api::protocol::on_cookie_response.async_notify({data, *session, session->shared_data_ref()});
            if (auto plugin = pluginManagement.get_bind_cookies(PluginManagement::registration_on::configuration, data.key); plugin) {
                if (auto response = plugin->OnConfigurationCookie(plugin, data.key, data.payload ? *data.payload : list_array<uint8_t>{}, session->shared_data_ref()); response)
                    session->shared_data().sendPacket(std::move(*response));
            }
        }

        void plugin_message(api::network::tcp::session* session, ArrayStream& packet) {
            std::string channel = packet.read_string(32769);
            auto it = pluginManagement.get_bind_plugin(PluginManagement::registration_on::configuration, channel);
            if (it != nullptr)
                if (auto res = it->OnConfigurationHandle(it, channel, packet.read_left(32767).to_vector(), session->shared_data_ref()); res)
                    session->shared_data().sendPacket(std::move(*res));
        }

        void configuration_complete(api::network::tcp::session* session, ArrayStream& packet) {
            auto& shared_data = session->shared_data();
            if (shared_data.packets_state.load_state != base_objects::SharedClientData::packets_state_t::configuration_load_state_t::done) {
                shared_data.sendPacket(api::packets::configuration::kick(session->shared_data(), "Configuration is not finished."));
                return;
            }
            if (!session->shared_data().packets_state.pending_resource_packs.empty()) {
                shared_data.sendPacket(api::packets::configuration::kick(session->shared_data(), "You are not downloaded all requested packs."));
                return;
            }
            shared_data.packets_state.state = base_objects::SharedClientData::packets_state_t::protocol_state::play;
            shared_data.switchToHandler(client_handler::abstract::createhandle_play(session));
        }

        void keep_alive(api::network::tcp::session* session, ArrayStream& packet) {
            int64_t keep_alive_packet_response = packet.read_value<int64_t>();
            session->shared_data().gotKeepAlive(keep_alive_packet_response);
        }

        void pong(api::network::tcp::session* session, ArrayStream& packet) {
            auto& shared_data = session->shared_data();
            int32_t pong = packet.read_value<int32_t>();
            if (pong == shared_data.packets_state.excepted_pong)
                shared_data.packets_state.excepted_pong = 0;
            else {
                shared_data.sendPacket(api::packets::configuration::kick(session->shared_data(), "Invalid pong"));
                return;
            }
            session->shared_data().ping = std::chrono::duration_cast<std::chrono::milliseconds>(shared_data.packets_state.pong_timer - std::chrono::system_clock::now());
        }

        void registry_resource_pack(api::network::tcp::session* session, ArrayStream& packet) {
            enbt::raw_uuid id = packet.read_uuid();
            int32_t result = packet.read_var<int32_t>();
            auto res = session->shared_data().packets_state.pending_resource_packs.find(id);
            if (res != session->shared_data().packets_state.pending_resource_packs.end()) {
                switch (result) {
                case 0:
                case 3:
                    session->shared_data().packets_state.active_resource_packs.insert(id);
                    session->shared_data().packets_state.pending_resource_packs.erase(res);
                    break;
                default:
                    if (res->second.required)
                        session->shared_data().sendPacket(api::packets::configuration::kick(session->shared_data(), "Resource pack is required"));
                    else
                        session->shared_data().packets_state.pending_resource_packs.erase(res);
                }
            }
        }

        void known_packs(api::network::tcp::session* session, ArrayStream& packet) {
            int32_t len = packet.read_var<int32_t>();
            list_array<base_objects::data_packs::known_pack> packs;
            for (int32_t i = 0; i < len; i++) {
                std::string name = packet.read_string(32769);
                std::string id = packet.read_string(32769);
                std::string version = packet.read_string(32769);

                packs.emplace_back(std::move(name), std::move(id), std::move(version));
            }
            session->shared_data().sendPacket(
                api::packets::configuration::registry_data(session->shared_data())
            );

            {
                base_objects::packets::tag_mapping block;
                block.registry = "minecraft:block";
                for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::block, "minecraft")) {
                    block.tags.push_back(
                        {.tag_name = id,
                         .entires = values.convert_fn(
                             [&session](auto& it) -> int32_t {
                                 return base_objects::block::get_protocol_block_id(it, session->protocol_version);
                             }
                         )}
                    );
                }

                base_objects::packets::tag_mapping item;
                item.registry = "minecraft:item";
                for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::item, "minecraft")) {
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
                for (auto& [id, values] : api::tags::view_tag("minecraft:fluid", "minecraft")) {
                    fluid.tags.push_back(
                        {.tag_name = id,
                         .entires = registers::convert_reg_pro_id("minecraft:fluid", values, session->protocol_version)
                        }
                    );
                }
                base_objects::packets::tag_mapping worldgen_biome;
                worldgen_biome.registry = "minecraft:worldgen/biome";
                for (auto& [id, values] : api::tags::view_tag("minecraft:worldgen/biome", "minecraft")) {
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
                for (auto& [id, values] : api::tags::view_tag(api::tags::builtin_entry::entity_type, "minecraft")) {
                    entity_type.tags.push_back(
                        {.tag_name = id,
                         .entires = values.convert_fn([&session](auto& it) -> int32_t { return base_objects::entity_data::get_entity(it).internal_entity_aliases.at(session->protocol_version); })}
                    );
                }

                base_objects::packets::tag_mapping game_event;
                game_event.registry = "minecraft:game_event";
                for (auto& [id, values] : api::tags::view_tag("minecraft:game_event", "minecraft")) {
                    game_event.tags.push_back(
                        {.tag_name = id,
                         .entires = registers::convert_reg_pro_id("minecraft:game_event", values, session->protocol_version)
                        }
                    );
                }


                session->shared_data().sendPacket(
                    api::packets::configuration::updateTags(
                        session->shared_data(),
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
                if (auto res = plugin->OnConfiguration_gotKnownPacks(session->shared_data_ref(), packs); res)
                    session->shared_data().sendPacket(std::move(*res));
            });

            session->shared_data().packets_state.load_state = base_objects::SharedClientData::packets_state_t::configuration_load_state_t::await_processing;
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