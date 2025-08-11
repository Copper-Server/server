/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_PACKETS_EFFECT_FLAGS
#define SRC_BASE_OBJECTS_PACKETS_EFFECT_FLAGS

namespace copper_server::base_objects::packets {
    struct effect_flags {
        bool is_ambient : 1 = false;
        bool show_particles : 1 = false;
        bool show_icon : 1 = false;
        bool use_blend : 1 = false;

        inline void set(signed char raw) {
            union u_t {
                effect_flags flag;
                signed char r;
            } u{.r = raw};

            *this = u.flag;
        }

        inline signed char get() const {
            union u_t {
                effect_flags flag;
                signed char r;
            } u{.flag = *this};

            return u.r;
        }
    };
}

#endif /* SRC_BASE_OBJECTS_PACKETS_EFFECT_FLAGS */
