#include "slot.hpp"
#include "../registers.hpp"

#include "../api/command.hpp"

namespace crafted_craft {
    namespace base_objects {
        bool item_potion_effect::effect_data::operator==(const effect_data& other) const {
            if (ambient != other.ambient
                | duration != other.duration
                | show_icon != other.show_icon
                | show_particles != other.show_particles
                | (!hidden_effect != !other.hidden_effect))
                return false;

            if (hidden_effect) {
                if (this == &other)
                    return true;
                if (!(*hidden_effect == *other.hidden_effect))
                    return false;
            }
            return true;
        }

        bool slot_component::bundle_contents::operator==(const bundle_contents& other) const {
            return items.equal(
                other.items,
                [](const slot_data* it, const slot_data* second) {
                    return *it == *second;
                }
            );
        }

        slot_component::container::container(const container& other) {
            for (int i = 0; i < 256; i++)
                items[i] = other.items[i] ? new slot_data(*other.items[i]) : nullptr;
        }

        slot_component::container& slot_component::container::operator=(const container& other) {
            for (int i = 0; i < 256; i++)
                items[i] = other.items[i] ? new slot_data(*other.items[i]) : nullptr;
            return *this;
        }

        slot_component::container::~container() {
            for (int i = 0; i < 256; i++) {
                delete items[i];
            }
        }

        void slot_component::container::set(uint8_t slot, slot_data&& item) {
            if (slot < 256) {
                if (items[slot])
                    delete items[slot];
                items[slot] = new slot_data(std::move(item));
            } else
                throw std::runtime_error("Slot out of range");
        }

        void slot_component::container::set(uint8_t slot, const slot_data& item) {
            if (slot < 256) {
                if (items[slot])
                    delete items[slot];
                items[slot] = new slot_data(item);
            } else
                throw std::runtime_error("Slot out of range");
        }

        slot_data* slot_component::container::get(uint8_t slot) {
            if (slot < 256) {
                if (items[slot])
                    return items[slot];
                else
                    return nullptr;
            } else
                throw std::runtime_error("Slot out of range");
        }

        bool slot_component::container::contains(uint8_t slot) {
            if (slot < 256)
                return items[slot] != nullptr;
            else
                throw std::runtime_error("Slot out of range");
        }

        std::optional<uint8_t> slot_component::container::contains(const slot_data& item) {
            std::optional<uint8_t> res;
            for_each([&res, &item](const slot_data& check, size_t index) {
                if (res) {
                    if (item == check)
                        res = (uint8_t)index;
                }
            });
            return res;
        }

        list_array<uint8_t> slot_component::container::contains(const std::string& id, size_t count) {
            list_array<uint8_t> res;
            int32_t real_id = slot_data::get_slot_data(id).internal_id;
            for_each([&res, real_id, &count](const slot_data& check, size_t index) {
                if (count) {
                    if (check.id == real_id) {
                        res.push_back(index);
                        if (count > check.count)
                            count -= check.count;
                        else
                            count = 0;
                    }
                }
            });
            return res;
        }

        void slot_component::container::remove(uint8_t slot) {
            if (slot < 256) {
                if (items[slot])
                    delete items[slot];
                items[slot] = nullptr;
            } else
                throw std::runtime_error("Slot out of range");
        }

        void slot_component::container::clear() {
            for (int i = 0; i < 256; i++) {
                if (items[i])
                    delete items[i];
                items[i] = nullptr;
            }
        }

        bool slot_component::container::operator==(const container& other) const {
            for (int i = 0; i < 256; i++) {
                if (!items[i] != !other.items[i])
                    return false;
                if (items[i])
                    if (*items[i] != *other.items[i])
                        return false;
            }
            return true;
        }


        std::string item_attribute::id_to_attribute_name(int32_t id) {
            switch (id) {
            case 0:
                return "generic.armor";
            case 1:
                return "generic.armor_toughness";
            case 2:
                return "generic.attack_damage";
            case 3:
                return "generic.attack_knockback";
            case 4:
                return "generic.attack_speed";
            case 5:
                return "generic.block_break_speed";
            case 6:
                return "generic.block_interaction_range";
            case 7:
                return "generic.entity_interaction_range";
            case 8:
                return "generic.fall_damage_multiplier";
            case 9:
                return "generic.flying_speed";
            case 10:
                return "generic.follow_range";
            case 11:
                return "generic.gravity";
            case 12:
                return "generic.jump_strength";
            case 13:
                return "generic.knockback_resistance";
            case 14:
                return "generic.luck";
            case 15:
                return "generic.max_absorption";
            case 16:
                return "generic.max_health";
            case 17:
                return "generic.movement_speed";
            case 18:
                return "generic.safe_fall_distance";
            case 19:
                return "generic.scale";
            case 20:
                return "generic.spawn_reinforcements";
            case 21:
                return "generic.step_height";
            case 22:
                return "generic.submerged_mining_speed";
            case 23:
                return "generic.sweeping_damage_ratio";
            case 24:
                return "generic.water_movement_efficiency";
            default:
                throw std::runtime_error("Unknown attribute id");
            }
        }

        int32_t item_attribute::attribute_name_to_id(const std::string& name) {
            if (name == "generic.armor")
                return 0;
            else if (name == "generic.armor_toughness")
                return 1;
            else if (name == "generic.attack_damage")
                return 2;
            else if (name == "generic.attack_knockback")
                return 3;
            else if (name == "generic.attack_speed")
                return 4;
            else if (name == "generic.block_break_speed")
                return 5;
            else if (name == "generic.block_interaction_range")
                return 6;
            else if (name == "generic.entity_interaction_range")
                return 7;
            else if (name == "generic.fall_damage_multiplier")
                return 8;
            else if (name == "generic.flying_speed")
                return 9;
            else if (name == "generic.follow_range")
                return 10;
            else if (name == "generic.gravity")
                return 11;
            else if (name == "generic.jump_strength")
                return 12;
            else if (name == "generic.knockback_resistance")
                return 13;
            else if (name == "generic.luck")
                return 14;
            else if (name == "generic.max_absorption")
                return 15;
            else if (name == "generic.max_health")
                return 16;
            else if (name == "generic.movement_speed")
                return 17;
            else if (name == "generic.safe_fall_distance")
                return 18;
            else if (name == "generic.scale")
                return 19;
            else if (name == "generic.spawn_reinforcements")
                return 20;
            else if (name == "generic.step_height")
                return 21;
            else if (name == "generic.submerged_mining_speed")
                return 22;
            else if (name == "generic.sweeping_damage_ratio")
                return 23;
            else if (name == "generic.water_movement_efficiency")
                return 24;
            else
                throw std::runtime_error("Unknown attribute name");
        }

        int32_t item_attribute::operation_to_id(operation_e id) {
            switch (id) {
            case operation_e::add:
                return 0;
            case operation_e::multiply_base:
                return 1;
            case operation_e::multiply_total:
                return 2;
            default:
                throw std::runtime_error("Unknown operation id");
            }
        }

        int32_t item_attribute::operation_to_id(const std::string& id) {
            if (id == "add")
                return 0;
            else if (id == "multiply_base")
                return 1;
            else if (id == "multiply_total")
                return 2;
            else
                throw std::runtime_error("Unknown operation id");
        }

        item_attribute::operation_e item_attribute::id_to_operation(int32_t id) {
            switch (id) {
            case 0:
                return operation_e::add;
            case 1:
                return operation_e::multiply_base;
            case 2:
                return operation_e::multiply_total;
            default:
                throw std::runtime_error("Unknown operation id");
            }
        }

        item_attribute::operation_e item_attribute::id_to_operation(const std::string& id) {
            if (id == "add")
                return operation_e::add;
            else if (id == "multiply_base")
                return operation_e::multiply_base;
            else if (id == "multiply_total")
                return operation_e::multiply_total;
            else
                throw std::runtime_error("Unknown operation id");
        }

        std::string item_attribute::id_to_operation_string(int32_t id) {
            switch (id) {
            case 0:
                return "add";
            case 1:
                return "multiply_base";
            case 2:
                return "multiply_total";
            default:
                throw std::runtime_error("Unknown operation id");
            }
        }

        std::string item_attribute::slot_to_name(slot_filter id) {
            switch (id) {
            case slot_filter::any:
                return "any";
            case slot_filter::main_hand:
                return "mainhand";
            case slot_filter::off_hand:
                return "offhand";
            case slot_filter::any_hand:
                return "hand";
            case slot_filter::feet:
                return "feet";
            case slot_filter::legs:
                return "legs";
            case slot_filter::chest:
                return "chest";
            case slot_filter::head:
                return "head";
            case slot_filter::armor:
                return "armor";
            case slot_filter::body:
                return "body";
            default:
                throw std::runtime_error("Unknown slot id");
            }
        }

        item_attribute::slot_filter item_attribute::name_to_slot(const std::string& id) {
            if (id == "any")
                return slot_filter::any;
            else if (id == "mainhand")
                return slot_filter::main_hand;
            else if (id == "offhand")
                return slot_filter::off_hand;
            else if (id == "hand")
                return slot_filter::any_hand;
            else if (id == "feet")
                return slot_filter::feet;
            else if (id == "legs")
                return slot_filter::legs;
            else if (id == "chest")
                return slot_filter::chest;
            else if (id == "head")
                return slot_filter::head;
            else if (id == "armor")
                return slot_filter::armor;
            else if (id == "body")
                return slot_filter::body;
            else
                throw std::runtime_error("Unknown slot name");
        }

        namespace slot_component {
            use_remainder::use_remainder(slot_data&& consume)
                : proxy_value(new slot_data(std::move(consume))) {}

            use_remainder::~use_remainder() {
                delete proxy_value;
            }
        }

        bool slot_data::operator==(const slot_data& other) const {
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

        bool slot_data::operator!=(const slot_data& other) const {
            return operator==(other);
        }

        void static_slot_data::reset_items() {
            slot_data::named_full_item_data.clear();
            slot_data::full_item_data_.clear();
        }

        void static_slot_data::initialize_items() {
            uint32_t id = 0;
            for (auto& item_ : slot_data::full_item_data_) {
                auto& item = *item_;
                item.internal_item_aliases.clear();
                for (auto& [protocol, assignations] : internal_item_aliases_protocol) {
                    if (assignations.find(item.id) != assignations.end()) {
                        item.internal_item_aliases[protocol] = assignations[item.id];
                    } else {
                        bool found = false;
                        for (auto& alias : item.item_aliases) {
                            if (assignations.find(alias.get()) != assignations.end()) {
                                item.internal_item_aliases[protocol] = assignations[alias.get()];
                                found = true;
                                break;
                            }
                        }
                        if (!found)
                            throw std::runtime_error("Item alias for " + item.id + '[' + std::to_string(id) + " not found in protocol " + std::to_string(protocol));
                    }
                }
                ++id;
            }
        }

        slot_data slot_data::create_item(const base_objects::shared_string& id) {
            try {
                auto res = named_full_item_data.at(id);
                return slot_data{
                    .components = res->default_components,
                    .count = 1,
                    .id = res->internal_id,
                };
            } catch (...) {
                throw std::runtime_error("Item not found: " + id.get());
            }
        }

        slot_data slot_data::create_item(uint32_t id) {
            auto res = full_item_data_.at(id);
            return slot_data{
                .components = res->default_components,
                .count = 1,
                .id = res->internal_id,
            };
        }

        static_slot_data& slot_data::get_slot_data(const base_objects::shared_string& id) {
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

        enbt::compound slot_data::to_enbt() const {
            return {};
        }

        slot_data slot_data::from_enbt(const enbt::compound_const_ref& compound) {
            return {};
        }
    }
}