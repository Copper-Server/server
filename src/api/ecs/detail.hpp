/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_ECS_DETAIL
#define SRC_API_ECS_DETAIL
#include <atomic>
#include <cstdint>
#include <library/fast_task/include/future.hpp>
#include <span>
#include <src/util/templates.hpp>
#include <type_traits>
#include <vector>

namespace copper_server::util {
    class nbt_read_stream;
    class nbt_write_stream;
}

namespace copper_server::api::ecs {
    using component_id = uint32_t;
    using whole_component_id = uint64_t;
    struct entity_recipe;
    struct entity;
    struct world;
    struct archetype;
    struct world_local_registry;

    enum class relation_type : uint8_t {
        strong,
        weak
    };

    enum class tick_phase {
        early_processing, //for cleanup or for other things before processing anything

        mobile_entity,
        block_entity,
    };

    template <class... components>
    struct dependent {};

    struct system_interface {
        using reads = dependent<>;
        using writes = dependent<>;

        virtual ~system_interface() noexcept = default;

        virtual void tick(world_local_registry& world) = 0;
    };

    struct relation_visitor {
        struct context_t {
            void (*on_unlink)(void*, ecs::entity self, ecs::entity target_holder);
            void* component;

            void make_unlink(ecs::entity self, ecs::entity target_holder) const;
        };

        std::move_only_function<void(ecs::entity target, relation_type type, context_t& context)> callback;
        context_t context;

        relation_visitor(std::move_only_function<void(ecs::entity target, relation_type type, context_t& context)>&& callback);

        relation_visitor(const relation_visitor&) = delete;
        relation_visitor(relation_visitor&&) = delete;
        relation_visitor& operator=(const relation_visitor&) = delete;
        relation_visitor& operator=(relation_visitor&&) = delete;

        void push(entity e, relation_type type);
    };


    enum class structural_changes {
        no_changes,
        modified,
        added,
        removed
    };

    namespace detail {
        void report_fault_destruction(const std::type_info&);
        void report_fault_destruction(const std::exception& ex, const std::type_info&);

        template <class T>
        concept has_relation_discovery = requires(T& it, relation_visitor& v) {
            it.get_relations(v);
        };

        template <class T>
        concept has_relation_unlink = requires(T& it, entity self, entity target) {
            it.on_unlink(self, target);
        };

        template <class T>
        concept tag_component = requires(const T& it, int32_t& check) {
            check = it.get_tag_id();
        };

        struct archetype_layout {
            std::vector<component_id> component_ids;
            std::unordered_map<component_id, uint32_t> component_index_map;
            std::vector<size_t> component_offsets;
        };

        struct component_type_info {
            using constructor_fn = void (*)(void* memory);
            using destructor_fn = void (*)(void* memory);
            using move_constructor_fn = void (*)(void* destination, void* source);
            using copy_assignation_fn = void (*)(void* destination, void* source);
            using move_fn = void (*)(void* destination, void* source);
            using reset_fn = void (*)(void* memory);

            using get_relations_fn = void (*)(void* memory, relation_visitor& visitor);
            using on_unlink_fn = void (*)(void* memory, entity self, entity target);

            size_t size = 0;
            size_t alignment = 0;
            constructor_fn construct = nullptr;
            move_constructor_fn move_construct = nullptr;
            copy_assignation_fn copy_assign = nullptr;
            destructor_fn destroy = nullptr;
            move_fn move = nullptr;
            reset_fn reset = nullptr;
            get_relations_fn get_flat_relations = nullptr;
            on_unlink_fn on_unlink = nullptr;
            bool is_trivial = false;
        };

        struct system_info {
            const std::type_info& info;
            std::vector<component_id> write_dependencies; // Components the system writes
            std::vector<component_id> read_dependencies;  // Components the system only reads
        };

        extern std::atomic<component_id> next_component_id;
        extern std::vector<component_type_info> component_info_registry;
        extern fast_task::mutex registry_mutex;

        template <class T>
            requires std::is_constructible_v<T>
                     && std::is_nothrow_move_constructible_v<T>
                     && std::is_nothrow_move_assignable_v<T>
                     && std::is_copy_assignable_v<T>
        component_id get_component_id() {
            static const component_id id = next_component_id++;
            if (id >= component_info_registry.size()) {
                std::lock_guard lock(registry_mutex);

                if (id >= component_info_registry.size())
                    component_info_registry.resize(id + 1);

                if (component_info_registry[id].size == 0) {
                    auto& info = component_info_registry[id] = {
                        .size = std::is_empty_v<T> ? sizeof(T) : 0,
                        .alignment = std::is_empty_v<T> ? alignof(T) : 1,
                        .construct = [](void* mem) { new (mem) T(); },
                        .move_construct = [](void* dest, void* src) { new (dest) T(std::move(*static_cast<T*>(src))); },
                        .copy_assign = [](void* dest, void* src) { *static_cast<T*>(dest) = *static_cast<T*>(src); },
                        .destroy = [](void* mem) {
                            if constexpr (std::is_trivially_destructible_v<T>) {
                                //ignore
                            } else if constexpr (std::is_nothrow_destructible_v<T>)
                                static_cast<T*>(mem)->~T();
                            else {
                                try {
                                    static_cast<T*>(mem)->~T();
                                } catch (...) {
                                    report_fault_destruction(typeid(T));
                                } catch (const std::exception& ex) {
                                    report_fault_destruction(ex, typeid(T));
                                }
                            } //
                        },
                        .move = [](void* dest, void* src) { *static_cast<T*>(dest) = std::move(*static_cast<T*>(src)); },
                        .reset = [](void* mem) { static_cast<T*>(mem)->~T(); new (mem) T(); },
                        .is_trivial = std::is_trivial_v<T>
                    };

                    if constexpr (has_relation_unlink<T>)
                        info.on_unlink = [](void* mem, entity self, entity target) {
                            static_cast<T*>(mem)->on_unlink(self, target);
                        };

                    if constexpr (has_relation_discovery<T>)
                        info.get_flat_relations = [](void* mem, relation_visitor& v) {
                            if constexpr (has_relation_unlink<T>) {
                                v.context.on_unlink = info.on_unlink;
                                v.context.component = mem;
                            } else
                                v.context.component = nullptr;

                            static_cast<T*>(mem)->get_flat_relations(v);
                        };
                }
            }
            return id;
        }

        struct components_holder {
            std::vector<std::pair<component_id, void*>> components_reference;
            std::vector<whole_component_id> tags;

            virtual ~components_holder() = default;
        };

        template <class T>
        struct extract_dependent_types {
            static auto create() {
                return std::vector<component_id>{};
            }
        };

        template <class... components>
        struct extract_dependent_types<dependent<components...>> {
            static auto create() {
                return std::vector<component_id>{get_component_id<components>()...};
            }
        };

        template <class T>
        concept has_write_depends = requires {
            typename T::writes;
        };

        template <class T>
        struct write_depends_extract {
            using create = std::conditional_t<has_write_depends<T>, extract_dependent_types<typename T::writes>, extract_dependent_types<void>>;
        };

        template <class T>
        concept has_read_depends = requires {
            typename T::reads;
        };

        template <class T>
        struct read_depends_extract {
            using create = std::conditional_t<has_read_depends<T>, extract_dependent_types<typename T::reads>, extract_dependent_types<void>>;
        };

        template <class T>
        const system_info& get_system_info() {
            static const system_info info = {
                .info = typeid(T),
                .write_dependencies = write_depends_extract<T>::create::create(),
                .read_dependencies = read_depends_extract<T>::create::create(),
            };
            return info;
        }

        template <class T>
        struct tag_mask {};

        template <class T>
        whole_component_id get_tag_entry(uint32_t id) {
            if (id > INT32_MAX)
                throw std::runtime_error("Tag entry id is too big. Expected uint31_t entry, got uint32_t.");
            return get_component_id<tag_mask<T>>() | (static_cast<uint64_t>(id) << 32) | (1ui64 << 63);
        }

        template <class T>
        whole_component_id get_tag_entry(int32_t id) {
            return get_tag_entry<T>(std::bit_cast<uint32_t>(id));
        }

        template <tag_component T>
        whole_component_id get_tag_entry(const T& value) {
            return get_tag_entry<T>(value.get_tag_id());
        }

        struct mutation_queue_item {
            uint32_t entity_id;
            uint32_t generation;
            component_id component;
            bool remove = false;
            std::vector<char> data;

            ~mutation_queue_item() noexcept {
                if (!remove && data.size())
                    detail::component_info_registry[component].destroy(data.data());
            }
        };

        void queue_command(mutation_queue_item&&);

        void* get_entity_component(uint32_t id, uint32_t generation, component_id component_id);

        //this is used for accelerating serialization/deserialization logic
        size_t get_entity_archetype_id(uint32_t id, uint32_t generation);
        //this is used for accelerating serialization/deserialization logic
        archetype_layout get_archetype_layout(uint32_t id, uint32_t generation);
        //this is used for accelerating serialization/deserialization logic
        void* get_entity_component_by_offset(uint32_t id, uint32_t generation, size_t offset, size_t component_size);

        //this function moves the component to heap allocated buffer, accepts reference to the component.
        template <class component>
        void queue_set_entity_component(uint32_t id, uint32_t generation, component_id component_id, component&& comp) {
            auto& info = component_info_registry.at(component_id);
            mutation_queue_item queue{id, generation, component_id};
            queue.data.resize(info.size);
            info.move_construct(queue.data.data(), &comp);
            queue_command(std::move(queue));
        }

        void queue_remove_entity_component(uint32_t id, uint32_t generation, component_id component_id);

        void queue_destroy_entity(uint32_t id, uint32_t generation);
        void queue_mark_dirty(uint32_t id, uint32_t generation, component_id component_id);
        bool has_entity_component(uint32_t id, uint32_t generation, whole_component_id component_id);
        bool is_valid(uint32_t id, uint32_t generation);
        std::optional<int32_t> get_entity_assigned_to_world(uint32_t id, uint32_t generation);

        void request_all_childs(uint32_t id, uint32_t generation, relation_visitor& visitor);

        fast_task::future_ptr<entity> create_entity(std::optional<world*> world_opt, std::unique_ptr<components_holder> components);
        fast_task::future_ptr<entity> create_entity(std::optional<world*> world_opt, const entity_recipe& recipe, std::unique_ptr<components_holder> components);
        fast_task::future_ptr<std::optional<entity>> copy_entity(std::optional<world*> world_opt, const api::ecs::entity& base_entity);

        entity load_ecs_entity(const std::string& named_id, util::nbt_read_stream& stream, std::optional<world*> world_opt);
        void store_ecs_entity(const std::string& named_id, util::nbt_write_stream& stream, entity);

        template <class... components>
        fast_task::future_ptr<entity> create_entity__cc(std::optional<world*> world_opt, components&&... args);
        template <class... components>
        fast_task::future_ptr<entity> create_entity_r_cc(std::optional<world*> world_opt, const entity_recipe& recipe, components&&... args);


        int32_t get_world_id(world*);
        int32_t get_world_id(world_local_registry&);
        world* get_world_by_id(int32_t);
        world* register_world(int32_t);
        void unregister_world(world*);

        size_t get_state_version(std::optional<int32_t> world_id);

        struct iteration_topology {
            struct arch_data_t {
                archetype* type;
                std::vector<size_t> required_layout_offsets;
                std::vector<uint32_t> required_clean_comp_indices; // Use correct type
                std::vector<uint32_t> required_dirty_comp_indices; // Use correct type
                std::vector<uint32_t> make_dirty_comp_indices;     // Use correct type

                arch_data_t(archetype* type) : type(type) {}
            };

            std::vector<arch_data_t> arch_data;
            std::vector<component_id> with_changes;

            void calculate_data(
                std::span<component_id> components,
                std::span<component_id> clean_components,
                std::span<component_id> dirty_components,
                std::span<component_id> mark_dirty_components,
                std::span<component_id> with_changes_
            );

            void mark_component_dirty(size_t archetype_index, size_t chunk_index, component_id component, size_t entity_index);
            bool is_entity_match(size_t archetype_index, size_t chunk_index, size_t entity_index) const;
            std::pair<uint32_t, uint32_t> get_current_entity(size_t archetype_index, size_t chunk_index, size_t entity_index);
            structural_changes get_component_change_state(size_t archetype_index, size_t chunk_index, size_t entity_index, component_id cid);
        };

        //transparent handle
        struct iteration_handle {
            struct iteration_data;
            std::shared_ptr<iteration_topology> topology;
            std::unique_ptr<iteration_data> data;

            iteration_handle() = default;
            iteration_handle(const std::shared_ptr<iteration_topology>& topology);
            iteration_handle(const iteration_handle&) = delete;
            iteration_handle(iteration_handle&& other) noexcept;
            ~iteration_handle();

            iteration_handle& operator=(const iteration_handle&) = delete;
            iteration_handle& operator=(iteration_handle&& other) noexcept;

            std::pair<size_t, void**> next(); //no op if data == nullptr, returns the chunk
            bool is_end() const;

            void mark_component_dirty(component_id, size_t index);
            bool is_entity_match(size_t current_index_in_chunk) const;
            std::pair<uint32_t, uint32_t> get_current_entity(size_t current_index_in_chunk);
            structural_changes get_component_change_state(size_t entity_index, component_id cid);

            struct preserved_state {
                size_t archetype_index;
                size_t chunk_index;
                void mark_component_dirty(iteration_handle&, component_id, size_t index);
                bool is_entity_match(iteration_handle&, size_t current_index_in_chunk) const;
                std::pair<uint32_t, uint32_t> get_current_entity(iteration_handle&, size_t current_index_in_chunk);
                structural_changes get_component_change_state(iteration_handle&, size_t entity_index, component_id cid);
            };

            preserved_state preserve_state();
        };

        std::shared_ptr<iteration_topology> iterate_components(
            int32_t world_id,
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<component_id> writes_components,
            std::span<component_id> with_dirty_components,
            std::span<component_id> with_clean_components,
            std::span<component_id> with_changes,
            std::span<whole_component_id> with_tag_components,
            std::span<whole_component_id> without_tag_components
        );

        std::shared_ptr<iteration_topology> iterate_components_global(
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<component_id> writes_components,
            std::span<component_id> with_dirty_components,
            std::span<component_id> with_clean_components,
            std::span<component_id> with_changes,
            std::span<whole_component_id> with_tag_components,
            std::span<whole_component_id> without_tag_components
        );

        iteration_handle make_handle(const std::shared_ptr<iteration_topology>& topology, size_t component_count);

        template <class... components>
        struct query_reads {};

        template <class... components>
        struct query_writes {};

        template <class... components>
        struct query_with_dirty {};

        template <class... components>
        struct query_with_clear {};

        template <class... components>
        struct query_without {};

        template <class... components>
        struct query_with {};

        template <class... components>
        struct query_with_changes {};

        struct read_operation_query {};

        struct write_operation_query {};

        struct filter_with_changes {};

        struct filter_with_dirty {};

        struct filter_with_clear {};

        struct filter_without {};

        struct filter_with {};

        template <typename T>
        using strip_const_t = std::remove_const_t<T>;

        template <typename TQueryParam>
        struct process_single_param;

        template <typename... Comps>
        struct process_single_param<query_reads<Comps...>> {
            using type = std::tuple<std::pair<strip_const_t<Comps>, read_operation_query>...>;
        };

        template <typename... Comps>
        struct process_single_param<query_writes<Comps...>> {
            using type = std::tuple<std::pair<strip_const_t<Comps>, write_operation_query>...>;
        };

        template <typename... Comps>
        struct process_single_param<query_with_dirty<Comps...>> {
            using type = std::tuple<std::pair<strip_const_t<Comps>, filter_with_dirty>...>;
        };

        template <typename... Comps>
        struct process_single_param<query_with_clear<Comps...>> {
            using type = std::tuple<std::pair<strip_const_t<Comps>, filter_with_clear>...>;
        };

        template <typename... Comps>
        struct process_single_param<query_without<Comps...>> {
            using type = std::tuple<std::pair<strip_const_t<Comps>, filter_without>...>;
        };

        template <typename... Comps>
        struct process_single_param<query_with<Comps...>> {
            using type = std::tuple<std::pair<strip_const_t<Comps>, filter_with>...>;
        };

        template <typename... Comps>
        struct process_single_param<query_with_changes<Comps...>> {
            using type = std::tuple<std::pair<strip_const_t<Comps>, filter_with_changes>...>;
        };

        template <class... Params>
        using build_meta_tuple = decltype(std::tuple_cat(
            std::declval<typename process_single_param<Params>::type>()...
        ));

        template <class TQueryParam>
        struct map_meta_pair_to_iterator_element {
            using type = std::tuple<>;
        };

        template <class... Comps>
        struct map_meta_pair_to_iterator_element<query_reads<Comps...>> {
            using type = std::tuple<std::add_const_t<Comps>...>;
        };

        template <class... Comps>
        struct map_meta_pair_to_iterator_element<query_writes<Comps...>> {
            using type = std::tuple<Comps...>;
        };

        template <class... Params>
        using build_iterator_tuple_from_meta = decltype(std::tuple_cat(
            std::declval<typename map_meta_pair_to_iterator_element<Params>::type>()...
        ));


        // --- TMP utility to check for duplicate types in a tuple ---
        template <class TTuple>
        struct has_duplicates_in_tuple;

        template <>
        struct has_duplicates_in_tuple<std::tuple<>> : std::false_type {};

        template <class T, class... TRest>
        struct has_duplicates_in_tuple<std::tuple<T, TRest...>> {
            static constexpr bool is_t_in_rest = (std::is_same_v<T, TRest> || ...);
            static constexpr bool value = is_t_in_rest || has_duplicates_in_tuple<std::tuple<TRest...>>::value;
        };

        // --- TMP utility to check if two tuples have any types in common ---
        template <class TTuple1, class TTuple2>
        struct are_tuples_disjoint;

        template <class TTuple2>
        struct are_tuples_disjoint<std::tuple<>, TTuple2> : std::true_type {};

        template <class T1, class... TRest1, class... T2>
        struct are_tuples_disjoint<std::tuple<T1, TRest1...>, std::tuple<T2...>> {
            static constexpr bool is_t1_in_tuple2 = (std::is_same_v<T1, T2> || ...);
            static constexpr bool value = !is_t1_in_tuple2 && are_tuples_disjoint<std::tuple<TRest1...>, std::tuple<T2...>>::value;
        };

        // --- TMP utility to extract component types from the meta-tuple based on access type ---
        template <class TMetaTuple, class TAccessFilter>
        struct extract_by_access;

        template <class TAccessFilter>
        struct extract_by_access<std::tuple<>, TAccessFilter> {
            using type = std::tuple<>;
        };

        template <class TComp, class TAccess, class... TRest, class TAccessFilter>
        struct extract_by_access<std::tuple<std::pair<TComp, TAccess>, TRest...>, TAccessFilter> {
            using rest_tuple = typename extract_by_access<std::tuple<TRest...>, TAccessFilter>::type;

            using type = std::conditional_t<
                std::is_same_v<TAccess, TAccessFilter>,
                decltype(std::tuple_cat(std::declval<std::tuple<TComp>>(), std::declval<rest_tuple>())),
                rest_tuple>;
        };

        struct iterator_view {
            iterator_view(iteration_handle& handle, size_t index) : handle(handle), index(index) {}

            ~iterator_view() = default;

            entity current_entity();

            template <class component>
            structural_changes get_change_state() {
                return handle.get_component_change_state(index, detail::get_component_id<component>());
            }

        private:
            iteration_handle& handle;
            size_t index;
        };

        template <class... written_components>
        struct iterator_view_dirty_mark {
            using written = std::tuple<written_components...>;

            iterator_view_dirty_mark(iteration_handle& handle, size_t index) : handle(handle), index(index) {}

            ~iterator_view_dirty_mark() = default;

            template <class component>
                requires((std::is_same_v<written_components, component> || ...))
            void mark_dirty() {
                handle.mark_component_dirty(detail::get_component_id<component>(), index);
            }

            entity current_entity();

            template <class component>
            structural_changes get_change_state() {
                return handle.get_component_change_state(index, detail::get_component_id<component>());
            }

        private:
            iteration_handle& handle;
            size_t index;
        };

        struct iterator_view_chunk {
            iterator_view_chunk(iteration_handle& handle) : handle(handle) {}

            ~iterator_view_chunk() = default;

            entity current_entity(size_t index);

            template <class component>
            structural_changes get_change_state(size_t index) {
                return handle.get_component_change_state(index, detail::get_component_id<component>());
            }

        private:
            iteration_handle& handle;
        };

        template <class... written_components>
        struct iterator_view_chunk_dirty_mark {
            using written = std::tuple<written_components...>;

            iterator_view_chunk_dirty_mark(iteration_handle& handle) : handle(handle) {}

            ~iterator_view_chunk_dirty_mark() = default;

            template <class component>
                requires((std::is_same_v<written_components, component> || ...))
            void mark_dirty(size_t index) {
                handle.mark_component_dirty(detail::get_component_id<component>(), index);
            }

            entity current_entity(size_t index);

            template <class component>
            structural_changes get_change_state(size_t index) {
                return handle.get_component_change_state(index, detail::get_component_id<component>());
            }

        private:
            iteration_handle& handle;
        };

        struct iterator_view_chunk_parallel {
            iterator_view_chunk_parallel(iteration_handle& handle) : handle(handle), state(handle.preserve_state()) {}

            ~iterator_view_chunk_parallel() = default;

            entity current_entity(size_t index);

            template <class component>
            structural_changes get_change_state(size_t index) {
                return state.get_component_change_state(index, detail::get_component_id<component>());
            }

        private:
            iteration_handle& handle;
            iteration_handle::preserved_state state;
        };

        template <class... written_components>
        struct iterator_view_chunk_parallel_dirty_mark {
            using written = std::tuple<written_components...>;

            iterator_view_chunk_parallel_dirty_mark(iteration_handle& handle) : handle(handle), state(handle.preserve_state()) {}

            ~iterator_view_chunk_parallel_dirty_mark() = default;

            template <class component>
                requires((std::is_same_v<written_components, component> || ...))
            void mark_dirty(size_t index) {
                state.mark_component_dirty(handle, detail::get_component_id<component>(), index);
            }

            entity current_entity(size_t index);

            template <class component>
            structural_changes get_change_state(size_t index) {
                return state.get_component_change_state(handle, index, detail::get_component_id<component>());
            }

        private:
            iteration_handle& handle;
            iteration_handle::preserved_state state;
        };

        template <template <bool, class, class...> class T, bool requires_shifting, class iterator_viewer, class Tuple>
        struct apply_tuple_to_iter;

        template <template <bool, class, class...> class T, bool requires_shifting, class iterator_viewer, class... Args>
        struct apply_tuple_to_iter<T, requires_shifting, iterator_viewer, std::tuple<Args...>> {
            using type = T<requires_shifting, iterator_viewer, Args...>;
        };

        template <class...>
        struct is_requires_shifting : std::false_type {};

        template <class... T, class... TArgs>
        struct is_requires_shifting<query_with_dirty<T...>, TArgs...> : std::true_type {};

        template <class... T, class... TArgs>
        struct is_requires_shifting<query_with_clear<T...>, TArgs...> : std::true_type {};

        template <class... T, class... TArgs>
        struct is_requires_shifting<query_with_changes<T...>, TArgs...> : std::true_type {};

        template <class T, class... TArgs>
        struct is_requires_shifting<T, TArgs...> : is_requires_shifting<TArgs...> {};

        template <class... TArgs>
        inline constexpr bool is_requires_shifting_v = is_requires_shifting<TArgs...>::value;

        template <class... T>
        dependent<T...> to_deps_tuple_help(std::tuple<T...>&& t) {
            return {};
        }

        template <class T>
        using to_deps = decltype(to_deps_tuple_help(std::declval<T>()));


        template <bool requires_shifting, class iterator_viewer, class... components>
        struct query_iterator {
            using value_type = std::tuple<iterator_viewer, components&...>;

            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using pointer = value_type*;
            using reference = value_type;

            std::tuple<components*...> component_arrays;
            size_t current_index_in_chunk = 0;
            size_t max_chunk_size = 0;
            iteration_handle handle;


            query_iterator() = default;

            query_iterator(iteration_handle&& handle) : handle(std::move(handle)) {
                operator++();
            }

            query_iterator(const query_iterator&) = delete;
            query_iterator& operator=(const query_iterator&) = delete;

            query_iterator(query_iterator&& other) noexcept : handle(std::move(other.handle)), component_arrays(std::move(other.component_arrays)), current_index_in_chunk(other.current_index_in_chunk), max_chunk_size(other.max_chunk_size) {
                other.current_index_in_chunk = 0;
                other.max_chunk_size = 0;
            }

            query_iterator& operator=(query_iterator&& other) noexcept {
                handle = std::move(other.handle);
                component_arrays = std::move(other.component_arrays);
                current_index_in_chunk = other.current_index_in_chunk;
                max_chunk_size = other.max_chunk_size;
                other.current_index_in_chunk = 0;
                other.max_chunk_size = 0;
            }

            query_iterator& operator++() {
                if constexpr (requires_shifting)
                    sifting_increment();
                else
                    fast_increment();
                return *this;
            }

            bool operator==(const query_iterator& other) const {
                return handle.is_end() == other.handle.is_end();
            }

            bool operator!=(const query_iterator& other) const {
                return !(*this == other);
            }

            reference operator*() {
                return std::tuple_cat(
                    std::make_tuple(iterator_viewer{handle, current_index_in_chunk}),
                    std::tie((*(std::get<components*>(component_arrays) + current_index_in_chunk))...)
                );
            }

            template <class FN>
            void chunk_iterate(FN&& fn) {
                query_iterator end;
                while (*this != end) {
                    std::apply(fn, std::tuple_cat(component_arrays, std::make_tuple(max_chunk_size)));
                    apply_next();
                }
            }

            template <class FN>
            void chunk_iterate_view(FN&& fn) {
                query_iterator end;
                while (*this != end) {
                    if constexpr (std::is_same_v<iterator_view, iterator_viewer>())
                        std::apply(fn, std::tuple_cat(std::make_tuple(iterator_view{*this}), component_arrays, std::make_tuple(max_chunk_size)));
                    else
                        std::apply(fn, std::tuple_cat(std::make_tuple(typename util::apply_tuple_to<iterator_view_chunk_dirty_mark, typename iterator_viewer::written>::type{*this}), component_arrays, std::make_tuple(max_chunk_size)));
                    apply_next();
                }
            }

            template <class FN>
            void chunk_iterate_parallel(FN&& fn) {
                std::vector<fast_task::cancelable_future_ptr<void>> futures;
                query_iterator end;
                while (*this != end) {
                    futures.push_back(fast_task::cancelable_future<void>::start([component_arrays, max_chunk_size, &fn]() { std::apply(fn, std::tuple_cat(component_arrays, std::make_tuple(max_chunk_size))); }));
                    apply_next();
                }

                try {
                    for (auto& future_ : futures)
                        future_->wait();
                } catch (...) {
                    for (auto& future_ : futures)
                        future_->cancel();
                    throw;
                }
            }

            template <class FN>
            void chunk_iterate_parallel_view(FN&& fn) {
                std::vector<fast_task::cancelable_future_ptr<void>> futures;
                query_iterator end;
                while (*this != end) {
                    if constexpr (std::is_same_v<iterator_view, iterator_viewer>())
                        futures.push_back(fast_task::cancelable_future<void>::start([view = iterator_view_chunk_parallel{*this}, max_chunk_size, component_arrays, &fn]() mutable {
                            std::apply(fn, std::tuple_cat(std::make_tuple(std::move(view)), component_arrays, std::make_tuple(max_chunk_size)));
                        }));
                    else
                        futures.push_back(fast_task::cancelable_future<void>::start([view = typename util::apply_tuple_to<iterator_view_chunk_parallel_dirty_mark, typename iterator_viewer::written>::type{*this}, max_chunk_size, component_arrays, &fn]() mutable {
                            std::apply(fn, std::tuple_cat(std::make_tuple(std::move(view)), component_arrays, std::make_tuple(max_chunk_size)));
                        }));
                    apply_next();
                }

                try {
                    for (auto& future_ : futures)
                        future_->wait();
                } catch (...) {
                    for (auto& future_ : futures)
                        future_->cancel();
                    throw;
                }
            }

        private:
            void apply_next() {
                auto [chunk_size, chunk] = handle.next();
                max_chunk_size = chunk_size;
                current_index_in_chunk = 0;
                if (chunk)
                    [chunk, this]<size_t... Is>(std::index_sequence<Is...>) {
                        ((std::get<Is>(component_arrays) = static_cast<std::tuple_element_t<Is, decltype(component_arrays)>>(chunk[Is])), ...);
                    }(std::make_index_sequence<sizeof...(components)>{});
            }

            void fast_increment() {
                if (current_index_in_chunk < max_chunk_size)
                    current_index_in_chunk++;
                else
                    apply_next();
            }

            void sifting_increment() {
                while (max_chunk_size > 0) {
                    for (size_t i = current_index_in_chunk + 1; i < max_chunk_size; ++i) {
                        if (handle.is_entity_match(i)) {
                            current_index_in_chunk = i;
                            return;
                        }
                    }
                    apply_next();
                }
                if (max_chunk_size == 0)
                    apply_next();
            }
        };

        template <class... Params>
        struct query_traits {
            using MetaTuple = detail::build_meta_tuple<Params...>;
            using ReadTypes = typename detail::extract_by_access<MetaTuple, detail::read_operation_query>::type;
            using WriteTypes = typename detail::extract_by_access<MetaTuple, detail::write_operation_query>::type;
            using WithoutTypes = typename detail::extract_by_access<MetaTuple, detail::filter_without>::type;
            using WithChangesTypes = typename detail::extract_by_access<MetaTuple, detail::filter_with_changes>::type;
            using IteratorTuple = detail::build_iterator_tuple_from_meta<Params...>;

            static_assert(!detail::has_duplicates_in_tuple<ReadTypes>::value, "COMPILE ERROR: A component was requested for read-access multiple times in the same query.");

            static_assert(!detail::has_duplicates_in_tuple<WriteTypes>::value, "COMPILE ERROR: A component was requested for write-access multiple times in the same query.");

            static_assert(detail::are_tuples_disjoint<ReadTypes, WithoutTypes>::value, "COMPILE ERROR: A component was requested for read, but the component also been requested to be skipped. The query result would be always empty.");

            static_assert(detail::are_tuples_disjoint<WriteTypes, WithoutTypes>::value, "COMPILE ERROR: A component was requested for write, but the component also been requested to be skipped. The query result would be always empty.");

            static_assert(detail::are_tuples_disjoint<ReadTypes, WriteTypes>::value, "COMPILE ERROR: A component was requested for both read and write access. Use .writes() for mutable access.");

            static std::span<component_id> get_all_component_ids() {
                static std::vector<component_id> ids = compute_component_ids<read_operation_query, write_operation_query>();
                return ids;
            }

            static constexpr size_t get_all_component_count() {
                return compute_component_count<read_operation_query, write_operation_query>();
            }

            static std::span<component_id> get_with_changes_ids() {
                static std::vector<component_id> ids = compute_component_ids<filter_with_changes>();
                return ids;
            }

            static std::span<component_id> get_writes_ids() {
                static std::vector<component_id> ids = compute_component_ids<write_operation_query>();
                return ids;
            }

            static std::span<component_id> get_dirty_ids() {
                static std::vector<component_id> ids = compute_component_ids<filter_with_dirty>();
                return ids;
            }

            static std::span<component_id> get_clear_ids() {
                static std::vector<component_id> ids = compute_component_ids<filter_with_clear>();
                return ids;
            }

            static std::span<component_id> get_without_ids() {
                static std::vector<component_id> ids = compute_component_ids<filter_without>();
                return ids;
            }

            static std::span<component_id> get_with_ids() {
                static std::vector<component_id> ids = compute_component_ids<filter_with>();
                return ids;
            }

        private:
            template <typename... AccessFilters>
            static std::vector<component_id> compute_component_ids() {
                std::vector<component_id> result_ids;
                auto filter_op = [&](auto... pairs) {
                    (add_if_access_match<AccessFilters...>(result_ids, pairs), ...);
                };
                std::apply(filter_op, MetaTuple{});
                return result_ids;
            }

            template <typename... AccessFilters>
            static constexpr size_t compute_component_count() {
                size_t count = 0;
                auto filter_op = [&](auto... pairs) {
                    (inc_if_access_match<AccessFilters...>(count, pairs), ...);
                };
                std::apply(filter_op, MetaTuple{});
                return count;
            }

            template <typename... AccessFilters, typename T, typename Access>
            static void add_if_access_match(std::vector<component_id>& ids, std::pair<T, Access>) {
                if constexpr ((std::is_same_v<Access, AccessFilters> || ...))
                    ids.push_back(detail::get_component_id<T>());
            }

            template <typename... AccessFilters, typename T, typename Access>
            static void inc_if_access_match(size_t& count, std::pair<T, Access>) {
                if constexpr ((std::is_same_v<Access, AccessFilters> || ...))
                    ++count;
            }
        };

        template <class Func, class Query>
        class lambda_system_adapter : public system_interface {
        public:
            using traits = query_traits<Query>;
            using reads = to_deps<typename traits::ReadTypes>;
            using writes = to_deps<typename traits::WriteTypes>;

        private:
            Func function;
            Query query_prototype;
            std::string name;

        public:
            lambda_system_adapter(Func&& f, std::string n)
                : function(std::move(f)), name(std::move(n)) {}

            virtual ~lambda_system_adapter() {}

            void tick(world_local_registry& world) override {
                Query local_query(query_prototype);
                local_query.set_world_id(get_world_id(world));

                if constexpr (std::is_invocable_r_v<void, Func, Query&>)
                    function(local_query);
                else {
                    auto task = function(local_query);
                    if constexpr (requires { task.await_task(); })
                        task.await_task();
                    else
                        task->await_task();
                }
            }
        };

#pragma region inline_system

        template <class T>
        struct function_traits : function_traits<decltype(&T::operator())> {};

        template <class C, class R, class... Args>
        struct function_traits<R (C::*)(Args...) const> {
            using args_tuple = std::tuple<Args...>;
            static constexpr size_t arity = sizeof...(Args);
        };

        template <class C, class R, class... Args>
        struct function_traits<R (C::*)(Args...)> {
            using args_tuple = std::tuple<Args...>;
            static constexpr size_t arity = sizeof...(Args);
        };

        template <class T>
        struct map_arg_to_query;

        template <class T>
        struct map_arg_to_query {
            using type = std::tuple<>;
        };

        template <class T>
        struct map_arg_to_query<const T&> {
            using type = std::tuple<query_reads<T>>;
        };

        template <class T>
        struct map_arg_to_query<T&> {
            using type = std::tuple<query_writes<T>>;
        };

        template <>
        struct map_arg_to_query<detail::iterator_view> {
            using type = std::tuple<>;
        };

        template <>
        struct map_arg_to_query<const detail::iterator_view&> {
            using type = std::tuple<>;
        };

        template <class... T>
        struct map_arg_to_query<detail::iterator_view_dirty_mark<T...>> {
            using type = std::tuple<>;
        };

        template <class Tuple>
        struct deduce_query_params;

        template <class... Args>
        struct deduce_query_params<std::tuple<Args...>> {
            using meta_tuple = decltype(std::tuple_cat(typename map_arg_to_query<Args>::type{}...));

            template <class T, template <class...> class wrap_in>
            struct make_wrap;

            template <template <class...> class wrap_in, class... Params>
            struct make_wrap<std::tuple<Params...>, wrap_in> {
                using type = wrap_in<Params...>;
            };


            template <template <class...> class wrap_in>
            using type = typename make_wrap<meta_tuple, wrap_in>::type;
        };

#pragma endregion

#pragma region parallel inline system

        template <typename T>
        struct is_view_type : std::false_type {};

        template <>
        struct is_view_type<detail::iterator_view> : std::true_type {};

        template <>
        struct is_view_type<const detail::iterator_view&> : std::true_type {};

        template <typename ChunkView>
        struct parallel_view_proxy {
            ChunkView& chunk_view;
            size_t index;

            auto current_entity() {
                return chunk_view.current_entity(index);
            }

            template <class Component>
            structural_changes get_change_state() {
                return chunk_view.template get_change_state<Component>(index);
            }

            // Proxy dirty marking if available
            template <class Component>
            void mark_dirty()
                requires requires { chunk_view.template mark_dirty<Component>(index); }
            {
                chunk_view.template mark_dirty<Component>(index);
            }
        };

#pragma endregion
    }

    template <class T>
    struct mutable_component {
        mutable_component(const mutable_component&) = delete;
        mutable_component& operator=(const mutable_component&) = delete;

        mutable_component(mutable_component&& other) noexcept
            : component_ptr_(other.component_ptr_), owner_entity_(other.owner_entity_), generation(other.generation) {
            other.component_ptr_ = nullptr;
        }

        mutable_component& operator=(mutable_component&& other) noexcept {
            if (this != &other) {
                mark_dirty_if_valid();
                component_ptr_ = other.component_ptr_;
                owner_entity_ = other.owner_entity_;
                generation = other.generation;
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
        friend struct entity;

        mutable_component(T* ptr, uint32_t owner_entity, uint32_t generation)
            : component_ptr_(ptr), owner_entity_(owner_entity), generation(generation) {}

        void mark_dirty_if_valid() {
            if (component_ptr_)
                detail::queue_mark_dirty(owner_entity_, generation, detail::get_component_id<T>());
        }

        T* component_ptr_;
        uint32_t owner_entity_;
        uint32_t generation;
    };
}
#endif /* SRC_API_ECS_DETAIL */
