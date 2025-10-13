/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_ECS
#define SRC_API_ECS
#include <src/api/detail/ecs.hpp>

//entity component system
namespace copper_server::api::ecs {
    struct entity_recipe {
        entity_recipe& with(const entity_recipe& recipe) {
            if (!is_frozen_) {
                component_ids.reserve(component_ids.size() + recipe.component_ids.size());
                component_ids.insert(component_ids.end(), recipe.component_ids.begin(), recipe.component_ids.end());
            }
            return *this;
        }

        entity_recipe& with(component_id id) {
            if (!is_frozen_)
                component_ids.push_back(id);
            return *this;
        }

        template <class component>
        entity_recipe& with() {
            if (!is_frozen_)
                component_ids.push_back(detail::get_component_id<component>());
            return *this;
        }

        void freeze();

        bool is_frozen() const {
            return is_frozen_;
        }

        const std::vector<component_id>& get_ids() const;

    private:
        std::vector<component_id> component_ids;
        bool is_frozen_ = false;
    };
    struct entity;

    template <class T>
    class mutable_component {
    public:
        // Rule of 5: This object is a temporary handle, it should not be copied.
        // Moving is okay, as it transfers the responsibility of marking dirty.
        mutable_component(const mutable_component&) = delete;
        mutable_component& operator=(const mutable_component&) = delete;

        mutable_component(mutable_component&& other) noexcept
            : component_ptr_(other.component_ptr_), owner_entity_(other.owner_entity_) {
            other.component_ptr_ = nullptr;
        }

        mutable_component& operator=(mutable_component&& other) noexcept {
            if (this != &other) {
                mark_dirty_if_valid();
                component_ptr_ = other.component_ptr_;
                owner_entity_ = other.owner_entity_;
                other.component_ptr_ = nullptr;
            }
            return *this;
        }

        ~mutable_component() {
            mark_dirty_if_valid();
        }

        T* operator->() const {
            return component_ptr_;
        }

        T& operator*() const {
            return *component_ptr_;
        }

    private:
        // Only the entity can create this object.
        friend struct entity;

        mutable_component(T* ptr, entity* owner)
            : component_ptr_(ptr), owner_entity_(owner) {}

        void mark_dirty_if_valid() {
            if (component_ptr_)
                // This calls a new method on entity that we need to add.
                owner_entity_->mark_dirty_impl(detail::get_component_id<T>());
        }

        T* component_ptr_;
        entity* owner_entity_;
    };

    struct entity {
        int32_t id;
        uint32_t generation;

        template <class component>
        std::optional<mutable_component<component>> try_modify() {
            void* comp_ptr = detail::get_entity_component(id, generation, detail::get_component_id<component>());
            if (comp_ptr)
                return mutable_component(static_cast<component*>(comp_ptr), this);
            return std::nullopt;
        }

        template <class component, class FN>
        void try_modify(FN&& callback) {
            auto res = try_modify<component>();
            if (res)
                callback(*res);
        }

        template <class component>
        mutable_component<component> modify() {
            auto res = try_modify<component>();
            assert(bool(res) && "Component requested via modify() does not exist on this entity!");
            return *res;
        }

        template <class component, class FN>
        void try_get(FN&& callback) const {
            auto res = try_get<component>();
            if (res)
                callback(*res);
        }

        template <class component>
        const component* try_get() const {
            return static_cast<const component*>(detail::get_entity_component(id, generation, detail::get_component_id<component>()));
        }

        template <class component>
        const component& get() const {
            auto res = try_get<component>();
            assert(comp_ptr != nullptr && "Component requested via get() does not exist on this entity!");
            return *comp_ptr;
        }

        //the components changes would not be accessible util next tick, all changes buffered
        template <class component, class... args>
        void add(args&&... args) {
            set(component(std::forward<args>(args)...));
        }

        //the components changes would not be accessible util next tick, all changes buffered
        template <class component>
        void set(component&& move) {
            detail::queue_set_entity_component(id, generation, detail::get_component_id<component>(), &move);
        }

        template <class component>
        void set(const component& copy) {
            component tmp(copy); //would be moved to internal cache
            detail::queue_set_entity_component(id, generation, detail::get_component_id<component>(), &tmp);
        }

        //the components changes would not be accessible util next tick, all changes cached
        template <class component>
        void remove() {
            detail::queue_remove_entity_component(id, generation, detail::get_component_id<component>());
        }

        template <class component>
        bool has() const {
            return detail::has_entity_component(id, generation, detail::get_component_id<component>());
        }

        void destroy() {
            detail::queue_destroy_entity(id, generation);
        }

    private:
        template <class T>
        friend class mutable_component;

        void mark_dirty_impl(component_id comp_id) {
            detail::queue_mark_dirty(id, generation, comp_id);
        }
    };

    template <class... params>
    struct query {
        query() : id() {}

        query(int32_t world_id) : id(world_id) {}

        template <class... without_components>
        query<params..., detail::query_without<without_components...>> without() {
            return query<params..., detail::query_without<without_components...>>{id};
        }

        template <class... reads_components>
        query<params..., detail::query_reads<reads_components...>> reads() {
            return query<params..., detail::query_reads<reads_components...>>{id};
        }

        template <class... writes_components>
        query<params..., detail::query_writes<writes_components...>> writes() {
            return query<params..., detail::query_writes<writes_components...>>{id};
        }

        template <class... with_dirty_components>
        query<params..., detail::query_with_dirty<with_dirty_components...>> with_dirty() {
            return query<params..., detail::query_with_dirty<with_dirty_components...>>{id};
        }

        //this operation marks all written components as dirty,
        // this only viable when the query definitely modifies ALL written items in query
        // using mindlessly would lead to system overload with too much unnecessary updates
        auto request_implicit_marking() {
            struct implicit_marking_struct {
                implicit_marking_struct(const query<params...>& q) : internal(q) {}

                auto begin() {
                    return internal.template begin_impl<true>();
                }

                auto end() {
                    return decltype(begin())();
                }

            private:
                query<params...> internal;
            };

            return implicit_marking_struct(*this);
        }

        auto begin() {
            return begin_impl<false>();
        }

        auto end() {
            return decltype(begin())();
        }

    private:
        template <bool explicit_marking>
        auto begin_impl() {
            using traits = detail::query_traits<params...>;
            const auto& all_ids = traits::get_all_component_ids();
            const auto& without_ids = traits::get_without_ids();
            const auto& dirty_ids = traits::get_dirty_ids();

            const std::vector<component_id>* writes_ids_ptr;
            if constexpr (implicit_marking_enabled) {
                static const std::vector<component_id> empty_vec;
                writes_ids_ptr = &empty_vec;
            } else
                writes_ids_ptr = &traits::get_writes_ids();
            const auto& writes_ids = *writes_ids_ptr;

            detail::iteration_handle handle
                = id
                      ? detail::iterate_components(
                            *id,
                            all_ids.data(),
                            all_ids.size(),
                            without_ids.data(),
                            without_ids.size(),
                            writes_ids.data(),
                            writes_ids.size(),
                            dirty_ids.data(),
                            dirty_ids.size()
                        )
                      : detail::iterate_components_global(
                            all_ids.data(),
                            all_ids.size(),
                            without_ids.data(),
                            without_ids.size(),
                            writes_ids.data(),
                            writes_ids.size(),
                            dirty_ids.data(),
                            dirty_ids.size()
                        );

            return typename detail::apply_tuple_to_iter<
                detail::query_iterator,
                implicit_marking_enabled,
                detail::has_dirty_filter_v<params...>,
                typename util::apply_tuple_to<detail::dirty_marker, typename traits::WriteTypes>::type,
                typename traits::IteratorTuple>::type(std::move(handle));
        }

        std::optional<int32_t> id;
    };

    struct world_local_registry {
        world_local_registry(int32_t id) : id(id) {}

        fast_task::future_ptr<bool> register_entity_async(entity& entity);
        fast_task::future_ptr<bool> unregister_entity_async(entity& entity);
        fast_task::future_ptr<bool> transfer_entity_async(entity& entity);              //the old world local registry received from entity's internal data
        fast_task::future_ptr<entity> create_entity_async(const entity_recipe& recipe); //the recipe should be static or live longer than future_ptr

        bool register_entity(entity& entity);
        bool unregister_entity(entity& entity);
        bool transfer_entity(entity& entity);
        entity create_entity(const entity_recipe& recipe);

        query<> view() {
            return query<>{id};
        }

    private:
        int32_t id;
    };

    namespace global_registry {
        query<> view() {
            return query<>{};
        }

        //recomended to use this to avoid short locks on fully loaded server
        template <class component>
        void register_component() {
            detail::get_component_id<component>();
        }

        fast_task::future_ptr<entity> create_entity_async(const entity_recipe& recipe); //the recipe should be static or live longer than future_ptr
        entity create_entity(const entity_recipe& recipe);
    }

    struct system_interface {
        using reads = dependent<>;
        using writes = dependent<>;

        virtual ~system_interface() = default;

        virtual void setup() {}

        virtual void shutdown() {}

        virtual void tick() = 0;
    };

    struct scheduler {
        scheduler();
        ~scheduler();

        template <typename T>
        void add_system() {
            add_system_impl(std::make_unique<T>(), detail::get_system_info<T>());
        }

        // Called each tick to run all systems in parallel
        //   also calls the dependency_graph if system_registry_changed
        void execute_frame(world_local_registry& registry);

    private:
        void add_system_impl(std::unique_ptr<system_interface> system, detail::system_info& info);
        struct data_t;
        data_t* data;
    };
}

#endif /* SRC_API_ECS */
