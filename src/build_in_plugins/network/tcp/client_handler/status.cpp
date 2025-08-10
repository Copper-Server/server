/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/configuration.hpp>
#include <src/api/packets.hpp>
#include <src/api/players.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/plugin/main.hpp>
#include <src/registers.hpp>
#include <src/util/conversions.hpp>

namespace copper_server::build_in_plugins::network::tcp {
    struct tcp_status : public PluginAutoRegister<"network/tcp_status", tcp_status> {
        static inline fast_task::task_mutex cached_mutex;
        static inline std::string cached_icon;
        static inline list_array<std::pair<std::string, enbt::raw_uuid>> sample_cache;
        static inline size_t sample_cache_check_size = size_t(-1);
        static inline const uint8_t* icon_data = nullptr;

        static list_array<std::pair<std::string, enbt::raw_uuid>> online_players_sample() {
            std::lock_guard lock(cached_mutex);
            if (sample_cache_check_size == api::players::size())
                return sample_cache;

            list_array<std::pair<std::string, enbt::raw_uuid>> result;
            size_t i = 0;
            api::players::iterate_players_not_state(copper_server::base_objects::SharedClientData::packets_state_t::protocol_state::initialization, [&](const copper_server::base_objects::SharedClientData& player) {
                if (i < api::configuration::get().status.sample_players_count) {
                    if (!player.data)
                        return true;
                    if (player.allow_server_listings)
                        result.push_back({player.name, player.data->uuid});
                    else
                        result.push_back({"Anonymous Player", enbt::raw_uuid::as_null()});
                    return true;
                } else
                    return false;
            });
            return sample_cache = result;
        }

        static Chat description() {
            return api::configuration::get().status.description;
        }

        //return empty string if no icon, icon must be 64x64 and png format in base64
        static std::string server_icon() {
            auto& icon = api::configuration::get().status.favicon;
            std::lock_guard lock(cached_mutex);
            if (cached_icon.size() > 0)
                if (icon_data == icon.data())
                    return cached_icon;

            if (icon.size() == 0) {
                return "";
            } else {
                icon_data = icon.data();
                return cached_icon = util::conversions::base64::encode(icon);
            }
        }

        static std::string build_response(base_objects::SharedClientData& client) {
            int32_t protocol_version = client.packets_state.protocol_version == (int32_t)registers::current_protocol_id
                                           ? registers::current_protocol_id
                                           : 0;

            std::string res = "{"
                              "\"version\": {"
                              "\"name\": \""
                              + api::configuration::get().status.server_name
                              + "\","
                                "\"protocol\": "
                              + std::to_string(protocol_version) + "},";

            if (api::configuration::get().status.show_players) {
                size_t max_count = api::configuration::get().server.max_players;
                size_t online = api::players::online_players();
                if (max_count == 0)
                    max_count = online + 1;

                res += "\"players\":{\"max\":" + std::to_string(max_count) + ",\"online\":" + std::to_string(online);

                auto players = online_players_sample();
                if (!players.empty()) {
                    res += ",\"sample\":[";
                    for (auto& it : players) {
                        res += "{\"name\":\"" + it.first + "\",\"id\":\"" + UUID2String(it.second) + "\"},";
                    }
                    res.pop_back();
                    res += "]";
                }

                res += "},";
            }


            res += "\"description\":" + description().ToStr();


            std::string base64_fav = server_icon();
            if (base64_fav != "")
                res += ",\"favicon\": \"data:image/png;base64," + base64_fav + "\"";


            res += ",\"preventChatReports\":" + std::string(api::configuration::get().server.prevent_chat_reports ? "true}" : "false}");
            return res;
        }

        void OnRegister(const PluginRegistrationPtr&) override {
            using status_req = api::packets::server_bound::status::status_request;
            using ping_req = api::packets::server_bound::status::ping_response;
            api::packets::register_server_bound_processor<status_req>([](status_req&& packet, base_objects::SharedClientData& client) {
                client << api::packets::client_bound::status::status_response{
                    .json_response = build_response(client)
                };
            });
            api::packets::register_server_bound_processor<ping_req>([](ping_req&& packet, base_objects::SharedClientData& client) {
                client << api::packets::client_bound::status::pong_response{
                    .timestamp = packet.timestamp
                };
            });
        }
    };
}