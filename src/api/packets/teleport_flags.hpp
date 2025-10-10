/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_TELEPORT_FLAGS
#define SRC_API_PACKETS_TELEPORT_FLAGS
#include <src/api/packets/types.hpp>

namespace copper_server::api::packets {
    struct teleport_flags {
        enum class flags_f {
            x_relative = 0x1,
            y_relative = 0x2,
            z_relative = 0x4,
            yaw_relative = 0x8,
            pitch_relative = 0x10,
            velocity_x_relative = 0x20,
            velocity_y_relative = 0x40,
            velocity_z_relative = 0x80,
            adjust_velocity_to_rotation = 0x100,
        };
        using enum flags_f;

        enum_as_flag<flags_f, int32_t> flags;
    };
}

inline copper_server::api::packets::teleport_flags::flags_f operator|(copper_server::api::packets::teleport_flags::flags_f a, copper_server::api::packets::teleport_flags::flags_f b) {
    return copper_server::api::packets::teleport_flags::flags_f(static_cast<int>(a) | static_cast<int>(b));
}
#endif /* SRC_API_PACKETS_TELEPORT_FLAGS */
