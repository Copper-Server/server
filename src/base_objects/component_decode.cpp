/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets.hpp> //for reflection //TODO change reflect_map to be more abstract;
#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/calculations.hpp> //for reflection //TODO change reflect_map to be more abstract;
#include <src/util/reflect.hpp>

namespace copper_server::base_objects {
    
    component component::parse_component(const std::string& name, const enbt::value& item) {
        return {};//TODO
    }
}