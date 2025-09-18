/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_TAGS
#define SRC_API_TAGS
#include <library/list_array.hpp>
#include <memory>
#include <string>

namespace copper_server::api::tags {
    namespace detail {
        struct _tag_entry_handle;

        struct destruct_tag_entry_handle {
            static void operator()(_tag_entry_handle* ptr);
        };
    }
    enum class builtin_entry : uint8_t { //to access string result from block entry use minecraft:block as custom entry
        banner_pattern,
        block,
        block_state, //virtual, uses block tag
        damage_type,
        enchantment,
        entity_type,
        fluid,
        game_event,
        instrument,
        item,
        painting_variant,
        //point_of_interest,
    };


    const list_array<int32_t>& unfold_tag(builtin_entry entry, std::string_view tag);
    const list_array<std::string>& unfold_tag(std::string_view custom_entry, std::string_view tag);

    bool contains(builtin_entry entry, std::string_view tag, int32_t id);
    bool contains(builtin_entry entry, std::string_view tag);
    bool contains(std::string_view custom_entry, std::string_view tag);

    std::unordered_map<std::string, list_array<int32_t>> view_tag(builtin_entry entry, std::string_view _namespace);
    std::unordered_map<std::string, list_array<std::string>> view_tag(std::string_view custom_entry, std::string_view _namespace);

    std::unordered_map<std::string, std::unordered_map<std::string, list_array<int32_t>>> view_entry(builtin_entry entry);
    std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>> view_entry(std::string_view custom_entry);

    int32_t resolve_entry_item(builtin_entry entry, const std::string& value);


    using tag_handle = std::unique_ptr<detail::_tag_entry_handle, detail::destruct_tag_entry_handle>;
    tag_handle get_tag_handle(builtin_entry entry, std::string_view tag);
    tag_handle get_tag_handle(std::string_view custom_entry, std::string_view tag);
    bool contains(const tag_handle&, int32_t id);
    bool contains(const tag_handle&);
    const list_array<int32_t>& unfold_tag_ids(const tag_handle&);
    const list_array<std::string>& unfold_tag_strings(const tag_handle&);
    const std::string& get_name(const tag_handle&);
    const std::string& get_entry(const tag_handle&);


    void loading_stage_begin(); //clear entries
    void add_tag(builtin_entry entry, std::string_view tag, const list_array<std::string>& items, bool allow_override = true);
    void add_tag(std::string_view custom_entry, std::string_view tag, const list_array<std::string>& items, bool allow_override = true);
    void loading_stage_end(); //optimize arrays, resolve cross references
}
#endif /* SRC_API_TAGS */
