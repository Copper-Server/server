/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_ENDIAN
#define SRC_UTIL_ENDIAN

#include <bit>
#include <cstdint>
#include <type_traits>

namespace copper_server::util {
    void endian_swap(void* value_ptr, std::size_t len);

    void convert_endian(void* value_ptr, std::size_t len);

    template <class T>
    T convert_endian(std::endian endian, T val) {
        if (std::endian::native == endian)
            endian_swap(&val, sizeof(T));
        return val;
    }

    template <class T>
    void convert_endian_arr(std::endian endian, T* val, std::size_t size) {
        if (std::endian::native == endian)
            for (std::size_t i = 0; i < size; i++)
                endian_swap(&val[i], sizeof(T));
    }

    template <class T>
    void convert_endian_arr(std::endian endian, std::vector<T>& val) {
        if (std::endian::native == endian)
            for (auto& it : val)
                endian_swap(&it, sizeof(T));
    }

    template <class T>
    T convert_endian(T val) {
        if constexpr (std::endian::native == std::endian::little)
            endian_swap(&val, sizeof(T));
        return val;
    }

    template <class T>
    void convert_endian_arr(T* val, std::size_t size) {
        if constexpr (std::endian::native == std::endian::little)
            for (std::size_t i = 0; i < size; i++)
                endian_swap(&val[i], sizeof(T));
    }

    template <class T>
    void convert_endian_arr(std::vector<T>& val) {
        if constexpr (std::endian::native == std::endian::little)
            for (auto& it : val)
                endian_swap(&it, sizeof(T));
    }
}
#endif /* SRC_UTIL_ENDIAN */
