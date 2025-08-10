/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_DIALOGS
#define SRC_API_DIALOGS
#include <library/enbt/enbt.hpp>
#include <src/base_objects/events/event.hpp>
#include <string>
#include <functional>

namespace copper_server::base_objects {
    struct SharedClientData;
}

namespace copper_server::api::dialogs {
    //client could be in two states, configuration and play
    void register_dialog(const std::string& id, std::function<void(base_objects::SharedClientData& client, enbt::value&& payload)>&& fn);
    void pass_dialog(const std::string& id, base_objects::SharedClientData& client, enbt::value&& payload);
    void unload_dialog(const std::string& id);
}

#endif /* SRC_API_DIALOGS */
