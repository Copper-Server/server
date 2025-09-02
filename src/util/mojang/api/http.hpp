/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_UTIL_MOJANG_API_HTTP
#define SRC_UTIL_MOJANG_API_HTTP
#include <string>
#include <library/list_array.hpp>

namespace copper_server::util::mojang::api::http {
    std::string request(const std::string& mode, const std::string& address, const std::string& query, uint16_t port = 80, uint8_t max_redirects = 4);
    std::string get(const std::string& address, const std::string& query, uint16_t port = 80, uint8_t max_redirects = 4);
    std::string post(const std::string& address, const std::string& query, uint16_t port = 80, uint8_t max_redirects = 4);
}
#endif /* SRC_UTIL_MOJANG_API_HTTP */
