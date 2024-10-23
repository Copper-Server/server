
#include "../../../api/command.hpp"
#include "../../../base_objects/slot.hpp"
#include "../../../util/readers.hpp"

namespace crafted_craft {
    namespace packets {
        namespace release_765 {
            namespace reader {
                using namespace base_objects;

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

                struct slot_data_old {
                    std::optional<enbt::value> nbt;
                    int32_t id = 0;
                    uint8_t count = 0;
                    slot_data to_new() const;
                };

                slot_data slot_data_old::to_new() const {
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
                            auto blocks = enbt::fixed_array::make_ref(value);
                            list_array<slot_component::inner::block_predicate> res;
                            for (auto& block : blocks) {
                                api::command::get_manager().parse_string(parsers::command::block(), block).and_then([&](parser& bl) {
                                    auto& parsed = std::get<parsers::block>(bl);
                                    slot_component::inner::block_predicate inner_res = enbt::compound();
                                    inner_res["block"] = parsed.block_id;
                                    if (parsed.states.size()) {
                                        enbt::compound states;
                                        for (auto& [key, value] : parsed.states)
                                            states[key] = value;
                                        inner_res["properties"] = states;
                                    }
                                    res.push_back(std::move(inner_res));
                                });
                            }
                            slot.components["can_destroy"] = slot_component::can_break{std::move(res), hide_flags.can_destroy};
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
                            auto blocks = enbt::fixed_array::make_ref(value);
                            list_array<slot_component::inner::block_predicate> res;
                            for (auto& block : blocks) {
                                api::command::get_manager().parse_string(parsers::command::block(), block).and_then([&](parser& bl) {
                                    auto& parsed = std::get<parsers::block>(bl);
                                    slot_component::inner::block_predicate inner_res = enbt::compound();
                                    inner_res["block"] = parsed.block_id;
                                    if (parsed.states.size()) {
                                        enbt::compound states;
                                        for (auto& [key, value] : parsed.states)
                                            states[key] = value;
                                        inner_res["properties"] = states;
                                    }
                                    res.push_back(std::move(inner_res));
                                });
                            }
                            slot.components["can_place_on"] = slot_component::can_place_on{std::move(res), hide_flags.can_place_on};

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
                        } else if (name == "StoredEnchantments") {
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

                slot_data_old to_old(const slot_data& new_slot) {
                    enbt::compound res;
                    HideFlags hide_flags;
                    hide_flags.value = 0;
                    bool add_flags = false;

                    for (auto& [key, value] : new_slot.components) {
                        if (key == "damage")
                            res["Damage"] = std::get<slot_component::damage>(value).value;
                        else if (key == "unbreakable")
                            res["Unbreakable"] = true;
                        else if (key == "can_destroy") {
                            enbt::fixed_array arr;
                            hide_flags.can_destroy = std::get<slot_component::can_break>(value).show_in_tooltip;
                            add_flags = true;
                            for (auto& predicate : std::get<slot_component::can_break>(value).predicates) {
                                std::string block = predicate.at("block");
                                auto properties = enbt::compound::make_ref(predicate.at("properties"));
                                if (properties.size()) {
                                    block.push_back('[');
                                    for (auto& [key, value] : properties)
                                        block += key + ':' + (std::string)value + ',';
                                    block.pop_back();
                                    block.push_back(']');
                                }
                                arr.push_back(std::move(block));
                            }
                            res["CanDestroy"] = std::move(arr);
                        } else if (key == "custom_model_data")
                            res["CustomModelData"] = std::get<slot_component::custom_model_data>(value).value;
                        else if (key == "attribute_modifiers") {
                            enbt::fixed_array attributes;
                            hide_flags.attributes = std::get<slot_component::attribute_modifiers>(value).show_in_tooltip;
                            add_flags = true;
                            for (auto& modifier : std::get<slot_component::attribute_modifiers>(value).attributes) {
                                enbt::compound mod;
                                mod["AttributeName"] = modifier.name;
                                mod["UUID"] = modifier.uid;
                                mod["Amount"] = modifier.value;
                                mod["Operation"] = item_attribute::operation_to_id(modifier.operation);
                                mod["Slot"] = item_attribute::slot_to_name(modifier.slot);
                                attributes.push_back(std::move(mod));
                            }
                            res["AttributeModifiers"] = std::move(attributes);
                        } else if (key == "can_place_on") {
                            hide_flags.can_place_on = std::get<slot_component::can_place_on>(value).show_in_tooltip;
                            add_flags = true;
                            enbt::fixed_array arr;
                            for (auto& predicate : std::get<slot_component::can_place_on>(value).predicates) {
                                std::string block = predicate.at("block");
                                auto properties = enbt::compound::make_ref(predicate.at("properties"));
                                if (properties.size()) {
                                    block.push_back('[');
                                    for (auto& [key, value] : properties)
                                        block += key + ':' + (std::string)value + ',';
                                    block.pop_back();
                                    block.push_back(']');
                                }
                                arr.push_back(std::move(block));
                            }
                            res["CanPlaceOn"] = std::move(arr);
                        } else if (key == "block_entity_data")
                            res["BlockEntityTag"] = std::get<slot_component::block_entity_data>(value).value;
                        else if (key == "block_state") {
                            enbt::compound states;
                            for (auto& [key, value] : std::get<slot_component::block_state>(value).properties)
                                states[key] = (std::string)value;
                            res["BlockStateTag"] = std::move(states);
                        } else if (name == "display") {
                            auto display = enbt::compound::make_ref(value);
                            if (display.contains("Name"))
                                res["custom_name"] = slot_component::custom_name{Chat::fromEnbt(display["Name"])};
                            if (display.contains("Lore")) {
                                auto lore = enbt::fixed_array::make_ref(display["Lore"]);
                                list_array<Chat> lores;
                                for (auto& line : lore)
                                    lores.push_back(Chat::fromEnbt(line));
                                res["lore"] = slot_component::lore{std::move(lores)};
                            }
                            if (display.contains("Color"))
                                res["dyed_color"] = slot_component::dyed_color{(int32_t)display["Color"]};
                        } else if (key == "enchantments") {
                            auto enchantments = enbt::fixed_array::make_ref(value);
                            slot_component::enchantments build_enchantments;

                            for (auto& enchantment : enchantments) {
                                build_enchantments.enchants.push_back(
                                    {0, //(std::string)enchantment["id"],
                                     (int32_t)enchantment["lvl"]}
                                );
                            }
                            res["Enchantments"] = std::move(build_enchantments);
                        } else if (name == "custom_potion_effects") {
                            auto custom_potion_effects = enbt::fixed_array::make_ref(value);
                            slot_component::potion_contents& potion_contents = view_or_default<slot_component::potion_contents>(res["custom_potion_effects"]);
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

                            slot_component::potion_contents& potion_contents = view_or_default<slot_component::potion_contents>(res["custom_potion_effects"]);
                            potion_contents.potion_id = 0; //(std::string)value,
                        } else if (name == "CustomPotionColor") {
                            slot_component::potion_contents& potion_contents = view_or_default<slot_component::potion_contents>(res["custom_potion_effects"]);
                            potion_contents.color_rgb = (int32_t)value;
                        } else if (name == "Trim") {
                            //res["trim"] =  slot_component::trim{
                            //    value[]
                            //}
                        } else if (name == "EntityTag") {
                            res["entity_data"] = slot_component::entity_data(value);
                        } else if (name == "pages") {
                            auto pages = enbt::fixed_array::make_ref(value);
                            list_array<std::string> res;
                        }
                    }
                }


                void WriteSlotItem(list_array<uint8_t>& data, const base_objects::slot& slot) {
                    auto converted = to_old(*slot);
                    WriteVar<int32_t>(converted.id, data);
                    data.push_back(converted.count);
                    if (converted.nbt)
                        data.push_back(NBT::build(converted.nbt.value()).get_as_network());
                    else
                        data.push_back(0); //TAG_End
                }

                void WriteSlot(list_array<uint8_t>& data, const base_objects::slot& slot) {
                    data.push_back((bool)slot);
                    if (slot) {
                        WriteSlotItem(data, slot);
                    }
                }

                base_objects::slot ReadSlotItem(ArrayStream& data) {
                    slot_data_old old_slot;
                    old_slot.id = ReadVar<int32_t>(data);
                    old_slot.count = data.read();
                    if (data.peek() == 0) {
                        size_t readed = 0;
                        old_slot.nbt = NBT::extract_from_array(data.data_read(), readed, data.size_read());
                        data.r += readed;
                    }
                    return old_slot.to_new();
                }

                base_objects::slot ReadSlot(ArrayStream& data) {
                    if (!data.read())
                        return std::nullopt;
                    else
                        return ReadSlotItem(data);
                }
            }
        }
    }
}
