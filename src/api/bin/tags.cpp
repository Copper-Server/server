/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <boost/unordered/unordered_map.hpp>
#include <library/fast_task.hpp>
#include <src/api/entity.hpp>
#include <src/api/registers.hpp>
#include <src/api/tags.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/slot.hpp>

namespace copper_server::api::tags {
    static const std::string builtin_entry_to_string_virtual[]{
        "minecraft:banner_pattern",
        "minecraft:block",
        "minecraft:block_state",
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

    static const std::string builtin_entry_to_string[]{
        "minecraft:banner_pattern",
        "minecraft:block",
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

    static boost::unordered_flat_map<std::string, builtin_entry> builtin_entry_from_string{
        {"minecraft:banner_pattern", builtin_entry::banner_pattern},
        {"minecraft:block", builtin_entry::block},
        {"minecraft:block_state", builtin_entry::block_state},
        {"minecraft:damage_type", builtin_entry::damage_type},
        {"minecraft:enchantment", builtin_entry::enchantment},
        {"minecraft:entity_type", builtin_entry::entity_type},
        {"minecraft:fluid", builtin_entry::fluid},
        {"minecraft:game_event", builtin_entry::game_event},
        {"minecraft:instrument", builtin_entry::instrument},
        {"minecraft:item", builtin_entry::item},
        {"minecraft:painting_variant", builtin_entry::painting_variant},
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
            safety(return (int32_t)api::registers::bannerPatterns.at(value).id;);
        case builtin_entry::block:
            safety(return base_objects::block::get_block(value).general_block_id;);
        case builtin_entry::block_state:
            safety(return base_objects::block::get_block(value).default_state;);
        case builtin_entry::damage_type:
            safety(return (int32_t)api::registers::damage_types.at(value).id;);
        case builtin_entry::enchantment:
            safety(return (int32_t)api::registers::enchantments.at(value).id;);
        case builtin_entry::entity_type:
            safety(return api::entity_data::get_entity(value).entity_id;);
        case builtin_entry::fluid:;
            safety(return api::registers::view_reg_pro_id("minecraft:fluid", value););
        case builtin_entry::game_event:
            safety(return api::registers::view_reg_pro_id("minecraft:game_event", value););
        case builtin_entry::instrument:
            safety(return (int32_t)api::registers::instruments.at(value).id;);
        case builtin_entry::item:
            safety(return base_objects::slot_data::get_slot_data(value).internal_id;);
        case builtin_entry::painting_variant:
            safety(return (int32_t)api::registers::painting_variants.at(value).id;);
        //case builtin_entry::point_of_interest:
        //  safety(return api::registers::.at(value).poi;);
        default:
            throw std::runtime_error("Invalid entry");
        }
#undef safety
    }

    using TagID = uint32_t;

    struct tags_entry {
        int32_t id;
        int32_t entry_id;

        mutable fast_task::task_mutex caching_mut;
        list_array<std::string> items;
        mutable list_array<int32_t> ids_cache{};
        mutable list_array<int32_t> state_ids_cache{};
        mutable boost::unordered_flat_set<int32_t> check_cache{};
        mutable boost::unordered_flat_set<int32_t> state_check_cache{};
        mutable bool allow_override = true;
        mutable bool need_update = true;

        tags_entry(int32_t id, int32_t entry_id) : id(id), entry_id(entry_id) {}

        tags_entry(const tags_entry& copy)
            : id(copy.id), entry_id(copy.entry_id),
              items(copy.items),
              ids_cache(copy.ids_cache),
              state_ids_cache(copy.state_ids_cache),
              check_cache(copy.check_cache),
              state_check_cache(copy.state_check_cache),
              allow_override(copy.allow_override),
              need_update(copy.need_update) {}

        tags_entry(int32_t id, int32_t entry_id, const list_array<std::string>& items)
            : id(id), entry_id(entry_id), items(items) {}

        tags_entry(int32_t id, int32_t entry_id, list_array<std::string>&& items)
            : id(id), entry_id(entry_id), items(std::move(items)) {}

        tags_entry(tags_entry&& move) noexcept
            : id(move.id),
              entry_id(move.entry_id),
              items(std::move(move.items)),
              ids_cache(std::move(move.ids_cache)),
              state_ids_cache(std::move(move.state_ids_cache)),
              check_cache(std::move(move.check_cache)),
              state_check_cache(std::move(move.state_check_cache)),
              allow_override(std::move(move.allow_override)),
              need_update(std::move(move.need_update)) {}

        tags_entry& operator=(tags_entry&& move) noexcept {
            id = move.id;
            entry_id = move.entry_id;
            items = std::move(move.items);
            ids_cache = std::move(move.ids_cache);
            state_ids_cache = std::move(move.state_ids_cache);
            check_cache = std::move(move.check_cache);
            state_check_cache = std::move(move.state_check_cache);
            allow_override = std::move(move.allow_override);
            need_update = std::move(move.need_update);
            return *this;
        }

#define safety(block)                                                                                                       \
    try {                                                                                                                   \
        block                                                                                                               \
    } catch (...) {                                                                                                         \
        throw std::runtime_error("Not found item " + it + " in entry: " + builtin_entry_to_string_virtual[(uint8_t)entry]); \
    }

        const list_array<int32_t>& as_ids(builtin_entry entry) const {
            if (need_update)
                ids_update(entry);
            if (entry != builtin_entry::block_state)
                return ids_cache;
            else
                return state_ids_cache;
        }

        bool contains(builtin_entry entry, int32_t id) const {
            if (need_update)
                ids_update(entry);
            return entry != builtin_entry::block_state ? check_cache.contains(id) : state_check_cache.contains(id);
        }

        void ids_update(builtin_entry entry) const {
            std::lock_guard lock(caching_mut);
            if (!need_update)
                return;
            switch (entry) {
            case builtin_entry::banner_pattern:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return api::registers::bannerPatterns.at(it).id;) });
                break;
            case builtin_entry::block:
            case builtin_entry::block_state:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return base_objects::block::get_block(it).general_block_id;) });
                state_ids_cache = items.convert<int32_t>([entry](auto& it) {
                    safety({
                        list_array<int32_t> res;
                        for (auto& [state_id, props] : base_objects::block::get_block(it).assigned_states_to_properties->left)
                            res.push_back(state_id);
                        return res;
                    });
                });
                break;
            case builtin_entry::damage_type:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return api::registers::damage_types.at(it).id;) });
                break;
            case builtin_entry::enchantment:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return api::registers::enchantments.at(it).id;) });
                break;
            case builtin_entry::entity_type:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return api::entity_data::get_entity(it).entity_id;) });
                break;
            case builtin_entry::fluid:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return api::registers::view_reg_pro_id("minecraft:fluid", it);) });
                break;
            case builtin_entry::game_event: {
                ids_cache = items.convert_fn([entry](auto& it) { safety(return api::registers::view_reg_pro_id("minecraft:game_event", it);) });
                break;
            }
            case builtin_entry::instrument:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return api::registers::instruments.at(it).id;) });
                break;
            case builtin_entry::item:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return base_objects::slot_data::get_slot_data(it).internal_id;) });
                break;
            case builtin_entry::painting_variant:
                ids_cache = items.convert_fn([entry](auto& it) { safety(return api::registers::painting_variants.at(it).id;) });
                break;
            //case builtin_entry::point_of_interest:
            //    ids_cache = items.convert_fn([entry](auto& it) { safety(return api::registers::.at(it).poi;) });
            //    break;
            default:
                throw std::runtime_error("Invalid entry");
            }
            check_cache.reserve(ids_cache.size());
            for (auto& it : ids_cache)
                check_cache.emplace(it);
            state_check_cache.reserve(state_ids_cache.size());
            for (auto& it : state_ids_cache)
                state_check_cache.emplace(it);
            need_update = false;
        }

#undef safety
    };

    struct string_hash {
        using hash_type = std::hash<std::string_view>;
        using is_transparent = void;

        size_t operator()(std::string_view str) const noexcept {
            return hash_type()(str);
        }

        size_t operator()(const std::string& str) const noexcept {
            return hash_type()(str);
        }
    };

    struct string_eq {
        using is_transparent = void;

        constexpr bool operator()(const std::string& left, const std::string& right) const noexcept {
            return left == right;
        }

        constexpr bool operator()(const std::string& left, const std::string_view& right) const noexcept {
            return left == right;
        }

        constexpr bool operator()(const std::string_view& left, const std::string& right) const noexcept {
            return left == right;
        }

        constexpr bool operator()(const std::string_view& left, const std::string_view& right) const noexcept {
            return left == right;
        }
    };

    using tags_map = std::unordered_map<std::string, std::shared_ptr<tags_entry>, string_hash, string_eq>;
    using namespace_map = std::unordered_map<std::string, tags_map, string_hash, string_eq>;

    struct entry_handle {
        int32_t id;
        namespace_map data;

        fast_task::task_rw_mutex mutex_;
        int32_t next_id_ = 0;
        std::unordered_map<std::string, int32_t> string_to_id_;
        std::unordered_map<int32_t, std::string> id_to_string_;

        int32_t get_or_create_id(const std::string& tag_string) {
            {
                fast_task::read_lock lock(mutex_);
                if (auto it = string_to_id_.find(tag_string); it != string_to_id_.end())
                    return it->second;
            }

            fast_task::write_lock lock(mutex_);
            if (auto it = string_to_id_.find(tag_string); it != string_to_id_.end())
                return it->second;

            int32_t new_id = next_id_++;
            string_to_id_[tag_string] = new_id;
            id_to_string_[new_id] = tag_string;
            return new_id;
        }

        std::optional<std::string> get_string(int32_t id) {
            fast_task::read_lock lock(mutex_);
            if (auto it = id_to_string_.find(id); it != id_to_string_.end()) {
                return it->second;
            }
            return std::nullopt;
        }
    };

    const char default_namespace[] = "minecraft";

    struct entry_map {
        mutable std::atomic_int32_t next_entry_id = 0;
        mutable std::unordered_map<std::string, entry_handle, string_hash, string_eq> map;

        entry_handle& get_entry_raw(std::string_view entry) {
            if (auto it = map.find(entry); it != map.end())
                return it->second;
            else
                return map.emplace(entry, next_entry_id++).first->second;
        }

        const entry_handle& get_entry_raw(std::string_view entry) const {
            if (auto it = map.find(entry); it != map.end())
                return it->second;
            else
                return map.emplace(entry, next_entry_id++).first->second;
        }

        entry_handle& get_entry(std::string_view entry) {
            if (entry.starts_with(':'))
                return get_entry_raw(default_namespace + std::string(entry));
            else if (!entry.contains(':'))
                return get_entry_raw(std::string(default_namespace) + ":" + std::string(entry));
            else
                return get_entry_raw(entry);
        }

        const entry_handle& get_entry(std::string_view entry) const {
            if (entry.starts_with(':'))
                return get_entry_raw(default_namespace + std::string(entry));
            else if (!entry.contains(':'))
                return get_entry_raw(std::string(default_namespace) + ":" + std::string(entry));
            else
                return get_entry_raw(entry);
        }

        void clear() {
            map.clear();
        }
    };

    fast_task::protected_value<entry_map> data;
    std::atomic_size_t tags_version = 0;

    static void get_namespace_and_tag(std::string_view& namespace_, std::string_view& tag_, std::string_view tag) {
        if (tag.starts_with('#'))
            tag = tag.substr(1);
        if (auto nam = tag.find(':'); nam != tag.npos) {
            namespace_ = tag.substr(0, nam);
            tag_ = tag.substr(nam + 1);
            if (namespace_.empty())
                namespace_ = default_namespace;
        } else {
            namespace_ = default_namespace;
            tag_ = tag;
        }
    }

    static const list_array<std::string>& unfold_tags_tag(const entry_map& tags, std::string_view type, std::string_view namespace_, std::string_view tag) {
        static std::string_view block_entry = "minecraft:block";
        static list_array<std::string> empty;
        auto& ns = tags.get_entry(type != "minecraft:block_state" ? type : block_entry);
        auto t = ns.data.find(namespace_);
        if (t == ns.data.end())
            return empty;
        auto y = t->second.find(tag);
        if (y == t->second.end())
            return empty;
        if (!y->second)
            return empty;
        return y->second->items;
    }

    static const list_array<std::string>& unfold_tags_tag(const entry_map& tags, std::string_view type, std::string_view tag) {
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);
        return unfold_tags_tag(tags, type, _namespace, _tag);
    }

    static const list_array<int32_t>& unfold_direct_tag(builtin_entry entry, std::string_view namespace_, std::string_view tag) {
        return data.get([&](auto& tags) -> const list_array<int32_t>& {
            static list_array<int32_t> empty;
            auto& ns = tags.tags.get_entry(builtin_entry_to_string[(uint8_t)entry]);
            auto t = ns.data.find(namespace_);
            if (t == ns.data.end())
                return empty;
            auto y = t->second.find(tag);
            if (y == t->second.end())
                return empty;
            if (!y->second)
                return empty;
            return y->second->as_ids(entry);
        });
    }

    const list_array<int32_t>& unfold_tag(builtin_entry entry, std::string_view tag) {
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);
        return unfold_direct_tag(entry, _namespace, (std::string)_tag);
    }

    const list_array<std::string>& unfold_tag(std::string_view custom_entry, std::string_view tag) {
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);
        return data.get([&](auto& tags) -> const list_array<std::string>& {
            return unfold_tags_tag(tags, custom_entry, _namespace, (std::string)_tag);
        });
    }

    bool contains(builtin_entry entry, std::string_view tag, int32_t id) {
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);

        return data.get([&](auto& tags) {
            auto& ns = tags.get_entry(builtin_entry_to_string[(uint8_t)entry]);
            auto t = ns.data.find(_namespace);
            if (t == ns.data.end())
                return false;
            auto y = t->second.find(_tag);
            if (y == t->second.end())
                return false;
            if (!y->second)
                return false;
            return entry != builtin_entry::block_state ? y->second->check_cache.contains(id) : y->second->state_check_cache.contains(id);
        });
    }

    bool contains(builtin_entry entry, std::string_view tag) {
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);

        return data.get([&](auto& tags) {
            auto& ns = tags.get_entry(builtin_entry_to_string[(uint8_t)entry]);
            auto t = ns.data.find(_namespace);
            if (t == ns.data.end())
                return false;
            return t->second.find(_tag) != t->second.end();
        });
    }

    bool contains(std::string_view custom_entry, std::string_view tag) {
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);

        return data.get([&](auto& tags) {
            static std::string_view block_entry = "minecraft:block";
            auto& ns = tags.get_entry(custom_entry != "minecraft:block_state" ? custom_entry : block_entry);
            auto t = ns.data.find(_namespace);
            if (t == ns.data.end())
                return false;
            return t->second.find(_tag) != t->second.end();
        });
    }

    void loading_stage_begin() {
        return data.set([&](auto& tags) {
            ++tags_version;
            tags.clear();
        });
    }

    void add_tag(builtin_entry entry, std::string_view tag, const list_array<std::string>& items, bool allow_override) {
        add_tag(builtin_entry_to_string[(uint8_t)entry], tag, items, allow_override);
    }

    void add_tag(std::string_view custom_entry, std::string_view tag, const list_array<std::string>& items, bool allow_override) {
        std::string entry(custom_entry);
        if (entry.starts_with(':'))
            entry = "minecraft" + entry;
        else if (!entry.contains(':'))
            entry = "minecraft:" + entry;
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);


        return data.set([&](auto& tags) {
            ++tags_version;
            static std::string_view block_entry = "minecraft:block";
            auto actual_entry(entry != "minecraft:block_state" ? entry : block_entry);
            auto& ns = tags.get_entry(actual_entry);
            auto t = ns.data.find(_namespace);
            if (t == ns.data.end()) {
                auto res = ns.data.insert({std::string(_namespace), {}});
                if (!res.second)
                    throw std::runtime_error("Failed to add namespace " + std::string(_namespace) + " in entry " + std::string(actual_entry));
                t = res.first;
            }
            auto y = t->second.find(_tag);
            if (y == t->second.end()) {
                auto res = t->second.insert({std::string(_tag), {}});
                if (!res.second)
                    throw std::runtime_error("Failed to add tag " + std::string(_tag) + " in namespace " + std::string(_namespace) + " in entry " + std::string(actual_entry));
                y = res.first;
            }
            auto& res = y->second;
            if (!res)
                res = std::make_shared<tags_entry>(ns.get_or_create_id(std::string(_namespace) + ":" + std::string(_tag)), ns.id, items);
            if (res->allow_override) {
                if (allow_override)
                    res->items += items;
                else
                    res->items = items;
                res->ids_cache.clear();
                res->state_ids_cache.clear();
                res->check_cache.clear();
                res->state_check_cache.clear();
                res->need_update = true;
            } else
                throw std::runtime_error("Tag " + std::string(_namespace) + ":" + std::string(_tag) + " in entry " + std::string(actual_entry) + " does not allow to override.");
        });
    }

    std::unordered_map<std::string, list_array<int32_t>> view_tag(builtin_entry entry, std::string_view _namespace) {
        return data.get([&](auto& tags) -> std::unordered_map<std::string, list_array<int32_t>> {
            auto& ns = tags.get_entry(builtin_entry_to_string[(uint8_t)entry]);
            auto t = ns.data.find((std::string)_namespace);
            if (t == ns.data.end())
                return {};
            std::unordered_map<std::string, list_array<int32_t>> res;
            res.reserve(t->second.size());
            for (auto&& [tag, decl] : t->second)
                if (decl)
                    res[tag] = decl->as_ids(entry);
            return res;
        });
    }

    std::unordered_map<std::string, list_array<std::string>> view_tag(std::string_view custom_entry, std::string_view _namespace) {
        return data.get([&](auto& tags) -> std::unordered_map<std::string, list_array<std::string>> {
            auto& ns = tags.get_entry(custom_entry);
            auto t = ns.data.find((std::string)_namespace);
            if (t == ns.data.end())
                return {};
            std::unordered_map<std::string, list_array<std::string>> res;
            res.reserve(t->second.size());
            for (auto&& [tag, decl] : t->second)
                if (decl)
                    res[tag] = decl->items;
            return res;
        });
    }

    std::unordered_map<std::string, std::unordered_map<std::string, list_array<int32_t>>> view_entry(builtin_entry entry) {
        return data.get([&](auto& tags) -> std::unordered_map<std::string, std::unordered_map<std::string, list_array<int32_t>>> {
            auto& ns = tags.get_entry(builtin_entry_to_string[(uint8_t)entry]);
            std::unordered_map<std::string, std::unordered_map<std::string, list_array<int32_t>>> res;
            res.reserve(ns.data.size());
            for (auto&& [namespace_, decl] : ns.data)
                for (auto&& [tag, dec] : decl)
                    if (dec)
                        res[namespace_][tag] = dec->as_ids(entry);
            return res;
        });
    }

    std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>> view_entry(std::string_view custom_entry) {
        return data.get([&](auto& tags) -> std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>> {
            auto ns = fixed_entry_map(custom_entry, tags);
            if (ns == tags.end())
                return {};
            std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>> res;
            res.reserve(ns->second.size());
            for (auto&& [namespace_, decl] : ns->second)
                for (auto&& [tag, dec] : decl)
                    if (dec)
                        res[namespace_][tag] = dec->items;
            return res;
        });
    }

    static void resolve_cross_references(bool secold_preset) {
        return data.set([&](auto& tags) {
            decltype(tags.map) tmp_obj = tags.map;
            for (auto&& [entry, decl] : tmp_obj) {
                for (auto&& [namespace_, dec] : decl.data) {
                    for (auto&& [tag, de] : dec) {
                        list_array<std::string> resolved_items;
                        if (!de)
                            de = std::make_shared<tags_entry>(decl.get_or_create_id(namespace_ + ":" + tag), decl.id);
                        for (auto& item : de->items) {
                            if (item.starts_with("#")) {
                                if (secold_preset)
                                    resolved_items.push_back(unfold_tags_tag(tags, entry, item).where([](const std::string& tag) {
                                        return !tag.starts_with("#");
                                    }));
                                else
                                    resolved_items.push_back(unfold_tags_tag(tags, entry, item));
                            } else
                                resolved_items.push_back(item);
                        }
                        de->items = std::move(resolved_items);
                    }
                }
            }
            tags.map = std::move(tmp_obj);
        });
    }

    void loading_stage_end() {
        resolve_cross_references(false);
        resolve_cross_references(true);
        return data.set([&](auto& tags) {
            ++tags_version;
            for (auto& _entry : tags.map)
                for (auto& _namespace : _entry.second.data)
                    for (auto& _tag : _namespace.second) {
                        _tag.second->items.unify();
                        _tag.second->items.commit();
                    }
        });
    }

    namespace detail {
        struct _tag_entry_handle {
            const std::string entry, namespace_, tag;
            std::shared_ptr<tags_entry> entry_ptr = nullptr;
            size_t version = 0;
            bool use_state_entry = false;

            void resolve() {
                entry_ptr = data.get([this](auto& tags) -> std::shared_ptr<tags_entry> {
                    static std::string_view block_entry = "minecraft:block";
                    auto actual_entry(entry != "minecraft:block_state" ? std::string_view(entry) : block_entry);
                    auto ns = fixed_entry_map(actual_entry, tags);
                    if (ns == tags.end())
                        return nullptr;
                    auto t = ns->second.find(namespace_);
                    if (t == ns->second.end())
                        return nullptr;
                    auto y = t->second.find(tag);
                    if (y == t->second.end())
                        return nullptr;
                    version = tags_version;
                    return y->second;
                });
                if (entry_ptr)
                    if (builtin_entry_from_string.contains(entry))
                        entry_ptr->ids_update(builtin_entry_from_string.at(entry));
            }

            const std::shared_ptr<tags_entry> get() {
                if (!entry_ptr)
                    resolve();
                else if (tags_version != version)
                    resolve();
                return entry_ptr;
            }
        };

        std::shared_ptr<_tag_entry_handle> copy(std::shared_ptr<_tag_entry_handle> copy) {
            return std::make_shared<_tag_entry_handle>(*copy);
        }
    }

    tag_handle::~tag_handle() = default;

    tag_handle get_tag_handle(std::string_view custom_entry, std::string_view tag) {
        std::string actual_entry;
        if (custom_entry.starts_with(':'))
            actual_entry = "minecraft" + std::string(custom_entry);
        else if (!custom_entry.contains(':'))
            actual_entry = "minecraft:" + std::string(custom_entry);
        else
            actual_entry = std::string(custom_entry);
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);
        return std::make_shared<detail::_tag_entry_handle>(std::move(actual_entry), std::string(_namespace), std::string(_tag));
    }

    tag_handle get_tag_handle(builtin_entry entry, std::string_view tag) {
        std::string_view _namespace;
        std::string_view _tag;
        get_namespace_and_tag(_namespace, _tag, tag);
        return std::make_shared<detail::_tag_entry_handle>(builtin_entry_to_string_virtual[(uint8_t)entry], std::string(_namespace), std::string(_tag));
    }

    bool contains(const tag_handle& handle) {
        return handle ? bool(handle->get()) : false;
    }

    bool contains(const tag_handle& handle, int32_t id) {
        if (handle) {
            auto h = handle->get();
            if (!h)
                return false;
            if (handle->use_state_entry)
                return h->state_check_cache.contains(id);
            else
                return h->check_cache.contains(id);
        } else
            return false;
    }

    const list_array<int32_t>& unfold_tag_ids(const tag_handle& handle) {
        static list_array<int32_t> empty;
        if (handle) {
            auto h = handle->get();
            if (!h)
                return empty;
            if (handle->use_state_entry)
                return h->state_ids_cache;
            else
                return h->ids_cache;
        } else
            return empty;
    }

    const list_array<std::string>& unfold_tag_strings(const tag_handle& handle) {
        static list_array<std::string> empty;
        if (handle) {
            auto h = handle->get();
            if (!h)
                return empty;
            else
                return h->items;
        } else
            return empty;
    }

    const std::string& get_name(const tag_handle& handle) {
        return handle->tag;
    }

    const std::string& get_namespace(const tag_handle& handle) {
        return handle->namespace_;
    }

    const std::string& get_full_name(const tag_handle& handle) {
        return handle->namespace_ + ":" + handle->tag;
    }

    const std::string& get_entry(const tag_handle& handle) {
        return handle->entry;
    }

    int32_t get_entry_id(const tag_handle& handle) {
        return handle->entry_ptr->entry_id;
    }

    int32_t get_tag_id(const tag_handle& handle) {
        return handle->entry_ptr->id;
    }
}
