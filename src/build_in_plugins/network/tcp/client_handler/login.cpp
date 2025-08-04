#include <src/api/configuration.hpp>
#include <src/api/mojang/session_server.hpp>
#include <src/api/network/tcp.hpp>
#include <src/api/new_packets.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/mojang/api/hash.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::network::tcp {
    struct tcp_login : public PluginAutoRegister<"network/tcp_login", tcp_login> {
        struct extra_data_t {
            uint8_t stage = 0;
            uint8_t verify_token[4];
            list_array<std::pair<std::string, PluginRegistrationPtr>> plugins_query;

            static extra_data_t& get(base_objects::SharedClientData& client) {
                if (!client.packets_state.extra_data) {
                    auto allocated = new extra_data_t{};
                    client.packets_state.extra_data = std::shared_ptr<void>((void*)allocated, [](void* d) { delete reinterpret_cast<extra_data_t*>(d); });
                    pluginManagement.inspect_plugin_bind(PluginManagement::registration_on::login, [&allocated](const std::pair<std::string, PluginRegistrationPtr>& it) {
                        allocated->plugins_query.push_back(it);
                    });
                }
                return *reinterpret_cast<extra_data_t*>(client.packets_state.extra_data.get());
            }
        };

        static void log_success(base_objects::SharedClientData& client) {
            extra_data_t::get(client).stage = 3;
            if (api::configuration::get().server.offline_mode)
                client.data = api::mojang::get_session_server().hasJoined(client.name, "", false);
            if (!client.data)
                client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Invalid protocol state, 0").ToStr()};
            else {
                std::vector<api::new_packets::client_bound::login::login_finished::property> properties;
                properties.reserve(client.data->properties.size());
                for (auto& it : client.data->properties)
                    properties.emplace_back(it.name, it.value, it.signature);

                client << api::new_packets::client_bound::login::login_finished{
                    .uuid = client.data->uuid,
                    .user_name = client.name,
                    .properties = std::move(properties)
                };
            }
        }

        static bool process_plugin_resp(PluginRegistration::login_response&& resp, base_objects::SharedClientData& client) {
            return std::visit(
                [&](auto&& it) -> bool {
                    using T = std::decay_t<decltype(it)>;
                    if constexpr (std::is_same_v<T, PluginRegistration::login_response::none>)
                        return false;
                    else if constexpr (std::is_same_v<T, PluginRegistration::login_response::request_cookie>) {
                        client << api::new_packets::client_bound::login::cookie_request{.key = it.identifier};
                        return true;
                    } else {
                        client << api::new_packets::client_bound::login::custom_query{
                            .channel = it.identifier,
                            .payload = std::move(it.data)
                        };
                        return true;
                    }
                },
                std::move(resp.value)
            );
        }

        static void switch_to_plugin_processing_stage(base_objects::SharedClientData& client) {
            if (extra_data_t::get(client).plugins_query.empty()) {
                log_success(client);
            } else {
                extra_data_t::get(client).stage = 2;
                bool sent = false;
                do {
                    auto& cur = extra_data_t::get(client).plugins_query.back();
                    sent = process_plugin_resp(cur.second->OnLoginStart(cur.second, cur.first, client), client);
                    if (!sent)
                        extra_data_t::get(client).plugins_query.take_back();
                } while (!sent);
            }
        }

        void OnRegister(const PluginRegistrationPtr&) override {
            using hello = api::new_packets::server_bound::login::hello;
            using cookie_response = api::new_packets::server_bound::login::cookie_response;
            using custom_query_answer = api::new_packets::server_bound::login::custom_query_answer;
            using key = api::new_packets::server_bound::login::key;
            using login_acknowledged = api::new_packets::server_bound::login::login_acknowledged;
            api::new_packets::register_server_bound_processor<hello>([](hello&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).stage != 0) {
                    client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Invalid protocol state, 0").ToStr()};
                    return;
                }

                client.data->uuid = packet.uuid;
                auto player = api::players::get_player(packet.name);
                if (player) {
                    if (api::configuration::get().protocol.connection_conflict == api::configuration::ServerConfiguration::Protocol::connection_conflict_t::prevent_join) {
                        client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Someone already connected with this nickname").ToStr()};
                        return;
                    } else
                        api::players::calls::on_player_kick({player, "Someone already connected with this nickname"});
                }
                client.name = packet.name;

                if (int32_t compression = api::configuration::get().protocol.compression_threshold; compression != -1)
                    client << api::new_packets::client_bound::login::login_compression{.threshold = compression};
                if (api::configuration::get().protocol.enable_encryption || !api::configuration::get().server.offline_mode) {
                    extra_data_t::get(client).stage = 1;
                    auto public_key = api::network::tcp::public_key_buffer();
                    static auto generate_ui8 = []() -> uint8_t {
                        static std::random_device rd;
                        static std::mt19937_64 gen;
                        static std::uniform_int_distribution<uint16_t> dis;
                        uint16_t ui16 = dis(gen);
                        return (uint8_t)((ui16 & 0xFF ^ (ui16 >> 8)) & 0xFF);
                    };
                    auto& vft = extra_data_t::get(client).verify_token;
                    vft[0] = generate_ui8();
                    vft[1] = generate_ui8();
                    vft[2] = generate_ui8();
                    vft[3] = generate_ui8();

                    client << api::new_packets::client_bound::login::hello{
                        .server_id = "",
                        .public_key = {public_key.begin(), public_key.end()},
                        .verify_token = {vft, vft + 4},
                        .should_authenticate = !api::configuration::get().server.offline_mode
                    };
                } else
                    switch_to_plugin_processing_stage(client);
            });
            api::new_packets::register_server_bound_processor<cookie_response>([](cookie_response&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).stage == 2) {
                    if (auto plugin = pluginManagement.get_bind_cookies(PluginManagement::registration_on::login, packet.key); plugin) {
                        auto response = plugin->OnLoginCookie(plugin, packet.key, packet.payload ? *packet.payload : list_array<uint8_t>{}, !!packet.payload, client);
                        process_plugin_resp(std::move(response), client);
                    }
                } else
                    client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Invalid protocol state, 2").ToStr()};
            });
            api::new_packets::register_server_bound_processor<custom_query_answer>([](custom_query_answer&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).stage == 2) {
                    bool sent = false;
                    do {
                        auto& cur = extra_data_t::get(client).plugins_query.back();
                        sent = process_plugin_resp(cur.second->OnLoginHandle(cur.second, cur.first, to_list_array(packet.payload), packet.payload.size(), client), client);
                        if (!sent) {
                            extra_data_t::get(client).plugins_query.take_back();
                            if (extra_data_t::get(client).plugins_query.empty()) {
                                log_success(client);
                                return;
                            }
                        }
                    } while (!sent);
                } else
                    client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Invalid protocol state, 2").ToStr()};
            });
            api::new_packets::register_server_bound_processor<key>([](key&& packet, base_objects::SharedClientData& client) {
                if (extra_data_t::get(client).stage == 1) {
                    auto vft = to_list_array(packet.verify_token);
                    if (!api::network::tcp::decrypt_data(vft)) {
                        client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Encryption error, invalid verify token").ToStr()};
                        return;
                    }
                    if (memcmp(vft.data(), extra_data_t::get(client).verify_token, 4)) {
                        client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Encryption error, invalid verify token").ToStr()};
                        return;
                    }
                    auto shs = to_list_array(packet.shared_secret);
                    if (!api::network::tcp::decrypt_data(shs)) {
                        client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Encryption error").ToStr()};
                        return;
                    }

                    mojang::api::hash serverId;
                    serverId.update(shs);
                    serverId.update(api::network::tcp::public_key_buffer().data(), api::network::tcp::public_key_buffer().size());

                    client.data = api::mojang::get_session_server().hasJoined(
                        client.name,
                        serverId.hexdigest(),
                        !api::configuration::get().server.offline_mode
                    );
                    client.get_session()->start_symmetric_encryption(shs, shs);
                    switch_to_plugin_processing_stage(client);
                } else
                    client << api::new_packets::client_bound::login::login_disconnect{.reason = Chat("Invalid protocol state, 1").ToStr()};
            });
            api::new_packets::register_server_bound_processor<login_acknowledged>([](login_acknowledged&& packet, base_objects::SharedClientData& client) {});
        }
    };
}