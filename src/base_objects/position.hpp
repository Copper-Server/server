/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_POSITION
#define SRC_BASE_OBJECTS_POSITION
#include <cstdint>
namespace copper_server::base_objects {
    struct position {
        int x : 26;
        int z : 26;
        int y : 12;

        inline void set(uint64_t raw) {
            union u_t {
                position flag;
                uint64_t r;
            } u{.r = raw};

            *this = u.flag;
        }

        inline uint64_t get() const {
            union u_t {
                position flag;
                uint64_t r;
            } u{.flag = *this};

            return u.r;
        }

        bool operator==(const position& other) const {
            return get() == other.get();
        }

        bool operator!=(const position& other) const {
            return get() != other.get();
        }

        auto operator<=>(const position& enbt) const = default;
    };
}

#endif /* SRC_BASE_OBJECTS_POSITION */
