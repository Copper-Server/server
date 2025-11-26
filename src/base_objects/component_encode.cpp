/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets/slot.hpp>
#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/reflect.hpp>
#include <src/util/reflect/api/packets/slot.hpp>
#include <src/util/reflect/api/packets/types.hpp>
#include <src/util/reflect/base_objects/component.hpp>
#include <src/util/reflect/base_objects/dye_color.hpp>

#include <src/util/encoding/nbt/serialization.hpp>

namespace copper_server::base_objects {
    void component::encode_component(const component& item, util::nbt_write_compound_stream& stream) {
        std::visit(
            [&stream](auto& it) {
                using T = std::decay_t<decltype(it)>;
                std::string nam;
                if constexpr (requires { T::actual_name::value; })
                    nam = "minecraft:" + std::string(T::actual_name::value);
                else
                    nam = "minecraft:" + std::string(reflect::get_pretty_type_name<T>());
                stream.write(nam, [&it](util::nbt_write_stream& stream) {
                    util::encoding::nbt::serialize_entry(stream, it);
                });
            },
            item.type
        );
    }
}
