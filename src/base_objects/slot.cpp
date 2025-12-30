/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/packets.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/nbt_stream.hpp>

namespace copper_server::base_objects {
    std::unordered_map<std::string, std::shared_ptr<static_slot_data>> slot_data::named_full_item_data;
    std::vector<std::shared_ptr<static_slot_data>> slot_data::full_item_data_;

    bool slot_data::operator==(const slot_data& other) const {
        if (id != other.id)
            return false;
        if (count != other.count)
            return false;
        if (components.size() != other.components.size())
            return false;

        for (auto& [key, value] : components) {
            if (!other.components.contains(key))
                return false;
            if (other.components.at(key) != value)
                return false;
        }
        return true;
    }

    bool slot_data::operator!=(const slot_data& other) const {
        return !operator==(other);
    }

    bool slot_data::is_same_def(const slot_data& other) const {
        if (id != other.id)
            return false;
        if (components.size() != other.components.size())
            return false;

        for (auto& [key, value] : components) {
            if (!other.components.contains(key))
                return false;
            if (other.components.at(key) != value)
                return false;
        }
        return true;
    }

    std::optional<int32_t> slot_data::spawns_entity_type() const {
        return get_slot_data().spawn_entity;
    }

    void static_slot_data::reset_items() {
        slot_data::named_full_item_data.clear();
        slot_data::full_item_data_.clear();
    }

    item_id_t::item_id_t(const std::string& id)
        : id(slot_data::get_slot_data(id).internal_id) {}

    const std::string& item_id_t::to_name() const {
        return slot_data::get_slot_data(id).id;
    }

    static_slot_data& item_id_t::get_data() const {
        return slot_data::get_slot_data(id);
    }

    slot_data::slot_data() {}

    slot_data::slot_data(slot_data&& move) noexcept : components(std::move(move.components)), id(move.id), count(move.count) {}

    slot_data::slot_data(const slot_data& copy) : components(copy.components), id(copy.id), count(copy.count) {}

    slot_data::slot_data(std::unordered_map<int32_t, component>&& components, int32_t id, int32_t count) noexcept : components(std::move(components)), id(id), count(count) {}

    slot_data::slot_data(const std::unordered_map<int32_t, component>& components, int32_t id, int32_t count) : components(components), id(id), count(count) {}

    slot_data& slot_data::operator=(const slot_data& copy) {
        components = copy.components;
        id = copy.id;
        count = copy.count;
        return *this;
    }

    slot_data& slot_data::operator=(slot_data&& move) noexcept {
        components = std::move(move.components);
        id = move.id;
        count = move.count;
        return *this;
    }

    slot_data slot_data::create_item(const std::string& id, int32_t count) {
        try {
            auto res = named_full_item_data.at(id);
            return slot_data{
                res->default_components,
                res->internal_id,
                count
            };
        } catch (...) {
            throw std::runtime_error("Item not found: " + id);
        }
    }

    slot_data slot_data::create_item(uint32_t id, int32_t count) {
        auto res = full_item_data_.at(id);
        return slot_data{
            res->default_components,
            res->internal_id,
            count
        };
    }

    static_slot_data& slot_data::get_slot_data(const std::string& id) {
        return *named_full_item_data.at(id);
    }

    static_slot_data& slot_data::get_slot_data(uint32_t id) {
        return *full_item_data_.at(id);
    }

    static_slot_data& slot_data::get_slot_data() const {
        return *full_item_data_.at(id);
    }

    void slot_data::add_slot_data(static_slot_data&& move) {
        if (named_full_item_data.contains(move.id))
            throw std::runtime_error("Slot data already registered, \"" + move.id + '"');
        if (full_item_data_.size() >= INT32_MAX)
            throw std::runtime_error("Too many slot data already registered");

        auto moved = std::make_shared<static_slot_data>(std::move(move));
        named_full_item_data[moved->id] = moved;
        full_item_data_.push_back(moved);
        moved->internal_id = int32_t(full_item_data_.size() - 1);
    }

    void slot_data::enumerate_slot_data(const std::function<void(static_slot_data&)>& fn) {
        for (auto& block : full_item_data_)
            fn(*block);
    }

    list_array<int32_t> slot_data::get_slot_data_ids() {
        list_array<int32_t> res;
        res.reserve(full_item_data_.size());
        for (auto& block : full_item_data_)
            res.push_back(block->internal_id);
        return res;
    }

    copper_server::api::packets::slot slot_data::to_packet() const {
        api::packets::slot result;
        result.count = count;
        if (result.count) {
            result.id = id;
            list_array<int32_t> removed_components;
            list_array<base_objects::component> added_components;
            auto& default_components = base_objects::slot_data::get_slot_data((int32_t)result.id).default_components;
            for (auto& [c_id, item] : default_components)
                if (!components.contains(c_id))
                    removed_components.push_back(c_id);
                else if (components.at(c_id) != item)
                    removed_components.push_back(c_id);
            for (auto& [c_id, item] : components)
                if (!default_components.contains(c_id))
                    added_components.push_back(item);
                else if (components.at(c_id) != item)
                    added_components.push_back(item);
            result.components_to_add = (int32_t)added_components.size();
            result.components_to_remove = (int32_t)removed_components.size();
            result.to_add = added_components.take();
            result.to_remove = removed_components.take().convert<var_int32::data_component_type>();
        } else {
            result.id = 0;
            result.components_to_add = 0;
            result.components_to_remove = 0;
        }
        return result;
    }

    slot_data slot_data::from_packet(copper_server::api::packets::slot&& slot) {
        if (slot.count) {
            slot_data res{
                base_objects::slot_data::get_slot_data((int32_t)slot.id).default_components,
                slot.id,
                slot.count
            };

            for (auto component : slot.to_remove)
                res.components.erase((int32_t)component);
            for (auto& component : slot.to_add)
                res.add_component(std::move(component));
            return res;
        } else
            return slot_data{};
    }

    void slot_data::to_nbt(util::nbt_write_stream& stream) const {
        stream.write_compound()
            .write("id", id)
            .write("count", count)
            .write("components", [this](auto& components_stream) {
                auto compound = components_stream.write_compound();
                for (auto& [_, component] : components)
                    component.encode_component(component, compound);
            });
    }

    slot_data slot_data::from_nbt(util::nbt_read_stream& stream) {
        slot_data res;
        util::nbt_collection::compound_relaxed collect;
        collect
            .collect_as("id", res.id)
            .collect_as("count", res.count)
            .collect_iterate("components", [&res](auto& name, auto& stream) {
                base_objects::component com;
                com.parse_component(com, name, stream);
                res.components[com.get_id()] = std::move(com);
            })
            .force_all_collect(stream);
    }

    void slot_data::to_nbt_base(util::nbt_write_compound_stream& stream) const {
        stream
            .write("id", id)
            .write("count", count)
            .write("components", [this](auto& components_stream) {
                auto compound = components_stream.write_compound();
                for (auto& [_, component] : components)
                    component.encode_component(component, compound);
            });
    }

    void slot_data::from_nbt_base(util::nbt_collection::compound_flex& collector) {
        collector
            .collect_as_required("id", id)
            .collect_as_required("count", count)
            .collect_iterate_required("components", [this](auto& name, auto& stream) {
                base_objects::component com;
                com.parse_component(com, name, stream);
                components[com.get_id()] = std::move(com);
            });
    }

    copper_server::api::packets::slot slot::to_packet() const {
        if (*this)
            return (*this)->to_packet();
        else {
            api::packets::slot result;
            result.count = 0;
            result.id = 0;
            result.components_to_add = 0;
            result.components_to_remove = 0;
            return result;
        }
    }

    slot slot::from_packet(copper_server::api::packets::slot&& s) {
        if (s.count)
            return slot_data::from_packet(std::move(s));
        else
            return std::nullopt;
    }
}
