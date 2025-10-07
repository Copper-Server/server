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
        int64_t x : 26;
        int64_t z : 26;
        int64_t y : 12;

        inline void set(uint64_t raw) {
            *this = std::bit_cast<position>(raw);
        }

        inline uint64_t get() const {
            return std::bit_cast<uint64_t>(*this);
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
