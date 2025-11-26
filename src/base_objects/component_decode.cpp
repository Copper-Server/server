/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/unordered/unordered_flat_map.hpp>
#include <src/api/packets/slot.hpp>
#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/reflect.hpp>
#include <src/util/reflect/api/packets/slot.hpp>
#include <src/util/reflect/api/packets/types.hpp>
#include <src/util/reflect/base_objects/component.hpp>
#include <src/util/reflect/base_objects/dye_color.hpp>
#include <src/util/templates.hpp>

#include <src/util/encoding/nbt/deserialization.hpp>

namespace copper_server::base_objects {
    boost::unordered_flat_map<std::string, void (*)(component& item, util::nbt_read_stream& stream)> create_nbt_map_io() {
        boost::unordered_flat_map<std::string, void (*)(component& item, util::nbt_read_stream& stream)> result;

        util::for_each_type<component::base::base>::each([&result]<class T>() {
            std::string nam;
            if constexpr (requires { T::actual_name::value; })
                nam = "minecraft:" + std::string(T::actual_name::value);
            else
                nam = "minecraft:" + std::string(reflect::get_pretty_type_name<T>());
            result[nam] = [](component& item, util::nbt_read_stream& stream) {
                T result{};
                util::encoding::nbt::deserialize_entry(result, stream, result);
                item = std::move(result);
            };
        });
        return result;
    }

    void component::parse_component(component& item, const std::string& name, util::nbt_read_stream& stream) {
        static auto map_of_deserializers = create_nbt_map_io();
        map_of_deserializers[name](item, stream);
    }
}