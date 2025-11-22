#ifndef SRC_BASE_OBJECTS_ITEM_PREDICATE
#define SRC_BASE_OBJECTS_ITEM_PREDICATE
#include <src/base_objects/component.hpp>
#include <src/util/nbt.hpp>
#include <src/base_objects/slot.hpp>


namespace copper_server::util {
    class nbt_read_stream;
    class nbt_write_stream;

    namespace nbt_collection {
        class compound_flex;
    }

    class nbt_write_compound_stream;
}

namespace copper_server::base_objects {
    struct item_predicate {
        struct count_minmax {
            std::optional<int32_t> min;
            std::optional<int32_t> max;
        };

        std::optional<std::variant<api::id::set::item, std::vector<api::id::item>>> items;
        std::optional<std::variant<count_minmax, int32_t>> count;
        std::unordered_map<int32_t, component> components;
        util::nbt predicates;//data component predicates //TODO add support

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

        void from_nbt_base_data(util::nbt_collection::compound_flex& collector);
        void to_nbt_base_data(util::nbt_write_compound_stream& collector);
        void from_nbt(util::nbt_read_stream& stream);
        void to_nbt(util::nbt_write_stream& stream);

        bool is_matches(base_objects::slot_data& slot) const;
    };
}
#endif /* SRC_BASE_OBJECTS_ITEM_PREDICATE */
