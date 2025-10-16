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
    entity iterator_view_chunk_parralel_dirty_mark<written_components...>::current_entity(size_t index) {
        auto it = state.get_current_entity(handle, index);
        return {it.first, it.second};
    }
}
#endif /* SRC_API_DETAIL_ECS_LATE_DEFINITION */