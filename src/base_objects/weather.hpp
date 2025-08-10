/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_WEATHER
#define SRC_BASE_OBJECTS_WEATHER
#include <string>
#include <unordered_map>
namespace copper_server::base_objects {
    struct weather {
        enum _value : uint8_t {
            clear,
            rain,
            thunder
        };

        weather(_value value) : value(value) {}

        weather(const weather& value) : value(value.value) {}

        static weather from_string(const std::string& view) {
            static std::unordered_map<std::string, _value> map{
                {"clear", clear},
                {"rain", rain},
                {"thunder", thunder},
            };
            return map.at(view);
        }

        std::string to_string() {
            std::unordered_map<_value, std::string> map{
                {clear, "clear"},
                {rain, "rain"},
                {thunder, "thunder"},
            };
            return map.at(value);
        }

        operator _value() const {
            return value;
        }

    private:
        _value value;
    };
}
#endif /* SRC_BASE_OBJECTS_WEATHER */
