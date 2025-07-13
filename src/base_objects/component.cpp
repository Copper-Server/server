#include <library/enbt/senbt.hpp>
#include <src/api/command.hpp>
#include <src/api/tags.hpp>
#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>
#include <src/registers.hpp>

namespace copper_server::base_objects {

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
        if (id == "add" || id == "add_value")
            return operation_e::add;
        else if (id == "multiply_base" || id == "add_multiplied_base")
            return operation_e::multiply_base;
        else if (id == "multiply_total" || id == "add_multiplied_total ")
            return operation_e::multiply_total;
        else
            throw std::runtime_error("Unknown operation id");
    }

    std::string item_attribute::id_to_operation_string(int32_t id) {
        switch (id) {
        case 0:
            return "add_value";
        case 1:
            return "add_multiplied_base";
        case 2:
            return "add_multiplied_total";
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

}

namespace copper_server::base_objects::component {
    bool bundle_contents::operator==(const bundle_contents& other) const {
        return items.equal(
            other.items,
            [](const slot_data* it, const slot_data* second) {
                return *it == *second;
            }
        );
    }

    bool charged_projectiles::operator==(const charged_projectiles& other) const {
        return data.equal(
            other.data,
            [](const slot_data* it, const slot_data* second) {
                return *it == *second;
            }
        );
    }

    container::container(const container& other) {
        for (int i = 0; i < 256; i++)
            items[i] = other.items[i] ? new slot_data(*other.items[i]) : nullptr;
    }

    container::container(container&& other) {
        for (int i = 0; i < 256; i++) {
            items[i] = other.items[i];
            other.items[i] = nullptr;
        }
    }

    container& container::operator=(const container& other) {
        for (int i = 0; i < 256; i++)
            items[i] = other.items[i] ? new slot_data(*other.items[i]) : nullptr;
        return *this;
    }

    container& container::operator=(container&& other) {
        for (int i = 0; i < 256; i++) {
            items[i] = other.items[i];
            other.items[i] = nullptr;
        }
        return *this;
    }

    container::~container() {
        for (int i = 0; i < 256; i++)
            if (items[i]) {
                delete items[i];
                items[i] = nullptr;
            }
    }

    std::optional<uint8_t> container::get_free_slot() {
        for (uint8_t i = 0; i < 256; i++)
            if (!items[i])
                return i;
        return std::nullopt;
    }

    int32_t container::add(const slot_data& item) {
        if (!item.count)
            return 0;
        int64_t to_add = item.count;
        auto max_count = item.get_component<max_stack_size>().value;
        if (max_count == 0)
            return 0;

        for (int slot = 0; slot < 256; slot++) {
            if (to_add <= 0)
                return item.count;
            if (items[slot]) {
                auto& to_insert = *items[slot];
                if (to_insert.id == item.id) {
                    if (to_insert.is_same_def(item)) {
                        int64_t add_res = int64_t(max_count) - to_insert.count;
                        if (add_res > 0) {
                            add_res = std::min(add_res, to_add);
                            to_insert.count += add_res;
                            to_add -= add_res;
                        }
                    }
                }
            }
        }

        auto find_slot = get_free_slot();
        if (!find_slot)
            return to_add - item.count;

        set(*find_slot, item);
        get(*find_slot).count = to_add;
        return item.count;
    }

    void container::set(uint8_t slot, slot_data&& item) {
        if (slot < 256) {
            if (items[slot])
                *items[slot] = std::move(item);
            else
                items[slot] = new slot_data(std::move(item));
        } else
            throw std::runtime_error("Slot out of range");
    }

    void container::set(uint8_t slot, const slot_data& item) {
        if (slot < 256) {
            if (items[slot])
                *items[slot] = item;
            else
                items[slot] = new slot_data(item);
        } else
            throw std::runtime_error("Slot out of range");
    }

    slot_data& container::get(uint8_t slot) {
        if (slot < 256) {
            if (items[slot])
                return *items[slot];
            else
                throw std::runtime_error("Slot is empty");
        } else
            throw std::runtime_error("Slot out of range");
    }

    bool container::contains(uint8_t slot) {
        if (slot < 256)
            return items[slot] != nullptr;
        else
            throw std::runtime_error("Slot out of range");
    }

    std::optional<uint8_t> container::contains(const slot_data& item) const {
        std::optional<uint8_t> res;
        for_each([&res, &item](const slot_data& check, size_t index) {
            if (res) {
                if (item == check)
                    res = (uint8_t)index;
            }
        });
        return res;
    }

    list_array<uint8_t> container::contains(const std::string& id, size_t count) const {
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

    void container::remove(uint8_t slot) {
        if (slot < 256) {
            if (items[slot])
                delete items[slot];
            items[slot] = nullptr;
        } else
            throw std::runtime_error("Slot out of range");
    }

    void container::clear(int32_t id, size_t count) {
        list_array<uint8_t> remove_slots;
        if (count == (size_t)-1) {
            for_each([&](auto& item, auto slot) {
                if (item.id == id)
                    remove_slots.push_back(slot);
            });
        } else {
            for_each([&](auto& item, auto slot) {
                if (item.id == id && count) {
                    if (item.count <= count) {
                        count -= item.count;
                        remove_slots.push_back(slot);
                    } else {
                        item.count -= count;
                        count = 0;
                    }
                }
            });
        }

        for (auto i : remove_slots)
            remove(i);
    }

    void container::clear() {
        for (int i = 0; i < 256; i++) {
            if (items[i])
                delete items[i];
            items[i] = nullptr;
        }
    }

    uint8_t container::count() const {
        uint8_t count = 0;
        for (int i = 0; i < 256; i++)
            count += (bool)(items[i]);
        return count;
    }

    bool container::operator==(const container& other) const {
        for (int i = 0; i < 256; i++) {
            if (!items[i] != !other.items[i])
                return false;
            if (items[i])
                if (*items[i] != *other.items[i])
                    return false;
        }
        return true;
    }

    use_remainder::use_remainder(slot_data&& consume)
        : proxy_value(new slot_data(std::move(consume))) {}

    use_remainder::use_remainder(weak_slot_data&& consume)
        : proxy_value(std::move(consume)) {}

    use_remainder::~use_remainder() {
        std::visit([](auto&& value) {
            if constexpr (std::is_same_v<slot_data, std::decay_t<decltype(value)>>)
                delete value;
        },
                   proxy_value);
    }

    bool use_remainder::operator==(const use_remainder& other) const {
        return std::visit([](auto&& value, auto&& other_value) {
            using ValueT = std::decay_t<decltype(value)>;
            using OtherValueT = std::decay_t<decltype(other_value)>;
            if constexpr (std::is_same_v<ValueT, OtherValueT>) {
                if constexpr (std::is_same_v<slot_data, ValueT>) {
                    return *value == *other_value;

                } else
                    return value == other_value;
            } else
                return false;
        },
                          proxy_value,
                          other.proxy_value);
    }

    std::string rarity::to_string() const {
        switch (value) {
        case common:
            return "common";
        case uncommon:
            return "uncommon";
        case rare:
            return "rare";
        case epic:
            return "epic";
        default:
            return "common";
        }
    }

    rarity rarity::from_string(const std::string& str) {
        if (str == "common")
            return common;
        else if (str == "uncommon")
            return uncommon;
        else if (str == "rare")
            return rare;
        else if (str == "epic")
            return epic;
        else
            return common;
    }

    void potion_contents::set_potion_id(const std::string& id) {
        set_potion_id((int32_t)registers::potions.at(id).id);
    }

    void potion_contents::set_potion_id(int32_t id) {
        if (std::holds_alternative<int32_t>(value))
            value = id;
        else
            std::get<full>(value).potion_id = id;
    }

    void potion_contents::set_custom_color(int32_t rgb) {
        if (std::holds_alternative<int32_t>(value))
            value = full{.potion_id = std::get<int32_t>(value), .color_rgb = rgb};
        else
            std::get<full>(value).color_rgb = rgb;
    }

    void potion_contents::clear_custom_color() {
        if (std::holds_alternative<int32_t>(value))
            return;
        else {
            auto& it = std::get<full>(value);
            it.color_rgb.reset();
            if (it.custom_effects.empty() && !it.custom_name && it.potion_id)
                value = *it.potion_id;
        }
    }

    void potion_contents::set_custom_name(const std::string& name) {
        if (std::holds_alternative<int32_t>(value))
            value = full{.potion_id = std::get<int32_t>(value), .custom_name = name};
        else
            std::get<full>(value).custom_name = name;
    }

    void potion_contents::clear_custom_name() {
        if (std::holds_alternative<int32_t>(value))
            return;
        else {
            auto& it = std::get<full>(value);
            it.custom_name.reset();
            if (it.custom_effects.empty() && !it.color_rgb && it.potion_id)
                value = *it.potion_id;
        }
    }

    void potion_contents::add_custom_effect(item_potion_effect&& effect) {
        if (std::holds_alternative<int32_t>(value))
            value = full{.potion_id = std::get<int32_t>(value), .custom_effects = {std::move(effect)}};
        else
            std::get<full>(value).custom_effects.push_back(std::move(effect));
    }

    void potion_contents::add_custom_effect(const item_potion_effect& effect) {
        if (std::holds_alternative<int32_t>(value))
            value = full{.potion_id = std::get<int32_t>(value), .custom_effects = {effect}};
        else
            std::get<full>(value).custom_effects.push_back(effect);
    }

    void potion_contents::clear_custom_effects() {
        if (std::holds_alternative<int32_t>(value))
            return;
        else {
            auto& it = std::get<full>(value);
            it.custom_effects.clear();
            if (!it.custom_name && !it.color_rgb && it.potion_id)
                value = *it.potion_id;
        }
    }

    void potion_contents::iterate_custom_effects(std::function<void(const item_potion_effect&)> fn) const {
        if (std::holds_alternative<int32_t>(value))
            return;
        else {
            auto& it = std::get<full>(value);
            it.custom_effects.for_each(fn);
        }
    }

    void potion_contents::iterate_custom_effects(std::function<void(size_t)> size_fn, std::function<void(const item_potion_effect&)> fn) const {
        if (std::holds_alternative<int32_t>(value)) {
            size_fn(0);
            return;
        } else {
            auto& it = std::get<full>(value);
            size_fn(it.custom_effects.size());
            it.custom_effects.for_each(fn);
        }
    }

    std::optional<int32_t> potion_contents::get_potion_id() const {
        if (std::holds_alternative<int32_t>(value))
            return std::get<int32_t>(value);
        else
            return std::get<full>(value).potion_id;
    }

    std::optional<int32_t> potion_contents::get_custom_color() const {
        if (std::holds_alternative<int32_t>(value))
            return std::nullopt;
        else
            return std::get<full>(value).color_rgb;
    }

    std::optional<std::string> potion_contents::get_custom_name() const {
        if (std::holds_alternative<int32_t>(value))
            return std::nullopt;
        else
            return std::get<full>(value).custom_name;
    }

    inner::block_predicate parse_block_predicate(enbt::compound_const_ref item) {
        inner::block_predicate pred;
        if (item["blocks"].is_string())
            pred.blocks = item["blocks"].as_string();
        else {
            std::vector<int32_t> arr;
            arr.reserve(item["blocks"].size());
            for (auto& it : item["blocks"].as_array())
                arr.push_back(base_objects::block::get_block(it.as_string()).general_block_id);
            pred.blocks = std::move(arr);
        }

        if (item.contains("state")) {
            std::vector<inner::block_predicate::property> props;
            props.reserve(item["state"].size());
            for (auto& [name, item] : item["state"].as_compound()) {
                inner::block_predicate::property it{.name = name};
                if (item.is_compound())
                    it.match = inner::block_predicate::property::ranged{.min = item["min"].as_string(), .max = item["max"].as_string()};
                else
                    it.match = inner::block_predicate::property::exact{item.as_string()};
                props.push_back(std::move(it));
            }
            pred.properties = std::move(props);
        }
        if (item.contains("nbt"))
            pred.nbt = item["nbt"];
        //TODO data_components
        //TODO partial_data_components
        return pred;
    }

    enbt::compound encode_block_predicate(const inner::block_predicate& item) {
        enbt::compound res;
        if (item.blocks) {
            std::visit(
                [&res](auto& it) {
                    if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                        res["blocks"] = it;
                    } else {
                        enbt::fixed_array arr;
                        arr.reserve(it.size());
                        for (auto& i : it)
                            arr.push_back(base_objects::block::get_general_block(i).name);
                        res["blocks"] = std::move(arr);
                    }
                },
                *item.blocks
            );
        }
        if (item.properties) {
            enbt::compound comp;
            for (auto& it : *item.properties) {
                std::visit(
                    [&comp, &it](auto& item) {
                        if constexpr (std::is_same_v<std::decay_t<decltype(item)>, inner::block_predicate::property::exact>)
                            comp[it.name] = item.value;
                        else
                            comp[it.name] = enbt::compound{{"min", item.min}, {"max", item.max}};
                    },
                    it.match
                );
            }
            res["state"] = std::move(comp);
        }
        if (item.nbt)
            res["nbt"] = *item.nbt;
        //TODO data_components
        //TODO partial_data_components
        return res;
    }

    base_objects::item_potion_effect parse_item_potion_effect(enbt::compound_const_ref ref) {
        base_objects::item_potion_effect eff;
        eff.potion_id = registers::effects.at(ref.at("id")).id;
        if (ref.contains("amplifier"))
            eff.data.amplifier = ref["amplifier"];
        if (ref.contains("duration"))
            eff.data.duration = ref["duration"];
        if (ref.contains("ambient"))
            eff.data.ambient = ref["ambient"];
        if (ref.contains("show_particles"))
            eff.data.show_particles = ref["show_particles"];
        if (ref.contains("show_icon"))
            eff.data.show_icon = ref["show_icon"];
        return eff;
    }

    enbt::compound encode_item_potion_effect(base_objects::item_potion_effect ref) {
        enbt::compound res;
        res["id"] = registers::effects_cache.at(ref.potion_id)->second.name;
        if (ref.data.amplifier)
            res["amplifier"] = ref.data.amplifier;
        if (ref.data.duration)
            res["duration"] = ref.data.duration;
        if (ref.data.ambient)
            res["ambient"] = ref.data.ambient;
        if (ref.data.show_particles)
            res["show_particles"] = ref.data.show_particles;
        if (ref.data.show_icon)
            res["show_icon"] = ref.data.show_icon;
        return res;
    }

    inner::sound_extended parse_sound_extended(const enbt::value& ref) {
        if (ref.is_string())
            return inner::sound_extended{.sound_name = ref.as_string()};
        inner::sound_extended sound;
        sound.sound_name = ref.at("sound").as_string();
        if (ref.contains("range"))
            sound.fixed_range = ref["range"];
        return sound;
    }

    std::variant<std::string, inner::sound_extended> parse_sound(const enbt::value& ref) {
        if (ref.is_string())
            return ref.as_string();
        else
            return parse_sound_extended(ref);
    }

    enbt::value encode_sound_extended(const inner::sound_extended& ref) {
        enbt::compound res;
        res["sound"] = ref.sound_name;
        if (ref.fixed_range)
            res["range"] = *ref.fixed_range;
        return res;
    }

    enbt::value encode_sound(const std::variant<std::string, inner::sound_extended>& ref) {
        if (std::holds_alternative<std::string>(ref))
            return std::get<std::string>(ref);
        else
            return encode_sound_extended(std::get<inner::sound_extended>(ref));
    }

    inner::application_effect parse_application_effect(enbt::compound_const_ref ref) {
        auto type = ref.at("type").as_string();
        if (type == "minecraft:apply_effects") {
            inner::apply_effects ap_ef;
            for (auto& it : ref.at("effects").as_array())
                ap_ef.effects.push_back(parse_item_potion_effect(it.as_compound()));
            ap_ef.effects.commit();
            if (ref.contains("probability"))
                ap_ef.probability = ref["probability"];
            return ap_ef;
        } else if (type == "minecraft:remove_effects") {
            inner::remove_effects rem_ef;
            if (ref.at("effects").is_string())
                rem_ef.effects = ref.at("effects").as_string();
            else {
                list_array<std::string> arr;
                for (auto& it : ref.at("effects").as_array())
                    arr.push_back(it.as_string());
                arr.commit();
                rem_ef.effects = arr;
            }
            return rem_ef;
        } else if (type == "minecraft:clear_all_effects")
            return inner::clear_all_effects{};
        else if (type == "minecraft:teleport_randomly") {
            if (ref.contains("diameter"))
                return inner::teleport_randomly{ref["diameter"]};
            else
                return inner::teleport_randomly{};
        } else if (type == "minecraft:play_sound")
            return inner::play_sound{.sound = parse_sound_extended(ref.at("sound"))};
        else
            throw std::runtime_error("Not implemented");
    }

    enbt::compound encode_application_effect(const inner::application_effect& ref) {
        enbt::compound res;
        std::visit(
            [&res](auto& it) {
                using T = std::decay_t<decltype(it)>;
                if constexpr (std::is_same_v<T, inner::apply_effects>) {
                    res["type"] = "minecraft:apply_effects";
                    enbt::fixed_array arr;
                    arr.reserve(it.effects.size());
                    for (auto& it : it.effects)
                        arr.push_back(encode_item_potion_effect(it));
                    res["effects"] = std::move(arr);
                    if (it.probability)
                        res["probability"] = it.probability;
                } else if constexpr (std::is_same_v<T, inner::remove_effects>) {
                    res["type"] = "minecraft:remove_effects";
                    std::visit(
                        [&res](auto& it) {
                            if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>)
                                res["effects"] = it;
                            else {
                                enbt::fixed_array arr;
                                arr.reserve(it.size());
                                for (auto& i : it)
                                    arr.push_back(i);
                                res["effects"] = std::move(arr);
                            }
                        },
                        it.effects
                    );
                } else if constexpr (std::is_same_v<T, inner::clear_all_effects>) {
                    res["type"] = "minecraft:clear_all_effects";
                } else if constexpr (std::is_same_v<T, inner::teleport_randomly>) {
                    res["type"] = "minecraft:teleport_randomly";
                    res["diameter"] = it.diameter;
                } else if constexpr (std::is_same_v<T, inner::play_sound>) {
                    res["type"] = "minecraft:play_sound";
                    res["sound"] = encode_sound_extended(it.sound);
                }
            },
            ref
        );
        return res;
    }

    enbt::compound firework_explosion_encoder(item_firework_explosion tmp) {
        enbt::compound res{
            {"has_trail", tmp.trail},
            {"has_twinkle", tmp.twinkle}
        };
        switch (tmp.shape) {
        case base_objects::item_firework_explosion::shape_e::small_ball:
            res["shape"] = "small_ball";
            break;
        case base_objects::item_firework_explosion::shape_e::large_ball:
            res["shape"] = "large_ball";
            break;
        case base_objects::item_firework_explosion::shape_e::star:
            res["shape"] = "star";
            break;
        case base_objects::item_firework_explosion::shape_e::creeper:
            res["shape"] = "creeper";
            break;
        case base_objects::item_firework_explosion::shape_e::burst:
            res["shape"] = "burst";
            break;
        default:
            break;
        }

        {
            enbt::fixed_array arr;
            arr.reserve(tmp.colors.size());
            for (auto& it : tmp.colors)
                arr.push_back(it);
            res["colors"] = std::move(arr);
        }
        {
            enbt::fixed_array arr;
            arr.reserve(tmp.fade_colors.size());
            for (auto& it : tmp.fade_colors)
                arr.push_back(it);
            res["fade_colors"] = std::move(arr);
        }
        return res;
    }

    std::unordered_map<std::string, unified (*)(const enbt::value&)> load_items_parser{
        {"minecraft:attribute_modifiers",
         [](const enbt::value& it) -> unified {
             attribute_modifiers attributes;
             for (auto& i : it.as_array()) {
                 base_objects::item_attribute modifier;
                 auto c = i.as_compound();
                 modifier.type = c.at("type").as_string();
                 modifier.id = c.at("id").as_string();
                 modifier.operation = base_objects::item_attribute::id_to_operation((std::string)c.at("operation").as_string());
                 modifier.amount = c.at("amount");
                 modifier.slot = base_objects::item_attribute::name_to_slot((std::string)c.at("slot").as_string());
                 attributes.value.push_back(modifier);
             }
             attributes.value.commit();
             return std::move(attributes);
         }},
        {"minecraft:banner_patterns",
         [](const enbt::value& it) -> unified {
             banner_patterns banner_patterns;
             for (auto& value : it.as_array()) {
                 auto comp = value.as_compound();
                 banner_patterns::pattern pattern;
                 pattern.color = base_objects::dye_color::from_string(comp.at("color").as_string());
                 auto& pattern_json = comp.at("pattern");
                 if (pattern_json.is_string())
                     pattern.pattern = (std::string)pattern_json.as_string();
                 else {
                     auto custom = pattern_json.as_compound();
                     banner_patterns::custom_pattern c;
                     c.asset_id = custom.at("asset_id").as_string();
                     c.translation_key = custom.at("translation_key").as_string();
                     pattern.pattern = std::move(c);
                 }
                 banner_patterns.value.push_back(pattern);
             }
             banner_patterns.value.commit();
             return std::move(banner_patterns);
         }},
        {"minecraft:base_color",
         [](const enbt::value& it) -> unified {
             auto& color = it.as_string();
             base_color base_color;
             if (color == "white")
                 base_color.color = base_objects::dye_color::white;
             else if (color == "orange")
                 base_color.color = base_objects::dye_color::orange;
             else if (color == "magenta")
                 base_color.color = base_objects::dye_color::magenta;
             else if (color == "light_blue")
                 base_color.color = base_objects::dye_color::light_blue;
             else if (color == "yellow")
                 base_color.color = base_objects::dye_color::yellow;
             else if (color == "lime")
                 base_color.color = base_objects::dye_color::lime;
             else if (color == "pink")
                 base_color.color = base_objects::dye_color::pink;
             else if (color == "gray")
                 base_color.color = base_objects::dye_color::gray;
             else if (color == "light_gray")
                 base_color.color = base_objects::dye_color::light_gray;
             else if (color == "cyan")
                 base_color.color = base_objects::dye_color::cyan;
             else if (color == "purple")
                 base_color.color = base_objects::dye_color::purple;
             else if (color == "blue")
                 base_color.color = base_objects::dye_color::blue;
             else if (color == "brown")
                 base_color.color = base_objects::dye_color::brown;
             else if (color == "green")
                 base_color.color = base_objects::dye_color::green;
             else if (color == "red")
                 base_color.color = base_objects::dye_color::red;
             else if (color == "black")
                 base_color.color = base_objects::dye_color::black;
             else
                 throw std::runtime_error("Unrecognized color");
             return std::move(base_color);
         }},
        {"minecraft:bees",
         [](const enbt::value& it) -> unified {
             bees bees;
             for (auto& bee_ : it.as_array()) {
                 auto bee = bee_.as_compound();
                 bees.values.push_back({bees::bee{
                     .entity_data = bee.at("entity_data"),
                     .ticks_in_hive = (int32_t)bee.at("ticks_in_hive"),
                     .min_ticks_in_hive = (int32_t)bee.at("min_ticks_in_hive"),
                 }});
             }
             bees.values.commit();
             return std::move(bees);
         }},
        {"minecraft:block_entity_data",
         [](const enbt::value& it) -> unified {
             block_entity_data res;
             res.value = it;
             return res;
         }},
        {"minecraft:block_state",
         [](const enbt::value& it) -> unified {
             block_state block_state;
             for (auto& [state, value] : it.as_compound())
                 block_state.properties.push_back({(std::string)state, (std::string)value.as_string()});
             block_state.properties.commit();
             return std::move(block_state);
         }},
        {"minecraft:bucket_entity_data",
         [](const enbt::value& it) -> unified {
             bucket_entity_data res;
             res.value = it;
             return res;
         }},
        {"minecraft:bundle_contents",
         [](const enbt::value& it) -> unified {
             bundle_contents bundle_contents;
             for (auto& item : it.as_array()) {
                 bundle_contents.items.push_back(new base_objects::slot_data(slot_data::from_enbt(item.as_compound())));
             }
             bundle_contents.items.commit();
             return std::move(bundle_contents);
         }},
        {"minecraft:can_break",
         [](const enbt::value& it) -> unified {
             can_break can_break;
             if (it.is_array()) {
                 for (auto& item : it.as_array())
                     can_break.value.push_back(parse_block_predicate(item.as_compound()));
             } else
                 can_break.value.push_back(parse_block_predicate(it.as_compound()));

             can_break.value.commit();
             return std::move(can_break);
         }},
        {"minecraft:can_place_on",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             can_place_on can_place_on;
             if (it.is_array()) {
                 for (auto& it : it.as_array())
                     can_place_on.value.push_back(parse_block_predicate(it.as_compound()));
             } else
                 can_place_on.value.push_back(parse_block_predicate(it.as_compound()));
             can_place_on.value.commit();
             return std::move(can_place_on);
         }},
        {"minecraft:charged_projectiles",
         [](const enbt::value& it) -> unified {
             charged_projectiles charged_projectiles;
             for (auto& item : it.as_array()) {
                 charged_projectiles.data.push_back(new base_objects::slot_data(slot_data::from_enbt(item.as_compound())));
             }
             charged_projectiles.data.commit();
             return std::move(charged_projectiles);
         }},
        {"minecraft:container",
         [](const enbt::value& it) -> unified {
             container container;
             for (auto& item : it.as_array()) {
                 auto comp = item.as_compound();
                 uint8_t slot = comp.at("slot");
                 container.set(slot, slot_data::from_enbt(comp.at("item").as_compound()));
             }
             return std::move(container);
         }},
        {"minecraft:container_loot",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             container_loot container_loot;
             container_loot.loot_table = comp.at("loot_table").as_string();
             container_loot.seed = comp.at("seed");
             return std::move(container_loot);
         }},
        {"minecraft:custom_data",
         [](const enbt::value& it) -> unified {
             custom_data custom_data;
             if (it.is_string()) {
                 try {
                     custom_data.value = senbt::parse(it.as_string());
                 } catch (...) {
                     custom_data.value = it;
                 }
             } else
                 custom_data.value = it;
             return std::move(custom_data);
         }},
        {"minecraft:custom_model_data",
         [](const enbt::value& it) -> unified {
             return custom_model_data{.value = (int32_t)it};
         }},
        {"minecraft:custom_name",
         [](const enbt::value& it) -> unified {
             return custom_name{
                 .value = Chat::fromEnbt(it)
             };
         }},
        {"minecraft:damage",
         [](const enbt::value& it) -> unified {
             return damage{
                 .value = (int32_t)it
             };
         }},
        {"minecraft:debug_stick_state",
         [](const enbt::value& it) -> unified {
             debug_stick_state debug_stick_state;
             debug_stick_state.previous_state = it;
             return std::move(debug_stick_state);
         }},
        {"minecraft:dyed_color",
         [](const enbt::value& it) -> unified {
             return dyed_color{.rgb = (int32_t)it};
         }},
        {"minecraft:enchantment_glint_override",
         [](const enbt::value& it) -> unified {
             enchantment_glint_override enchantment_glint_override;
             enchantment_glint_override.has_glint = it;
             return std::move(enchantment_glint_override);
         }},
        {"minecraft:enchantments",
         [](const enbt::value& it) -> unified {
             enchantments enchantments;
             enchantments.value.reserve(it.size());
             for (auto& [enchantment, level] : it.as_compound())
                 enchantments.value.push_back({registers::enchantments.at((std::string)enchantment).id, level});
             enchantments.value.commit();
             return std::move(enchantments);
         }},
        {"minecraft:entity_data",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             if (!comp.contains("id"))
                 throw std::runtime_error("Entity declaration must contain at least id of entity");
             entity_data entity_data;
             entity_data.value = it;
             return std::move(entity_data);
         }},
        {"minecraft:firework_explosion",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             auto& shape = comp.at("shape").as_string();
             auto colors = comp.at("colors").as_array();
             auto fade_colors = comp.at("fade_colors").as_array();
             auto has_trail = comp.at("has_trail");
             auto has_twinkle = comp.at("has_trail");

             base_objects::item_firework_explosion res;
             if (shape == "small_ball")
                 res.shape = base_objects::item_firework_explosion::shape_e::small_ball;
             else if (shape == "large_ball")
                 res.shape = base_objects::item_firework_explosion::shape_e::large_ball;
             else if (shape == "star")
                 res.shape = base_objects::item_firework_explosion::shape_e::star;
             else if (shape == "creeper")
                 res.shape = base_objects::item_firework_explosion::shape_e::creeper;
             else if (shape == "burst")
                 res.shape = base_objects::item_firework_explosion::shape_e::burst;
             else
                 throw std::runtime_error("Unrecognized firework shape");
             res.trail = has_trail;
             res.twinkle = has_twinkle;
             for (auto& it : colors)
                 res.colors.push_back(it);
             res.colors.commit();
             for (auto& it : fade_colors)
                 res.fade_colors.push_back(it);
             res.fade_colors.commit();

             return firework_explosion{.value = std::move(res)};
         }},
        {"minecraft:fireworks",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             fireworks fireworks;
             fireworks.duration = comp.at("flight_duration");
             auto& firework_explosion_parser = load_items_parser.at("minecraft:firework_explosion");
             if (comp.contains("explosions")) {
                 auto arr = comp.at("explosions").as_array();
                 fireworks.explosions.reserve(arr.size());
                 for (auto& it : arr) {
                     auto pre_res = std::get<firework_explosion>(
                         firework_explosion_parser(it)
                     );
                     fireworks.explosions.push_back(std::move(pre_res.value));
                 }
             }
             return std::move(fireworks);
         }},
        {"minecraft:food",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             food food;
             food.nutrition = comp.at("nutrition");
             food.saturation = comp.at("saturation");
             if (comp.contains("can_always_eat"))
                 food.can_always_eat = comp.at("can_always_eat");
             return std::move(food);
         }},
        {"minecraft:instrument",
         [](const enbt::value& it) -> unified {
             if (it.is_string())
                 return instrument{(std::string)it.as_string()};
             else {
                 auto comp = it.as_compound();
                 instrument::type_extended instrument;
                 instrument.duration = comp.at("use_duration");
                 instrument.range = comp.at("range");
                 instrument.sound = parse_sound(comp.at("sound"));
                 instrument.description = Chat::fromEnbt(comp.at("description"));
                 return component::instrument{instrument};
             }
         }},
        {"minecraft:intangible_projectile",
         [](const enbt::value& it) -> unified {
             return intangible_projectile{};
         }},
        {"minecraft:item_name",
         [](const enbt::value& it) -> unified {
             return item_name{.value = Chat::fromEnbt(it)};
         }},
        {"minecraft:jukebox_playable",
         [](const enbt::value& it) -> unified {
            if(it.is_compound()){
                return jukebox_playable{jukebox_playable::jukebox_extended{
                    .sound_event = parse_sound(it.at("sound")),
                    .description = Chat::fromEnbt(it["description"]),
                    .length_in_seconds = it["length_in_seconds"],
                    .comparator_output = it["comparator_output"]
                }};
            }else{
                auto& str = it.as_string();
                if(auto it = registers::jukebox_songs.find(str); it != registers::jukebox_songs.end())
                    return jukebox_playable{jukebox_playable::jukebox_compact{(int32_t)it->second.id}};
                else
                    return jukebox_playable{str};
            }
         }},
        {"minecraft:lock",
         [](const enbt::value& it) -> unified {
             return lock{.key = (std::string)it.as_string()};
         }},
        {"minecraft:lodestone_tracker",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             lodestone_tracker lodestone_tracker;
             if (comp.contains("tracked"))
                 lodestone_tracker.tracked = comp.at("tracked");
             if (comp.contains("target")) {
                 auto target = comp.at("target").as_compound();
                 auto pos = target.at("pos").as_array();
                 int32_t x = pos[0],
                         y = pos[1],
                         z = pos[2];


                 lodestone_tracker.global_pos = {
                     .dimension = (std::string)target.at("dimension").as_string(),
                     .position = {x, y, z}
                 };
             }
             return std::move(lodestone_tracker);
         }},
        {"minecraft:lore",
         [](const enbt::value& it) -> unified {
             lore lore;
             for (auto& line : it.as_array())
                 lore.value.push_back(Chat::fromEnbt(line));
             lore.value.commit();
             return std::move(lore);
         }},
        {"minecraft:map_color",
         [](const enbt::value& it) -> unified {
             return map_color{(int32_t)it};
         }},
        {"minecraft:map_decorations",
         [](const enbt::value& it) -> unified {
             map_decorations map_decorations;
             map_decorations.value = it;
             return std::move(map_decorations);
         }},
        {"minecraft:map_id",
         [](const enbt::value& it) -> unified {
             return map_id{it};
         }},
        {"minecraft:max_damage",
         [](const enbt::value& it) -> unified {
             return max_damage{it};
         }},
        {"minecraft:max_stack_size",
         [](const enbt::value& it) -> unified {
             return max_stack_size{it};
         }},
        {"minecraft:note_block_sound",
         [](const enbt::value& it) -> unified {
             return note_block_sound{(std::string)it.as_string()};
         }},
        {"minecraft:ominous_bottle_amplifier",
         [](const enbt::value& it) -> unified {
             return ominous_bottle_amplifier{it};
         }},
        {"minecraft:pot_decorations",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_array();
             pot_decorations pot_decorations;
             if (comp.size() >= 1)
                 pot_decorations.decorations[0] = comp[0].as_string();
             else
                 pot_decorations.decorations[0] = "minecraft:brick";

             if (comp.size() >= 2)
                 pot_decorations.decorations[1] = comp[1].as_string();
             else
                 pot_decorations.decorations[1] = "minecraft:brick";

             if (comp.size() >= 3)
                 pot_decorations.decorations[2] = comp[2].as_string();
             else
                 pot_decorations.decorations[3] = "minecraft:brick";

             if (comp.size() >= 4)
                 pot_decorations.decorations[3] = comp[3].as_string();
             else
                 pot_decorations.decorations[3] = "minecraft:brick";

             return std::move(pot_decorations);
         }},
        {"minecraft:potion_contents",
         [](const enbt::value& it) -> unified {
             potion_contents potion_contents;
             if (it.is_string())
                 potion_contents.set_potion_id(it.as_string());
             else {
                 auto comp = it.as_compound();
                 if (comp.contains("potion"))
                     potion_contents.set_potion_id(comp["potion"].as_string());
                 if (comp.contains("custom_color"))
                     potion_contents.set_custom_color(comp["custom_color"]);
                 if (comp.contains("custom_name"))
                     potion_contents.set_custom_name(comp["custom_name"]);
                 if (comp.contains("custom_effects")) {
                     for (auto& it : comp["custom_effects"].as_array())
                         potion_contents.add_custom_effect(parse_item_potion_effect(it.as_compound()));
                 }
             }
             return potion_contents;
         }},
        {"minecraft:profile",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             profile profile;
             if (comp.contains("name"))
                 profile.name = comp["name"].as_string();
             if (comp.contains("id"))
                 profile.uid = (enbt::raw_uuid)comp["id"];
             if (comp.contains("properties")) {
                 auto props = comp["properties"].as_array();
                 profile.properties.reserve(props.size());
                 for (auto& it : props) {
                     auto prop = it.as_compound();
                     profile::property_t p;
                     p.name = prop.at("name").as_string();
                     p.value = prop.at("value").as_string();
                     if (prop.contains("signature"))
                         p.signature = prop["signature"].as_string();
                     profile.properties.push_back(std::move(p));
                 }
             }
             return profile;
         }},
        {"minecraft:rarity",
         [](const enbt::value& it) -> unified {
             return rarity::from_string(it.as_string());
         }},
        {"minecraft:recipes",
         [](const enbt::value& it) -> unified {
             auto arr = it.as_array();
             recipes recipes;
             recipes.value.reserve(arr.size());
             for (auto& it : arr)
                 recipes.value.push_back(it.as_string());
             return recipes;
         }},
        {"minecraft:repairable",
         [](const enbt::value& it) -> unified {
             if (it.is_array()) {
                 auto arr = it.at("items").as_array();
                 std::vector<std::string> res;
                 res.reserve(arr.size());
                 for (auto& it : arr)
                     res.push_back(it.as_string());
                 return repairable{.items = std::move(res)};
             } else
                 return repairable{.items = it.at("items").as_string()};
         }},
        {"minecraft:repair_cost",
         [](const enbt::value& it) -> unified {
             return repair_cost{.value = it};
         }},
        {"minecraft:stored_enchantments",
         [](const enbt::value& it) -> unified {
             auto levels = it.as_compound();
             stored_enchantments stored_enchantments;
             stored_enchantments.enchants.reserve(levels.size());
             for (auto& [name, level] : levels) {
                 auto id = registers::enchantments.at(name).id;
                 stored_enchantments.enchants.push_back({id, (int32_t)level});
             }
             return stored_enchantments;
         }},
        {"minecraft:suspicious_stew_effects",
         [](const enbt::value& it) -> unified {
             auto arr = it.as_array();
             suspicious_stew_effects suspicious_stew_effects;
             suspicious_stew_effects.effects.reserve(arr.size());
             for (auto& it : arr) {
                 auto comp = it.as_compound();
                 auto id = registers::effects.at(comp.at("id")).id;
                 int32_t dur = 160;
                 if (comp.contains("duration"))
                     dur = comp["duration"];
                 suspicious_stew_effects.effects.push_back({id, dur});
             }
             return suspicious_stew_effects;
         }},
        {"minecraft:tool",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             tool tool;
             if (comp.contains("default_mining_speed"))
                 tool.default_mining_speed = comp["default_mining_speed"];
             if (comp.contains("damage_per_block"))
                 tool.damage_per_block = comp["damage_per_block"];
             if (comp.contains("rules")) {
                 auto arr = comp["rules"].as_array();
                 tool.rules.reserve(arr.size());
                 for (auto& it : arr) {
                     base_objects::item_rule rule;
                     auto rule_c = it.as_compound();
                     auto& it = rule_c.at("blocks");
                     if (it.is_string())
                         rule.value = it.as_string();
                     else {
                         auto arr_ref = it.as_array();
                         list_array<std::string> arr;
                         arr.reserve(arr_ref.size());
                         for (auto& it : arr_ref)
                             arr.push_back(it.as_string());
                     }
                     if (rule_c.contains("speed"))
                         rule.speed = (float)rule_c["speed"];

                     if (rule_c.contains("correct_for_drops"))
                         rule.correct_for_drops = (bool)rule_c["correct_for_drops"];
                 }
             }
             return tool;
         }},
        {"minecraft:tooltip_style",
         [](const enbt::value& it) -> unified {
             return tooltip_style{.value = it.as_string()};
         }},
        {"minecraft:trim",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             trim trim;
             trim.pattern = registers::armorTrimPatterns.at(comp.at("pattern").as_string()).id;
             trim.material = registers::armorTrimMaterials.at(comp.at("material").as_string()).id;
             return trim;
         }},
        {"minecraft:unbreakable",
         [](const enbt::value& it) -> unified {
             return unbreakable{};
         }},
        {"minecraft:use_cooldown",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             use_cooldown use_cooldown;
             use_cooldown.seconds = comp.at("seconds");
             if (comp.contains("cooldown_group"))
                 use_cooldown.cooldown_group = comp.at("cooldown_group").as_string();
             return use_cooldown;
         }},
        {"minecraft:use_remainder",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             if (!comp.contains("components")) {
                 return use_remainder(weak_slot_data(comp["id"].as_string(), comp["count"]));
             } else
                 return use_remainder(slot_data::from_enbt(it.as_compound()));
         }},
        {"minecraft:writable_book_content",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             writable_book_content writable_book_content;
             if (comp.contains("pages")) {
                 auto pages = comp.at("pages").as_array();
                 writable_book_content.pages.reserve(pages.size());
                 for (auto& it : pages) {
                     if (it.is_string())
                         writable_book_content.pages.push_back({.text = it.as_string()});
                     else {
                         auto com = it.as_compound();
                         auto raw = com.at("raw").as_string();
                         std::optional<std::string> filtered;
                         if (com.contains("filtered"))
                             filtered = com["filtered"].as_string();
                         writable_book_content.pages.push_back({raw, filtered});
                     }
                 }
             }
             return writable_book_content;
         }},
        {"minecraft:written_book_content",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             written_book_content written_book_content;
             {
                 auto title = comp.at("title").as_compound();
                 written_book_content.raw_title = title.at("raw").as_string();
                 if (title.contains("filtered"))
                     written_book_content.filtered_title = title.at("filtered").as_string();
             }
             written_book_content.author = comp.at("author").as_string();
             if (comp.contains("generation"))
                 written_book_content.generation = comp.at("generation");
             if (comp.contains("resolved"))
                 written_book_content.resolved = comp.at("resolved");
             auto pages = comp.at("pages").as_array();
             written_book_content.pages.reserve(pages.size());
             for (auto& it : pages) {
                 if (it.is_string())
                     written_book_content.pages.push_back({.text = Chat::fromEnbt(it)});
                 else {
                     auto com = it.as_compound();
                     auto raw = Chat::fromEnbt(com.at("raw"));
                     std::optional<Chat> filtered;
                     if (com.contains("filtered"))
                         filtered = Chat::fromEnbt(com["filtered"]);
                     written_book_content.pages.push_back({raw, filtered});
                 }
             }
             return written_book_content;
         }},
        {"minecraft:creative_slot_lock",
         [](const enbt::value& it) -> unified {
             return creative_slot_lock{};
         }},
        {"minecraft:map_post_processing",
         [](const enbt::value& it) -> unified {
             return map_post_processing{.value = it};
         }},
        {"minecraft:item_model",
         [](const enbt::value& it) -> unified {
             return item_model{.value = it.as_string()};
         }},
        {"minecraft:glider",
         [](const enbt::value& it) -> unified {
             return glider{};
         }},
        {"minecraft:equippable",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             equippable equippable;
             equippable.slot = comp.at("slot").as_string();
             if (comp.contains("model"))
                 equippable.model = comp.at("model").as_string();
             if (comp.contains("camera_overlay"))
                 equippable.camera_overlay = comp.at("camera_overlay").as_string();
             if (comp.contains("dispensable"))
                 equippable.dispensable = comp.at("dispensable");
             if (comp.contains("swappable"))
                 equippable.swappable = comp.at("swappable");
             if (comp.contains("damage_on_hurt"))
                 equippable.damage_on_hurt = comp.at("damage_on_hurt");
             if (comp.contains("equip_sound")) {
                 auto equip_sound = comp.at("equip_sound");
                 if (equip_sound.is_string())
                     equippable.equip_sound = equip_sound.as_string();
                 else
                     equippable.equip_sound = parse_sound_extended(equip_sound);
             } else
                 equippable.equip_sound = "minecraft:item.armor.equip_generic";

             if (comp.contains("allowed_entities")) {
                 auto allowed_entities = comp.at("allowed_entities");
                 if (allowed_entities.is_string())
                     equippable.allowed_entities = allowed_entities.as_string();
                 else {
                     auto allowed_entities_arr = allowed_entities.as_array();
                     std::vector<std::string> res;
                     res.reserve(allowed_entities_arr.size());
                     for (auto& it : allowed_entities_arr)
                         res.push_back(it.as_string());
                     equippable.allowed_entities = res;
                 }
             } else
                 equippable.allowed_entities = nullptr;
             return equippable;
         }},
        {"minecraft:enchantable",
         [](const enbt::value& it) -> unified {
             return enchantable{.value = it.at("value")};
         }},
        {"minecraft:death_protection",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             death_protection death_protection;
             if (comp.contains("death_effects")) {
                 list_array<inner::application_effect> res;
                 res.reserve(comp.at("death_effects").size());
                 for (auto& it : comp.at("death_effects").as_array())
                     res.push_back(parse_application_effect(it.as_compound()));
                 res.commit();
                 death_protection.death_effects = std::move(res);
             }
             return death_protection;
         }},
        {"minecraft:damage_resistant",
         [](const enbt::value& it) -> unified {
             return damage_resistant{.types = it.at("types").as_string()};
         }},
        {"minecraft:consumable",
         [](const enbt::value& it) -> unified {
             auto comp = it.as_compound();
             consumable consumable;
             if (comp.contains("consume_seconds"))
                 consumable.consume_seconds = comp.at("consume_seconds");
             if (comp.contains("animation"))
                 consumable.animation = comp.at("animation").as_string();
             if (comp.contains("sound"))
                 consumable.sound = parse_sound(comp.at("sound"));

             if (comp.contains("on_consume_effects")) {
                 list_array<inner::application_effect> res;
                 res.reserve(comp.at("on_consume_effects").size());
                 for (auto& it : comp.at("on_consume_effects").as_array())
                     res.push_back(parse_application_effect(it.as_compound()));
                 res.commit();
                 consumable.on_consume_effects = std::move(res);
             }
             return consumable;
         }},
        {"minecraft:axolotl_variant",
         [](const enbt::value& it) -> unified {
             return axolotl_variant{it.as_string()};
         }},
        {"minecraft:blocks_attacks",
         [](const enbt::value& it) -> unified {
             blocks_attacks res;
             if (it.contains("block_delay_seconds"))
                 res.block_delay_seconds = it.at("block_delay_seconds");
             else
                 res.disable_cooldown_scale = 0;

             if (it.contains("disable_cooldown_scale"))
                 res.disable_cooldown_scale = it.at("disable_cooldown_scale");
             else
                 res.disable_cooldown_scale = 1;
             if (it.contains("damage_reductions")) {
                 res.damage_reductions.reserve(it.at("damage_reductions").size());
                 for (auto& reduction_ : it.at("damage_reductions").as_array()) {
                     auto comp = reduction_.as_compound();
                     blocks_attacks::damage_reduction_t reduction;
                     auto type = comp.at("type");
                     if (type.is_string())
                         reduction.type = type.as_string();
                     reduction.base = comp.at("base");
                     reduction.factor = comp.at("factor");
                     if (comp.contains("horizontal_blocking_angle"))
                         reduction.horizontal_blocking_angle = comp.at("horizontal_blocking_angle");
                     else
                         reduction.horizontal_blocking_angle = 90;
                     res.damage_reductions.push_back(reduction);
                 }
             }

             {
                 auto item_damage = it.at("item_damage").as_compound();
                 res.item_damage.base = item_damage.at("base");
                 res.item_damage.factor = item_damage.at("factor");
                 res.item_damage.threshold = item_damage.at("threshold");
             }

             if (it.contains("bypassed_by"))
                 res.bypassed_by = it.at("bypassed_by").as_string();

             if (it.contains("block_sound"))
                 res.block_sound = parse_sound(it.at("block_sound"));

             if (it.contains("disabled_sound"))
                 res.disabled_sound = parse_sound(it.at("disabled_sound"));

             return res;
         }},
        {"minecraft:break_sound",
         [](const enbt::value& it) -> unified {
             return break_sound{it.as_string()};
         }},
        {"minecraft:chicken/variant",
         [](const enbt::value& it) -> unified {
             return chicken_variant{it.as_string()};
         }},
        {"minecraft:cat/collar", [](const enbt::value& it) -> unified {
             return cat_collar{dye_color::from_string(it.as_string())};
         }},
        {"minecraft:cat/variant", [](const enbt::value& it) -> unified {
             return cat_variant{it.as_string()};
         }},
        {"minecraft:cow/variant", [](const enbt::value& it) -> unified {
             return cow_variant{it.as_string()};
         }},
        {"minecraft:fox/variant", [](const enbt::value& it) -> unified {
             return fox_variant{it.as_string()};
         }},
        {"minecraft:frog/variant", [](const enbt::value& it) -> unified {
             return frog_variant{it.as_string()};
         }},
        {"minecraft:horse/variant", [](const enbt::value& it) -> unified {
             return horse_variant{it.as_string()};
         }},
        {"minecraft:llama/variant", [](const enbt::value& it) -> unified {
             return llama_variant{it.as_string()};
         }},
        {"minecraft:mooshroom/variant", [](const enbt::value& it) -> unified {
             return mooshroom_variant{it.as_string()};
         }},
        {"minecraft:painting/variant", [](const enbt::value& it) -> unified {
             return painting_variant{it.as_string()};
         }},
        {"minecraft:parrot/variant", [](const enbt::value& it) -> unified {
             return parrot_variant{it.as_string()};
         }},
        {"minecraft:pig/variant", [](const enbt::value& it) -> unified {
             return pig_variant{it.as_string()};
         }},
        {"minecraft:potion_duration_scale", [](const enbt::value& it) -> unified {
             return potion_duration_scale{it};
         }},
        {"minecraft:provides_banner_patterns", [](const enbt::value& it) -> unified {
             return provides_banner_patterns{it.as_string()};
         }},
        {"minecraft:provides_trim_material", [](const enbt::value& it) -> unified {
             return provides_trim_material{it.as_string()};
         }},
        {"minecraft:rabbit/variant", [](const enbt::value& it) -> unified {
             return rabbit_variant{it.as_string()};
         }},
        {"minecraft:salmon/size", [](const enbt::value& it) -> unified {
             return salmon_size{it};
         }},
        {"minecraft:sheep/color", [](const enbt::value& it) -> unified {
             return sheep_color{dye_color::from_string(it.as_string())};
         }},
        {"minecraft:shulker/color", [](const enbt::value& it) -> unified {
             return shulker_color{dye_color::from_string(it.as_string())};
         }},
        {"minecraft:tooltip_display", [](const enbt::value& it) -> unified {
             tooltip_display res;
             if (it.contains("hide_tooltip"))
                 res.hide_tooltip = it.at("hide_tooltip");
             if (it.contains("hidden_components")) {
                 auto hidden_components = it.at("hidden_components").as_array();
                 res.hidden_components.reserve(hidden_components.size());
                 for (auto& it : hidden_components)
                     res.hidden_components.push_back(it.as_string());
             }
             return res;
         }},
        {"minecraft:tropical_fish/base_color", [](const enbt::value& it) -> unified {
             return tropical_fish_base_color{dye_color::from_string(it.as_string())};
         }},
        {"minecraft:tropical_fish/pattern", [](const enbt::value& it) -> unified {
             return tropical_fish_pattern{it.as_string()};
         }},
        {"minecraft:tropical_fish/pattern_color", [](const enbt::value& it) -> unified {
             return tropical_fish_pattern_color{dye_color::from_string(it.as_string())};
         }},
        {"minecraft:villager/variant", [](const enbt::value& it) -> unified {
             return villager_variant{it.as_string()};
         }},
        {"minecraft:weapon", [](const enbt::value& it) -> unified {
             weapon weapon;
             if (it.contains("item_damage_per_attack"))
                 weapon.item_damage_per_attack = it.at("item_damage_per_attack");
             else
                 weapon.item_damage_per_attack = 1;
             if (it.contains("disable_blocking_for_seconds"))
                 weapon.disable_blocking_for_seconds = it.at("disable_blocking_for_seconds");
             else
                 weapon.disable_blocking_for_seconds = 0;
             return weapon;
         }},
        {"minecraft:wolf/collar", [](const enbt::value& it) -> unified {
             return wolf_collar{dye_color::from_string(it.as_string())};
         }},
        {"minecraft:wolf/sound_variant", [](const enbt::value& it) -> unified {
             return wolf_sound_variant{it.as_string()};
         }},
        {"minecraft:wolf/variant", [](const enbt::value& it) -> unified {
             return wolf_variant{it.as_string()};
         }},
    };

    std::unordered_map<std::string, enbt::value (*)(const unified&)> load_items_encoder{
        {"minecraft:attribute_modifiers",
         [](const unified& it) -> enbt::value {
             auto& attributes = std::get<attribute_modifiers>(it);
             enbt::fixed_array res;
             res.reserve(attributes.value.size());
             for (auto& i : attributes.value) {
                 enbt::compound modifier;
                 modifier["type"] = i.type;
                 modifier["id"] = i.id;
                 modifier["operation"] = base_objects::item_attribute::id_to_operation_string(base_objects::item_attribute::operation_to_id(i.operation));
                 modifier["amount"] = i.amount;
                 modifier["slot"] = base_objects::item_attribute::slot_to_name(i.slot);
                 res.push_back(std::move(modifier));
             }
             return std::move(res);
         }},
        {"minecraft:banner_patterns",
         [](const unified& it) -> enbt::value {
             auto& patterns = std::get<banner_patterns>(it);
             enbt::fixed_array res;
             res.reserve(patterns.value.size());
             for (auto& i : patterns.value) {
                 enbt::compound pattern;
                 pattern["color"] = i.color.to_string();
                 std::visit(
                     [&](auto& it) {
                         if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>) {
                             pattern["pattern"] = it;
                         } else {
                             enbt::compound custom_pattern;
                             custom_pattern["asset_id"] = it.asset_id;
                             custom_pattern["translation_key"] = it.translation_key;
                             pattern["pattern"] = std::move(custom_pattern);
                         }
                     },
                     i.pattern
                 );
                 res.push_back(std::move(pattern));
             }
             return std::move(res);
         }},
        {"minecraft:base_color",
         [](const unified& it) -> enbt::value {
             switch (std::get<base_color>(it).color) {
             case base_objects::dye_color::white:
                 return "white";
             case base_objects::dye_color::orange:
                 return "orange";
             case base_objects::dye_color::magenta:
                 return "magenta";
             case base_objects::dye_color::light_blue:
                 return "light_blue";
             case base_objects::dye_color::yellow:
                 return "yellow";
             case base_objects::dye_color::lime:
                 return "lime";
             case base_objects::dye_color::pink:
                 return "pink";
             case base_objects::dye_color::gray:
                 return "gray";
             case base_objects::dye_color::light_gray:
                 return "light_gray";
             case base_objects::dye_color::cyan:
                 return "cyan";
             case base_objects::dye_color::purple:
                 return "purple";
             case base_objects::dye_color::blue:
                 return "blue";
             case base_objects::dye_color::brown:
                 return "brown";
             case base_objects::dye_color::green:
                 return "green";
             case base_objects::dye_color::red:
                 return "red";
             case base_objects::dye_color::black:
                 return "black";
             default:
                 return "black";
             }
         }},
        {"minecraft:bees",
         [](const unified& it) -> enbt::value {
             auto& bee_arr = std::get<bees>(it);
             enbt::fixed_array res;
             res.reserve(bee_arr.values.size());
             for (auto& i : bee_arr.values) {
                 enbt::compound bee;
                 bee["type"] = i.entity_data;
                 bee["id"] = i.ticks_in_hive;
                 bee["operation"] = i.min_ticks_in_hive;
                 res.push_back(std::move(bee));
             }
             return std::move(res);
         }},
        {"minecraft:block_entity_data",
         [](const unified& it) -> enbt::value {
             return std::get<block_entity_data>(it).value;
         }},
        {"minecraft:block_state",
         [](const unified& it) -> enbt::value {
             enbt::compound res;
             for (auto& [state, value] : std::get<block_state>(it).properties)
                 res[state] = value;
             return res;
         }},
        {"minecraft:bucket_entity_data",
         [](const unified& it) -> enbt::value {
             return std::get<bucket_entity_data>(it).value;
         }},
        {"minecraft:bundle_contents",
         [](const unified& it) -> enbt::value {
             auto& bundle = std::get<bundle_contents>(it);
             enbt::fixed_array res;
             res.reserve(bundle.items.size());
             for (auto& i : bundle.items)
                 res.push_back(i->to_enbt());
             return res;
         }},
        {"minecraft:can_break",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<can_break>(it);
             if (tmp.value.size() == 1)
                 return encode_block_predicate(tmp.value[0]);
             else {
                 enbt::fixed_array arr;
                 arr.reserve(tmp.value.size());
                 for (auto& it : tmp.value)
                     arr.push_back(encode_block_predicate(it));
                 return arr;
             }
         }},
        {"minecraft:can_place_on",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<can_place_on>(it);
             if (tmp.value.size() == 1)
                 return encode_block_predicate(tmp.value[0]);
             else {
                 enbt::fixed_array arr;
                 arr.reserve(tmp.value.size());
                 for (auto& it : tmp.value)
                     arr.push_back(encode_block_predicate(it));
                 return arr;
             }
         }},
        {"minecraft:charged_projectiles",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<charged_projectiles>(it);
             enbt::fixed_array arr;
             arr.reserve(tmp.data.size());
             for (auto& item : tmp.data)
                 arr.push_back(item->to_enbt());
             return arr;
         }},
        {"minecraft:container",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<container>(it);
             enbt::fixed_array res;
             res.reserve(tmp.count());
             tmp.for_each([&res](auto& item, auto slot) {
                 res.push_back(enbt::compound{{"slot", slot}, {"item", item.to_enbt()}});
             });
             return res;
         }},
        {"minecraft:container_loot",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<container_loot>(it);
             return enbt::compound{{"loot_table", tmp.loot_table}, {"seed", tmp.seed}};
         }},
        {"minecraft:custom_data",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<custom_data>(it);
             return senbt::serialize(tmp.value, true, true);
         }},
        {"minecraft:custom_model_data",
         [](const unified& it) -> enbt::value {
             return std::get<custom_model_data>(it).value;
         }},
        {"minecraft:custom_name",
         [](const unified& it) -> enbt::value {
             return std::get<custom_name>(it).value.ToENBT();
         }},
        {"minecraft:damage",
         [](const unified& it) -> enbt::value {
             return std::get<damage>(it).value;
         }},
        {"minecraft:debug_stick_state",
         [](const unified& it) -> enbt::value {
             return std::get<debug_stick_state>(it).previous_state;
         }},
        {"minecraft:dyed_color",
         [](const unified& it) -> enbt::value {
             return std::get<dyed_color>(it).rgb;
         }},
        {"minecraft:enchantment_glint_override",
         [](const unified& it) -> enbt::value {
             return std::get<enchantment_glint_override>(it).has_glint;
         }},
        {"minecraft:enchantments",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<enchantments>(it);
             enbt::compound res;
             res.reserve(tmp.value.size());
             for (auto& [enchantment, level] : tmp.value)
                 res[registers::enchantments_cache.at(enchantment)->first] = level;
             return res;
         }},
        {"minecraft:entity_data",
         [](const unified& it) -> enbt::value {
             return std::get<entity_data>(it).value;
         }},
        {"minecraft:firework_explosion",
         [](const unified& it) -> enbt::value {
             return firework_explosion_encoder(std::get<firework_explosion>(it).value);
         }},
        {"minecraft:fireworks",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<fireworks>(it);
             enbt::compound res{
                 {"flight_duration", tmp.duration}
             };
             enbt::fixed_array arr;
             arr.reserve(tmp.explosions.size());
             for (auto& i : tmp.explosions)
                 arr.push_back(firework_explosion_encoder(i));
             res["explosions"] = std::move(arr);
             return res;
         }},
        {"minecraft:food",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<food>(it);
             return enbt::compound{
                 {"nutrition", tmp.nutrition},
                 {"saturation", tmp.saturation},
                 {"can_always_eat", tmp.can_always_eat}
             };
         }},
        {"minecraft:instrument",
         [](const unified& it) -> enbt::value {
             return std::visit(
                 [](auto& it) -> enbt::value {
                     if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>)
                         return it;
                     else
                         return enbt::compound{
                             {"duration", it.duration},
                             {"range", it.range},
                             {"sound", encode_sound(it.sound)},
                             {"description", it.description.ToENBT()}
                         };
                 },
                 std::get<instrument>(it).type
             );
         }},
        {"minecraft:intangible_projectile",
         [](const unified& it) -> enbt::value {
             return enbt::compound{};
         }},
        {"minecraft:item_name",
         [](const unified& it) -> enbt::value {
             return std::get<item_name>(it).value.ToENBT();
         }},
        {"minecraft:jukebox_playable",
         [](const unified& it) -> enbt::value {
             return std::visit(
                 [](auto& it) -> enbt::value {
                     if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>)
                         return it;
                     else if constexpr (std::is_same_v<std::decay_t<decltype(it)>, jukebox_playable::jukebox_compact>) {
                         return registers::jukebox_songs_cache.at((uint32_t)it.song_type)->first;
                     } else {
                         return enbt::compound{
                             {"comparator_output", it.comparator_output},
                             {"length_in_seconds", it.length_in_seconds},
                             {"sound", encode_sound(it.sound_event)},
                             {"description", it.description.ToENBT()}
                         };
                     }
                 },
                 std::get<jukebox_playable>(it).song
             );
         }},
        {"minecraft:lock",
         [](const unified& it) -> enbt::value {
             return std::get<lock>(it).key;
         }},
        {"minecraft:lodestone_tracker",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<lodestone_tracker>(it);
             enbt::compound res{
                 {"tracked", tmp.tracked},
             };
             if (tmp.global_pos) {
                 res["target"] = enbt::compound{
                     {"pos", enbt::fixed_array{tmp.global_pos->position.x, tmp.global_pos->position.y, tmp.global_pos->position.z}},
                     {"dimension", tmp.global_pos->dimension}
                 };
             }
             return res;
         }},
        {"minecraft:lore",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<lore>(it);
             enbt::dynamic_array res;
             res.reserve(tmp.value.size());
             for (auto& it : tmp.value)
                 res.push_back(it.ToENBT());
             return res;
         }},
        {"minecraft:map_color",
         [](const unified& it) -> enbt::value {
             return std::get<map_color>(it).rgb;
         }},
        {"minecraft:map_decorations",
         [](const unified& it) -> enbt::value {
             return std::get<map_decorations>(it).value;
         }},
        {"minecraft:map_id",
         [](const unified& it) -> enbt::value {
             return std::get<map_id>(it).value;
         }},
        {"minecraft:max_damage",
         [](const unified& it) -> enbt::value {
             return std::get<max_damage>(it).value;
         }},
        {"minecraft:max_stack_size",
         [](const unified& it) -> enbt::value {
             return std::get<max_stack_size>(it).value;
         }},
        {"minecraft:note_block_sound",
         [](const unified& it) -> enbt::value {
             return std::get<note_block_sound>(it).sound;
         }},
        {"minecraft:ominous_bottle_amplifier",
         [](const unified& it) -> enbt::value {
             return std::get<ominous_bottle_amplifier>(it).amplifier;
         }},
        {"minecraft:pot_decorations",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<pot_decorations>(it);
             return enbt::fixed_array{
                 tmp.decorations[0],
                 tmp.decorations[1],
                 tmp.decorations[2],
                 tmp.decorations[3],
             };
         }},
        {"minecraft:potion_contents",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<potion_contents>(it);
             auto pot_id = tmp.get_potion_id();
             auto color = tmp.get_custom_color();
             auto name = tmp.get_custom_name();
             enbt::fixed_array effects;
             tmp.iterate_custom_effects([&effects](size_t count) { effects.reserve(count); }, [&effects](const item_potion_effect& effect) { effects.push_back(encode_item_potion_effect(effect)); });
             if (effects.empty() && !name && !color && pot_id) {
                 return *pot_id;
             } else {
                 enbt::compound res;
                 if (pot_id)
                     res["potion"] = registers::effects_cache.at(*pot_id)->second.name;
                 if (color)
                     res["custom_color"] = *color;
                 if (name)
                     res["custom_name"] = *name;
                 if (effects.size())
                     res["custom_effects"] = std::move(effects);
                 return res;
             }
         }},
        {"minecraft:profile",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<profile>(it);
             enbt::compound res;
             if (tmp.name)
                 res["name"] = *tmp.name;
             if (tmp.uid)
                 res["id"] = *tmp.uid;
             if (tmp.properties.size()){
                 enbt::fixed_array props;
                 props.reserve(tmp.properties.size());
                 for (auto& it : tmp.properties){
                     enbt::compound item{{"name", it.name}, {"value", it.value}};
                     if (it.signature)
                         item["signature"] = *it.signature;
                     props.push_back(std::move(item));
                 }
                 res["properties"] = std::move(props);
             }
             return res;
         }},
        {"minecraft:rarity",
         [](const unified& it) -> enbt::value {
             return std::get<rarity>(it).to_string();
         }},
        {"minecraft:recipes",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<recipes>(it).value;
             enbt::fixed_array res;
             res.reserve(tmp.size());
             for (auto& it : tmp)
                 res.push_back(it);
             return res;
         }},
        {"minecraft:repairable",
         [](const unified& it) -> enbt::value {
             return std::visit(
                 [](auto& it) {
                     if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>)
                         return enbt::compound{{"items", it}};
                     else {
                         enbt::fixed_array arr;
                         arr.reserve(it.size());
                         for (auto& item : it)
                             arr.push_back(item);
                         return enbt::compound{{"items", std::move(arr)}};
                     }
                 },
                 std::get<repairable>(it).items
             );
         }},
        {"minecraft:repair_cost",
         [](const unified& it) -> enbt::value {
             return std::get<repair_cost>(it).value;
         }},
        {"minecraft:stored_enchantments",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<stored_enchantments>(it);
             enbt::compound res;
             res.reserve(tmp.enchants.size());
             for (auto& [enchantment, lvl] : tmp.enchants)
                 res[registers::enchantments_cache.at(enchantment)->first] = lvl;
             return res;
         }},
        {"minecraft:suspicious_stew_effects",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<suspicious_stew_effects>(it);
             enbt::fixed_array res;
             res.reserve(tmp.effects.size());
             for (auto& effect : tmp.effects)
                 res.push_back(enbt::compound{{"id", registers::effects_cache.at(effect.first)->first}, {"duration", effect.second}});
             return res;
         }},
        {"minecraft:tool",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<tool>(it);
             enbt::compound res{
                 {"default_mining_speed", tmp.default_mining_speed},
                 {"damage_per_block", tmp.damage_per_block}
             };
             if (tmp.rules.size()) {
                 enbt::fixed_array arr;
                 arr.reserve(tmp.rules.size());
                 for (auto& it : tmp.rules) {
                     enbt::compound rule;
                     if (it.speed)
                         rule["speed"] = *it.speed;
                     if (it.correct_for_drops)
                         rule["correct_for_drops"] = *it.correct_for_drops;
                     std::visit(
                         [&rule](auto& it) {
                             if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>)
                                 rule["blocks"] = it;
                             else {
                                 enbt::fixed_array arr;
                                 arr.reserve(it.size());
                                 for (auto& item : it)
                                     arr.push_back(item);
                                 rule["blocks"] = std::move(arr);
                             }
                         },
                         it.value
                     );
                     arr.push_back(std::move(rule));
                 }
                 res["rules"] = std::move(arr);
             }
             return res;
         }},
        {"minecraft:tooltip_style",
         [](const unified& it) -> enbt::value {
             return std::get<tooltip_style>(it).value;
         }},
        {"minecraft:trim",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<trim>(it);
             return enbt::compound{
                 {"pattern", registers::armorTrimPatterns_cache.at(std::get<uint32_t>(tmp.pattern))->first},
                 {"material", registers::armorTrimMaterials_cache.at(std::get<uint32_t>(tmp.material))->first}
             };
         }},
        {"minecraft:unbreakable",
         [](const unified& it) -> enbt::value {
             return enbt::compound{};
         }},
        {"minecraft:use_cooldown",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<use_cooldown>(it);
             enbt::compound res{{"seconds", tmp.seconds}};
             if (tmp.cooldown_group)
                 res["cooldown_group"] = *tmp.cooldown_group;
             return res;
         }},
        {"minecraft:use_remainder",
         [](const unified& it) -> enbt::value {
             return std::visit(
                 [](auto& it) {
                     if constexpr (std::is_same_v<std::decay_t<decltype(it)>, weak_slot_data>)
                         return enbt::compound{{"id", it.id},{"count", it.count}};
                     else
                         return it->to_enbt();
                 },
                 std::get<use_remainder>(it).proxy_value
             );
         }},
        {"minecraft:writable_book_content",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<writable_book_content>(it);
             enbt::compound res;
             if(tmp.pages.size()) {
                 enbt::dynamic_array arr;
                 arr.reserve(tmp.pages.size());
                 for (auto& it : tmp.pages) {
                     if (it.filtered)
                         arr.push_back(enbt::compound{{"text", it.text}, {"filtered", *it.filtered}});
                     else
                         arr.push_back(it.text);
                 }
                 res["pages"] = std::move(arr);
             }
             return res;
         }},
        {"minecraft:written_book_content",
         [](const unified& it) -> enbt::value {
             auto& tmp = std::get<written_book_content>(it);
             enbt::compound res{
                 {"author", tmp.author},
                 {"title", enbt::compound{{"raw", tmp.raw_title}}},
                 {"generation", tmp.generation},
                 {"resolved", tmp.resolved}
             };
             if (tmp.filtered_title)
                 res["title"]["filtered"] = *tmp.filtered_title;
             if(tmp.pages.size()) {
                 enbt::dynamic_array arr;
                 arr.reserve(tmp.pages.size());
                 for (auto& it : tmp.pages) {
                     if (it.filtered)
                         arr.push_back(enbt::compound{{"text", it.text.ToENBT()}, {"filtered", it.filtered->ToENBT()}});
                     else
                         arr.push_back(it.text.ToENBT());
                 }
                 res["pages"] = std::move(arr);
             }
             return res;
         }},
        {"minecraft:creative_slot_lock",
         [](const unified& it) -> enbt::value {
             return enbt::compound{};
         }},
        {"minecraft:map_post_processing",
         [](const unified& it) -> enbt::value {
             return std::get<map_post_processing>(it).value;
         }},
        {"minecraft:item_model",
         [](const unified& it) -> enbt::value {
             return std::get<item_model>(it).value;
         }},
        {"minecraft:glider", [](const unified& it) -> enbt::value {
             return enbt::compound{};
         }},
        {"minecraft:equippable", [](const unified& it) -> enbt::value {
             auto tmp = std::get<equippable>(it);
             enbt::compound res{
                 {"slot", tmp.slot},
                 {"dispensable", tmp.dispensable},
                 {"swappable", tmp.swappable},
                 {"damage_on_hurt", tmp.damage_on_hurt},
                 {"equip_on_interact", tmp.equip_on_interact},
             };
             if (tmp.model)
                 res["model"] = *tmp.model;
             if (tmp.camera_overlay)
                 res["camera_overlay"] = *tmp.camera_overlay;
             std::visit(
                 [&](auto& it) {
                     if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>)
                         res["equip_sound"] = it;
                     else
                         res["equip_sound"] = encode_sound_extended(it);
                 },
                 tmp.equip_sound
             );

             std::visit(
                 [&](auto& it) {
                     if constexpr (std::is_same_v<std::decay_t<decltype(it)>, nullptr_t>)
                         ;
                     else if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>)
                         res["allowed_entities"] = it;
                     else {
                         enbt::fixed_array arr;
                         arr.reserve(it.size());
                         for (auto& i : it)
                             arr.push_back(i);
                         res["allowed_entities"] = std::move(arr);
                     }
                 },
                 tmp.allowed_entities
             );
             return res;
         }},
        {"minecraft:enchantable", [](const unified& it) -> enbt::value {
             return enbt::compound{{"value", std::get<enchantable>(it).value}};
         }},
        {"minecraft:death_protection", [](const unified& it) -> enbt::value {
             auto tmp = std::get<death_protection>(it);
             enbt::compound res;
             if (tmp.death_effects.size()) {
                 enbt::fixed_array arr;
                 arr.reserve(tmp.death_effects.size());
                 for (auto& it : tmp.death_effects)
                     arr.push_back(encode_application_effect(it));
                 res["death_effects"] = std::move(arr);
             }
             return res;
         }},
        {"minecraft:damage_resistant", [](const unified& it) -> enbt::value {
             return enbt::compound{{"types", std::get<damage_resistant>(it).types}};
         }},
        {"minecraft:consumable", [](const unified& it) -> enbt::value {
             auto tmp = std::get<consumable>(it);
             enbt::compound res{
                 {"consume_seconds", tmp.consume_seconds},
                 {"animation", tmp.animation},
                 {"sound", encode_sound(tmp.sound)},
             };

             if (tmp.on_consume_effects.size()) {
                 enbt::fixed_array arr;
                 arr.reserve(tmp.on_consume_effects.size());
                 for (auto& it : tmp.on_consume_effects)
                     arr.push_back(encode_application_effect(it));
                 res["animation"] = std::move(arr);
             }
             return res;
         }},
        {"minecraft:axolotl_variant", [](const unified& it) -> enbt::value {
             return std::get<axolotl_variant>(it).value;
         }},
        {"minecraft:blocks_attacks", [](const unified& it) -> enbt::value {
             auto tmp = std::get<blocks_attacks>(it);
             enbt::compound res{
                 {"block_delay_seconds", tmp.block_delay_seconds},
                 {"disable_cooldown_scale", tmp.disable_cooldown_scale},
                 {"item_damage", enbt::compound{{"base", tmp.item_damage.base}, {"factor", tmp.item_damage.factor}, {"threshold", tmp.item_damage.threshold}}},
             };

             if (tmp.damage_reductions.size()) {
                 enbt::fixed_array arr;
                 arr.reserve(tmp.damage_reductions.size());
                 for (auto& reduction : tmp.damage_reductions)
                     arr.push_back(enbt::compound{
                         {"type", reduction.type},
                         {"base", reduction.base},
                         {"factor", reduction.factor},
                         {"horizontal_blocking_angle", reduction.horizontal_blocking_angle},
                     });
                 res["damage_reductions"] = std::move(arr);
             }


             if (tmp.bypassed_by)
                 res["bypassed_by"] = *tmp.bypassed_by;

             if (tmp.bypassed_by)
                 res["block_sound"] = encode_sound(*tmp.bypassed_by);

             if (tmp.disabled_sound)
                 res["disabled_sound"] = encode_sound(*tmp.disabled_sound);

             return res;
         }},
        {"minecraft:break_sound", [](const unified& it) -> enbt::value {
             return encode_sound(std::get<break_sound>(it).value);
         }},
        {"minecraft:chicken/variant", [](const unified& it) -> enbt::value {
             return std::get<chicken_variant>(it).value;
         }},
        {"minecraft:cat/collar", [](const unified& it) -> enbt::value {
             return std::get<cat_collar>(it).value.to_string();
         }},
        {"minecraft:cat/variant", [](const unified& it) -> enbt::value {
             return std::get<cat_variant>(it).value;
         }},
        {"minecraft:cow/variant", [](const unified& it) -> enbt::value {
             return std::get<cow_variant>(it).value;
         }},
        {"minecraft:fox/variant", [](const unified& it) -> enbt::value {
             return std::get<fox_variant>(it).value;
         }},
        {"minecraft:frog/variant", [](const unified& it) -> enbt::value {
             return std::get<frog_variant>(it).value;
         }},
        {"minecraft:horse/variant", [](const unified& it) -> enbt::value {
             return std::get<horse_variant>(it).value;
         }},
        {"minecraft:llama/variant", [](const unified& it) -> enbt::value {
             return std::get<llama_variant>(it).value;
         }},
        {"minecraft:mooshroom/variant", [](const unified& it) -> enbt::value {
             return std::get<mooshroom_variant>(it).value;
         }},
        {"minecraft:painting/variant", [](const unified& it) -> enbt::value {
             return std::get<painting_variant>(it).value;
         }},
        {"minecraft:parrot/variant", [](const unified& it) -> enbt::value {
             return std::get<parrot_variant>(it).value;
         }},
        {"minecraft:pig/variant", [](const unified& it) -> enbt::value {
             return std::get<pig_variant>(it).value;
         }},
        {"minecraft:potion_duration_scale", [](const unified& it) -> enbt::value {
             return std::get<potion_duration_scale>(it).value;
         }},
        {"minecraft:provides_banner_patterns", [](const unified& it) -> enbt::value {
             return std::get<provides_banner_patterns>(it).patterns;
         }},
        {"minecraft:provides_trim_material", [](const unified& it) -> enbt::value {
             return std::get<provides_trim_material>(it).patterns;
         }},
        {"minecraft:rabbit/variant", [](const unified& it) -> enbt::value {
             return std::get<rabbit_variant>(it).value;
         }},
        {"minecraft:salmon/size", [](const unified& it) -> enbt::value {
             return std::get<salmon_size>(it).value;
         }},
        {"minecraft:sheep/color", [](const unified& it) -> enbt::value {
             return std::get<sheep_color>(it).value.to_string();
         }},
        {"minecraft:shulker/color", [](const unified& it) -> enbt::value {
             return std::get<shulker_color>(it).value.to_string();
         }},
        {"minecraft:tooltip_display", [](const unified& it) -> enbt::value {
             auto tmp = std::get<tooltip_display>(it);
             enbt::compound res;
             res["hide_tooltip"] = tmp.hide_tooltip;
             if (tmp.hidden_components.size()) {
                 enbt::fixed_array array;
                 array.reserve(tmp.hidden_components.size());
                 for (auto& it : tmp.hidden_components)
                     array.push_back(it);
                 res["hidden_components"] = std::move(array);
             }
             return std::move(res);
         }},
        {"minecraft:tropical_fish/pattern", [](const unified& it) -> enbt::value {
             return std::get<tropical_fish_pattern>(it).pattern;
         }},
        {"minecraft:tropical_fish/pattern_color", [](const unified& it) -> enbt::value {
             return std::get<tropical_fish_pattern_color>(it).color.to_string();
         }},
        {"minecraft:villager/variant", [](const unified& it) -> enbt::value {
             return std::get<villager_variant>(it).value;
         }},
        {"minecraft:weapon", [](const unified& it) -> enbt::value {
             auto tmp = std::get<weapon>(it);
             return enbt::compound{
                 {"item_damage_per_attack", tmp.item_damage_per_attack},
                 {"disable_blocking_for_seconds", tmp.disable_blocking_for_seconds}
             };
         }},
        {"minecraft:wolf/collar", [](const unified& it) -> enbt::value {
             return std::get<wolf_collar>(it).value.to_string();
         }},
        {"minecraft:wolf/sound_variant", [](const unified& it) -> enbt::value {
             return std::get<wolf_sound_variant>(it).value;
         }},
        {"minecraft:wolf/variant", [](const unified& it) -> enbt::value {
             return std::get<wolf_variant>(it).value;
         }},
    };

    unified parse_component(const std::string& name, const enbt::value& item) {
        return load_items_parser.at(name)(item);
    }

    std::pair<std::string, enbt::value> encode_component(const unified& item) {
        return std::visit(
            [](auto& it) { return std::pair<std::string, enbt::value>{it.component_name, load_items_encoder.at(it.component_name)(it)}; },
            item
        );
    }
}