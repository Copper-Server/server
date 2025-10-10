/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_PACKETS_SLOT
#define SRC_API_PACKETS_SLOT
#include <src/api/packets/types.hpp>
#include <src/base_objects/box.hpp>
#include <src/base_objects/component.hpp>

namespace copper_server::api::packets {
    struct slot {
        depends_next<var_int32> count;
        var_int32::item id;
        var_int32 components_to_add;
        var_int32 components_to_remove;
        list_array_no_size<base_objects::component, &slot::components_to_add> to_add;
        list_array_no_size<var_int32::data_component_type, &slot::components_to_remove> to_remove;
    };

    struct slot_display {
        struct empty : public enum_item<0> {};

        struct any_fuel : public enum_item<1> {};

        struct item : public enum_item<2> {
            var_int32::item type;
        };

        struct item_stack : public enum_item<3> {
            slot item_stack;
        };

        struct tag : public enum_item<4> {
            identifier tag;
        };

        struct smithing_trim : public enum_item<5> {
            base_objects::box<slot_display> base;
            base_objects::box<slot_display> material;
            base_objects::box<slot_display> pattern;
        };

        struct with_remainder : public enum_item<6> {
            base_objects::box<slot_display> ingredient;
            base_objects::box<slot_display> remainder;
        };

        struct composite : public enum_item<7> {
            list_array<base_objects::box<slot_display>> ingredient;
        };

        enum_switch<
            var_int32,
            empty,
            any_fuel,
            item,
            item_stack,
            tag,
            smithing_trim,
            with_remainder,
            composite>
            display;
    };

    struct recipe_display {
        struct crafting_shapeless : public enum_item<0> {
            list_array<slot_display> ingredients;
            slot_display result;
            slot_display crafting_station;
        };

        struct crafting_shaped : public enum_item<1> {
            var_int32 width;
            var_int32 height;
            list_array<slot_display> ingredients;
            slot_display result;
            slot_display crafting_station;
        };

        struct furnace : public enum_item<2> {
            slot_display ingredient;
            slot_display fuel;
            slot_display result;
            slot_display crafting_station;
            var_int32 cooking_time;
            float experience;
        };

        struct stonecutter : public enum_item<3> {
            slot_display ingredient;
            slot_display result;
            slot_display crafting_station;
        };

        struct smithing : public enum_item<4> {
            slot_display template_;
            slot_display base;
            slot_display addition;
            slot_display result;
            slot_display crafting_station;
        };

        enum_switch<
            var_int32,
            crafting_shapeless,
            crafting_shaped,
            furnace,
            stonecutter,
            smithing>
            display;

        static recipe_display create(const base_objects::recipe&);
    };
}


#endif /* SRC_API_PACKETS_SLOT */
