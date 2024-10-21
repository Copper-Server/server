
#include "../base_objects/slot.hpp"
#include "../registers.hpp"
#include "conversions.hpp"
#include <boost/json.hpp>

namespace crafted_craft::util {
    base_objects::slot_data parse_slot(boost::json::object& item);
    std::unordered_map<std::string, base_objects::slot_component::unified (*)(boost::json::value&)> load_items_parser{
        {"minecraft:attribute_modifiers",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::attribute_modifiers attributes;
             if (comp.contains("show_in_tooltip"))
                 attributes.show_in_tooltip = comp["show_in_tooltip"].as_bool();

             if (comp.contains("modifiers")) {
                 list_array<base_objects::item_attribute> modifiers;
                 for (auto& i : comp["modifiers"].as_array()) {
                     base_objects::item_attribute modifier;

                     auto& c = i.as_object();
                     modifier.id = base_objects::item_attribute::attribute_name_to_id((std::string)c.at("id").as_string());
                     modifier.operation = base_objects::item_attribute::id_to_operation((std::string)c.at("operation").as_string());
                     modifier.name = c.at("name").as_string();
                     modifier.value = c.at("value").as_double();
                 }
                 modifiers.commit();
                 attributes.attributes = std::move(modifiers);
             }
             return std::move(attributes);
         }
        },
        {"minecraft:banner_patterns",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::banner_patterns banner_patterns;
             for (auto& value : it.as_array()) {
                 auto& comp = value.as_object();
                 base_objects::slot_component::banner_patterns::pattern pattern;
                 pattern.color = comp.at("color").as_string();
                 auto& pattern_json = comp.at("pattern");
                 if (pattern_json.is_string())
                     pattern.pattern = (std::string)pattern_json.get_string();
                 else {
                     auto& custom = pattern_json.as_object();
                     base_objects::slot_component::banner_patterns::custom_pattern c;
                     c.asset_id = custom.at("asset_id").as_string();
                     c.translation_key = custom.at("translation_key").as_string();
                     pattern.pattern = std::move(c);
                 }
                 banner_patterns.value.push_back(pattern);
             }
             banner_patterns.value.commit();
             return std::move(banner_patterns);
         }
        },
        {"minecraft:base_color",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& color = it.as_string();
             base_objects::slot_component::base_color base_color;
             if (color == "white")
                 base_color.color = base_objects::item_color::white;
             else if (color == "orange")
                 base_color.color = base_objects::item_color::orange;
             else if (color == "magenta")
                 base_color.color = base_objects::item_color::magenta;
             else if (color == "light_blue")
                 base_color.color = base_objects::item_color::light_blue;
             else if (color == "yellow")
                 base_color.color = base_objects::item_color::yellow;
             else if (color == "lime")
                 base_color.color = base_objects::item_color::lime;
             else if (color == "pink")
                 base_color.color = base_objects::item_color::pink;
             else if (color == "gray")
                 base_color.color = base_objects::item_color::gray;
             else if (color == "light_gray")
                 base_color.color = base_objects::item_color::light_gray;
             else if (color == "cyan")
                 base_color.color = base_objects::item_color::cyan;
             else if (color == "purple")
                 base_color.color = base_objects::item_color::purple;
             else if (color == "blue")
                 base_color.color = base_objects::item_color::blue;
             else if (color == "brown")
                 base_color.color = base_objects::item_color::brown;
             else if (color == "green")
                 base_color.color = base_objects::item_color::green;
             else if (color == "red")
                 base_color.color = base_objects::item_color::red;
             else if (color == "black")
                 base_color.color = base_objects::item_color::black;
             else
                 throw std::runtime_error("Unrecognized color");
             return std::move(base_color);
         }
        },
        {"minecraft:bees",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::bees bees;
             for (auto& bee_ : it.as_array()) {
                 auto& bee = bee_.as_object();
                 bees.values.push_back({base_objects::slot_component::bees::bee{
                     .entity_data = conversions::json::from_json(bee.at("entity_data")),
                     .ticks_in_hive = (int32_t)bee.at("ticks_in_hive").to_number<int32_t>(),
                     .min_ticks_in_hive = (int32_t)bee.at("min_ticks_in_hive").to_number<int32_t>(),
                 }});
             }
             bees.values.commit();
             return std::move(bees);
         }
        },
        {"minecraft:block_entity_data",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::block_entity_data{
                 .value = conversions::json::from_json(it)
             };
         }
        },
        {"minecraft:block_state",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::block_state block_state;
             for (auto& [state, value] : it.as_object())
                 block_state.properties.push_back({(std::string)state, (std::string)value.as_string()});
             block_state.properties.commit();
             return std::move(block_state);
         }
        },
        {"minecraft:bucket_entity_data",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::bucket_entity_data{
                 .value = conversions::json::from_json(it)
             };
         }
        },
        {"minecraft:bundle_contents",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::bundle_contents bundle_contents;
             for (auto& item : it.as_array()) {
                 bundle_contents.items.push_back(new base_objects::slot_data(parse_slot(item.as_object())));
             }
             bundle_contents.items.commit();
             return std::move(bundle_contents);
         }
        },
        {"minecraft:can_break",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::can_break can_break;
             if (comp.contains("show_in_tooltip"))
                 can_break.show_in_tooltip = comp["show_in_tooltip"].as_bool();
             if (comp.contains("blocks")) {
                 enbt::compound pred;
                 pred["blocks"] = conversions::json::from_json(comp["blocks"]);
                 if (comp.contains("state"))
                     pred["state"] = conversions::json::from_json(comp["state"]);
                 if (comp.contains("nbt"))
                     pred["nbt"] = conversions::json::from_json(comp["nbt"]);
                 can_break.predicates.push_back(std::move(pred));
             }
             if (comp.contains("predicates")) {
                 for (auto& it : comp.at("predicates").as_array()) {
                     auto& item = it.as_object();
                     enbt::compound pred;
                     pred["blocks"] = conversions::json::from_json(item["blocks"]);
                     if (item.contains("state"))
                         pred["state"] = conversions::json::from_json(item["state"]);
                     if (item.contains("nbt"))
                         pred["nbt"] = conversions::json::from_json(item["nbt"]);
                     can_break.predicates.push_back(std::move(pred));
                 }
             }
             can_break.predicates.commit();
             return std::move(can_break);
         }
        },
        {"minecraft:can_place_on",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::can_place_on can_place_on;
             if (comp.contains("show_in_tooltip"))
                 can_place_on.show_in_tooltip = comp["show_in_tooltip"].as_bool();
             if (comp.contains("blocks")) {
                 enbt::compound pred;
                 pred["blocks"] = conversions::json::from_json(comp["blocks"]);
                 if (comp.contains("state"))
                     pred["state"] = conversions::json::from_json(comp["state"]);
                 if (comp.contains("nbt"))
                     pred["nbt"] = conversions::json::from_json(comp["nbt"]);
                 can_place_on.predicates.push_back(std::move(pred));
             }
             if (comp.contains("predicates")) {
                 for (auto& it : comp.at("predicates").as_array()) {
                     auto& item = it.as_object();
                     enbt::compound pred;
                     pred["blocks"] = conversions::json::from_json(item["blocks"]);
                     if (item.contains("state"))
                         pred["state"] = conversions::json::from_json(item["state"]);
                     if (item.contains("nbt"))
                         pred["nbt"] = conversions::json::from_json(item["nbt"]);
                     can_place_on.predicates.push_back(std::move(pred));
                 }
             }
             can_place_on.predicates.commit();
             return std::move(can_place_on);
         }
        },
        {"minecraft:charged_projectiles",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::charged_projectiles charged_projectiles;
             for (auto& item : it.as_array()) {
                 charged_projectiles.data.push_back(new base_objects::slot_data(parse_slot(item.as_object())));
             }
             charged_projectiles.data.commit();
             return std::move(charged_projectiles);
         }
        },
        {"minecraft:container",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::container container;
             for (auto& item : it.as_array()) {
                 auto& comp = item.as_object();
                 uint8_t slot = comp.at("slot").to_number<uint8_t>();
                 container.set(slot, parse_slot(comp.at("item").as_object()));
             }
             return std::move(container);
         }
        },
        {"minecraft:container_loot",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::container_loot container_loot;
             container_loot.loot_table = comp.at("loot_table").as_string();
             container_loot.seed = comp.at("seed").to_number<int32_t>();
             return std::move(container_loot);
         }
        },
        {"minecraft:custom_data",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::custom_data custom_data;
             custom_data.value = conversions::json::from_json(it);
             return std::move(custom_data);
         }
        },
        {"minecraft:custom_model_data",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::custom_model_data{.value = (int32_t)it.to_number<int32_t>()};
         }
        },
        {"minecraft:custom_name",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::custom_name{
                 .value = Chat::fromEnbt(conversions::json::from_json(it))
             };
         }
        },
        {"minecraft:damage",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::damage{
                 .value = (int32_t)it.to_number<int32_t>()
             };
         }
        },
        {"minecraft:debug_stick_state",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::debug_stick_state debug_stick_state;
             debug_stick_state.previous_state = conversions::json::from_json(it);
             return std::move(debug_stick_state);
         }
        },
        {"minecraft:dyed_color",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::dyed_color dyed_color;
             if (comp.contains("show_in_tooltip"))
                 dyed_color.show_in_tooltip = comp["show_in_tooltip"].as_bool();
             dyed_color.rgb = comp.at("rgb").to_number<int32_t>();
             return std::move(dyed_color);
         }
        },
        {"minecraft:enchantment_glint_override",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::enchantment_glint_override enchantment_glint_override;
             enchantment_glint_override.has_glint = it.as_bool();
             return std::move(enchantment_glint_override);
         }
        },
        {"minecraft:enchantments",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::enchantments enchantments;
             for (auto& [enchantment, level] : comp.at("levels").as_object())
                 enchantments.enchants.push_back({registers::enchantments.at((std::string)enchantment).id, level.to_number<int32_t>()});
             enchantments.enchants.commit();
             if (comp.contains("show_in_tooltip"))
                 enchantments.show_in_tooltip = comp.at("show_in_tooltip").as_bool();
             return std::move(enchantments);
         }
        },
        {"minecraft:entity_data",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             if (!comp.contains("id"))
                 throw std::runtime_error("Entity declaration must contain at least id of entity");
             base_objects::slot_component::entity_data entity_data;
             entity_data.value = conversions::json::from_json(it);
             return std::move(entity_data);
         }
        },
        {"minecraft:fire_resistant",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::fire_resistant{};
         }
        },
        {"minecraft:entity_data",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             if (!comp.contains("id"))
                 throw std::runtime_error("Entity declaration must contain at least id of entity");
             base_objects::slot_component::entity_data entity_data;
             entity_data.value = conversions::json::from_json(it);
             return std::move(entity_data);
         }
        },
        {"minecraft:firework_explosion",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             auto& shape = comp.at("shape").as_string();
             auto& colors = comp.at("shape").as_array();
             auto& fade_colors = comp.at("shape").as_array();
             auto has_trail = comp.at("has_trail").as_bool();
             auto has_twinkle = comp.at("has_trail").as_bool();

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
                 res.colors.push_back(it.to_number<int32_t>());
             res.colors.commit();
             for (auto& it : fade_colors)
                 res.fade_colors.push_back(it.to_number<int32_t>());
             res.fade_colors.commit();

             return base_objects::slot_component::firework_explosion{.value = std::move(res)};
         }
        },
        {"minecraft:fireworks",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::fireworks fireworks;
             fireworks.duration = comp.at("flight_duration").to_number<int32_t>();
             auto& firework_explosion_parser = load_items_parser.at("minecraft:firework_explosion");
             for (auto& it : comp.at("explosions").as_array()) {
                 auto pre_res = std::get<base_objects::slot_component::firework_explosion>(
                     firework_explosion_parser(it)
                 );
                 fireworks.explosions.push_back(std::move(pre_res.value));
             }
             fireworks.explosions.commit();
             return std::move(fireworks);
         }
        },
        {"minecraft:food",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             if (!comp.contains("id"))
                 throw std::runtime_error("Entity declaration must contain at least id of entity");
             base_objects::slot_component::food food;
             food.nutrition = comp.at("nutrition").to_number<int32_t>();
             food.saturation = comp.at("saturation").as_double();
             if (comp.contains("can_always_eat"))
                 food.can_always_eat = comp.at("can_always_eat").as_bool();
             return std::move(food);
         }
        },
        {"minecraft:hide_additional_tooltip",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::hide_additional_tooltip{};
         }
        },
        {"minecraft:hide_tooltip",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::hide_tooltip{};
         }
        },
        {"minecraft:instrument",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             if (it.is_string())
                 return base_objects::slot_component::instrument{(std::string)it.as_string()};
             else {
                 auto& comp = it.as_object();
                 base_objects::slot_component::instrument::type_extended instrument;
                 instrument.duration = comp.at("use_duration").to_number<float>();
                 instrument.range = comp.at("range").to_number<float>();
                 if (comp.at("sound").is_string())
                     instrument.sound = (std::string)comp.at("sound").as_string();
                 else {
                     auto& sound = comp.at("sound").as_object();
                     base_objects::slot_component::inner::sound_extended sound_extended;
                     sound_extended.sound_name = sound.at("sound_id").as_string();
                     if (sound.contains("range"))
                         sound_extended.fixed_range = sound.at("range").to_number<float>();
                     instrument.sound = sound_extended;
                 }
                 return base_objects::slot_component::instrument{instrument};
             }
         }
        },
        {"minecraft:intangible_projectile",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::intangible_projectile{};
         }
        },
        {"minecraft:item_name",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::item_name{.value = Chat::fromStr((std::string)it.as_string())};
         }
        },
        {"minecraft:jukebox_playable",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::jukebox_playable jukebox_playable;
             jukebox_playable.song = (std::string)comp.at("song").as_string();
             if (comp.contains("show_in_tooltip"))
                 jukebox_playable.show_in_tooltip = comp.at("show_in_tooltip").as_bool();
             return std::move(jukebox_playable);
         }
        },
        {"minecraft:lock",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::lock{.key = (std::string)it.as_string()};
         }
        },
        {"minecraft:lodestone_tracker",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::lodestone_tracker lodestone_tracker;
             if (comp.contains("tracked"))
                 lodestone_tracker.tracked = comp.at("tracked").as_bool();
             if (comp.contains("target")) {
                 auto& target = comp.at("target").as_object();
                 auto& pos = target.at("pos").as_array();

                 int32_t x = pos[0].to_number<int32_t>(),
                         y = pos[1].to_number<int32_t>(),
                         z = pos[2].to_number<int32_t>();


                 lodestone_tracker.global_pos = {
                     .dimension = (std::string)target.at("dimension").as_string(),
                     .position = {x, y, z}
                 };
             }
             return std::move(lodestone_tracker);
         }
        },
        {"minecraft:lore",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::lore lore;
             for (auto& line : it.as_array())
                 lore.value.push_back(Chat::fromStr((std::string)line.as_string()));
             lore.value.commit();
             return std::move(lore);
         }
        },
        {"minecraft:map_color",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::map_color{(int32_t)it.to_number<int32_t>()};
         }
        },
        {"minecraft:map_decorations",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             base_objects::slot_component::map_decorations map_decorations;
             map_decorations.value = conversions::json::from_json(it);
             return std::move(map_decorations);
         }
        },
        {"minecraft:map_id",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::map_id{it.to_number<int32_t>()};
         }
        },
        {"minecraft:max_damage",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::max_damage{it.to_number<int32_t>()};
         }
        },
        {"minecraft:max_stack_size",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::max_stack_size{it.to_number<uint8_t>()};
         }
        },
        {"minecraft:note_block_sound",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::note_block_sound{(std::string)it.as_string()};
         }
        },
        {"minecraft:ominous_bottle_amplifier",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::ominous_bottle_amplifier{it.to_number<int32_t>()};
         }
        },
        {"minecraft:pot_decorations",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_array();
             base_objects::slot_component::pot_decorations pot_decorations;
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
         }
        },
        {"minecraft:potion_contents",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             auto& comp = it.as_object();
             base_objects::slot_component::potion_contents potion_contents;
             //TODO continue
             potion_contents.potion_id = (std::string)comp.at("potion").as_string();
             if (comp.contains("effects")) {
                 for (auto& effect : comp.at("effects").as_array()) {
                     auto& eff = effect.as_object();
                     base_objects::item_potion_effect potion_effect;
                     potion_effect.potion_id = (std::string)eff.at("id").as_string();
                     potion_effect.amplifier = eff.at("amplifier").to_number<int8_t>();
                     potion_effect.duration = eff.at("duration").to_number<int32_t>();
                     potion_effect.ambient = eff.at("ambient").as_bool();
                     potion_effect.show_particles = eff.at("show_particles").as_bool();
                     potion_effect.show_icon = eff.at("show_icon").as_bool();
                     potion_contents.effects.push_back(potion_effect);
                 }
                 potion_contents.effects.commit();
             }
         }
        },
        {"minecraft:hide_tooltip",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::hide_additional_tooltip{};
         }
        },
        {"minecraft:hide_tooltip",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::hide_additional_tooltip{};
         }
        },
        {"minecraft:hide_tooltip",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::hide_additional_tooltip{};
         }
        },
        {"minecraft:hide_tooltip",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::hide_additional_tooltip{};
         }
        },
        {"minecraft:hide_tooltip",
         [](boost::json::value& it) -> base_objects::slot_component::unified {
             return base_objects::slot_component::hide_additional_tooltip{};
         }
        },
    };

    base_objects::slot_component::unified parse_component(const std::string& name, boost::json::value& item) {
        return load_items_parser.at(name)(item);
    }

    base_objects::slot_data parse_slot(boost::json::object& item) {
        auto& id = item.at("id").as_string();
        base_objects::slot_data slot_data = base_objects::slot_data::create_item((std::string)id);
        if (item.contains("count"))
            slot_data.count = item.at("count").to_number<int32_t>();
        std::unordered_map<std::string, base_objects::slot_component::unified> components;
        for (auto& [name, value] : item.at("components").as_object())
            slot_data.add_component(load_items_parser.at(name)(value));
        return slot_data;
    }
}
