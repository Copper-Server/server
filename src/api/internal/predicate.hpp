/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_INTERNAL_PREDICATE
#define SRC_API_INTERNAL_PREDICATE
#include <src/base_objects/predicate_processor.hpp>

namespace copper_server::api::predicate {
    void register_processor(base_objects::predicate_processor& processor);
    void unregister_processor();
}

#endif /* SRC_API_INTERNAL_PREDICATE */
