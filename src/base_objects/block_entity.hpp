#ifndef SRC_BASE_OBJECTS_BLOCK_ENTITY
#define SRC_BASE_OBJECTS_BLOCK_ENTITY
#include <src/base_objects/block.hpp>
#include <src/base_objects/component.hpp>
#include <src/util/nbt.hpp>

namespace copper_server::base_objects {
    //struct block_entity {
    //    block_id_t id;
    //    bool keep_packed : 1;
    //    std::unordered_map<int32_t, component> components;
    //
    //
    //    block_entity();
    //    block_entity(slot_data&& move) noexcept;
    //    block_entity(const slot_data& copy);
    //
    //    block_entity& operator=(const block_entity&);
    //    block_entity& operator=(block_entity&&) noexcept;
    //
    //    template <class T>
    //    T& get_component() {
    //        return std::get<T>(components.at(T::item_id::value).type);
    //    }
    //
    //    template <class T>
    //    T& access_component() {
    //        if (components.contains(T::item_id::value))
    //            return std::get<T>(components[T::item_id::value].type);
    //        else
    //            return std::get<T>(components[T::item_id::value].type = T{});
    //    }
    //
    //    template <class T>
    //    const T& get_component() const {
    //        return std::get<T>(components.at(T::item_id::value).type);
    //    }
    //
    //    template <class T>
    //    void remove_component() {
    //        components.erase(T::item_id::value);
    //    }
    //
    //    void add_component(component&& copy) {
    //        std::visit(
    //            [this, &copy](auto& component) {
    //                using T = std::decay_t<decltype(component)>;
    //                components[T::item_id::value] = std::move(copy);
    //            },
    //            copy.type
    //        );
    //    }
    //
    //    void add_component(const component& copy) {
    //        std::visit(
    //            [this, &copy](auto& component) {
    //                using T = std::decay_t<decltype(component)>;
    //                components[T::item_id::value] = copy;
    //            },
    //            copy.type
    //        );
    //    }
    //
    //    template <class T>
    //    void add_component(const T& copy) {
    //        components[T::item_id::value].type = copy;
    //    }
    //
    //    template <class T>
    //    void add_component(T&& copy) {
    //        components[T::item_id::value].type = std::move(copy);
    //    }
    //
    //    template <class T>
    //    bool has_component() const {
    //        return components.contains(T::item_id::value);
    //    }
    //};
    //
    //struct block_entity_banner : public block_entity{
    //    std::optional<std::string> custom_name;
    //};
}

#endif /* SRC_BASE_OBJECTS_BLOCK_ENTITY */
