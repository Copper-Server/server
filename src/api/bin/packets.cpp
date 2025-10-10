/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <cstdint>
#include <string>
#include <unordered_map>

namespace copper_server {
    namespace api::packets {
        int32_t java_name_to_protocol(const std::string& name_or_number) {
            static const std::unordered_map<std::string, int32_t> map{
                {"773", 773},
                {"772", 772},
                {"771", 771},
                {"770", 770},
                {"769", 769},
                {"768", 768},
                {"767", 767},
                {"1.21.10", 773},
                {"1.21.9", 773},
                {"1.21.8", 772},
                {"1.21.7", 771},
                {"1.21.6", 771},
                {"1.21.5", 770},
                {"1.21.4", 769},
                {"1.21.3", 768},
                {"1.21.2", 768},
                {"1.21.1", 767},
                {"1.21", 767}
            };
            return map.at(name_or_number);
        }

        const char* protocol_to_java_name(int32_t id) {
            static const std::unordered_map<int32_t, const char*> map{
                {773, "1.21.10"},
                {772, "1.21.8"},
                {771, "1.21.6"},
                {770, "1.21.5"},
                {769, "1.21.4"},
                {768, "1.21.3"},
                {768, "1.21.2"},
                {767, "1.21.1"},
                {767, "1.21"}
            };
            return map.at(id);
        }
    }
}
