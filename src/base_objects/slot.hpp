#ifndef SRC_BASE_OBJECTS_SLOT
#define SRC_BASE_OBJECTS_SLOT
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <optional>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/dye_color.hpp>
#include <src/base_objects/position.hpp>
#include <src/util/readers.hpp>
#include <string>
#include <src/base_objects/component.hpp>

namespace copper_server::api::packets {
    struct slot;
}

namespace copper_server::base_objects {
    struct static_slot_data {
        struct alias_data {
            uint32_t local_id;
            std::string local_named_id;
        };

        std::string id;
        std::unordered_map<int32_t, component> default_components;
        int32_t internal_id;

        enbt::compound server_side;

        //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
        static void reset_items(); //INTERNAL
    };

    struct item_id_t {
        uint32_t id;

        item_id_t(const std::string& id);
        item_id_t(uint32_t id);
        item_id_t(const item_id_t& id);
        item_id_t();

        const std::string& to_name() const;
        static_slot_data& get_data() const;
    };

    struct slot_data {
        std::unordered_map<int32_t, component> components;
        int32_t count = 0;
        int32_t id = 0;

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
            return components.erase(T::item_id::value);
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

        enbt::compound to_enbt() const;
        static slot_data from_enbt(enbt::compound_const_ref compound);

        bool operator==(const slot_data& other) const;
        bool operator!=(const slot_data& other) const;

        bool is_same_def(const slot_data& other) const;

        static slot_data create_item(const std::string& id, int32_t count = 1);
        static slot_data create_item(uint32_t id, int32_t count = 1);
        static static_slot_data& get_slot_data(const std::string& id);
        static static_slot_data& get_slot_data(uint32_t id);

        static void add_slot_data(static_slot_data&& move);

        static_slot_data& get_slot_data();

        static void enumerate_slot_data(const std::function<void(static_slot_data&)>& fn);

        copper_server::api::packets::slot to_packet() const;
        static slot_data from_packet(copper_server::api::packets::slot&&);

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
