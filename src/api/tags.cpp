#include <library/fast_task.hpp>
#include <src/api/tags.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/slot.hpp>
#include <src/registers.hpp>

namespace copper_server::api::tags {
    fast_task::task_mutex mut;

    static const std::string builtin_entry_to_string[]{
        "minecraft:banner_pattern",
        "minecraft:block",
        "minecraft:damage_type",
        "minecraft:enchantment",
        "minecraft:entity_type",
        "minecraft:fluid",
        "minecraft:game_event",
        "minecraft:instrument",
        "minecraft:item",
        "minecraft:painting_variant",
        //"minecraft:point_of_interest",
    };

    int32_t resolve_entry_item(builtin_entry entry, const std::string& value) {

#define safety(block)                                                                                                  \
    try {                                                                                                              \
        block                                                                                                          \
    } catch (...) {                                                                                                    \
        throw std::runtime_error("Not found item " + value + " in entry: " + builtin_entry_to_string[(uint8_t)entry]); \
    }

        switch (entry) {
        case builtin_entry::banner_pattern:
            safety(return (int32_t)registers::bannerPatterns.at(value).id;);
        case builtin_entry::block:
            safety(return base_objects::block::get_block(value).general_block_id;);
        case builtin_entry::damage_type:
            safety(return (int32_t)registers::damageTypes.at(value).id;);
        case builtin_entry::enchantment:
            safety(return (int32_t)registers::enchantments.at(value).id;);
        case builtin_entry::entity_type:
            safety(return base_objects::entity_data::get_entity(value).entity_id;);
        case builtin_entry::fluid:;
            safety(return registers::view_reg_pro_id("minecraft:fluid", value););
        case builtin_entry::game_event:
            safety(return registers::view_reg_pro_id("minecraft:game_event", value););
        case builtin_entry::instrument:
            safety(return (int32_t)registers::instruments.at(value).id;);
        case builtin_entry::item:
            safety(return base_objects::slot_data::get_slot_data(value).internal_id;);
        case builtin_entry::painting_variant:
            safety(return (int32_t)registers::paintingVariants.at(value).id;);
        //case builtin_entry::point_of_interest:
        //  safety(return registers::.at(value).poi;);
        default:
            throw std::runtime_error("Invalid entry");
        }
#undef safety
    }

    struct tags_entry {
        list_array<std::string> items;
        list_array<int32_t> ids_cache;
        bool allow_override = true;


#define safety(block)                                                                                               \
    try {                                                                                                           \
        block                                                                                                       \
    } catch (...) {                                                                                                 \
        throw std::runtime_error("Not found item " + it + " in entry: " + builtin_entry_to_string[(uint8_t)entry]); \
    }

        const list_array<int32_t>& as_ids(builtin_entry entry) {
            if (ids_cache.size() != items.size()) {
                std::lock_guard lock(mut);
                switch (entry) {
                case builtin_entry::banner_pattern:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return registers::bannerPatterns.at(it).id;) });
                    break;
                case builtin_entry::block:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return base_objects::block::get_block(it).general_block_id;) });
                    break;
                case builtin_entry::damage_type:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return registers::damageTypes.at(it).id;) });
                    break;
                case builtin_entry::enchantment:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return registers::enchantments.at(it).id;) });
                    break;
                case builtin_entry::entity_type:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return base_objects::entity_data::get_entity(it).entity_id;) });
                    break;
                case builtin_entry::fluid:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return registers::view_reg_pro_id("minecraft:fluid", it);) });
                    break;
                case builtin_entry::game_event: {
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return registers::view_reg_pro_id("minecraft:game_event", it);) });
                    break;
                }
                case builtin_entry::instrument:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return registers::instruments.at(it).id;) });
                    break;
                case builtin_entry::item:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return base_objects::slot_data::get_slot_data(it).internal_id;) });
                    break;
                case builtin_entry::painting_variant:
                    ids_cache = items.convert_fn([entry](auto& it) { safety(return registers::paintingVariants.at(it).id;) });
                    break;
                //case builtin_entry::point_of_interest:
                //    ids_cache = items.convert_fn([entry](auto& it) { safety(return registers::.at(it).poi;) });
                //    break;
                default:
                    throw std::runtime_error("Invalid entry");
                }
            }
            return ids_cache;
        }

#undef safety
    };

    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, tags_entry>>> tags;

    const char default_namespace[] = "minecraft";

    const list_array<std::string>& unfold_tags_tag(const std::string& type, const std::string& namespace_, const std::string& tag) {
        static list_array<std::string> empty;
        auto ns = tags.find(type);
        if (ns == tags.end())
            return empty;
        auto t = ns->second.find(namespace_);
        if (t == ns->second.end())
            return empty;
        auto y = t->second.find(tag);
        if (y == t->second.end())
            return empty;
        return y->second.items;
    }

    const list_array<int32_t>& unfold_direct_tag(builtin_entry entry, const std::string& namespace_, const std::string& tag) {
        static list_array<int32_t> empty;
        auto ns = tags.find(builtin_entry_to_string[(uint8_t)entry]);
        if (ns == tags.end())
            return empty;
        auto t = ns->second.find(namespace_);
        if (t == ns->second.end())
            return empty;
        auto y = t->second.find(tag);
        if (y == t->second.end())
            return empty;
        return y->second.as_ids(entry);
    }

    const list_array<int32_t>& unfold_tag(builtin_entry entry, std::string_view tag) {

        if (tag.starts_with('#'))
            tag = tag.substr(1);

        std::string _namespace;
        std::string _tag;
        if (auto nam = tag.find(':'); nam != tag.npos) {
            _namespace = tag.substr(0, nam - 1);
            _tag = tag.substr(nam);
            if (_namespace.empty())
                _namespace = default_namespace;
        }

        return unfold_direct_tag(entry, default_namespace, _tag);
    }

    const list_array<std::string>& unfold_tag(std::string_view custom_entry, std::string_view tag) {
        if (tag.starts_with('#'))
            tag = tag.substr(1);

        std::string _entry(custom_entry);
        std::string _namespace;
        std::string _tag;
        if (auto nam = tag.find(':'); nam != tag.npos) {
            _namespace = tag.substr(0, nam - 1);
            _tag = tag.substr(nam);
            if (_namespace.empty())
                _namespace = default_namespace;
        }

        return unfold_tags_tag(_entry, default_namespace, _tag);
    }

    void loading_stage_begin() {
        std::lock_guard lock(mut);
        tags.clear();
    }

    void add_tag(builtin_entry entry, std::string_view tag, const list_array<std::string>& items, bool allow_override) {
        add_tag(builtin_entry_to_string[(uint8_t)entry], tag, items, allow_override);
    }

    void add_tag(std::string_view custom_entry, std::string_view tag, const list_array<std::string>& items, bool allow_override) {
        if (tag.starts_with('#'))
            tag = tag.substr(1);

        std::string entry(custom_entry);
        std::string _namespace;
        std::string _tag;
        if (auto nam = tag.find(':'); nam != tag.npos) {
            _namespace = tag.substr(0, nam - 1);
            _tag = tag.substr(nam);
            if (_namespace.empty())
                _namespace = default_namespace;
        }

        std::lock_guard lock(mut);
        auto& res = tags[entry][_namespace][_tag];
        if (res.allow_override) {
            if (allow_override)
                res.items += items;
            else
                res.items = items;
            res.ids_cache.clear();
        } else
            throw std::runtime_error("Tag " + _namespace + ":" + _tag + " in entry " + entry + " does not allow to override.");
    }

    std::unordered_map<std::string, list_array<int32_t>> view_tag(builtin_entry entry, std::string_view _namespace) {
        auto ns = tags.find(builtin_entry_to_string[(uint8_t)entry]);
        if (ns == tags.end())
            return {};
        auto t = ns->second.find((std::string)_namespace);
        if (t == ns->second.end())
            return {};
        std::unordered_map<std::string, list_array<int32_t>> res;
        res.reserve(t->second.size());
        for (auto&& [tag, decl] : t->second)
            res[tag] = decl.as_ids(entry);
        return res;
    }

    std::unordered_map<std::string, list_array<std::string>> view_tag(std::string_view custom_entry, std::string_view _namespace) {
        auto ns = tags.find((std::string)custom_entry);
        if (ns == tags.end())
            return {};
        auto t = ns->second.find((std::string)_namespace);
        if (t == ns->second.end())
            return {};
        std::unordered_map<std::string, list_array<std::string>> res;
        res.reserve(t->second.size());
        for (auto&& [tag, decl] : t->second)
            res[tag] = decl.items;
        return res;
    }

    std::unordered_map<std::string, std::unordered_map<std::string, list_array<int32_t>>> view_entry(builtin_entry entry) {
        auto ns = tags.find(builtin_entry_to_string[(uint8_t)entry]);
        if (ns == tags.end())
            return {};
        std::unordered_map<std::string, std::unordered_map<std::string, list_array<int32_t>>> res;
        res.reserve(ns->second.size());
        for (auto&& [namespace_, decl] : ns->second)
            for (auto&& [tag, dec] : decl)
                res[namespace_][tag] = dec.as_ids(entry);
        return res;
    }

    std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>> view_entry(std::string_view custom_entry) {
        auto ns = tags.find((std::string)custom_entry);
        if (ns == tags.end())
            return {};
        std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>> res;
        res.reserve(ns->second.size());
        for (auto&& [namespace_, decl] : ns->second)
            for (auto&& [tag, dec] : decl)
                res[namespace_][tag] = dec.items;
        return res;
    }

    void resolve_cross_references(bool secold_preset) {
        decltype(tags) tmp_obj = tags;
        for (auto&& [entry, decl] : tmp_obj) {
            for (auto&& [namespace_, dec] : decl) {
                for (auto&& [tag, de] : dec) {
                    list_array<std::string> resolved_items;
                    for (auto& item : de.items) {
                        if (item.starts_with("#")) {
                            if (secold_preset)
                                resolved_items.push_back(unfold_tag(entry, item).where([](const std::string& tag) {
                                    return !tag.starts_with("#");
                                }));
                            else
                                resolved_items.push_back(unfold_tag(entry, item));
                        }
                    }
                    de = tags_entry{.items = std::move(resolved_items)};
                }
            }
        }
        tags = std::move(tmp_obj);
    }

    void loading_stage_end() {
        resolve_cross_references(false);
        resolve_cross_references(true);
        for (auto& _entry : tags)
            for (auto& _namespace : _entry.second)
                for (auto& _tag : _namespace.second) {
                    _tag.second.items.unify();
                    _tag.second.items.commit();
                }
    }
}