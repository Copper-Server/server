/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/json.hpp>
#include <library/fast_task.hpp>
#include <src/api/configuration.hpp>
#include <src/api/mojang/session_server.hpp>
#include <src/base_objects/uuid.hpp>
#include <src/util/conversions.hpp>
#include <src/util/mojang/api/http.hpp>

namespace copper_server::api::mojang {
    std::shared_ptr<session_server::player_data> session_server::hasJoined(const std::string& username, const std::string& serverId, bool online_mode, std::optional<std::string> player_ip, bool cache_result) {
        auto cache_key = cache.find(username);
        if (cache_key != cache.end()) {
            if (!cache_key->second->online_data)
                if (!online_mode)
                    return cache_key->second;

            if (std::chrono::system_clock::now() - cache_key->second->last_check < cache_duration)
                return cache_key->second;
            cache.erase(cache_key);
        }

        if (online_mode) {
            std::string response = util::mojang::api::http::get((std::string)copper_server::api::configuration::server_configuration::Mojang::session_server, "/session/minecraft/hasJoined?username=" + username + "&serverId=" + serverId + (player_ip ? ("&ip=" + *player_ip) : ""));
            auto value = boost::json::parse(response).as_object();
            player_data data;

            data.uuid = copper_server::util::conversions::uuid::from(value["id"].as_string());
            data.uuid_str = copper_server::util::conversions::uuid::to(data.uuid);
            data.online_data = true;
            if (value.contains("properties")) {
                std::vector<player_data::property> properties;
                properties.reserve(value["properties"].as_array().size());
                for (auto& prop : value["properties"].as_array()) {
                    auto& tree = prop.as_object();
                    player_data::property convert;
                    convert.name = tree["name"].as_string();
                    convert.name = tree["value"].as_string();
                    if (tree.contains("signature"))
                        convert.signature = tree["signature"].as_string();
                    properties.push_back(std::move(convert));
                }
                data.properties = std::move(properties);
            }
            data.last_check = std::chrono::system_clock::now();
            if (cache_result)
                return cache[username] = std::make_shared<player_data>(std::move(data));
            else
                return std::make_shared<player_data>(std::move(data));

        } else {
            auto uuid = base_objects::uuid::create_offline(username);
            if (cache_result)
                return (cache[username] = std::make_shared<player_data>(copper_server::util::conversions::uuid::to(uuid), uuid, std::chrono::system_clock::now(), false));
            else
                return std::make_shared<player_data>(copper_server::util::conversions::uuid::to(uuid), uuid, std::chrono::system_clock::now(), false);
        }
    }

    session_server& get_session_server() {
        static session_server res;
        return res;
    }

    void get_mojang_certificate_public_keys(std::function<void(const std::vector<std::string>&)>&& fn) {
        struct data_t {
            std::vector<std::string> keys;
            std::chrono::system_clock::time_point point = std::chrono::system_clock::time_point::min();
        };

        static fast_task::protected_value<data_t> res;
        bool need_update = false;
        do {
            if (need_update) {
                res.set([](auto& data) {
                    data.keys.clear();
                    std::string response = util::mojang::api::http::get((std::string)copper_server::api::configuration::server_configuration::Mojang::services_server, "/publickeys");
                    auto value = boost::json::parse(response).as_object();
                    auto& arr = value.at("playerCertificateKeys").as_array();
                    for (auto& it : arr)
                        data.keys.push_back((std::string)it.at("publicKey").as_string());
                    data.point = std::chrono::system_clock::now();
                });
                need_update = false;
            }
            need_update = res.get([&fn](auto& data) {
                if (data.keys.empty())
                    return true;
                if (std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now() - data.point).count())
                    return true;
                fn(data.keys);
                return false;
            });
        } while (need_update);
    }
}