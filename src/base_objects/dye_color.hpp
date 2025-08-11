/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_DYE_COLOR
#define SRC_BASE_OBJECTS_DYE_COLOR
#include <string>

namespace copper_server::base_objects{
    enum class dye_color : uint8_t {
        white = 0,
        orange = 1,
        magenta = 2,
        light_blue = 3,
        yellow = 4,
        lime = 5,
        pink = 6,
        gray = 7,
        light_gray = 8,
        cyan = 9,
        purple = 10,
        blue = 11,
        brown = 12,
        green = 13,
        red = 14,
        black = 15,
    };
}

#endif /* SRC_BASE_OBJECTS_DYE_COLOR */
