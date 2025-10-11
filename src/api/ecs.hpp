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
#include <atomic>
#include <cstdint>
#include <functional>
#include <library/fast_task.hpp>
#include <type_traits>
#include <vector>

//entity component system
namespace copper_server::api::ecs {
    using component_id = uint32_t;

    template <class... components>
    struct dependent {};

    namespace detail {
        struct component_type_info {
            using constructor_fn = void (*)(void* memory);
            using destructor_fn = void (*)(void* memory);
            using move_constructor_fn = void (*)(void* destination, void* source);
            using move_fn = void (*)(void* destination, void* source);

            size_t size = 0;
            size_t alignment = 0;
            constructor_fn construct = nullptr;
            move_constructor_fn move_construct = nullptr;
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
            requires std::is_constructible_v<T> && std::is_nothrow_destructible_v<T> && std::is_nothrow_move_constructible_v<T>
        component_id get_component_id() {
            static const component_id id = next_component_id++;
            if (id >= component_info_registry.size() || component_info_registry[id].size == 0) {
                std::lock_guard<std::mutex> lock(registry_mutex);

                if (id >= component_info_registry.size())
                    component_info_registry.resize(id + 1);

                if (component_info_registry[id].size == 0) {
                    component_info_registry[id] = {
                        .size = sizeof(T),
                        .alignment = alignof(T),
                        .construct = [](void* mem) { new (mem) T(); },
                        .move_construct = [](void* dest, void* src) { new (dest) T(std::move(*static_cast<T*>(src))); },
                        .destroy = [](void* mem) { static_cast<T*>(mem)->~T(); },
                        .move = [](void* dest, void* src) {
                            new (dest) T(std::move(*static_cast<T*>(src)));
                            static_cast<T*>(src)->~T(); //
                        }
                    };
                }
            }
            return id;
        }

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
            using create = extract_dependent_types<void>;
        };

        template <has_write_depends T>
        struct write_depends_extract {
            using create = extract_dependent_types<typename T::writes>;
        };

        template <class T>
        concept has_read_depends = requires {
            typename T::reads;
        };

        template <class T>
        struct read_depends_extract {
            using create = extract_dependent_types<void>;
        };

        template <has_read_depends T>
        struct read_depends_extract {
            using create = extract_dependent_types<typename T::reads>;
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

        void* get_entity_component(int32_t id, uint32_t generation, component_id component_id);
        void queue_set_entity_component(int32_t id, uint32_t generation, component_id component_id, void* component);
        void queue_remove_entity_component(int32_t id, uint32_t generation, component_id component_id);
        void queue_destroy_entity(int32_t id, uint32_t generation);
        bool has_entity_component(int32_t id, uint32_t generation, component_id component_id);

        struct iteration_handle {
            struct iteration_data;
            iteration_data* data = nullptr; //if data == nullptr the iteration reached the end
            iteration_handle() = default;
            iteration_handle(const iteration_handle&) = delete;

            iteration_handle(iteration_handle&& other) noexcept : data(other.data) {
                other.data = nullptr;
            }

            iteration_handle& operator=(const iteration_handle&) = delete;

            iteration_handle& operator=(iteration_handle&& other) noexcept {
                data = other.data;
                other.data = nullptr;
            }

            ~iteration_handle();

            std::pair<size_t, void**> next(); //no op if data == nullptr, returns the chunk
        };

        iteration_handle iterate_components(int32_t world_id, component_id* components, size_t components_size, component_id* without_components, size_t without_components_size);
        iteration_handle iterate_components_global(component_id* components, size_t components_size, component_id* without_components, size_t without_components_size);
    }

    struct entity_recipe {
        entity_recipe& with(entity_recipe& id) {
            if (!_is_frozen)
                component_ids.insert(component_ids.end(), id.component_ids.begin(), id.component_ids.end());
            return *this;
        }

        entity_recipe& with(component_id id) {
            if (!_is_frozen)
                component_ids.push_back(id);
            return *this;
        }

        template <class component>
        entity_recipe& with() {
            if (!_is_frozen)
                component_ids.push_back(detail::get_component_id<component>());
            return *this;
        }

        void freeze();
        bool is_frozen() const{
            return _is_frozen;
        }
        const std::vector<component_id>& get_ids() const;

    private:
        std::vector<component_id> component_ids;
        bool _is_frozen = false;
    };

    struct entity {
        int32_t id;
        uint32_t generation;

        //the components would not be accessible util next tick
        template <class component, class... args>
        void add(args&&... args) {
            set(component(std::forward<args>(args)...));
        }

        template <class component>
        component& get() {
            return *static_cast<component*>(detail::get_entity_component(id, generation, detail::get_component_id<component>()));
        }

        template <class component>
        const component& get() const {
            return *static_cast<const component*>(detail::get_entity_component(id, generation, detail::get_component_id<component>()));
        }

        //the components would not be accessible and changed util next tick
        template <class component>
        void set(component&& component) {
            detail::queue_set_entity_component(id, generation, detail::get_component_id<component>(), &component);
        }

        //the components would be accessible util next tick
        template <class component>
        void remove() {
            detail::queue_remove_entity_component(id, generation, detail::get_component_id<component>());
        }

        template <class component>
        bool has() {
            return detail::has_entity_component(id, generation, detail::get_component_id<component>());
        }

        void destroy() {
            detail::queue_destroy_entity(id, generation);
        }
    };

    template <class... components>
    struct query_iterator {
        std::tuple<components*...> component_arrays;
        size_t current_index_in_chunk = 0;
        size_t max_chunk_size = 0;
        detail::iteration_handle handle;


        using iterator_category = std::input_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = std::tuple<components*...>;
        using pointer = value_type*;
        using reference = value_type&;

        query_iterator() = default;

        query_iterator(detail::iteration_handle&& handle) : handle(std::move(handle)) {
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
            if (current_index_in_chunk < max_chunk_size)
                current_index_in_chunk++;
            else {
                auto [chunk_size, chunk] = handle.next();
                max_chunk_size = chunk_size;
                current_index_in_chunk = 0;
                [chunk, this]<size_t... Is>(std::index_sequence<Is...>) {
                    reinterpret_cast<void*&>(std::get<Is>(component_arrays)) = chunk[Is];
                }(std::make_index_sequence<sizeof...(components)>{});
            }
            return *this;
        }

        bool operator==(const query_iterator& other) const {
            return handle.data == other.handle.data;
        }

        bool operator!=(const query_iterator& other) const {
            return !(*this == other);
        }

        std::tuple<components*...> operator*() {
            return std::make_tuple((std::get<components*>(component_arrays) + current_index_in_chunk)...);
        }
    };

    template <class... components>
    struct query {
        using iterator = query_iterator<components...>;

        template <class... without_components>
        struct without_query {
            without_query(int32_t world_id) : world_id(world_id) {}

            iterator begin() {
                static component_id _components[] = {detail::get_component_id<components>()...};
                static component_id _without_components[] = {detail::get_component_id<without_components>()...};

                return iterator{
                    detail::iterate_components(
                        id,
                        _components,
                        sizeof...(components),
                        _without_components,
                        sizeof...(without_components)
                    )
                };
            }

            iterator end() {
                return iterator{detail::iteration_handle{}};
            }

        private:
            int32_t world_id;
        };

        query(int32_t world_id) : world_id(world_id) {}

        iterator begin() {
            static component_id _components[] = {detail::get_component_id<components>()...};

            return iterator{
                detail::iterate_components(
                    world_id,
                    _components,
                    sizeof...(components),
                    nullptr,
                    0
                )
            };
        }

        iterator end() {
            return iterator{detail::iteration_handle{}};
        }

        template <class... without_components>
        without_query<without_components...> without() {
            return without_query<without_components...>{};
        }

    private:
        int32_t world_id;
    };

    template <class... components>
    struct global_query {
        using iterator = query_iterator<components...>;

        template <class... without_components>
        struct without_query {
            without_query() = default;

            iterator begin() {
                static component_id _components[] = {detail::get_component_id<components>()...};
                static component_id _without_components[] = {detail::get_component_id<without_components>()...};

                return iterator{
                    detail::iterate_components_global(
                        _components,
                        sizeof...(components),
                        _without_components,
                        sizeof...(without_components)
                    )
                };
            }

            iterator end() {
                return iterator{detail::iteration_handle{}};
            }
        };

        query() = default;

        iterator begin() {
            static component_id _components[] = {detail::get_component_id<components>()...};

            return iterator{
                detail::iterate_components_global(
                    _components,
                    sizeof...(components),
                    nullptr,
                    0
                )
            };
        }

        iterator end() {
            return iterator{detail::iteration_handle{}};
        }

        template <class... without_components>
        without_query<without_components...> without() {
            return without_query<without_components...>{};
        }
    };

    struct world_local_registry {
        world_local_registry(int32_t id) : id(id) {}

        fast_task::future_ptr<bool> register_entity(entity& entity);
        fast_task::future_ptr<bool> unregister_entity(entity& entity);
        fast_task::future_ptr<bool> transfer_entity(entity& entity); //the old world local registry received from entity's internal data
        fast_task::future_ptr<entity> create_entity(entity_recipe& recipe);

        template <class... components>
        query<components...> view() {
            return query<components...>{id};
        }

    private:
        int32_t id;
    };

    namespace global_registry {
        template <class... components>
        global_query<components...> view() {
            return global_query<components...>{};
        }

        //recomended to use this to avoid short locks on fully loaded server
        template <class component>
        void register_component() {
            detail::get_component_id<component>();
        }

        fast_task::future_ptr<entity> create_entity(entity_recipe& recipe);
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
            _add_system(std::make_unique<T>(), detail::get_system_info<T>());
        }

        // Called each tick to run all systems in parallel
        //   also calls the dependency_graph if system_registry_changed
        void execute_frame(world_local_registry& registry);

    private:
        void _add_system(std::unique_ptr<system_interface> system, detail::system_info& info);
        struct data_t;
        data_t* data;
    };
}

#endif /* SRC_API_ECS */
