/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <algorithm>
#include <bit>
#include <cstdint>
#include <type_traits>

namespace copper_server::util {
    void endian_swap(void* value_ptr, std::size_t len) {
        std::byte* prox = static_cast<std::byte*>(value_ptr);
        std::reverse(prox, prox + len);
    }

    void convert_endian(void* value_ptr, std::size_t len) {
        if constexpr (std::endian::native == std::endian::little)
            endian_swap(value_ptr, len);
    }
}
