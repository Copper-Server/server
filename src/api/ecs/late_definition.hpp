/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_DETAIL_ECS_LATE_DEFINITION
#define SRC_API_DETAIL_ECS_LATE_DEFINITION

namespace copper_server::api::ecs::detail {
    template <class... written_components>
    entity iterator_view_dirty_mark<written_components...>::current_entity() {
        auto it = handle.get_current_entity(index);
        return {it.first, it.second};
    }

    template <class... written_components>
    entity iterator_view_chunk_dirty_mark<written_components...>::current_entity(size_t index) {
        auto it = handle.get_current_entity(index);
        return {it.first, it.second};
    }

    template <class... written_components>
    entity iterator_view_chunk_parallel_dirty_mark<written_components...>::current_entity(size_t index) {
        auto it = state.get_current_entity(handle, index);
        return {it.first, it.second};
    }

    template <class... components>
    fast_task::future_ptr<entity> create_entity__cc(std::optional<int32_t> world_id, components&&... args) {
        struct comp_hold : public detail::components_holder {
            std::tuple<components...> components;

            virtual ~comp_hold() override = default;
        };

        auto data = std::make_unique<comp_hold>(comp_hold{.components = {std::forward<components>(args)...}});

        data.components_reference.resize(sizeof...(components));
        [&data]<size_t... I>(std::index_sequence<I...>) {
            data.components_reference.at(I) = {detail::get_component_id<decltype(std::get<I>(data.components))>(), (void*)&std::get<I>(data.components)};
        }(std::make_index_sequence<std::tuple_size_v<std::tuple<components...>>>());

        return detail::create_entity(world_id, std::move(data));
    }

    template <class... components>
    fast_task::future_ptr<entity> create_entity_r_cc(std::optional<int32_t> world_id, const entity_recipe& recipe, components&&... args) {
        struct comp_hold : public detail::components_holder {
            std::tuple<components...> components;

            virtual ~comp_hold() override = default;
        };

        auto data = std::make_unique<comp_hold>(comp_hold{.components = {std::forward<components>(args)...}});

        data.components_reference.resize(sizeof...(components));
        [&data]<size_t... I>(std::index_sequence<I...>) {
            data.components_reference.at(I) = {detail::get_component_id<decltype(std::get<I>(data.components))>(), (void*)&std::get<I>(data.components)};
        }(std::make_index_sequence<std::tuple_size_v<std::tuple<components...>>>());

        return detail::create_entity(world_id, recipe, std::move(data));
    }
}
#endif /* SRC_API_DETAIL_ECS_LATE_DEFINITION */