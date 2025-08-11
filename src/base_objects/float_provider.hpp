/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_FLOAT_PROVIDER
#define SRC_BASE_OBJECTS_FLOAT_PROVIDER

#include <variant>

namespace copper_server::base_objects {
    struct float_provider_constant {
        float value;
    };

    struct float_provider_uniform {
        float min_inclusive;
        float max_exclusive;
    };

    struct float_provider_clamped_normal {
        float mean;
        float deviation;
        float min;
        float max;
    };

    struct float_provider_trapezoid {
        float min;
        float max;
        float plateau;
    };

    using float_provider = std::variant<float_provider_constant, float_provider_uniform, float_provider_clamped_normal, float_provider_trapezoid>;
}
#endif /* SRC_BASE_OBJECTS_FLOAT_PROVIDER */
