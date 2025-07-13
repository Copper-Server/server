#include <src/base_objects/slot.hpp>

namespace copper_server::base_objects {
    std::unordered_map<std::string, std::shared_ptr<static_slot_data>> slot_data::named_full_item_data;
    std::vector<std::shared_ptr<static_slot_data>> slot_data::full_item_data_;


    slot_data slot_data::from_enbt(enbt::compound_const_ref compound) {
        auto& id = compound.at("id").as_string();
        base_objects::slot_data slot_data = base_objects::slot_data::create_item(id);
        if (compound.contains("count"))
            slot_data.count = compound.at("count");
        std::unordered_map<std::string, base_objects::component::unified> components;
        for (auto& [name, value] : compound.at("components").as_compound())
            slot_data.add_component(component::parse_component(name, value));
        return slot_data;
    }

    enbt::compound slot_data::to_enbt() const {
        enbt::compound compound;
        compound["id"] = id;
        compound["count"] = count;
        enbt::compound comp;
        comp.reserve(components.size());
        for (auto& [name, value] : components)
            comp[name] = std::move(component::encode_component(value).second);
        compound["components"] = std::move(comp);
        return compound;
    }

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
        return operator==(other);
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

    void static_slot_data::reset_items() {
        slot_data::named_full_item_data.clear();
        slot_data::full_item_data_.clear();
    }


    item_id_t::item_id_t(const std::string& id)
        : id(slot_data::get_slot_data(id).internal_id) {}

    item_id_t::item_id_t(uint32_t id)
        : id(id) {}

    item_id_t::item_id_t(const item_id_t& id)
        : id(id.id) {}

    item_id_t::item_id_t()
        : id(0) {}

    const std::string& item_id_t::to_name() const {
        return slot_data::get_slot_data(id).id;
    }

    static_slot_data& item_id_t::get_data() const {
        return slot_data::get_slot_data(id);
    }

    slot_data slot_data::create_item(const std::string& id, int32_t count) {
        try {
            auto res = named_full_item_data.at(id);
            return slot_data{
                .components = res->default_components,
                .count = count,
                .id = res->internal_id,
            };
        } catch (...) {
            throw std::runtime_error("Item not found: " + id);
        }
    }

    slot_data slot_data::create_item(uint32_t id, int32_t count) {
        auto res = full_item_data_.at(id);
        return slot_data{
            .components = res->default_components,
            .count = count,
            .id = res->internal_id,
        };
    }

    static_slot_data& slot_data::get_slot_data(const std::string& id) {
        return *named_full_item_data.at(id);
    }

    static_slot_data& slot_data::get_slot_data(uint32_t id) {
        return *full_item_data_.at(id);
    }

    static_slot_data& slot_data::get_slot_data() {
        return *full_item_data_.at(id);
    }

    void slot_data::add_slot_data(static_slot_data&& move) {
        if (named_full_item_data.contains(move.id))
            throw std::runtime_error("Slot data already registered, \"" + move.id + '"');
        auto moved = std::make_shared<static_slot_data>(std::move(move));
        named_full_item_data[moved->id] = moved;
        full_item_data_.push_back(moved);
        moved->internal_id = full_item_data_.size() - 1;
    }

    void slot_data::enumerate_slot_data(const std::function<void(static_slot_data&)>& fn) {
        for (auto& block : full_item_data_)
            fn(*block);
    }
}