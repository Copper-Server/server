/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_CTS
#define SRC_UTIL_CTS
#include <cinttypes>

namespace copper_server::util {
    template <std::size_t N>
    struct CTS {
        char data[N]{};

        consteval CTS(const char (&str)[N]) {
            std::copy_n(str, N, data);
        }
    };
}


#endif /* SRC_UTIL_CTS */
