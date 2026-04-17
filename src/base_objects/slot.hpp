/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_SLOT
#define SRC_BASE_OBJECTS_SLOT
#include <library/list_array.hpp>
#include <optional>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/component.hpp>
#include <src/base_objects/dye_color.hpp>
#include <src/base_objects/position.hpp>
#include <src/util/nbt.hpp>
#include <src/util/readers.hpp>
#include <string>

namespace copper_server::util {
    class nbt_read_stream;
    class nbt_write_stream;

    namespace nbt_collection {
        class compound_flex;
    }

    class nbt_write_compound_stream;
}

namespace copper_server::api::packets {
    struct slot;
}

namespace copper_server::base_objects {
    struct static_slot_data {
        struct alias_data {
            uint32_t local_id = 0;
            std::string local_named_id;
        };

        std::string id;
        std::unordered_map<int32_t, component> default_components;
        int32_t internal_id = 0;
        std::optional<int32_t> spawn_entity;
        uint32_t fuel_time = 0;                 //0 == not fuel
        float composter_increase_chance = 0.0f; //0.0f == should not be used in composter

        util::nbt_compound server_side;

        //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
        static void reset_items(); //INTERNAL
    };

    struct item_id_t {
        uint32_t id;

        item_id_t(const std::string& id);

        constexpr item_id_t(uint32_t id)
            : id(id) {}

        constexpr item_id_t(const item_id_t& id)
            : id(id.id) {}

        constexpr item_id_t()
            : id(0) {}

        constexpr item_id_t& operator=(const item_id_t& copy) {
            id = copy.id;
            return *this;
        }

        const std::string& to_name() const;
        static_slot_data& get_data() const;
    };

    struct slot_data {
        std::unordered_map<int32_t, component> components;
        int32_t id = 0;
        int32_t count = 0;

        slot_data();
        slot_data(slot_data&& move) noexcept;
        slot_data(const slot_data& copy);

        slot_data(std::unordered_map<int32_t, component>&& components, int32_t id = 0, int32_t count = 0) noexcept;
        slot_data(const std::unordered_map<int32_t, component>& components, int32_t id = 0, int32_t count = 0);
        slot_data& operator=(const slot_data&);
        slot_data& operator=(slot_data&&) noexcept;


        template <class T>
        T& get_component() {
            return std::get<T>(components.at(T::item_id::value).type);
        }

        template <class T>
        T& access_component() {
            if (components.contains(T::item_id::value))
                return std::get<T>(components[T::item_id::value].type);
            else
                return std::get<T>(components[T::item_id::value].type = T{});
        }

        template <class T>
        const T& get_component() const {
            return std::get<T>(components.at(T::item_id::value).type);
        }

        template <class T>
        void remove_component() {
            components.erase(T::item_id::value);
        }

        void add_component(component&& copy) {
            std::visit(
                [this, &copy](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    components[T::item_id::value] = std::move(copy);
                },
                copy.type
            );
        }

        void add_component(const component& copy) {
            std::visit(
                [this, &copy](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    components[T::item_id::value] = copy;
                },
                copy.type
            );
        }

        template <class T>
        void add_component(const T& copy) {
            components[T::item_id::value].type = copy;
        }

        template <class T>
        void add_component(T&& copy) {
            components[T::item_id::value].type = std::move(copy);
        }

        template <class T>
        bool has_component() const {
            return components.contains(T::item_id::value);
        }

        bool operator==(const slot_data& other) const;
        bool operator!=(const slot_data& other) const;

        bool is_same_def(const slot_data& other) const;
        std::optional<int32_t> spawns_entity_type() const;

        static slot_data create_item(const std::string& id, int32_t count = 1);
        static slot_data create_item(int32_t id, int32_t count = 1);
        static static_slot_data& get_slot_data(const std::string& id);
        static static_slot_data& get_slot_data(int32_t id);

        static void add_slot_data(static_slot_data&& move);

        static_slot_data& get_slot_data() const;

        static void enumerate_slot_data(const std::function<void(static_slot_data&)>& fn);
        static list_array<int32_t> get_slot_data_ids();

        copper_server::api::packets::slot to_packet() const;
        static slot_data from_packet(copper_server::api::packets::slot&&);

        void to_nbt(util::nbt_write_stream& stream) const;
        static slot_data from_nbt(util::nbt_read_stream& stream);

        void to_nbt_base(util::nbt_write_compound_stream& stream) const;
        void from_nbt_base(util::nbt_collection::compound_flex& collector);

    private:
        friend struct static_slot_data;
        static std::unordered_map<std::string, std::shared_ptr<static_slot_data>> named_full_item_data;
        static std::vector<std::shared_ptr<static_slot_data>> full_item_data_;

        template <typename T, typename = void>
        struct has_component_name : std::false_type {};

        template <typename T>
        struct has_component_name<T, decltype((void)T::component_name, void())> : std::true_type {};
    };

    struct slot : public std::optional<slot_data> {
        using std::optional<slot_data>::optional;

        slot(std::nullopt_t) : std::optional<slot_data>(std::nullopt) {}

        slot(std::optional<slot_data>&& opt) : std::optional<slot_data>(std::move(opt)) {}

        slot(const std::optional<slot_data>& opt) : std::optional<slot_data>(opt) {}

        copper_server::api::packets::slot to_packet() const;
        static slot from_packet(copper_server::api::packets::slot&&);

        bool operator==(const slot& other) const {
            if ((bool)*this == (bool)other)
                if (*this)
                    return *(*this) == *other;
            return false;
        }

        bool operator!=(const slot& other) const {
            return !operator==(other);
        }

        bool operator==(const std::optional<slot_data>& other) const {
            if ((bool)*this == (bool)other)
                if (*this)
                    return *(*this) == *other;
            return false;
        }

        bool operator!=(const std::optional<slot_data>& other) const {
            return !operator==(other);
        }

        bool operator==(std::nullopt_t) const {
            return ((std::optional<slot_data>&)*this) == std::nullopt;
        }

        bool operator!=(std::nullopt_t) const {
            return ((std::optional<slot_data>&)*this) != std::nullopt;
        }
    };
}

#endif /* SRC_BASE_OBJECTS_SLOT */
