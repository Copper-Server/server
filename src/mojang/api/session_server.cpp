/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/json.hpp>
#include <src/mojang/api/http.hpp>
#include <src/mojang/api/session_server.hpp>
#include <src/util/conversions.hpp>

namespace mojang::api {
    const std::shared_ptr<session_server::player_data> session_server::hasJoined(const std::string& username, const std::string& serverId, bool online_mode, bool cache_result) {
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
            std::string response = http::get("sessionserver.mojang.com", "/session/minecraft/hasJoined?username=" + username + "&serverId=" + serverId);
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
            auto uuid = enbt::raw_uuid::from_string(username);
            if (cache_result)
                return (cache[username] = std::make_shared<player_data>(copper_server::util::conversions::uuid::to(uuid), uuid, std::chrono::system_clock::now(), false));
            else
                return std::make_shared<player_data>(copper_server::util::conversions::uuid::to(uuid), uuid, std::chrono::system_clock::now(), false);
        }
    }
}