/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_ENTITY_METADATA
#define SRC_BASE_OBJECTS_ENTITY_METADATA
#include <src/api/id.hpp>
#include <src/api/packets/types.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/particle_data.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/calculations.hpp>

namespace copper_server::base_objects {
    struct entity_metadata {
        struct byte : public enum_item<0> {
            int8_t value = 0;
        };

        struct var_int : public enum_item<1> {
            var_int32 value = 0;
        };

        struct var_long : public enum_item<2> {
            var_int64 value = 0;
        };

        struct float_ : public enum_item<3> {
            float value = 0;
        };

        struct string : public enum_item<4> {
            string_sized<32767> value;
        };

        struct text_component : public enum_item<5> {
            base_objects::chat value;
        };

        struct optional_text_component : public enum_item<6> {
            std::optional<base_objects::chat> value;
        };

        struct slot : public enum_item<7> {
            base_objects::slot value;
        };

        struct boolean : public enum_item<8> {
            bool value = false;
        };

        struct rotations : public enum_item<9>, util::xyz<float> {
            using util::xyz<float>::xyz;
        };

        struct position : public enum_item<10> {
            base_objects::position value;
        };

        struct optional_position : public enum_item<11> {
            std::optional<base_objects::position> value;
        };

        struct direction : public enum_item<12> {
            enum enum_t : int32_t {
                down = 0,
                up = 1,
                north = 2,
                south = 3,
                west = 4,
                east = 5
            };

            enum_as<enum_t, var_int32> value = direction::down;
        };

        struct optional_living_entity_reference : public enum_item<13> {
            std::optional<enbt::raw_uuid> value;
        };

        struct block_state : public enum_item<14> {
            var_int32::block_state value;
        };

        struct optional_block_state : public enum_item<15> {
            optional_var_int32::block_state value = 0;
        };

        struct particle : public enum_item<16> {
            var_int32::particle_type id;
            particle_data data;
        };

        struct particles : public enum_item<17> {
            list_array<particle> value;
        };

        struct villager_data : public enum_item<18> {
            var_int32::villager_type type;
            var_int32::villager_profession profession;
            int32_t level = 0;
        };

        struct optional_var_int : public enum_item<19> {
            optional_var_int32 value = 0;
        };

        struct entity_pose : public enum_item<20> {
            enum enum_t : int32_t {
                standing = 0,
                fall_flying = 1,
                sleeping = 2,
                swimming = 3,
                spin_attack = 4,
                sneaking = 5,
                long_jumping = 6,
                dying = 7,
                croaking = 8,
                using_tongue = 9,
                sitting = 10,
                roaring = 11,
                sniffing = 12,
                emerging = 13,
                digging = 14,
                sliding = 15,  //1.21.3
                shooting = 16, //1.21.3
                inhaling = 17, //1.21.3
            };

            enum_as<enum_t, var_int32> value = entity_pose::standing;
        };

        struct cat_variant : public enum_item<21> {
            var_int32::cat_variant value = 0;
        };

        struct cow_variant : public enum_item<22> {
            var_int32::cow_variant value = 0;
        };

        struct wolf_variant : public enum_item<23> {
            var_int32::wolf_variant value = 0;
        };

        struct wolf_sound_variant : public enum_item<24> {
            var_int32::wolf_sound_variant value = 0;
        };

        struct frog_variant : public enum_item<25> {
            var_int32::frog_variant value = 0;
        };

        struct pig_variant : public enum_item<26> {
            var_int32::pig_variant value = 0;
        };

        struct chicken_variant : public enum_item<27> {
            var_int32::chicken_variant value = 0;
        };

        struct optional_global_position : public enum_item<28> {
            struct value_t {
                identifier world_id;
                position pos;

                bool operator==(const value_t& other) const = default;
            };

            std::optional<value_t> value;
        };

        struct painting_variant : public enum_item<29> {
            struct inline_def {
                int32_t height;
                int32_t width;
                identifier asset_id;
                std::optional<base_objects::chat> title;
                std::optional<base_objects::chat> author;

                bool operator==(const inline_def& other) const = default;
            };

            or_<var_int32::painting_variant, inline_def> value = var_int32::painting_variant{0};
        };

        struct sniffer_state : public enum_item<30> {
            enum class state_e {
                idle = 0,
                felling_happy = 1,
                scenting = 2,
                sniffing = 3,
                searching = 4,
                digging = 5,
                rising = 6,
            };
            using enum state_e;

            enum_as<state_e, var_int32> value = state_e::idle;
        };

        struct armadillo_state : public enum_item<31> {
            enum class state_e {
                idle = 0,
                rolling = 1,
                scared = 2,
                unrolling = 3,
            };
            using enum state_e;

            enum_as<state_e, var_int32> value = state_e::idle;
        };

        struct copper_golem_state : public enum_item<32> {
            enum class state_e {
                idle = 0,
                getting_item = 1,
                getting_no_item = 2,
                dropping_item = 3,
                dropping_no_item = 4,
            };
            using enum state_e;

            enum_as<state_e, var_int32> value = state_e::idle;
        };

        struct weathering_copper_state : public enum_item<33> {
            enum class state_e {
                unaffected = 0,
                exposed = 1,
                weathered = 2,
                oxidized = 3,
            };
            using enum state_e;

            enum_as<state_e, var_int32> value = state_e::unaffected;
        };

        struct vector3 : public enum_item<34>, util::xyz<float> {
            using util::xyz<float>::xyz;
        };

        struct vector4 : public enum_item<35>, util::xyzw<float> {
            using util::xyzw<float>::xyzw;
        };

        struct resolvable_profile : public enum_item<36> {
            struct value_t {
            };

            value_t value;
        };

        using enum_sw = enum_switch< //
            uint8_t,
            byte,
            var_int,
            var_long,
            float_,
            string,
            text_component,
            optional_text_component,
            slot,
            boolean,
            rotations,
            position,
            optional_position,
            direction,
            optional_living_entity_reference,
            block_state,
            optional_block_state,
            particle,
            particles,
            villager_data,
            optional_var_int,
            entity_pose,
            cat_variant,
            cow_variant,
            wolf_variant,
            wolf_sound_variant,
            frog_variant,
            pig_variant,
            chicken_variant,
            optional_global_position,
            painting_variant,
            sniffer_state,
            armadillo_state,
            copper_golem_state,
            weathering_copper_state,
            vector3,
            vector4,
            resolvable_profile>;

        enum_sw value;

        entity_metadata() = default;
        entity_metadata(const entity_metadata& other) = default;
        entity_metadata(entity_metadata&& other) = default;

        entity_metadata(enum_sw&& other) : value(std::move(other)) {}
        entity_metadata& operator=(const entity_metadata& other) = default;
        entity_metadata& operator=(entity_metadata&& other) = default;
        ~entity_metadata() = default;
    };
}

#endif /* SRC_BASE_OBJECTS_ENTITY_METADATA */
