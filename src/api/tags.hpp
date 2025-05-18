#ifndef SRC_API_TAGS
#define SRC_API_TAGS
#include <library/list_array.hpp>
#include <string>
namespace copper_server::api::tags {
    enum class builtin_entry : uint8_t { //to access string result from block entry use minecraft:block as custom entry
        banner_pattern,
        block,
        damage_type,
        enchantment,
        entity_type,
        //fluid,
        //game_event,
        instrument,
        item,
        painting_variant,
        //point_of_interest,
    };


    const list_array<int32_t>& unfold_tag(builtin_entry entry, std::string_view tag);
    const list_array<std::string>& unfold_tag(std::string_view custom_entry, std::string_view tag);

    int32_t resolve_entry_item(builtin_entry entry, const std::string& value);

    void loading_stage_begin();//clear entries
    void add_tag(builtin_entry entry, std::string_view tag, const list_array<std::string>& items, bool allow_override = true);
    void add_tag(std::string_view custom_entry, std::string_view tag, const list_array<std::string>& items, bool allow_override = true);
    void loading_stage_end(); //optimize arrays, resolve cross references
}
#endif /* SRC_API_TAGS */
