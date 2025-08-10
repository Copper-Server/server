/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_INTERNAL_CONSOLE
#define SRC_API_INTERNAL_CONSOLE
#include <src/base_objects/virtual_client.hpp>

namespace copper_server::api::console {
    void register_virtual_client(base_objects::virtual_client& client);
    void unregister_virtual_client();
}

#endif /* SRC_API_INTERNAL_CONSOLE */
