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
#include <src/base_objects/uuid.hpp>
#include <src/util/cts.hpp>
#include <variant>

//entity component system
namespace copper_server::api::ecs {
    enum class relation_type : uint8_t {
        strong,
        weak
    };

    struct relation_entry {
        ecs::entity target;
        relation_type type;

        relation_entry(ecs::entity e, relation_type t = relation_type::strong)
            : target(e), type(t) {}
    };

    struct relation_visitor {
        struct context_t {
            void (*on_unlink)(void*, ecs::entity self, ecs::entity target_holder);
            void* component;

            void make_unlink(ecs::entity self, ecs::entity target_holder) const {
                on_unlink(component, self, target_holder);
            }
        };

        std::move_only_function<void(ecs::entity target, relation_type type, context_t& context)> callback;
        context_t context;

        relation_visitor(std::move_only_function<void(ecs::entity target, relation_type type, context_t& context)>&& callback)
            : callback(std::move(callback)) {}

        relation_visitor(const relation_visitor&) = delete;
        relation_visitor(relation_visitor&&) = delete;
        relation_visitor& operator=(const relation_visitor&) = delete;
        relation_visitor& operator=(relation_visitor&&) = delete;

        void push(entity e, relation_type type) {
            callback(e, type, context);
        }
    };

    //Low level api, use entity_definition instead of this
    struct entity_recipe {
        entity_recipe();
        ~entity_recipe();
        entity_recipe(const entity_recipe& other);
        entity_recipe(entity_recipe&& other) noexcept;
        entity_recipe& operator=(const entity_recipe& other);
        entity_recipe& operator=(entity_recipe&& other) noexcept;

        entity_recipe& with(const entity_recipe& recipe);
        entity_recipe& with(component_id id);

        template <class component>
        entity_recipe& with() {
            if (!is_frozen_)
                component_ids.push_back(detail::get_component_id<component>());
            return *this;
        }

        template <class component>
        entity_recipe& with_value(const component& value) {
            if (!is_frozen_) {
                component_id id = detail::get_component_id<component>();
                component_ids.push_back(id);

                // Remove existing default if present
                if (auto it = default_values.find(id); it != default_values.end()) {
                    const auto& info = detail::component_info_registry.at(id);
                    info.destroy(it->second);
                    ::operator delete(it->second);
                    default_values.erase(it);
                }

                // Allocate and copy
                const auto& info = detail::component_info_registry.at(id);
                void* mem = ::operator new(info.size, std::align_val_t(info.alignment));
                info.construct(mem);                  // Default construct first
                info.copy_assign(mem, (void*)&value); // Then assign
                default_values[id] = mem;
            }
            return *this;
        }

        template <class component>
        entity_recipe& with_tag(uint32_t value) {
            if (!is_frozen_)
                component_ids.push_back(detail::get_tag_entry<component>(value));
            return *this;
        }

        template <class component>
        entity_recipe& with_tag(int32_t value) {
            if (!is_frozen_)
                component_ids.push_back(detail::get_tag_entry<component>(value));
            return *this;
        }

        entity_recipe& freeze();

        bool is_frozen() const;

        const std::vector<whole_component_id>& get_ids() const;

        const std::unordered_map<component_id, void*>& get_defaults() const {
            return default_values;
        }

        size_t get_hash() const;

    private:
        void cleanup();
        void clone_value(component_id id, void* src);

        std::vector<whole_component_id> component_ids;
        std::unordered_map<component_id, void*> default_values; // Owns the memory
        size_t hash = 0;
        bool is_frozen_ = false;
    };

    struct entity {
        mutable uint32_t id;
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
        [[nodiscard]] bool has() const {
            return detail::has_entity_component(id, generation, detail::get_component_id<component>());
        }

        template <class T>
        [[nodiscard]] bool has_tag(int32_t value) const {
            return detail::has_entity_component(id, generation, detail::get_tag_entry<component>(value));
        }

        template <class T>
        [[nodiscard]] bool has_tag(uint32_t value) const {
            return detail::has_entity_component(id, generation, detail::get_tag_entry<component>(value));
        }

        template <detail::tag_component T>
        [[nodiscard]] bool has_tag(T value) const {
            return detail::has_entity_component(id, generation, detail::get_tag_entry(value));
        }

        void destroy();

        //could throw depending on components
        std::optional<entity> copy_and_wait() const;

        bool is_assigned_to_world(int32_t world_id) const {
            return detail::get_entity_assigned_to_world(id, generation) == world_id;
        }

        std::optional<int32_t> get_assigned_world_id() const {
            return detail::get_entity_assigned_to_world(id, generation);
        }

        void get_all_relations(relation_visitor& visitor) const {
            detail::request_all_childs(id, generation, visitor);
        }

        bool operator==(const entity& other) const {
            return id == other.id && generation == other.generation;
        }

        bool operator!=(const entity& other) const {
            return id != other.id || generation != other.generation;
        }

        bool is_valid() const {
            if (id == UINT32_MAX)
                return false;
            else if (detail::is_valid(id, generation)) {
                id = UINT32_MAX;
                return false;
            } else
                return true;
        }
    };

    struct entity_ref {
        std::variant<base_objects::uuid, entity> value;
        entity get_entity();
        base_objects::uuid get_uuid();

        bool is_resolved();
        bool is_valid() const;

        bool try_resolve();

        bool operator==(const entity_ref& other) const;
        bool operator!=(const entity_ref& other) const;

        bool operator==(const entity& other) const;
        bool operator!=(const entity& other) const;

        bool operator==(const base_objects::uuid& other) const;
        bool operator!=(const base_objects::uuid& other) const;
    };

    struct unique_entity : public entity {
        using entity::entity;
        using entity::operator=;

        unique_entity() = default;

        ~unique_entity();

        entity release();

        bool has_value();
    };

    template <class... params>
    struct query {
        query() : id() {}

        query(query&& mov) noexcept : id(mov.id), with_relations(std::move(mov.with_relations)), with_tag_components(std::move(with_tag_components)), without_tag_components(std::move(without_tag_components)) {}

        query(const query& copy) : id(copy.id), with_relations(copy.with_relations), with_tag_components(with_tag_components), without_tag_components(without_tag_components) {}

        query(int32_t world_id) : id(world_id) {}

        query(std::optional<int32_t> id, list_array<std::pair<component_id, entity>>&& with_relations, list_array<whole_component_id>&& with_tag_components, list_array<whole_component_id>&& without_tag_components)
            : id(id), with_relations(std::move(with_relations)), with_tag_components(std::move(with_tag_components)), without_tag_components(std::move(without_tag_components)) {}

        query& operator=(query&& mov) noexcept {
            id = mov.id;
            with_relations = std::move(mov.with_relations);
            with_tag_components = std::move(mov.with_tag_components);
            without_tag_components = std::move(mov.without_tag_components);
            return *this;
        }

        query& operator=(const query& copy) {
            id = copy.id;
            with_relations = copy.with_relations;
            with_tag_components = copy.with_tag_components;
            without_tag_components = copy.without_tag_components;
            return *this;
        }

        template <class... with_components>
        [[nodiscard]] query<params..., detail::query_with<with_components...>> with() && {
            return query<params..., detail::query_with<with_components...>>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components)};
        }

        template <class... without_components>
        [[nodiscard]] query<params..., detail::query_without<without_components...>> without() && {
            return query<params..., detail::query_without<without_components...>>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components)};
        }

        template <class with_tag_component>
        [[nodiscard]] query<params...> with_tag(int32_t value) && {
            return query<params...>{id, std::move(with_relations), std::move(with_tag_components).push_back(detail::get_tag_entry<with_tag_component>(value)), std::move(mov.without_tag_components)};
        }

        template <class with_tag_component>
        [[nodiscard]] query<params...> with_tag(uint32_t value) && {
            return query<params...>{id, std::move(with_relations), std::move(with_tag_components).push_back(detail::get_tag_entry<with_tag_component>(value)), std::move(mov.without_tag_components)};
        }

        template <detail::tag_component with_tag_component>
        [[nodiscard]] query<params...> with_tag(const with_tag_component& value) && {
            return query<params...>{id, std::move(with_relations), std::move(with_tag_components).push_back(detail::get_tag_entry(value)), std::move(mov.without_tag_components)};
        }

        template <class without_tag_component>
        [[nodiscard]] query<params...> without_tag(int32_t value) && {
            return query<params...>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components).push_back(detail::get_tag_entry<without_tag_component>(value))};
        }

        template <class without_tag_component>
        [[nodiscard]] query<params...> without_tag(uint32_t value) && {
            return query<params...>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components).push_back(detail::get_tag_entry<without_tag_component>(value))};
        }

        template <detail::tag_component without_tag_component>
        [[nodiscard]] query<params...> without_tag(const without_tag_component& value) && {
            return query<params...>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components).push_back(detail::get_tag_entry(value))};
        }

        template <class... reads_components>
        [[nodiscard]] query<params..., detail::query_reads<reads_components...>> reads() && {
            return query<params..., detail::query_reads<reads_components...>>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components)};
        }

        template <class... writes_components>
        [[nodiscard]] query<params..., detail::query_writes<writes_components...>> writes() && {
            return query<params..., detail::query_writes<writes_components...>>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components)};
        }

        template <class... with_dirty_components>
        [[nodiscard]] query<params..., detail::query_with_dirty<with_dirty_components...>> with_dirty() && {
            return query<params..., detail::query_with_dirty<with_dirty_components...>>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components)};
        }

        template <class... with_clear_components>
        [[nodiscard]] query<params..., detail::query_with_clear<with_clear_components...>> with_clear() && {
            return query<params..., detail::query_with_clear<with_clear_components...>>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components)};
        }

        template <class... with_changed_components>
        [[nodiscard]] query<params..., detail::query_with_changes<with_changed_components...>> with_changes() && {
            return query<params..., detail::query_with_changes<with_changed_components...>>{id, std::move(with_relations), std::move(with_tag_components), std::move(mov.without_tag_components)};
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

        void set_world_id(int32_t world_id) {
            id = world_id;
        }

        void reset_world_id() {
            id = std::nullopt;
        }

    private:
        template <bool explicit_marking>
        std::shared_ptr<detail::iteration_topology> begin_topology_create() {
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

            return id
                       ? detail::iterate_components(
                             *id,
                             all_ids,
                             with_ids,
                             without_ids,
                             writes_ids,
                             dirty_ids,
                             clean_ids,
                             changes_ids,
                             {with_tag_components.data(), with_tag_components.size()},
                             {without_tag_components.data(), without_tag_components.size()}
                         )
                       : detail::iterate_components_global(
                             all_ids,
                             with_ids,
                             without_ids,
                             writes_ids,
                             dirty_ids,
                             clean_ids,
                             changes_ids,
                             {with_tag_components.data(), with_tag_components.size()},
                             {without_tag_components.data(), without_tag_components.size()}
                         );
        }

        template <bool explicit_marking>
        std::shared_ptr<detail::iteration_topology> begin_topology() {
            if constexpr (explicit_marking) {
                if (!exp_mark_cached_topology) {
                    exp_mark_cached_version = detail::get_state_version(_q.id);
                    return exp_mark_cached_topology = begin_topology_create<explicit_marking>();
                } else {
                    auto current_version = detail::get_state_version(_q.id);
                    if (exp_mark_cached_version == current_version)
                        return exp_mark_cached_topology;
                    else {
                        exp_mark_cached_version = current_version;
                        return exp_mark_cached_topology = begin_topology_create<explicit_marking>();
                    }
                }
            } else {
                if (!cached_topology) {
                    cached_version = detail::get_state_version(_q.id);
                    return cached_topology = begin_topology_create<explicit_marking>();
                } else {
                    auto current_version = detail::get_state_version(_q.id);
                    if (cached_version == current_version)
                        return cached_topology;
                    else {
                        cached_version = current_version;
                        return cached_topology = begin_topology_create<explicit_marking>();
                    }
                }
            }
        }

        template <bool explicit_marking>
        detail::iteration_handle begin_handle() {
            using traits = detail::query_traits<params...>;
            return detail::make_handle(begin_topology(), traits::get_all_component_count());
        }

        template <bool explicit_marking>
        auto begin_wrap(detail::iteration_handle&& handle) {
            using traits = detail::query_traits<params...>;

            return typename detail::apply_tuple_to_iter<
                detail::query_iterator,
                detail::is_requires_shifting_v<params...>,
                std::conditional_t<explicit_marking, typename util::apply_tuple_to<detail::iterator_view_dirty_mark, typename traits::WriteTypes>::type, detail::iterator_view>,
                typename traits::IteratorTuple>::type(std::move(handle));
        }

        template <bool explicit_marking>
        auto begin_impl() {
            return begin_wrap(begin_handle<explicit_marking>());
        }

        std::optional<int32_t> id;
        list_array<whole_component_id> with_tag_components;
        list_array<whole_component_id> without_tag_components;

        mutable std::shared_ptr<detail::iteration_topology> exp_mark_cached_topology;
        mutable size_t exp_mark_cached_version = 0;
        mutable std::shared_ptr<detail::iteration_topology> cached_topology;
        mutable size_t cached_version = 0;
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
            return detail::create_entity__cc(world_ptr, std::forward<components>(args)...);
        }

        template <class... components>
        fast_task::future_ptr<entity> create_entity_with_recipe_async(const entity_recipe& recipe, components&&... args) {
            return detail::create_entity_r_cc(world_ptr, recipe, std::forward<components>(args)...);
        }

        [[nodiscard]] bool register_entity_and_block(entity& entity);
        [[nodiscard]] bool unregister_entity_and_block(entity& entity);
        [[nodiscard]] bool transfer_entity_and_block(entity& entity);
        [[nodiscard]] entity allocate_entity_and_wait(const entity_recipe& recipe);

        template <class... components>
        entity create_entity_and_wait(components&&... args) {
            return detail::create_entity__cc(world_ptr, std::forward<components>(args)...)->take();
        }

        template <class... components>
        entity create_entity_with_recipe_and_wait(const entity_recipe& recipe, components&&... args) {
            return detail::create_entity_r_cc(world_ptr, recipe, std::forward<components>(args)...)->take();
        }

        [[nodiscard]] query<> view() {
            return query<>{get_id()};
        }

        int32_t get_id() const {
            return detail::get_world_id(world_ptr);
        }

        //note for yourself, you need to parse the stream twice to get the entity, first one before calling this function to get id and second one to get result from load_ecs_entity
        entity load_ecs_entity(const std::string& named_id, util::nbt_read_stream& stream) {
            return detail::load_ecs_entity(named_id, stream, world_ptr);
        }

        entity load_entity(const std::string& named_id, util::nbt_read_stream& stream) {
            return detail::load_ecs_entity("@entity:" + named_id, stream, world_ptr);
        }

        entity load_block_entity(const std::string& named_id, util::nbt_read_stream& stream) {
            return detail::load_ecs_entity("@block_entity:" + named_id, stream, world_ptr);
        }

        void store_ecs_entity(const std::string& id, util::nbt_write_stream& stream, entity ecs_e) {
            detail::store_ecs_entity(id, stream, ecs_e);
        }

        void store_entity(const std::string& id, util::nbt_write_stream& stream, entity e) {
            detail::store_ecs_entity("@entity:" + id, stream, e);
        }

        void store_block_entity(const std::string& id, util::nbt_write_stream& stream, entity block_e) {
            detail::store_ecs_entity("@block_entity:" + id, stream, block_e);
        }

        world* get_ecs_world_ref() const noexcept {
            return world_ptr;
        }

    private:
        world* world_ptr;
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

        entity load_ecs_entity(const std::string& id, util::nbt_read_stream& stream) {
            return detail::load_ecs_entity(id, stream, std::nullopt);
        }

        entity load_entity(const std::string& id, util::nbt_read_stream& stream) {
            return detail::load_ecs_entity("@entity:" + id, stream, std::nullopt);
        }

        entity load_block_entity(const std::string& id, util::nbt_read_stream& stream) {
            return detail::load_ecs_entity("@block_entity:" + id, stream, std::nullopt);
        }

        void store_ecs_entity(const std::string& id, util::nbt_write_stream& stream, entity ecs_e) {
            detail::store_ecs_entity(id, stream, ecs_e);
        }

        void store_entity(const std::string& id, util::nbt_write_stream& stream, entity e) {
            detail::store_ecs_entity("@entity:" + id, stream, e);
        }

        void store_block_entity(const std::string& id, util::nbt_write_stream& stream, entity block_e) {
            detail::store_ecs_entity("@block_entity:" + id, stream, block_e);
        }

        void global_tick();
    }

    struct system_interface {
        using reads = dependent<>;
        using writes = dependent<>;

        virtual ~system_interface() noexcept = default;

        virtual void tick(world_local_registry& world) = 0;
    };

    enum class tick_phase {
        early_processing, //for cleanup or for other things before processing anything

        mobile_entity,
        block_entity,
    };

    struct scheduler {
        template <class... params>
        struct system_builder {
            system_builder() = default;

            system_builder(scheduler& sched, query<params...>&& query_prototype) : sched(sched), query_prototype(std::move(query_prototype)) {}

            template <class... with_components>
            [[nodiscard]] system_builder<params..., detail::query_with<with_components...>> with() && {
                return {sched, std::move(query_prototype).with<with_components...>()};
            }

            template <class... without_components>
            [[nodiscard]] system_builder<params..., detail::query_without<without_components...>> without() && {
                return {sched, std::move(query_prototype).without<without_components...>()};
            }

            template <class with_tag_component>
            [[nodiscard]] query<params...> with_tag(int32_t value) && {
                return {sched, std::move(query_prototype).with_tag<with_tag_component>(value)};
            }

            template <class with_tag_component>
            [[nodiscard]] query<params...> with_tag(uint32_t value) && {
                return {sched, std::move(query_prototype).with_tag<with_tag_component>(value)};
            }

            template <detail::tag_component with_tag_component>
            [[nodiscard]] query<params...> with_tag(const with_tag_component& value) && {
                return {sched, std::move(query_prototype).with_tag(value)};
            }

            template <class without_tag_component>
            [[nodiscard]] query<params...> without_tag(int32_t value) && {
                return {sched, std::move(query_prototype).without_tag<without_tag_component>(value)};
            }

            template <class without_tag_component>
            [[nodiscard]] query<params...> without_tag(uint32_t value) && {
                return {sched, std::move(query_prototype).without_tag<without_tag_component>(value)};
            }

            template <detail::tag_component without_tag_component>
            [[nodiscard]] query<params...> without_tag(const without_tag_component& value) && {
                return {sched, std::move(query_prototype).without_tag(value)};
            }

            template <class... reads_components>
            [[nodiscard]] system_builder<params..., detail::query_reads<reads_components...>> reads() && {
                return {sched, std::move(query_prototype).reads<reads_components...>()};
            }

            template <class... writes_components>
            [[nodiscard]] system_builder<params..., detail::query_writes<writes_components...>> writes() && {
                return {sched, std::move(query_prototype).writes<writes_components...>()};
            }

            template <class... with_dirty_components>
            [[nodiscard]] system_builder<params..., detail::query_with_dirty<with_dirty_components...>> with_dirty() && {
                return {sched, std::move(query_prototype).with_dirty<with_dirty_components...>()};
            }

            template <class... with_clear_components>
            [[nodiscard]] system_builder<params..., detail::query_with_clear<with_clear_components...>> with_clear() && {
                return {sched, std::move(query_prototype).with_clear<with_clear_components...>()};
            }

            template <class... with_changed_components>
            [[nodiscard]] system_builder<params..., detail::query_with_changes<with_changed_components...>> with_changes() && {

                return {sched, std::move(query_prototype).with_changes<with_changed_components...>()};
            }

            template <class Func>
            void finish(std::string name, tick_phase phase, Func&& f) && {
                using Adapter = detail::lambda_system_adapter<std::decay_t<Func>, query<params...>>;

                auto ptr = std::make_unique<Adapter>(
                    std::forward<Func>(f),
                    std::move(query_prototype),
                    std::move(name)
                );

                const auto& info = detail::get_system_info<Adapter>();
                sched.add_system_impl(std::move(ptr), info, phase);
            }

        private:
            scheduler& sched;
            query<params...> query_prototype;
        };

        scheduler();
        ~scheduler();

        template <typename T>
        void add_system(tick_phase phase) {
            add_system_impl(std::make_unique<T>(), detail::get_system_info<T>(), phase);
        }

        [[nodiscard]] system_builder<> build() { //builds system to be used in lambda
            return system_builder<>{*this, {}};
        }

        // Called each tick to run all systems in parallel
        //  also builds the dependency graph if system added
        void execute_frame(world_local_registry& registry, tick_phase phase);

    private:
        void add_system_impl(std::unique_ptr<system_interface> system, const detail::system_info& info, tick_phase);
        struct scheduler_data;
        std::unique_ptr<scheduler_data> data;
    };
}

template <copper_server::util::CTS path>
struct ecs_nbt_path {
    static inline constexpr std::string_view value = []() { return custom_name.data; }();
};

#include <src/api/ecs/late_definition.hpp>

namespace std {
    template <>
    struct hash<copper_server::api::ecs::entity> {
        size_t operator()(const copper_server::api::ecs::entity& ent) const noexcept {
            return std::hash<int32_t>{}(ent.id) ^ (std::hash<uint32_t>{}(ent.generation) << 1);
        }
    };
}

//Small self doc:
//This ecs been created specifically for copper_server to archive some features:
// like tick_phase
// "flat" relations using components
// compile time query builder
// non pod components
//      (with some limitations:
//           std::is_constructible_v<T>
//           && std::is_nothrow_destructible_v<T>
//           && std::is_nothrow_move_constructible_v<T>
//           && std::is_nothrow_move_assignable_v<T>
//           && std::is_copy_assignable_v<T>
//      )
// integrated nbt de/serialization support
// integrated fast_task support
//
//
//The flat relation is dynamically calculated for each component that declares special member function:
//      void get_relations(api::ecs::relation_visitor& visitor);
//  to notify the weak childs about removed relation use optional member function:
//      void on_unlink(ecs::entity self, ecs::entity target_holder);
//
//To remove entity, just add com::dead_mark
// this component is used by the deletion_system to remove it's owned child entities
// for weak relations system calls on_unlink
// the deletion_system is automatically registered for all worlds and could not be removed

#endif /* SRC_API_ECS */
