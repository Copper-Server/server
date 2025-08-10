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
#include <src/base_objects/predicate_processor.hpp>

namespace copper_server::api::predicate {
    bool process_predicate(const enbt::compound_ref& predicate, const base_objects::command_context& context);
    void register_handler(const std::string& name, base_objects::predicate_processor::handler handler);
    void unregister_handler(const std::string& name);
    const base_objects::predicate_processor::handler& get_handler(const std::string& name);
    bool has_handler(const std::string& name);

    bool registered();
}
#endif /* SRC_API_PREDICATE */
