/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_MOJANG_SESSION_SERVER
#define SRC_API_MOJANG_SESSION_SERVER
#include <src/mojang/api/session_server.hpp>

namespace copper_server::api::mojang {
    ::mojang::api::session_server& get_session_server();
}


#endif /* SRC_API_MOJANG_SESSION_SERVER */
