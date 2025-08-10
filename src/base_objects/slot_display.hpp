/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_SLOT_DISPLAY
#define SRC_BASE_OBJECTS_SLOT_DISPLAY
#include <functional>
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <src/base_objects/slot.hpp>
#include <string>
#include <vector>

namespace copper_server::base_objects {
    struct slot_display;

    namespace slot_displays {
        namespace minecraft {
            struct empty {
                static constexpr const char name[] = "minecraft:empty";
            };

            struct any_fuel {
                static constexpr const char name[] = "minecraft:any_fuel";
            };

            struct item {
                std::string item;
                static constexpr const char name[] = "minecraft:item";
            };

            struct item_stack {
                slot_data item;
                static constexpr const char name[] = "minecraft:item_stack";
            };

            struct tag {
                std::string tag; //without #
                static constexpr const char name[] = "minecraft:tag";
            };

            struct smithing_trim {
                slot_display* base;
                slot_display* material;
                slot_display* pattern;
                smithing_trim(slot_display&& base, slot_display&& material, slot_display&& pattern);
                ~smithing_trim();
                static constexpr const char name[] = "minecraft:smithing_trim";
            };

            struct with_remainder {
                slot_display* ingredient;
                slot_display* remainder;
                with_remainder(slot_display&& ingredient, slot_display&& remainder);
                ~with_remainder();
                static constexpr const char name[] = "minecraft:with_remainder";
            };

            struct composite {
                std::vector<slot_display*> options;
                composite(std::vector<slot_display*>&& options);
                ~composite();
                static constexpr const char name[] = "minecraft:composite";
            };
        }

        struct custom {
            enbt::compound value;

            //if empty, will be ignored
            std::function<list_array<slot>(const enbt::compound&)> to_slots;
            static constexpr const char name[] = "copper_server:custom";
        };

        using unified = std::variant<
            minecraft::empty,
            minecraft::any_fuel,
            minecraft::item,
            minecraft::item_stack,
            minecraft::tag,
            minecraft::smithing_trim,
            minecraft::with_remainder,
            minecraft::composite,
            custom>;

    }

    struct slot_display {
        slot_displays::unified value;

        list_array<slot> to_slots() const;
    };
}

#endif /* SRC_BASE_OBJECTS_SLOT_DISPLAY */
