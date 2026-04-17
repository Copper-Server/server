/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_ECS_BLOCK_ENTITY_COMPONENTS
#define SRC_API_ECS_BLOCK_ENTITY_COMPONENTS
#include <library/list_array.hpp>
#include <memory>

#include <src/api/id.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/component.hpp>
#include <src/base_objects/container.hpp>
#include <src/base_objects/item_predicate.hpp>
#include <src/base_objects/pool.hpp>
#include <src/base_objects/slot.hpp>
#include <src/base_objects/uuid.hpp>
#include <src/util/cts.hpp>
#include <src/generated/block_entity.hpp>

namespace copper_server::util {
    class nbt_write_compound_stream;

    namespace nbt_collection {
        class compound_flex;
    }
}

namespace copper_server::api::ecs::com::block_entity {
    struct type {
        int32_t id;
    };

    struct base_data {
        std::unique_ptr<std::unordered_map<int32_t, base_objects::component>> components;
        base_objects::block id;
        int32_t x, y, z;
        bool keep_packed = false;

        base_data() : components(std::make_unique<std::unordered_map<int32_t, base_objects::component>>()),
                      id(0),
                      x(0),
                      y(0),
                      z(0) {}

        base_data(base_data&&) noexcept = default;
        base_data& operator=(base_data&&) noexcept = default;

        base_data(const base_data& other) {
            if (other.components)
                components = std::make_unique<std::unordered_map<int32_t, base_objects::component>>(*components);
            id = other.id;
            x = other.x;
            y = other.y;
            z = other.z;
            keep_packed = other.keep_packed;
        }

        base_data& operator=(const base_data& other) {
            if (this != &other) {
                if (other.components) {
                    if (!components) {
                        components = std::make_unique<std::unordered_map<int32_t, base_objects::component>>(*other.components);
                    } else
                        *components = *other.components;
                } else
                    components.reset();
            }
            id = other.id;
            x = other.x;
            y = other.y;
            z = other.z;
            keep_packed = other.keep_packed;
            return *this;
        }

        template <class T>
        T& get_component() {
            return std::get<T>(components->at(T::item_id::value).type);
        }

        template <class T>
        T& access_component() {
            if (components->contains(T::item_id::value))
                return std::get<T>((*components)[T::item_id::value].type);
            else
                return std::get<T>((*components)[T::item_id::value].type = T{});
        }

        template <class T>
        const T& get_component() const {
            return std::get<T>(components->at(T::item_id::value).type);
        }

        template <class T>
        void remove_component() {
            components->erase(T::item_id::value);
        }

        void add_component(base_objects::component&& copy) {
            std::visit(
                [this, &copy](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    (*components)[T::item_id::value] = std::move(copy);
                },
                copy.type
            );
        }

        void add_component(const base_objects::component& copy) {
            std::visit(
                [this, &copy](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    (*components)[T::item_id::value] = copy;
                },
                copy.type
            );
        }

        template <class T>
        void add_component(const T& copy) {
            (*components)[T::item_id::value].type = copy;
        }

        template <class T>
        void add_component(T&& copy) {
            (*components)[T::item_id::value].type = std::move(copy);
        }

        template <class T>
        bool has_component() const {
            return components->contains(T::item_id::value);
        }

        inline bool is_tickable() const {
            return id.is_tickable();
        }

        inline bool is_solid() const {
            return id.is_solid();
        }

        const std::vector<base_objects::shape_data*>& collision_shapes() const {
            return id.collision_shapes();
        }

        const base_objects::chat& display_name() const {
            return id.display_name();
        }

        const std::string& instrument() const {
            return id.instrument();
        }

        const std::string& piston_behavior() const {
            return id.piston_behavior();
        }

        const std::string& name() const {
            return id.name();
        }

        const std::string& translation_key() const {
            return id.translation_key();
        }

        inline base_objects::block_id_t general_block_id() const {
            return id.general_block_id();
        }

        inline float slipperiness() const {
            return id.slipperiness();
        }

        inline float velocity_multiplier() const {
            return id.velocity_multiplier();
        }

        inline float jump_velocity_multiplier() const {
            return id.jump_velocity_multiplier();
        }

        inline float hardness() const {
            return id.hardness();
        }

        inline float blast_resistance() const {
            return id.blast_resistance();
        }

        inline int32_t map_color_rgb() const {
            return id.map_color_rgb();
        }

        inline int32_t block_entity_id() const {
            return id.block_entity_id();
        }

        inline int32_t item_id() const {
            return id.item_id();
        }

        inline int32_t experience() const {
            return id.experience();
        }

        inline base_objects::block_id_t default_state() const {
            return id.default_state();
        }

        inline uint8_t luminance() const {
            return id.luminance();
        }

        inline uint8_t opacity() const {
            return id.opacity();
        }

        inline bool is_air() const {
            return id.is_air();
        }

        inline bool is_liquid() const {
            return id.is_liquid();
        }

        inline bool is_burnable() const {
            return id.is_burnable();
        }

        inline bool is_emits_redstone() const {
            return id.is_emits_redstone();
        }

        inline bool is_full_cube() const {
            return id.is_full_cube();
        }

        inline bool is_tool_required() const {
            return id.is_tool_required();
        }

        inline bool is_sided_transparency() const {
            return id.is_sided_transparency();
        }

        inline bool is_replaceable() const {
            return id.is_replaceable();
        }

        inline bool is_block_entity() const {
            return id.is_block_entity();
        }

        using nbt_path = util::cts_string<"">;
        void to_nbt(util::nbt_write_compound_stream& stream) const;
        void from_nbt(util::nbt_collection::compound_flex& stream);
    };

    using namespace ::copper_server::generated::block_entity;
}
#endif