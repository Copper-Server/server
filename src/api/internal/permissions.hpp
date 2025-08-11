/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_INTERNAL_PERMISSIONS
#define SRC_API_INTERNAL_PERMISSIONS
#include <src/storage/permissions_manager.hpp>

namespace copper_server::api::permissions {
    void init_permissions(storage::permissions_manager& manager);
}

#endif /* SRC_API_INTERNAL_PERMISSIONS */
