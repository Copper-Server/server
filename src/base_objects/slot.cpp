#include "slot.hpp"
#include "../registers.hpp"

namespace crafted_craft {
    namespace base_objects {


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

        union HideFlags {
            struct {
                bool enchants : 1;     //1
                bool attributes : 1;   //2
                bool unbreakable : 1;  //4
                bool can_destroy : 1;  //8
                bool can_place_on : 1; //16
                bool other : 1;        //32
                bool dyed : 1;         //64
                bool armor_trim : 1;   //128
            };

            int32_t value;
        };

        template <class T>
        T& view(slot_component::unified& it) {
            return std::get<T>(it);
        }

        template <class T>
        T& view_or_else(slot_component::unified& it, const T& assign) {
            try {
                return std::get<T>(it);
            } catch (...) {
                return std::get<T>(it = assign);
            }
        }

        template <class T>
        T& view_or_default(slot_component::unified& it) {
            try {
                return std::get<T>(it);
            } catch (...) {
                return std::get<T>(it = T());
            }
        }

        slot_data slot_data_storage::unpack() const {
            slot_data slot;
            slot.id = id;
            slot.count = count;
            if (!nbt)
                return slot;
            HideFlags hide_flags;
            hide_flags.value = nbt->contains("HideFlags") ? (int32_t)(*nbt)["HideFlags"] : 0;

            auto view = enbt::compound::make_ref(*nbt);
            for (auto& [name, value] : view) {
                if (name == "HideFlags")
                    continue;
                else if (name == "Damage")
                    slot.components["damage"]
                        = slot_component::damage{(int32_t)value};
                else if (name == "Unbreakable")
                    slot.components["unbreakable"]
                        = slot_component::unbreakable{};
                else if (name == "CanDestroy") {
                    //TODO
                } else if (name == "CustomModelData")
                    slot.components["custom_model_data"]
                        = slot_component::custom_model_data{(int32_t)value};
                else if (name == "AttributeModifiers") {
                    auto modifiers = enbt::fixed_array::make_ref(value);
                    slot_component::attribute_modifiers build_modifiers;
                    for (auto& modifier : modifiers) {
                        int32_t type_id = item_attribute::attribute_name_to_id(modifier["AttributeName"]);
                        enbt::raw_uuid uid = modifier["UUID"];
                        std::string name = (std::string)modifier["AttributeName"];
                        double value = (double)modifier["Amount"];
                        auto operation = item_attribute::id_to_operation((int32_t)modifier["Operation"]);
                        auto slot = item_attribute::name_to_slot((std::string)modifier["Slot"]);

                        build_modifiers.attributes.push_back(item_attribute{
                            type_id,
                            uid,
                            name,
                            value,
                            operation,
                            slot
                        });
                    }

                    build_modifiers.show_in_tooltip = !hide_flags.attributes;
                    slot.components["attribute_modifiers"] = std::move(build_modifiers);
                } else if (name == "CanPlaceOn") {
                    //TODO
                } else if (name == "BlockEntityTag")
                    slot.components["block_entity_data"]
                        = slot_component::block_entity_data{value};

                else if (name == "BlockStateTag") {
                    auto states = enbt::compound::make_ref(value);
                    slot_component::block_state state;
                    for (auto& [key, value] : states)
                        state.properties.push_back({key, (std::string)value});
                    slot.components["block_state"] = std::move(state);
                } else if (name == "display") {
                    auto display = enbt::compound::make_ref(value);
                    if (display.contains("Name"))
                        slot.components["custom_name"] = slot_component::custom_name{Chat::fromEnbt(display["Name"])};
                    if (display.contains("Lore")) {
                        auto lore = enbt::fixed_array::make_ref(display["Lore"]);
                        list_array<Chat> lores;
                        for (auto& line : lore)
                            lores.push_back(Chat::fromEnbt(line));
                        slot.components["lore"] = slot_component::lore{std::move(lores)};
                    }
                    if (display.contains("Color"))
                        slot.components["dyed_color"] = slot_component::dyed_color{(int32_t)display["Color"]};
                } else if (name == "Enchantments") {
                    auto enchantments = enbt::fixed_array::make_ref(value);
                    slot_component::enchantments build_enchantments;

                    for (auto& enchantment : enchantments) {
                        build_enchantments.enchants.push_back(
                            {0, //(std::string)enchantment["id"],
                             (int32_t)enchantment["lvl"]}
                        );
                    }
                    slot.components["enchantments"] = std::move(build_enchantments);
                } else if (name == "custom_potion_effects") {
                    auto custom_potion_effects = enbt::fixed_array::make_ref(value);
                    slot_component::potion_contents& potion_contents = view_or_default<slot_component::potion_contents>(slot.components["custom_potion_effects"]);
                    for (auto& effect : custom_potion_effects) {
                        potion_contents
                            .custom_effects
                            .push_back(
                                item_potion_effect{
                                    0, //(std::string)effect["id"],
                                    item_potion_effect::effect_data{
                                        (int32_t)effect["duration"],
                                        (bool)effect["ambient"],
                                        (bool)effect["show_particles"],
                                        (bool)effect["show_icon"],
                                    }
                                }
                            );
                    }
                } else if (name == "Potion") {
                    slot_component::potion_contents& potion_contents = view_or_default<slot_component::potion_contents>(slot.components["custom_potion_effects"]);
                    potion_contents.potion_id = 0; //(std::string)value,
                } else if (name == "CustomPotionColor") {
                    slot_component::potion_contents& potion_contents = view_or_default<slot_component::potion_contents>(slot.components["custom_potion_effects"]);
                    potion_contents.color_rgb = (int32_t)value;
                } else if (name == "Trim") {
                    //slot.components["trim"] =  slot_component::trim{
                    //    value[]
                    //}
                } else if (name == "EntityTag") {
                    slot.components["entity_data"] = slot_component::entity_data(value);
                } else if (name == "pages") {
                    auto pages = enbt::fixed_array::make_ref(value);
                    list_array<std::string> res;
                }
            }
            return slot;
        }

        bool slot_data::operator==(const slot_data& other) const {
            //TODO
            return false;
        }

        bool slot_data::operator!=(const slot_data& other) const {
            return operator==(other);
        }

        slot_data_storage slot_data::pack() const {
            return slot_data_storage{};
        }
    }
}