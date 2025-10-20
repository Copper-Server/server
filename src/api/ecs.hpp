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
#include <cassert>
#include <library/list_array.hpp>
#include <src/api/ecs/detail.hpp>

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

        entity_recipe& freeze();

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
            return std::move(*res);
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
        template <class component, class... Args>
        void add(Args&&... args) {
            set(component(std::forward<Args>(args)...));
        }

        //the components changes would not be accessible util next tick, all changes buffered
        template <class component>
        void set(component&& move) {
            detail::queue_set_entity_component(id, generation, detail::get_component_id<component>(), std::move(move));
        }

        //the components changes would not be accessible util next tick, all changes buffered
        template <class component>
        void set(const component& copy) {
            detail::queue_set_entity_component(id, generation, detail::get_component_id<component>(), component(copy));
        }

        //the components changes would not be accessible util next tick, all changes cached
        template <class component>
        void remove() {
            detail::queue_remove_entity_component(id, generation, detail::get_component_id<component>());
        }

        template <class component>
        void add_relation(entity child) {
            detail::queue_add_relation(detail::get_component_id<component>(), *this, child);
        }

        template <class component>
        void remove_relation(entity child) {
            detail::queue_remove_relation(detail::get_component_id<component>(), *this, child);
        }

        template <class component>
        bool has_relation(entity child) {
            return detail::has_relation(detail::get_component_id<component>(), *this, child);
        }

        template <class component>
        [[nodiscard]] bool has() const {
            return detail::has_entity_component(id, generation, detail::get_component_id<component>());
        }

        //could throw depending on components
        std::optional<entity> copy_and_wait() const;

        void destroy() {
            detail::queue_destroy_entity(id, generation);
        }

        bool is_assigned_to_world(int32_t world_id) const {
            return detail::get_entity_assigned_to_world(id, generation) == world_id;
        }

        std::optional<int32_t> get_assigned_world_id() const {
            return detail::get_entity_assigned_to_world(id, generation);
        }

        bool operator==(const entity& other) const {
            return id == other.id && generation == other.generation;
        }

        bool operator!=(const entity& other) const {
            return id != other.id || generation != other.generation;
        }
    };

    template <class... params>
    struct query {
        query() : id() {}

        query(query&& mov) : id(mov.id), with_relations(std::move(mov.with_relations)) {}

        query(int32_t world_id) : id(world_id) {}
        
        query(std::optional<int32_t> id, list_array<std::pair<component_id, entity>>&& with_relations) : id(id), with_relations(std::move(with_relations)) {}

        query& operator=(query&& mov) {
            id = mov.id;
            with_relations = std::move(mov.with_relations);
            return *this;
        }

        template <class... with_components>
        [[nodiscard]] query<params..., detail::query_with<with_components...>> with() {
            return query<params..., detail::query_with<with_components...>>{id, std::move(with_relations)};
        }

        template <class... without_components>
        [[nodiscard]] query<params..., detail::query_without<without_components...>> without() {
            return query<params..., detail::query_without<without_components...>>{id, std::move(with_relations)};
        }

        template <class... reads_components>
        [[nodiscard]] query<params..., detail::query_reads<reads_components...>> reads() {
            return query<params..., detail::query_reads<reads_components...>>{id, std::move(with_relations)};
        }

        template <class... writes_components>
        [[nodiscard]] query<params..., detail::query_writes<writes_components...>> writes() {
            return query<params..., detail::query_writes<writes_components...>>{id, std::move(with_relations)};
        }

        template <class... with_dirty_components>
        [[nodiscard]] query<params..., detail::query_with_dirty<with_dirty_components...>> with_dirty() {
            return query<params..., detail::query_with_dirty<with_dirty_components...>>{id, std::move(with_relations)};
        }

        template <class... with_clear_components>
        [[nodiscard]] query<params..., detail::query_with_clear<with_clear_components...>> with_clear() {
            return query<params..., detail::query_with_clear<with_clear_components...>>{id, std::move(with_relations)};
        }

        template <class... with_changed_components>
        [[nodiscard]] query<params..., detail::query_with_changes<with_changed_components...>> with_changes() {
            return query<params..., detail::query_with_changes<with_changed_components...>>{id, std::move(with_relations)};
        }

        template <class tag_component>
        [[nodiscard]] query<params..., detail::has_relation_query<tag_component>> with_relation(entity child) {
            with_relations.emplace_back(detail::get_component_id<tag_component>(), child);
            return query<params..., detail::has_relation_query<tag_component>>{id, std::move(with_relations)};
        }

        //this operation marks all written components as dirty,
        // this only viable when the query definitely modifies ALL written items in query
        // using mindlessly would lead to system overload with too much unnecessary updates
        [[nodiscard]] auto request_implicit_marking() && {
            struct implicit_marking_struct {
                implicit_marking_struct(query<params...>&& q) : internal(std::move(q)) {}

                auto begin() {
                    return internal.template begin_impl<true>();
                }

                auto end() {
                    return decltype(begin())();
                }

            private:
                query<params...> internal;
            };

            return implicit_marking_struct(std::move(*this));
        }

        [[nodiscard]] auto begin() {
            return begin_impl<false>();
        }

        [[nodiscard]] auto end() {
            return decltype(begin())();
        }

        template <class FN>
        void for_each_chunk(FN&& fn) {
            begin().chunk_iterate(std::forward<FN>(fn));
        }

        template <class FN>
        void for_each_chunk_view(FN&& fn) {
            begin().chunk_iterate_view(std::forward<FN>(fn));
        }

        template <class FN>
        void par_for_each_chunk(FN&& fn) {
            begin().chunk_iterate_parallel(std::forward<FN>(fn));
        }

        template <class FN>
        void par_for_each_chunk_view(FN&& fn) {
            begin().chunk_iterate_parallel_view(std::forward<FN>(fn));
        }

    private:
        template <bool explicit_marking>
        auto begin_impl() {
            using traits = detail::query_traits<params...>;
            auto all_ids = traits::get_all_component_ids();
            auto with_ids = traits::get_with_ids();
            auto without_ids = traits::get_without_ids();
            auto dirty_ids = traits::get_dirty_ids();
            auto clean_ids = traits::get_clear_ids();
            auto changes_ids = traits::get_with_changes_ids();

            std::span<component_id> writes_ids;
            if constexpr (explicit_marking)
                writes_ids = traits::get_writes_ids();

            detail::iteration_handle handle
                = id
                      ? detail::iterate_components(
                            *id,
                            all_ids,
                            with_ids,
                            without_ids,
                            writes_ids,
                            dirty_ids,
                            clean_ids,
                            changes_ids,
                            {with_relations.data(), with_relations.size()}
                        )
                      : detail::iterate_components_global(
                            all_ids,
                            with_ids,
                            without_ids,
                            writes_ids,
                            dirty_ids,
                            clean_ids,
                            changes_ids,
                            {with_relations.data(), with_relations.size()}
                        );

            return typename detail::apply_tuple_to_iter<
                detail::query_iterator,
                detail::is_requires_shifting_v<params...>,
                std::conditional_t<explicit_marking, typename util::apply_tuple_to<detail::iterator_view_dirty_mark, typename traits::WriteTypes>::type, detail::iterator_view>,
                typename traits::IteratorTuple>::type(std::move(handle));
        }

        std::optional<int32_t> id;
        list_array<std::pair<component_id, entity>> with_relations;
    };

    struct world_local_registry {
        world_local_registry(int32_t id);
        ~world_local_registry();

        fast_task::future_ptr<bool> register_entity_async(entity& entity);
        fast_task::future_ptr<bool> unregister_entity_async(entity& entity);
        fast_task::future_ptr<bool> transfer_entity_async(entity& entity);                //the old world local registry received from entity's internal data
        fast_task::future_ptr<entity> allocate_entity_async(const entity_recipe& recipe); //the recipe should be static or live longer than future_ptr

        template <class... components>
        fast_task::future_ptr<entity> create_entity_async(components&&... args) {
            return detail::create_entity__cc(id, std::forward<components>(args)...);
        }

        template <class... components>
        fast_task::future_ptr<entity> create_entity_with_recipe_async(const entity_recipe& recipe, components&&... args) {
            return detail::create_entity_r_cc(id, recipe, std::forward<components>(args)...);
        }

        [[nodiscard]] bool register_entity_and_block(entity& entity);
        [[nodiscard]] bool unregister_entity_and_block(entity& entity);
        [[nodiscard]] bool transfer_entity_and_block(entity& entity);
        [[nodiscard]] entity allocate_entity_and_wait(const entity_recipe& recipe);

        template <class... components>
        entity create_entity_and_wait(components&&... args) {
            return detail::create_entity__cc(id, std::forward<components>(args)...)->take();
        }

        template <class... components>
        entity create_entity_with_recipe_and_wait(const entity_recipe& recipe, components&&... args) {
            return detail::create_entity_r_cc(id, recipe, std::forward<components>(args)...)->take();
        }

        [[nodiscard]] query<> view() {
            return query<>{id};
        }

        int32_t get_id() const {
            return id;
        }

    private:
        int32_t id;
    };

    namespace global_registry {
        inline [[nodiscard]] query<> view() {
            return query<>{};
        }

        //recommended to use this to avoid short locks on fully loaded server
        template <class component>
        void register_component() {
            detail::get_component_id<component>();
        }

        fast_task::future_ptr<entity> allocate_entity_async(const entity_recipe& recipe); //the recipe should be static or live longer than future_ptr
        [[nodiscard]] entity allocate_entity_and_wait(const entity_recipe& recipe);

        template <class... components>
        fast_task::future_ptr<entity> create_entity_async(components&&... args) {
            return detail::create_entity__cc(std::nullopt, std::forward<components>(args)...);
        }

        template <class... components>
        fast_task::future_ptr<entity> create_entity_with_recipe_async(const entity_recipe& recipe, components&&... args) {
            return detail::create_entity_r_cc(std::nullopt, recipe, std::forward<components>(args)...);
        }

        template <class... components>
        entity create_entity_and_wait(components&&... args) {
            return detail::create_entity__cc(std::nullopt, std::forward<components>(args)...)->take();
        }

        template <class... components>
        entity create_entity_with_recipe_and_wait(const entity_recipe& recipe, components&&... args) {
            return detail::create_entity_r_cc(std::nullopt, recipe, std::forward<components>(args)...)->take();
        }

        template <class... Params>
        std::vector<entity> execute_query_immediate(query<Params...>&& query_obj) {
            fast_task::unique_lock lock(detail::immediate_lock());
            std::vector<entity> entity;
            for (auto&& req : query_obj)
                entity.push_back(std::get<0>(req).current_entity());
            return entity;
        }

        void global_tick();
    }

    struct system_interface {
        using reads = dependent<>;
        using writes = dependent<>;

        virtual ~system_interface() noexcept = default;

        virtual void tick(world_local_registry& world) = 0;
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
        void add_system_impl(std::unique_ptr<system_interface> system, const detail::system_info& info);
        struct scheduler_data;
        std::unique_ptr<scheduler_data> data;
    };
}

#include <src/api/ecs/late_definition.hpp>

namespace std {
    template <>
    struct hash<copper_server::api::ecs::entity> {
        size_t operator()(const copper_server::api::ecs::entity& ent) const noexcept {
            return std::hash<int32_t>{}(ent.id) ^ (std::hash<uint32_t>{}(ent.generation) << 1);
        }
    };
}

#endif /* SRC_API_ECS */
