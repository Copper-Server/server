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

        size_t get_hash() const;

    private:
        std::vector<component_id> component_ids;
        size_t hash = 0;
        bool is_frozen_ = false;
    };

    struct entity {
        int32_t id;
        uint32_t generation;

        template <class component>
        [[nodiscard]] std::optional<mutable_component<component>> try_modify() {
            void* comp_ptr = detail::get_entity_component(id, generation, detail::get_component_id<component>());
            if (comp_ptr)
                return mutable_component(static_cast<component*>(comp_ptr), id, generation);
            return std::nullopt;
        }

        template <class component>
        [[nodiscard]] mutable_component<component> modify() {
            auto res = try_modify<component>();
            assert(bool(res) && "Component requested via modify() does not exist on this entity!");
            return *res;
        }

        template <class component>
        [[nodiscard]] const component* try_get() const {
            return static_cast<const component*>(detail::get_entity_component(id, generation, detail::get_component_id<component>()));
        }

        template <class component>
        [[nodiscard]] const component& get() const {
            auto res = try_get<component>();
            assert(res != nullptr && "Component requested via get() does not exist on this entity!");
            return *res;
        }

        //the components changes would not be accessible util next tick, all changes buffered
        template <class component, class... args>
        void add(args&&... args) {
            set(component(std::forward<args>(args)...));
        }

        //the components changes would not be accessible util next tick, all changes buffered
        template <class component>
        void set(component&& move) {
            queue_set_entity_component(id, generation, detail::get_component_id<component>(), std::move(move));
        }

        //the components changes would not be accessible util next tick, all changes buffered
        template <class component>
        void set(const component& copy) {
            queue_set_entity_component(id, generation, detail::get_component_id<component>(), component(copy));
        }

        //the components changes would not be accessible util next tick, all changes cached
        template <class component>
        void remove() {
            detail::queue_remove_entity_component(id, generation, detail::get_component_id<component>());
        }

        template <class component>
        [[nodiscard]] bool has() const {
            return detail::has_entity_component(id, generation, detail::get_component_id<component>());
        }

        void destroy() {
            detail::queue_destroy_entity(id, generation);
        }
    };

    template <class... params>
    struct query {
        query() : id() {}

        query(int32_t world_id) : id(world_id) {}

        template <class... without_components>
        [[nodiscard]] query<params..., detail::query_without<without_components...>> without() {
            return query<params..., detail::query_without<without_components...>>{id};
        }

        template <class... reads_components>
        [[nodiscard]] query<params..., detail::query_reads<reads_components...>> reads() {
            return query<params..., detail::query_reads<reads_components...>>{id};
        }

        template <class... writes_components>
        [[nodiscard]] query<params..., detail::query_writes<writes_components...>> writes() {
            return query<params..., detail::query_writes<writes_components...>>{id};
        }

        template <class... with_dirty_components>
        [[nodiscard]] query<params..., detail::query_with_dirty<with_dirty_components...>> with_dirty() {
            return query<params..., detail::query_with_dirty<with_dirty_components...>>{id};
        }

        template <class... with_changed_components>
        [[nodiscard]] query<params..., detail::query_with_changes<with_changed_components...>> with_changes() {
            return query<params..., detail::query_with_changes<with_changed_components...>>{id};
        }

        //this operation marks all written components as dirty,
        // this only viable when the query definitely modifies ALL written items in query
        // using mindlessly would lead to system overload with too much unnecessary updates
        [[nodiscard]] auto request_implicit_marking() {
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

        [[nodiscard]] auto begin() {
            return begin_impl<false>();
        }

        [[nodiscard]] auto end() {
            return decltype(begin())();
        }

    private:
        template <bool explicit_marking>
        auto begin_impl() {
            using traits = detail::query_traits<params...>;
            const auto& all_ids = traits::get_all_component_ids();
            const auto& without_ids = traits::get_without_ids();
            const auto& dirty_ids = traits::get_dirty_ids();
            const auto& changes_ids = traits::get_with_changes_ids();

            const std::vector<component_id>* writes_ids_ptr;
            if constexpr (explicit_marking) {
                static const std::vector<component_id> empty_vec;
                writes_ids_ptr = &empty_vec;
            } else
                writes_ids_ptr = &traits::get_writes_ids();
            const auto& writes_ids = *writes_ids_ptr;

            detail::iteration_handle handle
                = id
                      ? detail::iterate_components(
                            *id,
                            {all_ids.data(), all_ids.size()},
                            {without_ids.data(), without_ids.size()},
                            {writes_ids.data(), writes_ids.size()},
                            {dirty_ids.data(), dirty_ids.size()},
                            {changes_ids.data(), changes_ids.size()}
                        )
                      : detail::iterate_components_global(
                            {all_ids.data(), all_ids.size()},
                            {without_ids.data(), without_ids.size()},
                            {writes_ids.data(), writes_ids.size()},
                            {dirty_ids.data(), dirty_ids.size()},
                            {changes_ids.data(), changes_ids.size()}
                        );

            return typename detail::apply_tuple_to_iter<
                detail::query_iterator,
                detail::is_requires_shifting_v<params...>,
                std::conditional_t<explicit_marking, typename util::apply_tuple_to<detail::iterator_view_dirty_mark, typename traits::WriteTypes>::type, detail::iterator_view>,
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

        [[nodiscard]] bool register_entity_and_block(entity& entity);
        [[nodiscard]] bool unregister_entity_and_block(entity& entity);
        [[nodiscard]] bool transfer_entity_and_block(entity& entity);
        [[nodiscard]] entity create_entity_and_wait(const entity_recipe& recipe);

        [[nodiscard]] query<> view() {
            return query<>{id};
        }

    private:
        int32_t id;
    };

    namespace global_registry {
        [[nodiscard]] query<> view() {
            return query<>{};
        }

        //recomended to use this to avoid short locks on fully loaded server
        template <class component>
        void register_component() {
            detail::get_component_id<component>();
        }

        fast_task::future_ptr<entity> create_entity_async(const entity_recipe& recipe); //the recipe should be static or live longer than future_ptr
        [[nodiscard]] entity create_entity_and_wait(const entity_recipe& recipe);
    }

    struct system_interface {
        using reads = dependent<>;
        using writes = dependent<>;

        virtual ~system_interface() noexcept = default;

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
        //  also builds the dependency graph if system added
        void execute_frame(world_local_registry& registry);

    private:
        void add_system_impl(std::unique_ptr<system_interface> system, detail::system_info& info);
        struct scheduler_data;
        std::unique_ptr<scheduler_data> data;
    };
}

#endif /* SRC_API_ECS */
