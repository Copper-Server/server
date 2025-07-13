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

namespace copper_server::base_objects {


    struct static_slot_data {
        struct alias_data {
            uint32_t local_id;
            std::string local_named_id;
        };

        std::string id;
        std::unordered_map<std::string, component::unified> default_components;
        int32_t internal_id;

        enbt::compound server_side;
        //{
        //  "pot_decoration_aliases": ["...",...]
        //}


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
        std::unordered_map<std::string, component::unified> components;
        int32_t count = 0;
        int32_t id = 0;

        template <class T>
        T& get_component() {
            return std::get<T>(components.at(T::component_name));
        }

        template <class T>
        T& access_component() {
            if (components.contains(T::component_name))
                return std::get<T>(components[T::component_name]);
            else
                return std::get<T>(components[T::component_name] = T{});
        }

        template <class T>
        const T& get_component() const {
            return std::get<T>(components.at(T::component_name));
        }

        template <class T>
        void remove_component() {
            return components.erase(T::component_name);
        }

        void add_component(component::unified&& copy) {
            std::visit(
                [this](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    components[T::component_name] = std::move(component);
                },
                copy
            );
        }

        template <class T>
        void add_component(T&& move) {
            components[T::component_name] = std::move(move);
        }

        void add_component(const component::unified& copy) {
            std::visit(
                [this](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    components[T::component_name] = component;
                },
                copy
            );
        }

        template <class T>
        void add_component(const T& copy) {
            components[T::component_name] = copy;
        }

        template <class T>
        bool has_component() const {
            return components.contains(T::component_name);
        }

        //any data
        void remove_component(const std::string& name) {
            components.erase(name);
        }

        //any data
        bool has_component(const std::string& name) const {
            return components.contains(name);
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

    private:
        friend class static_slot_data;
        static std::unordered_map<std::string, std::shared_ptr<static_slot_data>> named_full_item_data;
        static std::vector<std::shared_ptr<static_slot_data>> full_item_data_;

        template <typename T, typename = void>
        struct has_component_name : std::false_type {};

        template <typename T>
        struct has_component_name<T, decltype((void)T::component_name, void())> : std::true_type {};
    };

    using slot = std::optional<slot_data>;
}

#endif /* SRC_BASE_OBJECTS_SLOT */
