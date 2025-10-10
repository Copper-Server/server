/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_SERVER_BOUND_CONFIGURATION
#define SRC_API_PACKETS_SERVER_BOUND_CONFIGURATION
#include <library/enbt/enbt.hpp>
#include <optional>
#include <src/api/packets/ops.hpp>
#include <src/api/packets/types.hpp>

namespace copper_server::api::packets::server_bound::config {
    struct client_information : public packet<0x00> {
        enum class chat_mode_e : uint8_t {
            disabled = 0,
            commands_only = 1,
            hidden = 2,
        };
        enum class displayer_skin_parts_f : uint8_t {
            cape = 0x1,
            jacket = 0x2,
            left_sleeve = 0x4,
            right_sleeve = 0x8,
            left_pants = 0x10,
            right_pants = 0x20,
            hat = 0x40,
            _unused = 0x80
        };
        enum class main_hand_e : uint8_t {
            left = 0,
            right = 1
        };
        enum class particle_status_e : uint8_t {
            all = 0,
            decreased = 1,
            minimal = 2,
        };
        string_sized<16> locale;
        uint8_t view_distance;
        enum_as<chat_mode_e, var_int32> chat_mode;
        bool enable_chat_colors;
        enum_as_flag<displayer_skin_parts_f, uint8_t> displayed_skin_parts;
        enum_as<main_hand_e, var_int32> main_hand;
        bool enable_text_filtering;
        bool allow_server_listings;
        enum_as<particle_status_e, var_int32> particle_status;
    };

    struct cookie_response : public packet<0x01> {
        identifier key;
        std::optional<list_array_sized<uint8_t, 5120>> payload = std::nullopt;
    };

    struct custom_payload : public packet<0x02> {
        identifier channel;
        list_array_sized_siz_from_packet<uint8_t, 32767> payload;
    };

    struct finish_configuration : public packet<0x03>, switches_to::play {};

    struct keep_alive : public packet<0x04> {
        uint64_t keep_alive_id;
    };

    struct pong : public packet<0x05> {
        ordered_id<int32_t, "ping"> ping_request_id;
    };

    struct resource_pack : public packet<0x06> {
        enum class result_e : uint8_t {
            success = 0,
            declined = 1,
            download_failed = 2,
            accepted = 3,
            downloaded = 4,
            invalid_url = 5,
            reload_failed = 6,
            discarded = 7
        };
        using enum result_e;
        enbt::raw_uuid uuid;
        enum_as<result_e, var_int32> result;
    };

    struct select_known_packs : public packet<0x07> {
        struct pack {
            std::string _namespace;
            std::string id;
            std::string version;
        };

        list_array<pack> packs;
    };

    struct custom_click_action : public packet<0x08> {
        identifier id;
        enbt::value payload;
    };

    struct accept_code_of_conduct : public packet<0x09> {};
}

namespace copper_server::api::packets {
    extern template packet_ops<server_bound::config::client_information>;
    extern template packet_ops<server_bound::config::cookie_response>;
    extern template packet_ops<server_bound::config::custom_payload>;
    extern template packet_ops<server_bound::config::finish_configuration>;
    extern template packet_ops<server_bound::config::keep_alive>;
    extern template packet_ops<server_bound::config::pong>;
    extern template packet_ops<server_bound::config::resource_pack>;
    extern template packet_ops<server_bound::config::select_known_packs>;
    extern template packet_ops<server_bound::config::custom_click_action>;
    extern template packet_ops<server_bound::config::accept_code_of_conduct>;
    using server_bound_config_ops = state_ops<
        server_bound::config::client_information,
        server_bound::config::cookie_response,
        server_bound::config::custom_payload,
        server_bound::config::finish_configuration,
        server_bound::config::keep_alive,
        server_bound::config::pong,
        server_bound::config::resource_pack,
        server_bound::config::select_known_packs,
        server_bound::config::custom_click_action,
        server_bound::config::accept_code_of_conduct>;
}


#endif /* SRC_API_PACKETS_SERVER_BOUND_CONFIGURATION */
