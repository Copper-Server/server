#include <src/api/command.hpp>
#include <src/base_objects/slot.hpp>
#include <src/registers.hpp>

namespace copper_server {
    namespace base_objects {
        std::unordered_map<std::string, std::shared_ptr<static_slot_data>> slot_data::named_full_item_data;
        std::vector<std::shared_ptr<static_slot_data>> slot_data::full_item_data_;
        std::unordered_map<uint32_t, std::unordered_map<std::string, uint32_t>> static_slot_data::internal_item_aliases_protocol;

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

        base_objects::slot_component::inner::application_effect parse_application_effect(enbt::compound_const_ref ref) {
            auto type = ref.at("type").as_string();
            if (type == "minecraft:apply_effects") {
                base_objects::slot_component::inner::apply_effects ap_ef;
                for (auto& it : ref.at("effects").as_array())
                    ap_ef.effects.push_back(parse_item_potion_effect(it.as_compound()));
                ap_ef.effects.commit();
                if (ref.contains("probability"))
                    ap_ef.probability = ref["probability"];
                return ap_ef;
            } else if (type == "minecraft:remove_effects") {
                base_objects::slot_component::inner::remove_effects rem_ef;
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
                return base_objects::slot_component::inner::clear_all_effects{};
            else if (type == "minecraft:teleport_randomly") {
                if (ref.contains("diameter"))
                    return base_objects::slot_component::inner::teleport_randomly{ref["diameter"]};
                else
                    return base_objects::slot_component::inner::teleport_randomly{};
            } else if (type == "minecraft:play_sound") {
                if (ref.at("sound").is_string())
                    return base_objects::slot_component::inner::play_sound{.sound = ref["sound"].as_string()};
                else {
                    auto c = ref.at("sound").as_compound();
                    base_objects::slot_component::inner::sound_extended sound;
                    sound.sound_name = c.at("sound_id").as_string();
                    if (c.contains("range"))
                        sound.fixed_range = c["range"];
                    return base_objects::slot_component::inner::play_sound{.sound = sound};
                }
            } else
                throw std::runtime_error("Not implemented");
        }

        std::unordered_map<std::string, base_objects::slot_component::unified (*)(const enbt::value&)> load_items_parser{
            {"minecraft:attribute_modifiers",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::attribute_modifiers attributes;
                 if (comp.contains("show_in_tooltip"))
                     attributes.show_in_tooltip = comp["show_in_tooltip"];

                 if (comp.contains("modifiers")) {
                     list_array<base_objects::item_attribute> modifiers;
                     for (auto& i : comp["modifiers"].as_array()) {
                         base_objects::item_attribute modifier;

                         auto c = i.as_compound();
                         modifier.type = c.at("type").as_string();
                         modifier.id = c.at("id").as_string();
                         modifier.operation = base_objects::item_attribute::id_to_operation((std::string)c.at("operation").as_string());
                         modifier.amount = c.at("amount");
                         modifier.slot = base_objects::item_attribute::name_to_slot((std::string)c.at("slot").as_string());
                         modifiers.push_back(modifier);
                     }
                     modifiers.commit();
                     attributes.attributes = std::move(modifiers);
                 }
                 return std::move(attributes);
             }
            },
            {"minecraft:banner_patterns",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::banner_patterns banner_patterns;
                 for (auto& value : it.as_array()) {
                     auto comp = value.as_compound();
                     base_objects::slot_component::banner_patterns::pattern pattern;
                     pattern.color = base_objects::item_color::from_string(comp.at("color").as_string());
                     auto& pattern_json = comp.at("pattern");
                     if (pattern_json.is_string())
                         pattern.pattern = (std::string)pattern_json.as_string();
                     else {
                         auto custom = pattern_json.as_compound();
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
             [](const enbt::value& it) -> base_objects::slot_component::unified {
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
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::bees bees;
                 for (auto& bee_ : it.as_array()) {
                     auto bee = bee_.as_compound();
                     bees.values.push_back({base_objects::slot_component::bees::bee{
                         .entity_data = bee.at("entity_data"),
                         .ticks_in_hive = (int32_t)bee.at("ticks_in_hive"),
                         .min_ticks_in_hive = (int32_t)bee.at("min_ticks_in_hive"),
                     }});
                 }
                 bees.values.commit();
                 return std::move(bees);
             }
            },
            {"minecraft:block_entity_data",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::block_entity_data res;
                 res.value = it;
                 return res;
             }
            },
            {"minecraft:block_state",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::block_state block_state;
                 for (auto& [state, value] : it.as_compound())
                     block_state.properties.push_back({(std::string)state, (std::string)value.as_string()});
                 block_state.properties.commit();
                 return std::move(block_state);
             }
            },
            {"minecraft:bucket_entity_data",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::bucket_entity_data res;
                 res.value = it;
                 return res;
             }
            },
            {"minecraft:bundle_contents",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::bundle_contents bundle_contents;
                 for (auto& item : it.as_array()) {
                     bundle_contents.items.push_back(new base_objects::slot_data(slot_data::from_enbt(item.as_compound())));
                 }
                 bundle_contents.items.commit();
                 return std::move(bundle_contents);
             }
            },
            {"minecraft:can_break",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::can_break can_break;
                 if (comp.contains("show_in_tooltip"))
                     can_break.show_in_tooltip = comp["show_in_tooltip"];
                 if (comp.contains("blocks")) {
                     enbt::compound pred;
                     pred["blocks"] = comp["blocks"];
                     if (comp.contains("state"))
                         pred["state"] = comp["state"];
                     if (comp.contains("nbt"))
                         pred["nbt"] = comp["nbt"];
                     can_break.predicates.push_back(std::move(pred));
                 }
                 if (comp.contains("predicates")) {
                     for (auto& it : comp.at("predicates").as_array()) {
                         auto item = it.as_compound();
                         enbt::compound pred;
                         pred["blocks"] = item["blocks"];
                         if (item.contains("state"))
                             pred["state"] = item["state"];
                         if (item.contains("nbt"))
                             pred["nbt"] = item["nbt"];
                         can_break.predicates.push_back(std::move(pred));
                     }
                 }
                 can_break.predicates.commit();
                 return std::move(can_break);
             }
            },
            {"minecraft:can_place_on",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::can_place_on can_place_on;
                 if (comp.contains("show_in_tooltip"))
                     can_place_on.show_in_tooltip = comp["show_in_tooltip"];
                 if (comp.contains("blocks")) {
                     enbt::compound pred;
                     pred["blocks"] = comp["blocks"];
                     if (comp.contains("state"))
                         pred["state"] = comp["state"];
                     if (comp.contains("nbt"))
                         pred["nbt"] = comp["nbt"];
                     can_place_on.predicates.push_back(std::move(pred));
                 }
                 if (comp.contains("predicates")) {
                     for (auto& it : comp.at("predicates").as_array()) {
                         auto item = it.as_compound();
                         enbt::compound pred;
                         pred["blocks"] = item["blocks"];
                         if (item.contains("state"))
                             pred["state"] = item["state"];
                         if (item.contains("nbt"))
                             pred["nbt"] = item["nbt"];
                         can_place_on.predicates.push_back(std::move(pred));
                     }
                 }
                 can_place_on.predicates.commit();
                 return std::move(can_place_on);
             }
            },
            {"minecraft:charged_projectiles",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::charged_projectiles charged_projectiles;
                 for (auto& item : it.as_array()) {
                     charged_projectiles.data.push_back(new base_objects::slot_data(slot_data::from_enbt(item.as_compound())));
                 }
                 charged_projectiles.data.commit();
                 return std::move(charged_projectiles);
             }
            },
            {"minecraft:container",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::container container;
                 for (auto& item : it.as_array()) {
                     auto comp = item.as_compound();
                     uint8_t slot = comp.at("slot");
                     container.set(slot, slot_data::from_enbt(comp.at("item").as_compound()));
                 }
                 return std::move(container);
             }
            },
            {"minecraft:container_loot",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::container_loot container_loot;
                 container_loot.loot_table = comp.at("loot_table").as_string();
                 container_loot.seed = comp.at("seed");
                 return std::move(container_loot);
             }
            },
            {"minecraft:custom_data",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::custom_data custom_data;
                 if (it.is_string()) {
                     try {
                         custom_data.value = senbt::parse(it.as_string());
                     } catch (...) {
                         custom_data.value = it;
                     }
                 } else
                     custom_data.value = it;
                 return std::move(custom_data);
             }
            },
            {"minecraft:custom_model_data",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::custom_model_data{.value = (int32_t)it};
             }
            },
            {"minecraft:custom_name",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::custom_name{
                     .value = Chat::fromEnbt(it)
                 };
             }
            },
            {"minecraft:damage",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::damage{
                     .value = (int32_t)it
                 };
             }
            },
            {"minecraft:debug_stick_state",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::debug_stick_state debug_stick_state;
                 debug_stick_state.previous_state = it;
                 return std::move(debug_stick_state);
             }
            },
            {"minecraft:dyed_color",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::dyed_color dyed_color;
                 if (comp.contains("show_in_tooltip"))
                     dyed_color.show_in_tooltip = comp["show_in_tooltip"];
                 dyed_color.rgb = comp.at("rgb");
                 return std::move(dyed_color);
             }
            },
            {"minecraft:enchantment_glint_override",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::enchantment_glint_override enchantment_glint_override;
                 enchantment_glint_override.has_glint = it;
                 return std::move(enchantment_glint_override);
             }
            },
            {"minecraft:enchantments",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::enchantments enchantments;
                 for (auto& [enchantment, level] : comp.at("levels").as_compound())
                     enchantments.enchants.push_back({registers::enchantments.at((std::string)enchantment).id, level});
                 enchantments.enchants.commit();
                 if (comp.contains("show_in_tooltip"))
                     enchantments.show_in_tooltip = comp.at("show_in_tooltip");
                 return std::move(enchantments);
             }
            },
            {"minecraft:entity_data",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 if (!comp.contains("id"))
                     throw std::runtime_error("Entity declaration must contain at least id of entity");
                 base_objects::slot_component::entity_data entity_data;
                 entity_data.value = it;
                 return std::move(entity_data);
             }
            },
            {"minecraft:entity_data",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 if (!comp.contains("id"))
                     throw std::runtime_error("Entity declaration must contain at least id of entity");
                 base_objects::slot_component::entity_data entity_data;
                 entity_data.value = it;
                 return std::move(entity_data);
             }
            },
            {"minecraft:firework_explosion",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 auto& shape = comp.at("shape").as_string();
                 auto colors = comp.at("shape").as_array();
                 auto fade_colors = comp.at("shape").as_array();
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

                 return base_objects::slot_component::firework_explosion{.value = std::move(res)};
             }
            },
            {"minecraft:fireworks",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::fireworks fireworks;
                 fireworks.duration = comp.at("flight_duration");
                 auto& firework_explosion_parser = load_items_parser.at("minecraft:firework_explosion");
                 if (comp.contains("explosions")) {
                     auto arr = comp.at("explosions").as_array();
                     fireworks.explosions.reserve(arr.size());
                     for (auto& it : arr) {
                         auto pre_res = std::get<base_objects::slot_component::firework_explosion>(
                             firework_explosion_parser(it)
                         );
                         fireworks.explosions.push_back(std::move(pre_res.value));
                     }
                 }
                 return std::move(fireworks);
             }
            },
            {"minecraft:food",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::food food;
                 food.nutrition = comp.at("nutrition");
                 food.saturation = comp.at("saturation");
                 if (comp.contains("can_always_eat"))
                     food.can_always_eat = comp.at("can_always_eat");
                 return std::move(food);
             }
            },
            {"minecraft:hide_additional_tooltip",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::hide_additional_tooltip{};
             }
            },
            {"minecraft:hide_tooltip",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::hide_tooltip{};
             }
            },
            {"minecraft:instrument",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 if (it.is_string())
                     return base_objects::slot_component::instrument{(std::string)it.as_string()};
                 else {
                     auto comp = it.as_compound();
                     base_objects::slot_component::instrument::type_extended instrument;
                     instrument.duration = comp.at("use_duration");
                     instrument.range = comp.at("range");
                     if (comp.at("sound").is_string())
                         instrument.sound = (std::string)comp.at("sound").as_string();
                     else {
                         auto sound = comp.at("sound").as_compound();
                         base_objects::slot_component::inner::sound_extended sound_extended;
                         sound_extended.sound_name = sound.at("sound_id").as_string();
                         if (sound.contains("range"))
                             sound_extended.fixed_range = sound.at("range");
                         instrument.sound = sound_extended;
                     }
                     return base_objects::slot_component::instrument{instrument};
                 }
             }
            },
            {"minecraft:intangible_projectile",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::intangible_projectile{};
             }
            },
            {"minecraft:item_name",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::item_name{.value = Chat::fromStr((std::string)it.as_string())};
             }
            },
            {"minecraft:jukebox_playable",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::jukebox_playable jukebox_playable;
                 jukebox_playable.song = (std::string)comp.at("song").as_string();
                 if (comp.contains("show_in_tooltip"))
                     jukebox_playable.show_in_tooltip = comp.at("show_in_tooltip");
                 return std::move(jukebox_playable);
             }
            },
            {"minecraft:lock",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::lock{.key = (std::string)it.as_string()};
             }
            },
            {"minecraft:lodestone_tracker",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::lodestone_tracker lodestone_tracker;
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
             }
            },
            {"minecraft:lore",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::lore lore;
                 for (auto& line : it.as_array())
                     lore.value.push_back(Chat::fromStr((std::string)line.as_string()));
                 lore.value.commit();
                 return std::move(lore);
             }
            },
            {"minecraft:map_color",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::map_color{(int32_t)it};
             }
            },
            {"minecraft:map_decorations",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::map_decorations map_decorations;
                 map_decorations.value = it;
                 return std::move(map_decorations);
             }
            },
            {"minecraft:map_id",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::map_id{it};
             }
            },
            {"minecraft:max_damage",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::max_damage{it};
             }
            },
            {"minecraft:max_stack_size",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::max_stack_size{it};
             }
            },
            {"minecraft:note_block_sound",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::note_block_sound{(std::string)it.as_string()};
             }
            },
            {"minecraft:ominous_bottle_amplifier",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::ominous_bottle_amplifier{it};
             }
            },
            {"minecraft:pot_decorations",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_array();
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
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 base_objects::slot_component::potion_contents potion_contents;
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
             }
            },
            {"minecraft:profile",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::profile profile;
                 if (comp.contains("name"))
                     profile.name = comp["name"].as_string();
                 if (comp.contains("id"))
                     profile.uid = (enbt::raw_uuid)comp["id"];
                 if (comp.contains("properties")) {
                     auto props = comp["properties"].as_array();
                     profile.properties.reserve(props.size());
                     for (auto& it : props) {
                         auto prop = it.as_compound();
                         base_objects::slot_component::profile::property_t p;
                         p.name = prop.at("name").as_string();
                         p.value = prop.at("value").as_string();
                         if (prop.contains("signature"))
                             p.signature = prop["signature"].as_string();
                         profile.properties.push_back(std::move(p));
                     }
                 }
                 return profile;
             }
            },
            {"minecraft:rarity",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::rarity::from_string(it.as_string());
             }
            },
            {"minecraft:recipes",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto arr = it.as_array();
                 base_objects::slot_component::recipes recipes;
                 recipes.value.reserve(arr.size());
                 for (auto& it : arr)
                     recipes.value.push_back(it.as_string());
                 return recipes;
             }
            },
            {"minecraft:repairable",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 if (it.is_array()) {
                     auto arr = it.at("items").as_array();
                     std::vector<std::string> res;
                     res.reserve(arr.size());
                     for (auto& it : arr)
                         res.push_back(it.as_string());
                     return base_objects::slot_component::repairable{.items = std::move(res)};
                 } else
                     return base_objects::slot_component::repairable{.items = it.at("items").as_string()};
             }
            },
            {"minecraft:repair_cost",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::repair_cost{.value = it};
             }
            },
            {"minecraft:stored_enchantments",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::stored_enchantments stored_enchantments;
                 if (comp.contains("show_in_tooltip"))
                     stored_enchantments.show_in_tooltip = comp["show_in_tooltip"];
                 auto levels = comp.at("levels").as_compound();
                 stored_enchantments.enchants.reserve(levels.size());
                 for (auto& [name, level] : levels) {
                     auto id = registers::enchantments.at(name).id;
                     stored_enchantments.enchants.push_back({id, (int32_t)level});
                 }
                 return stored_enchantments;
             }
            },
            {"minecraft:suspicious_stew_effects",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto arr = it.as_array();
                 base_objects::slot_component::suspicious_stew_effects suspicious_stew_effects;
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
             }
            },
            {"minecraft:tool",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::tool tool;
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
             }
            },
            {"minecraft:tooltip_style",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::tooltip_style{.value = it.as_string()};
             }
            },
            {"minecraft:trim",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::trim trim; //TODO extend
                 trim.pattern = registers::armorTrimPatterns.at(comp.at("pattern").as_string()).id;
                 trim.material = registers::armorTrimMaterials.at(comp.at("material").as_string()).id;
                 if (comp.contains("show_in_tooltip"))
                     trim.show_in_tooltip = comp["show_in_tooltip"];
                 return trim;
             }
            },
            {"minecraft:unbreakable",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::unbreakable{.value = it.at("show_in_tooltip")};
             }
            },
            {"minecraft:use_cooldown",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::use_cooldown use_cooldown;
                 use_cooldown.seconds = comp.at("seconds");
                 if (comp.contains("cooldown_group"))
                     use_cooldown.cooldown_group = comp.at("cooldown_group").as_string();
                 return use_cooldown;
             }
            },
            {"minecraft:use_remainder",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 if (!comp.contains("components")) {
                     return base_objects::slot_component::use_remainder(weak_slot_data(comp["id"].as_string(), comp["count"]));
                 } else
                     return base_objects::slot_component::use_remainder(slot_data::from_enbt(it.as_compound()));
             }
            },
            {"minecraft:writable_book_content",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::writable_book_content writable_book_content;
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
             }
            },
            {"minecraft:written_book_content",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::written_book_content written_book_content;
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
                         written_book_content.pages.push_back({.text = it.as_string()});
                     else {
                         auto com = it.as_compound();
                         auto raw = com.at("raw").as_string();
                         std::optional<std::string> filtered;
                         if (com.contains("filtered"))
                             filtered = com["filtered"].as_string();
                         written_book_content.pages.push_back({raw, filtered});
                     }
                 }
                 return written_book_content;
             }
            },
            {"minecraft:creative_slot_lock",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::creative_slot_lock{};
             }
            },
            {"minecraft:map_post_processing",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::map_post_processing{.value = it};
             }
            },
            {"minecraft:item_model",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::item_model{.value = it.as_string()};
             }
            },
            {"minecraft:glider",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::glider{};
             }
            },
            {"minecraft:equippable",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::equippable equippable;
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
                     else {
                         auto equip_sound_com = equip_sound.as_compound();
                         equippable.equip_sound = base_objects::slot_component::equippable::equip_sound_custom{
                             .sound_id = equip_sound_com.at("sound_id").as_string(),
                             .range = equip_sound_com.at("range")
                         };
                     }
                 } else
                     equippable.equip_sound = nullptr;
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
             }
            },
            {"minecraft:enchantable",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::enchantable{.value = it.at("value")};
             }
            },
            {"minecraft:death_protection",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::death_protection death_protection;
                 if (comp.contains("death_effects")) {
                     list_array<base_objects::slot_component::inner::application_effect> res;
                     for (auto& it : comp.at("death_effects").as_array())
                         res.push_back(parse_application_effect(it.as_compound()));
                     res.commit();
                     death_protection.death_effects = res;
                 }
                 return death_protection;
             }
            },
            {"minecraft:damage_resistant",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 return base_objects::slot_component::damage_resistant{.types = it.at("types").as_string()};
             }
            },
            {"minecraft:consumable",
             [](const enbt::value& it) -> base_objects::slot_component::unified {
                 auto comp = it.as_compound();
                 base_objects::slot_component::consumable consumable;
                 if (comp.contains("consume_seconds"))
                     consumable.consume_seconds = comp["consume_seconds"];
                 if (comp.contains("animation"))
                     consumable.animation = comp["animation"].as_string();
                 if (comp.contains("sound")) {
                     if (comp["sound"].is_string())
                         consumable.sound = comp["sound"].as_string();
                     else {
                         auto c = comp["sound"].as_compound();
                     }
                 }
                 if (comp.contains("on_consume_effects")) {
                     list_array<base_objects::slot_component::inner::application_effect> res;
                     for (auto& it : comp.at("on_consume_effects").as_array())
                         res.push_back(parse_application_effect(it.as_compound()));
                     res.commit();
                     consumable.on_consume_effects = res;
                 }
                 return consumable;
             }
            },
        };

        base_objects::slot_component::unified slot_component::parse_component(const std::string& name, const enbt::value& item) {
            return load_items_parser.at(name)(item);
        }

        slot_data slot_data::from_enbt(enbt::compound_const_ref compound) {
            auto& id = compound.at("id").as_string();
            base_objects::slot_data slot_data = base_objects::slot_data::create_item(id);
            if (compound.contains("count"))
                slot_data.count = compound.at("count");
            std::unordered_map<std::string, base_objects::slot_component::unified> components;
            for (auto& [name, value] : compound.at("components").as_compound())
                slot_data.add_component(load_items_parser.at(name)(value));
            return slot_data;
        }

        enbt::compound slot_data::to_enbt() const {
            return {};
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


        std::string item_color::to_string() const {
            switch (value) {
            case white:
                return "white";
            case orange:
                return "orange";
            case magenta:
                return "magenta";
            case light_blue:
                return "light_blue";
            case yellow:
                return "yellow";
            case lime:
                return "lime";
            case pink:
                return "pink";
            case gray:
                return "gray";
            case light_gray:
                return "light_gray";
            case cyan:
                return "cyan";
            case purple:
                return "purple";
            case blue:
                return "blue";
            case brown:
                return "brown";
            case green:
                return "green";
            case red:
                return "red";
            case black:
                return "black";
            default:
                return "undefined";
            }
        }

        item_color item_color::from_string(const std::string& color) {
            if (color == "white")
                return white;
            else if (color == "orange")
                return orange;
            else if (color == "magenta")
                return magenta;
            else if (color == "light_blue")
                return light_blue;
            else if (color == "yellow")
                return yellow;
            else if (color == "lime")
                return lime;
            else if (color == "pink")
                return pink;
            else if (color == "gray")
                return gray;
            else if (color == "light_gray")
                return light_gray;
            else if (color == "cyan")
                return cyan;
            else if (color == "purple")
                return purple;
            else if (color == "blue")
                return blue;
            else if (color == "brown")
                return brown;
            else if (color == "green")
                return green;
            else if (color == "red")
                return red;
            else if (color == "black")
                return black;
            else
                return white;
        }

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

        bool slot_component::charged_projectiles::operator==(const charged_projectiles& other) const {
            return data.equal(
                other.data,
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

        std::optional<uint8_t> slot_component::container::contains(const slot_data& item) const {
            std::optional<uint8_t> res;
            for_each([&res, &item](const slot_data& check, size_t index) {
                if (res) {
                    if (item == check)
                        res = (uint8_t)index;
                }
            });
            return res;
        }

        list_array<uint8_t> slot_component::container::contains(const std::string& id, size_t count) const {
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

        namespace slot_component {
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
                            if (assignations.find(alias) != assignations.end()) {
                                item.internal_item_aliases[protocol] = assignations[alias];
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
}