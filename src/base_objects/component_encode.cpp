/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets.hpp>
#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/reflect.hpp>
#include <src/util/reflect/component.hpp>
#include <src/util/reflect/dye_color.hpp>
#include <src/util/reflect/packets.hpp>
#include <src/util/reflect/packets_help.hpp>

#include <src/util/encoding/enbt/serialization.hpp>

namespace copper_server::base_objects {
    enbt::value component::encode_component(const component& item) {
        return std::visit(
            [](auto& it) {
                using T = std::decay_t<decltype(it)>;
                enbt::value res;
                util::encoding::enbt::serialize_entry(res, it);
                std::string nam;
                if constexpr (requires { T::actual_name::value; })
                    nam = "minecraft:" + std::string(T::actual_name::value);
                else
                    nam = "minecraft:" + std::string(reflect::get_pretty_type_name<T>());


                return enbt::compound{{std::move(nam), std::move(res)}};
            },
            item.type
        );
    }

    void component::encode_component(const component& item, enbt::io_helper::value_write_stream& stream) {
        std::visit(
            [&stream](auto& it) {
                using T = std::decay_t<decltype(it)>;
                std::string nam;
                if constexpr (requires { T::actual_name::value; })
                    nam = "minecraft:" + std::string(T::actual_name::value);
                else
                    nam = "minecraft:" + std::string(reflect::get_pretty_type_name<T>());
                stream.write_compound(1).write(nam, [&it](enbt::io_helper::value_write_stream& stream) {
                    util::encoding::enbt::serialize_entry(stream, it);
                });
            },
            item.type
        );
    }
}
