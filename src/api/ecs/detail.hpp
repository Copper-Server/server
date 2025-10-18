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

namespace copper_server::api::ecs {
    using component_id = uint32_t;
    struct entity_recipe;
    struct entity;

    template <class... components>
    struct dependent {};

    enum class structural_changes {
        no_changes,
        modified,
        added,
        removed
    };

    namespace detail {
        struct component_type_info {
            using constructor_fn = void (*)(void* memory);
            using destructor_fn = void (*)(void* memory);
            using move_constructor_fn = void (*)(void* destination, void* source);
            using copy_assignation_fn = void (*)(void* destination, void* source);
            using move_fn = void (*)(void* destination, void* source);

            size_t size = 0;
            size_t alignment = 0;
            constructor_fn construct = nullptr;
            move_constructor_fn move_construct = nullptr;
            copy_assignation_fn copy_assign = nullptr;
            destructor_fn destroy = nullptr;
            move_fn move = nullptr;
        };

        struct system_info {
            std::type_info info;
            std::vector<component_id> write_dependencies; // Components the system writes
            std::vector<component_id> read_dependencies;  // Components the system only reads
        };

        extern std::atomic<component_id> next_component_id;
        extern std::vector<component_type_info> component_info_registry;
        extern fast_task::mutex registry_mutex;

        template <class T>
            requires std::is_constructible_v<T>
                     && std::is_nothrow_destructible_v<T>
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
                    component_info_registry[id] = {
                        .size = sizeof(T),
                        .alignment = alignof(T),
                        .construct = [](void* mem) { new (mem) T(); },
                        .move_construct = [](void* dest, void* src) { new (dest) T(std::move(*static_cast<T*>(src))); },
                        .copy_assign = [](void* dest, void* src) { *static_cast<T*>(dest) = *static_cast<T*>(src); },
                        .destroy = [](void* mem) { static_cast<T*>(mem)->~T(); },
                        .move = [](void* dest, void* src) { *static_cast<T*>(dest) = std::move(*static_cast<T*>(src)); }
                    };
                }
            }
            return id;
        }

        struct components_holder {
            std::vector<std::pair<component_id, void*>> components_reference;

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

        struct mutation_queue_item {
            int32_t entity_id;
            uint32_t generation;
            component_id component;
            std::vector<char> data; //if nullptr the component will be removed

            ~mutation_queue_item() noexcept {
                if (data.size())
                    detail::component_info_registry[component].destroy(data.data());
            }
        };

        void queue_command(mutation_queue_item&&);

        void* get_entity_component(int32_t id, uint32_t generation, component_id component_id);

        //this function moves the component to heap allocated buffer, accepts reference to the component.
        template <class component>
        void queue_set_entity_component(int32_t id, uint32_t generation, component_id component_id, component&& comp) {
            auto& info = component_info_registry.at(component_id);
            mutation_queue_item queue{id, generation, component_id};
            queue.data.resize(info.size);
            info.move_construct(queue.data.data(), &comp);
            queue_command(std::move(queue));
        }

        inline void queue_remove_entity_component(int32_t id, uint32_t generation, component_id component_id) {
            queue_command(mutation_queue_item{id, generation, component_id});
        }

        void queue_destroy_entity(int32_t id, uint32_t generation);
        void queue_mark_dirty(int32_t id, uint32_t generation, component_id component_id);
        bool queue_add_relation(component_id component_id, entity parent, entity child);
        bool queue_remove_relation(component_id component_id, entity parent, entity child);
        bool has_relation(component_id component_id, entity parent, entity child);
        bool has_entity_component(int32_t id, uint32_t generation, component_id component_id);
        std::optional<int32_t> get_entity_assigned_to_world(int32_t id, uint32_t generation);

        fast_task::future_ptr<entity> create_entity(std::optional<int32_t> world_id, std::unique_ptr<components_holder> components);
        fast_task::future_ptr<entity> create_entity(std::optional<int32_t> world_id, const entity_recipe& recipe, std::unique_ptr<components_holder> components);
        fast_task::future_ptr<std::optional<entity>> copy_entity(std::optional<int32_t> world_id, const api::ecs::entity& base_entity);
        fast_task::task_rw_mutex& immediate_lock();

        template <class... components>
        fast_task::future_ptr<entity> create_entity__cc(std::optional<int32_t> world_id, components&&... args);
        template <class... components>
        fast_task::future_ptr<entity> create_entity_r_cc(std::optional<int32_t> world_id, const entity_recipe& recipe, components&&... args);


        struct iteration_handle {
            struct iteration_data;
            struct relational_iteration_data;
            std::variant<iteration_data*, relational_iteration_data*, std::nullptr_t> data = nullptr;
            iteration_handle() = default;
            iteration_handle(const iteration_handle&) = delete;

            iteration_handle(iteration_handle&& other) noexcept;

            iteration_handle& operator=(const iteration_handle&) = delete;

            iteration_handle& operator=(iteration_handle&& other) noexcept {
                data = std::move(other.data);
            }

            ~iteration_handle();

            std::pair<size_t, void**> next(); //no op if data == nullptr, returns the chunk
            bool is_end() const;

            void mark_component_dirty(component_id, size_t index);
            bool is_entity_match(size_t current_index_in_chunk) const;
            std::pair<int32_t, uint32_t> get_current_entity(size_t current_index_in_chunk);
            structural_changes get_component_change_state(size_t entity_index, component_id cid);

            struct preserved_state {
                size_t archetype_index;
                size_t chunk_index;
                void mark_component_dirty(iteration_handle&, component_id, size_t index);
                bool is_entity_match(iteration_handle&, size_t current_index_in_chunk) const;
                std::pair<int32_t, uint32_t> get_current_entity(iteration_handle&, size_t current_index_in_chunk);
                structural_changes get_component_change_state(iteration_handle&, size_t entity_index, component_id cid);
            };

            preserved_state preserve_state();
        };

        iteration_handle iterate_components(
            int32_t world_id,
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<component_id> writes_components,
            std::span<component_id> with_dirty_components,
            std::span<component_id> with_clean_components,
            std::span<component_id> with_changes,
            std::span<std::pair<component_id, entity>> with_relation
        );

        iteration_handle iterate_components_global(
            std::span<component_id> components,
            std::span<component_id> with_components,
            std::span<component_id> without_components,
            std::span<component_id> writes_components,
            std::span<component_id> with_dirty_components,
            std::span<component_id> with_clean_components,
            std::span<component_id> with_changes,
            std::span<std::pair<component_id, entity>> with_relation
        );

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

        template <class... _>
        struct has_relation_query {};

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

        template <typename... Comps>
        struct process_single_param<has_relation_query<Comps...>> {
            using type = std::tuple<>;
        };

        template <class... Params>
        using build_meta_tuple = decltype(std::tuple_cat(
            std::declval<typename process_single_param<Params>::type>()...
        ));


        template <typename TMetaPair>
        struct map_meta_pair_to_iterator_element;

        template <typename T>
        struct map_meta_pair_to_iterator_element<std::pair<T, read_operation_query>> {
            using type = const T;
        };

        template <typename T>
        struct map_meta_pair_to_iterator_element<std::pair<T, write_operation_query>> {
            using type = T;
        };

        template <typename TMetaTuple>
        struct build_iterator_tuple_from_meta;

        template <typename... TMetaPairs>
        struct build_iterator_tuple_from_meta<std::tuple<TMetaPairs...>> {
            using type = std::tuple<typename map_meta_pair_to_iterator_element<TMetaPairs>::type...>;
        };


        // --- TMP utility to check for duplicate types in a tuple ---
        template <typename TTuple>
        struct has_duplicates_in_tuple;

        template <>
        struct has_duplicates_in_tuple<std::tuple<>> : std::false_type {};

        template <typename T, typename... TRest>
        struct has_duplicates_in_tuple<std::tuple<T, TRest...>> {
            static constexpr bool is_t_in_rest = (std::is_same_v<T, TRest> || ...);
            static constexpr bool value = is_t_in_rest || has_duplicates_in_tuple<std::tuple<TRest...>>::value;
        };

        // --- TMP utility to check if two tuples have any types in common ---
        template <typename TTuple1, typename TTuple2>
        struct are_tuples_disjoint;

        template <typename TTuple2>
        struct are_tuples_disjoint<std::tuple<>, TTuple2> : std::true_type {};

        template <typename T1, typename... TRest1, typename... T2>
        struct are_tuples_disjoint<std::tuple<T1, TRest1...>, std::tuple<T2...>> {
            static constexpr bool is_t1_in_tuple2 = (std::is_same_v<T1, T2> || ...);
            static constexpr bool value = !is_t1_in_tuple2 && are_tuples_disjoint<std::tuple<TRest1...>, std::tuple<T2...>>::value;
        };

        // --- TMP utility to extract component types from the meta-tuple based on access type ---
        template <typename TMetaTuple, typename TAccessFilter>
        struct extract_by_access;

        template <typename TAccessFilter>
        struct extract_by_access<std::tuple<>, TAccessFilter> {
            using type = std::tuple<>;
        };

        template <typename TComp, typename TAccess, typename... TRest, typename TAccessFilter>
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
                return handle.get_component_change_state(index);
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
                return handle.get_component_change_state(index);
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
                return handle.get_component_change_state(index);
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
                return handle.get_component_change_state(index);
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
                return state.get_component_change_state(index);
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
                return state.get_component_change_state(handle, index);
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

        //template <class... T, class... TArgs>
        //struct is_requires_shifting<has_relation_query<T...>, TArgs...> : std::true_type {};

        template <class T, class... TArgs>
        struct is_requires_shifting<T, TArgs...> : is_requires_shifting<TArgs...> {};

        template <class... TArgs>
        inline constexpr bool is_requires_shifting_v = is_requires_shifting<TArgs...>::value;

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
                [chunk, this]<size_t... Is>(std::index_sequence<Is...>) {
                    reinterpret_cast<void*&>(std::get<Is>(component_arrays)) = chunk[Is];
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
            }
        };

        template <class... Params>
        struct query_traits {
            using MetaTuple = detail::build_meta_tuple<Params...>;
            using ReadTypes = typename detail::extract_by_access<MetaTuple, detail::read_operation_query>::type;
            using WriteTypes = typename detail::extract_by_access<MetaTuple, detail::write_operation_query>::type;
            using WithoutTypes = typename detail::extract_by_access<MetaTuple, detail::filter_without>::type;
            using WithChangesTypes = typename detail::extract_by_access<MetaTuple, detail::filter_with_changes>::type;
            using IteratorTuple = typename detail::build_iterator_tuple_from_meta<MetaTuple>::type;

            static_assert(!detail::has_duplicates_in_tuple<ReadTypes>::value, "COMPILE ERROR: A component was requested for read-access multiple times in the same query.");

            static_assert(!detail::has_duplicates_in_tuple<WriteTypes>::value, "COMPILE ERROR: A component was requested for write-access multiple times in the same query.");

            static_assert(detail::are_tuples_disjoint<ReadTypes, WithoutTypes>::value, "COMPILE ERROR: A component was requested for read, but the component also been requested to be skipped. The query result would be always empty.");

            static_assert(detail::are_tuples_disjoint<WriteTypes, WithoutTypes>::value, "COMPILE ERROR: A component was requested for write, but the component also been requested to be skipped. The query result would be always empty.");

            static_assert(detail::are_tuples_disjoint<ReadTypes, WriteTypes>::value, "COMPILE ERROR: A component was requested for both read and write access. Use .writes() for mutable access.");

            static const std::vector<component_id>& get_all_component_ids() {
                static const std::vector<component_id> ids = compute_component_ids<read_operation_query, write_operation_query>();
                return ids;
            }

            static const std::vector<component_id>& get_with_changes_ids() {
                static const std::vector<component_id> ids = compute_component_ids<filter_with_changes>();
                return ids;
            }

            static const std::vector<component_id>& get_writes_ids() {
                static const std::vector<component_id> ids = compute_component_ids<write_operation_query>();
                return ids;
            }

            static const std::vector<component_id>& get_dirty_ids() {
                static const std::vector<component_id> ids = compute_component_ids<filter_with_dirty>();
                return ids;
            }

            static const std::vector<component_id>& get_clear_ids() {
                static const std::vector<component_id> ids = compute_component_ids<filter_with_clear>();
                return ids;
            }

            static const std::vector<component_id>& get_without_ids() {
                static const std::vector<component_id> ids = compute_component_ids<filter_without>();
                return ids;
            }

            static const std::vector<component_id>& get_with_ids() {
                static const std::vector<component_id> ids = compute_component_ids<filter_with>();
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

            template <typename... AccessFilters, typename T, typename Access>
            static void add_if_access_match(std::vector<component_id>& ids, std::pair<T, Access>) {
                if constexpr ((std::is_same_v<Access, AccessFilters> || ...)) {
                    ids.push_back(detail::get_component_id<T>());
                }
            }
        };
    }

    template <class T>
    struct mutable_component {
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
        friend struct entity;

        mutable_component(T* ptr, int32_t owner_entity, uint32_t generation)
            : component_ptr_(ptr), owner_entity_(owner_entity), generation(generation) {}

        void mark_dirty_if_valid() {
            if (component_ptr_)
                detail::queue_mark_dirty(owner_entity_, generation, detail::get_component_id<T>());
        }

        T* component_ptr_;
        int32_t owner_entity_;
        uint32_t generation;
    };
}
#endif /* SRC_API_ECS_DETAIL */
