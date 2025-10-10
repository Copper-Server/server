/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_CHAT_TYPE
#define SRC_API_PACKETS_CHAT_TYPE
#include <src/api/packets/types.hpp>
#include <src/base_objects/chat.hpp>
#include <string>
namespace copper_server::api::packets {
    //base_objects::box should always hold value
    struct chat_type {
        struct decoration {
            enum class param_e : uint8_t {
                sender = 0,
                target = 1,
                content = 2
            };
            using enum param_e;
            std::string translation_key;
            list_array<enum_as<param_e, var_int32>> parameters;
            std::optional<Chat> style = std::nullopt;
        };
        decoration chat;
        decoration narration;
    };
}

#endif /* SRC_API_PACKETS_CHAT_TYPE */
