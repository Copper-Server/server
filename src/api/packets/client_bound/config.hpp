/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_CLIENT_BOUND_CONFIG
#define SRC_API_PACKETS_CLIENT_BOUND_CONFIG

#include <library/enbt/enbt.hpp>
#include <src/api/packets/ops.hpp>
#include <src/api/packets/types.hpp>
#include <src/base_objects/chat.hpp>

namespace copper_server::api::packets::client_bound::config {
    struct cookie_request : public packet<0x00> {
        identifier key;
    };

    struct custom_payload : public packet<0x01> {
        identifier channel;
        list_array_sized_siz_from_packet<uint8_t, 1048576> payload;
    };

    struct disconnect : public packet<0x02>, disconnect_after {
        Chat reason;
    };

    struct finish_configuration : public packet<0x03> {};

    struct keep_alive : public packet<0x04> {
        uint64_t keep_alive_id;
    };

    struct ping : public packet<0x05> {
        ordered_id<int32_t, "ping"> ping_request_id;
    };

    struct reset_chat : public packet<0x06> {};

    struct registry_data : public packet<0x07> {
        struct entry {
            identifier entry_id;
            std::optional<enbt::value> data = std::nullopt;
        };

        identifier registry_id;
        list_array<entry> entries;
    };

    struct resource_pack_pop : public packet<0x08> {
        std::optional<enbt::raw_uuid> uuid = std::nullopt;
    };

    struct resource_pack_push : public packet<0x09> {
        enbt::raw_uuid uuid;
        string_sized<32767> url;
        string_sized<40> hash;
        bool forced;
        std::optional<Chat> prompt_message = std::nullopt;
    };

    struct store_cookie : public packet<0x0A> {
        identifier key;
        list_array_sized<uint8_t, 5120> payload;
    };

    struct transfer : public packet<0x0B> {
        string_sized<32767> host;
        var_int32 port;
    };

    struct update_enabled_features : public packet<0x0C> {
        list_array<identifier> features;
    };

    struct update_tags : public packet<0x0D> {
        struct tag {
            identifier tag_name;
            list_array<var_int32> values;
        };

        struct entry {
            identifier registry_id;
            list_array<tag> tags;
        };

        list_array<entry> entries;
    };

    struct select_known_packs : public packet<0x0E> {
        struct pack {
            string_sized<32767> pack_namespace;
            string_sized<32767> id;
            string_sized<32767> version;
        };

        list_array<pack> packs;
    };

    struct custom_report_details : public packet<0x0F> {
        struct detail {
            string_sized<128> title;
            string_sized<4096> description;
        };

        list_array_sized<detail, 32> details;
    };

    struct server_links : public packet<0x10> {
        enum class link_type : uint8_t {
            bug_report = 0,
            community_guidelines = 1,
            support = 2,
            status = 3,
            feedback = 4,
            community = 5,
            website = 6,
            forums = 7,
            news = 8,
            announcements = 9,
        };
        using enum link_type;

        struct link {
            or_<enum_as<link_type, var_int32>, Chat> label;
            std::string url;
        };

        list_array<link> links;
    };

    struct clear_dialog : public packet<0x11> {};

    struct show_dialog : public packet<0x12> {
        enbt::value dialog;
    };

    struct code_of_conduct : public packet<0x13> {
        string_sized<32767> text;
    };
}

namespace copper_server::api::packets {
    extern template packet_ops<client_bound::config::cookie_request>;
    extern template packet_ops<client_bound::config::custom_payload>;
    extern template packet_ops<client_bound::config::disconnect>;
    extern template packet_ops<client_bound::config::finish_configuration>;
    extern template packet_ops<client_bound::config::keep_alive>;
    extern template packet_ops<client_bound::config::ping>;
    extern template packet_ops<client_bound::config::reset_chat>;
    extern template packet_ops<client_bound::config::registry_data>;
    extern template packet_ops<client_bound::config::resource_pack_pop>;
    extern template packet_ops<client_bound::config::resource_pack_push>;
    extern template packet_ops<client_bound::config::store_cookie>;
    extern template packet_ops<client_bound::config::transfer>;
    extern template packet_ops<client_bound::config::update_enabled_features>;
    extern template packet_ops<client_bound::config::update_tags>;
    extern template packet_ops<client_bound::config::select_known_packs>;
    extern template packet_ops<client_bound::config::custom_report_details>;
    extern template packet_ops<client_bound::config::server_links>;
    extern template packet_ops<client_bound::config::clear_dialog>;
    extern template packet_ops<client_bound::config::show_dialog>;
    extern template packet_ops<client_bound::config::code_of_conduct>;

    using client_bound_config_ops = state_ops<
        client_bound::config::cookie_request,
        client_bound::config::custom_payload,
        client_bound::config::disconnect,
        client_bound::config::finish_configuration,
        client_bound::config::keep_alive,
        client_bound::config::ping,
        client_bound::config::reset_chat,
        client_bound::config::registry_data,
        client_bound::config::resource_pack_pop,
        client_bound::config::resource_pack_push,
        client_bound::config::store_cookie,
        client_bound::config::transfer,
        client_bound::config::update_enabled_features,
        client_bound::config::update_tags,
        client_bound::config::select_known_packs,
        client_bound::config::custom_report_details,
        client_bound::config::server_links,
        client_bound::config::clear_dialog,
        client_bound::config::show_dialog,
        client_bound::config::code_of_conduct>;
}
#endif /* SRC_API_PACKETS_CLIENT_BOUND_CONFIG */
