/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PLUGIN
#define SRC_API_PLUGIN
#include <functional>
#include <string>

namespace copper_server::api::plugin {
    void load(const std::string& name);
    void unload(const std::string& name);
    void reload(const std::string& name);
    void reload_all();
    void iterate(std::function<void(const std::string& name)> callback);
    void get(const std::string& name, std::function<void(const std::string& name)> callback);
    void resolve_id(const std::string& name, std::function<void(const std::string& name)> callback);
    void pre_load(const std::string& name, std::function<void(const std::string& name)> initialization = nullptr);
    void create(const std::string& name);
    void create(const std::string& name, std::function<void(const std::string& name)> callback);
}

#endif /* SRC_API_PLUGIN */
