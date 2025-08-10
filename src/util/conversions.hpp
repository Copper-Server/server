/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_CONVERSIONS
#define SRC_UTIL_CONVERSIONS
#include <boost/json.hpp>
#include <cstddef>
#include <cstdint>
#include <library/enbt/enbt.hpp>
#include <string>
#include <vector>

namespace copper_server::util::conversions {
    namespace base64 {
        std::string encode(const void* data, std::size_t size);
        std::string encode(const std::vector<std::uint8_t>& data);
        std::vector<std::uint8_t> decode(std::string_view data);
    }

    namespace hex {
        std::string encode(const void* data, std::size_t size);

        std::string encode(const std::vector<std::uint8_t>& data);
        std::vector<std::uint8_t> decode(std::string_view data);
    }

    namespace uuid {
        std::string to(enbt::raw_uuid id);
        enbt::raw_uuid from(std::string_view id);
    }

    namespace string {
        std::string to_direct(std::string_view string);
        size_t direct_find(const std::string& string, char item);
        std::string to_transport(std::string_view string);
    }

    namespace json {
        boost::json::value to_json(const enbt::value& enbt);
        boost::json::object to_json(const enbt::compound& enbt);
        boost::json::array to_json(const enbt::dynamic_array& enbt);
        boost::json::array to_json(const enbt::fixed_array& enbt);
        enbt::value from_json(const boost::json::value& json);
        enbt::compound from_json(const boost::json::object& json);
        enbt::dynamic_array from_json(const boost::json::array& json);
    }
}
#endif /* SRC_UTIL_CONVERSIONS */
