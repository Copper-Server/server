/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PREDICATE
#define SRC_API_PREDICATE
#include <functional>
#include <library/enbt/enbt.hpp>

namespace copper_server::base_objects {
    struct command_context;
}

namespace copper_server::api::predicate {
    using handler = std::function<bool(const enbt::compound_const_ref&, const base_objects::command_context&)>;

    bool process_predicate(const enbt::compound_const_ref& predicate, const base_objects::command_context& context);
    void register_handler(const std::string& name, handler handler);
    void unregister_handler(const std::string& name);
    const handler& get_handler(const std::string& name);
    void reset_handlers();
    bool has_handler(const std::string& name);
}
#endif /* SRC_API_PREDICATE */
