/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_COMPONENT
#define SRC_BASE_OBJECTS_COMPONENT
#include <array>
#include <compare>
#include <library/list_array.hpp>
#include <optional>
#include <src/api/ecs.hpp>
#include <src/api/packets/types.hpp>
#include <src/base_objects/box.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/dye_color.hpp>
#include <src/base_objects/position.hpp>
#include <src/util/cts.hpp>
#include <src/util/nbt.hpp>
#include <src/util/readers.hpp>
#include <string>

namespace copper_server::util {
    class nbt_read_stream;
    class nbt_write_stream;

    namespace nbt_collection {
        class compound_flex;
    }

    class nbt_write_compound_stream;
}

namespace copper_server::base_objects {
    struct slot;
    struct slot_data;

    struct item_firework_explosion {
        enum class shape_e : uint8_t {
            small_ball = 0,
            large_ball = 1,
            star = 2,
            creeper = 3,
            burst = 4,
        };
        enum_as<shape_e, var_int32> shape;
        list_array<int32_t> colors = {};      //rgb
        list_array<int32_t> fade_colors = {}; //rgb
        bool has_trail = false;
        bool has_twinkle = false;
        bool operator==(const item_firework_explosion& other) const = default;
    };

    struct sound_event_t {
        identifier sound_id;
        std::optional<float> range = std::nullopt;
        bool operator==(const sound_event_t& other) const = default;
    };

    struct potion_effect {
        struct data_t {
            var_int32 amplifier = 0;
            var_int32 duration = 0;
            bool ambient = false;
            bool show_particles = true;
            bool show_icon = false;
            std::optional<box<data_t>> hidden_effect = std::nullopt;

            data_t();
            data_t(data_t&&);
            data_t(const data_t&);
            data_t(var_int32 amplifier, var_int32 duration, bool is_ambient, bool show_particles, bool show_icon, std::optional<box<data_t>>&& hidden_effect);
            data_t(var_int32 amplifier, var_int32 duration, bool is_ambient = false, bool show_particles = true, bool show_icon = false, const std::optional<box<data_t>>& hidden_effect = std::nullopt);

            data_t& operator=(data_t&&);
            data_t& operator=(const data_t&);


            bool operator==(const data_t& other) const;
            bool operator!=(const data_t& other) const;

            using nbt_inline = void;
        };

        var_int32::potion id;
        data_t data;
        bool operator==(const potion_effect& other) const = default;
    };

    struct block_predicate {
        struct property {
            struct range : public enum_item<false> {
                std::optional<std::string> min;
                std::optional<std::string> max;
                bool operator==(const range& other) const = default;
            };

            struct exact : public enum_item<true> {
                std::string value;
                bool operator==(const exact& other) const = default;
                using nbt_inline = void;
            };

            enum_switch<bool, range, exact> is_exact;
            bool operator==(const property& other) const = default;
            using nbt_inline = void;
        };

        std::optional<id_set<var_int32::block_type>> blocks = std::nullopt;
        std::optional<std::unordered_map<std::string, property>> state = std::nullopt;
        std::optional<util::nbt> nbt = std::nullopt;
        list_array<component> full_components_match;            //for block entity
        list_array<partial_component> partial_components_match; //for block entity
    };

    struct consume_effect {
        struct apply_effects : public enum_item<0> {
            list_array<potion_effect> effects;
            float probability = 1.0f;
            bool operator==(const apply_effects& other) const = default;
        };

        struct remove_effects : public enum_item<1> {
            id_set<var_int32::mob_effect> effects;
            bool operator==(const remove_effects& other) const = default;
        };

        struct clear_all_effects : public enum_item<2> {
            bool operator==(const clear_all_effects& other) const = default;
        };

        struct teleport_randomly : public enum_item<3> {
            float diameter = 16.0f;
            bool operator==(const teleport_randomly& other) const = default;
        };

        struct play_sound : public enum_item<4> {
            sound_event_t sound;
            bool operator==(const play_sound& other) const = default;
        };

        enum_switch<
            var_int32,
            apply_effects,
            remove_effects,
            clear_all_effects,
            teleport_randomly,
            play_sound>
            effect;

        bool operator==(const consume_effect& other) const = default;
    };

    struct partial_component {
        var_int32::data_component_type type;
        util::nbt value;
        bool operator==(const partial_component& other) const = default;
    };

    struct weak_slot {
        depends_next<var_int32> count;
        var_int32::item id;
        bool operator==(const weak_slot& other) const = default;
    };

    struct trim_material {
        std::string asset_name;
        std::unordered_map<std::string, std::string> override_armor_assets;
        base_objects::chat description;
        bool operator==(const trim_material& other) const = default;
    };

    struct trim_pattern {
        std::string asset_name;
        var_int32::item template_item;
        base_objects::chat description;
        bool decal;
        bool operator==(const trim_pattern& other) const = default;
    };

    struct instrument {
        or_<var_int32::sound_event, sound_event_t> sound_event;
        float use_duration;
        float range;
        base_objects::chat description;
        bool operator==(const instrument& other) const = default;
    };

    struct jukebox_song {
        or_<var_int32::sound_event, sound_event_t> sound_event;
        base_objects::chat description;
        float length_in_seconds;
        var_int32 comparator_output;
        bool operator==(const jukebox_song& other) const = default;
    };

    template <util::CTS custom_name>
    struct component_custom_name {
        static inline constexpr std::string_view value = []() { return custom_name.data; }();
    };

    struct component {
        //for all components applied "minecraft" namespace except when component has custom_namespace
        //struct example_component{
        //
        //
        //    using actual_name = component_custom_name<"example/component">;
        //    using custom_namespace = component_custom_name<"copper_server">;
        //};


        struct custom_data : public enum_item<0> {
            util::nbt data;
            bool operator==(const custom_data& other) const = default;

            using nbt_inline = void;
        };

        struct max_stack_size : public enum_item<1> {
            limited_num<var_int32, 1, 99> size;
            bool operator==(const max_stack_size& other) const = default;
        };

        struct max_damage : public enum_item<2> {
            var_int32 dmg;
            bool operator==(const max_damage& other) const = default;
        };

        struct damage : public enum_item<3> {
            var_int32 dmg;
            bool operator==(const damage& other) const = default;
        };

        struct unbreakable : public enum_item<4> {
            bool operator==(const unbreakable& other) const = default;
        };

        struct custom_name : public enum_item<5> {
            base_objects::chat name;
            bool operator==(const custom_name& other) const = default;
            using nbt_inline = void;
        };

        struct item_name : public enum_item<6> {
            base_objects::chat name;
            bool operator==(const item_name& other) const = default;
            using nbt_inline = void;
        };

        struct item_model : public enum_item<7> {
            identifier model;
            bool operator==(const item_model& other) const = default;
            using nbt_inline = void;
        };

        struct lore : public enum_item<8> {
            list_array_sized<base_objects::chat, 256> lines;
            bool operator==(const lore& other) const = default;
            using nbt_inline = void;
        };

        struct rarity : public enum_item<9> {
            enum class rarity_e : uint8_t {
                common = 0,
                uncommon = 1,
                rare = 2,
                epic = 3,
            };
            using enum rarity_e;

            enum_as<rarity_e, var_int32> rarity;
            bool operator==(const struct rarity& other) const = default;
            using nbt_inline = void;
        };

        struct enchantments : public enum_item<10> {
            std::unordered_map<
                var_int32::enchantment,
                limited_num<var_int32, 1, 255>>
                enchantments;

            bool operator==(const struct enchantments& other) const = default;
        };

        struct can_place_on : public enum_item<11> {
            list_array<block_predicate> predicates;

            bool operator==(const can_place_on& other) const;
            using nbt_inline = void;
        };

        struct can_break : public enum_item<12> {
            list_array<block_predicate> predicates;

            bool operator==(const can_break& other) const;
            using nbt_inline = void;
        };

        struct attribute_modifiers : public enum_item<13> {
            struct attribute {
                enum class operation_e {
                    add_value = 0,
                    add_multiplied_base = 1,
                    add_multiplied_total = 2,
                };
                using enum operation_e;

                enum class slot_e {
                    any = 0,
                    main_hand = 1,
                    off_hand = 2,
                    hand = 3,
                    feet = 4,
                    legs = 5,
                    chest = 6,
                    head = 7,
                    armor = 8,
                    body = 9,
                };

                struct display_t {
                    struct default_t : public enum_item<0> {
                        bool operator==(const default_t& other) const = default;
                        using name_override = util::cts_string<"default">;
                    };

                    struct hidden_t : public enum_item<1> {
                        bool operator==(const hidden_t& other) const = default;
                        using name_override = util::cts_string<"hidden">;
                    };

                    struct override_t : public enum_item<3> {
                        base_objects::chat value;

                        bool operator==(const override_t& other) const = default;
                        using name_override = util::cts_string<"override">;
                    };

                    enum_switch<uint8_t, default_t, hidden_t, override_t> value;

                    bool operator==(const display_t& other) const = default;
                    using nbt_inline = void;
                };

                var_int32::attribute type;
                identifier id;
                double amount;
                enum_as<operation_e, var_int32> operation;
                enum_as<slot_e, var_int32> slot;
                display_t display = {display_t::default_t()};

                bool operator==(const attribute& other) const = default;
            };

            list_array<attribute> attributes;

            std::strong_ordering operator<=>(const attribute_modifiers& other) const {
                return attributes <=> other.attributes;
            }
            using nbt_inline = void;
        };

        struct custom_model_data : public enum_item<14> {
            list_array<float> floats = {};
            list_array<bool> flags = {};
            list_array<std::string> strings = {};
            list_array<int32_t> colors = {};
            bool operator==(const custom_model_data& other) const = default;
        };

        struct tooltip_display : public enum_item<15> {
            bool hide_tooltip = false;
            list_array<var_int32::data_component_type> hidden_components = {};
            bool operator==(const tooltip_display& other) const = default;
        };

        struct repair_cost : public enum_item<16> {
            var_int32 cost;
            bool operator==(const repair_cost& other) const = default;
            using nbt_inline = void;
        };

        struct creative_slot_lock : public enum_item<17> {
            bool operator==(const creative_slot_lock& other) const = default;
        };

        struct enchantment_glint_override : public enum_item<18> {
            bool has;
            bool operator==(const enchantment_glint_override& other) const = default;
            using nbt_inline = void;
        };

        struct intangible_projectile : public enum_item<19> {
            util::nbt value_compound;
            bool operator==(const intangible_projectile& other) const = default;
            using nbt_inline = void;
        };

        struct food : public enum_item<20> {
            var_int32 nutrition;
            float saturation;
            bool can_always_eat = false;
            bool operator==(const food& other) const = default;
        };

        struct consumable : public enum_item<21> {
            enum class animation_e : uint8_t {
                none = 0,
                eat = 1,
                dring = 2,
                block = 3,
                bow = 4,
                spear = 5,
                crossbow = 6,
                spyglass = 7,
                toot_horn = 8,
                brush = 9,
            };
            float consume_seconds = 1.6f;
            enum_as<animation_e, var_int32> animation = animation_e::eat;
            or_<var_int32::sound_event, sound_event_t> sound = var_int32::sound_event("entity.generic.eat");
            bool has_consume_particles = true;
            list_array<consume_effect> on_consume_effects;
            bool operator==(const consumable& other) const = default;
        };

        struct use_remainder : public enum_item<22> {
            box<slot> remainder;

            use_remainder();
            use_remainder(const slot& remainder);
            use_remainder(slot&& remainder);
            use_remainder(const box<slot>& remainder);
            use_remainder(box<slot>&& remainder);
            use_remainder(const use_remainder& remainder);
            use_remainder(use_remainder&& remainder);
            ~use_remainder();

            use_remainder& operator=(const slot& remainder);
            use_remainder& operator=(slot&& remainder);
            use_remainder& operator=(const box<slot>& remainder);
            use_remainder& operator=(box<slot>&& remainder);
            use_remainder& operator=(const use_remainder& remainder);
            use_remainder& operator=(use_remainder&& remainder);

            bool operator==(const use_remainder& other) const;
            using nbt_inline = void;
        };

        struct use_cooldown : public enum_item<23> {
            float seconds;
            std::optional<identifier> cooldown_group = std::nullopt;
            bool operator==(const use_cooldown& other) const = default;
        };

        struct damage_resistant : public enum_item<24> {
            identifier types; //tag without #
            bool operator==(const damage_resistant& other) const = default;
        };

        struct tool : public enum_item<25> {
            struct rule {
                id_set<var_int32::block_type> blocks;
                std::optional<float> speed = std::nullopt;
                std::optional<bool> correct_for_drops = std::nullopt;
                bool operator==(const rule& other) const = default;
            };

            list_array<rule> rules;
            float default_mining_speed = 1.0f;
            var_int32 damage_per_block = 1;
            bool can_destroy_blocks_in_creative = true;
            bool operator==(const tool& other) const = default;
        };

        struct weapon : public enum_item<26> {
            var_int32 item_damage_per_attack = 1;
            float disable_blocking_for_seconds = 0.0f; //axe 5.0f
            bool operator==(const weapon& other) const = default;
        };

        struct enchantable : public enum_item<27> {
            var_int32 value;
            bool operator==(const enchantable& other) const = default;
        };

        struct equippable : public enum_item<28> {
            enum class equippable_on_e {
                main_hand = 0,
                feet = 1,
                legs = 2,
                chest = 3,
                head = 4,
                off_hand = 5,
                body = 6,
            };
            enum_as<equippable_on_e, var_int32> slot;
            or_<var_int32::sound_event, sound_event_t> equip_sound = var_int32::sound_event("item.armor.equip_generic");
            std::optional<identifier> asset_id = std::nullopt;
            std::optional<identifier> camera_overlay = std::nullopt;
            std::optional<id_set<var_int32::entity_type>> allowed_entities = std::nullopt;
            bool dispensable = true;
            bool swappable = true;
            bool equip_on_interact = false;
            bool can_be_sheared = false;
            or_<var_int32::sound_event, sound_event_t> shearing_sound = var_int32::sound_event("item.shears.snip");

            bool operator==(const equippable& other) const = default;
        };

        struct repairable : public enum_item<29> {
            id_set<var_int32::item> items;
            bool operator==(const repairable& other) const = default;
        };

        struct glider : public enum_item<30> {
            bool operator==(const glider& other) const = default;
        };

        struct tooltip_style : public enum_item<31> {
            identifier style;
            bool operator==(const tooltip_style& other) const = default;
            using nbt_inline = void;
        };

        struct death_protection : public enum_item<32> {
            list_array<consume_effect> death_effects = {};

            static death_protection totem_instance() {
                death_protection result;
                result.death_effects.emplace_back(consume_effect::clear_all_effects{});
                result.death_effects.emplace_back(
                    consume_effect::apply_effects{
                        .effects = {
                            potion_effect{
                                .id = var_int32::potion("minecraft:regeneration"),
                                .data = potion_effect::data_t{1, 900}
                            },
                            potion_effect{
                                .id = var_int32::potion("minecraft:absorption"),
                                .data = potion_effect::data_t{1, 100}
                            },
                            potion_effect{
                                .id = var_int32::potion("minecraft:fire_resistance"),
                                .data = potion_effect::data_t{0, 800}
                            }
                        }
                    }
                );
                return result;
            }

            bool operator==(const death_protection& other) const = default;
        };

        struct blocks_attacks : public enum_item<33> {
            struct damage_reductions {
                float horizontal_blocking_angle = 90.0f;
                std::optional<id_set<var_int32::damage_type>> type = std::nullopt;
                float base = 0.0f;
                float factor = 1.0f;
                bool operator==(const damage_reductions& other) const = default;
            };

            struct item_damage_t {
                float threshold = 1.0f;
                float base = 0.0f;
                float factor = 1.0f;
            };

            float block_delay_seconds = 0.0f;
            float disable_cooldown_scale = 1.0f;
            list_array<damage_reductions> damage_reductions = {{}};
            std::optional<item_damage_t> item_damage = std::make_optional<item_damage_t>();
            std::optional<identifier> bypassed_by = std::nullopt;
            std::optional<or_<var_int32::sound_event, sound_event_t>> block_sound = std::nullopt;
            std::optional<or_<var_int32::sound_event, sound_event_t>> disabled_sound = std::nullopt;
            bool operator==(const blocks_attacks& other) const = default;
        };

        struct stored_enchantments : public enum_item<34> {
            std::unordered_map<
                var_int32::enchantment,
                limited_num<var_int32, 1, 255>>
                enchantments;
            bool operator==(const stored_enchantments& other) const = default;
            using nbt_inline = void;
        };

        struct dyed_color : public enum_item<35> {
            int32_t rgb = 0xFFA0'6540;
            bool operator==(const dyed_color& other) const = default;
            using nbt_inline = void;
        };

        struct map_color : public enum_item<36> {
            int32_t rgb = 0x0046'402E;
            bool operator==(const map_color& other) const = default;
            using nbt_inline = void;
        };

        struct map_id : public enum_item<37> {
            var_int32 id;
            bool operator==(const map_id& other) const = default;
            using nbt_inline = void;
        };

        struct map_decorations : public enum_item<38> {
            struct decoration_t {
                api::id::map_decoration_type type;
                double x;
                double y;
                float rotation;

                bool operator==(const decoration_t& other) const = default;
            };

            std::unordered_map<std::string, decoration_t> decorations;
            bool operator==(const map_decorations& other) const = default;
            using nbt_inline = void;
            using packet_as_nbt = void;
        };

        struct map_post_processing : public enum_item<39> {
            enum class type_e : uint8_t {
                lock = 0,
                scale = 1,
            };

            enum_as<type_e, var_int32> type;
            bool operator==(const map_post_processing& other) const = default;
        };

        struct charged_projectiles : public enum_item<40> {
            list_array<slot> projectiles;
            bool operator==(const charged_projectiles& other) const;
            using nbt_inline = void;
        };

        struct bundle_contents : public enum_item<41> {
            list_array<slot> content;
            bool operator==(const bundle_contents& other) const;
            using nbt_inline = void;
        };

        struct potion_contents : public enum_item<42> {
            std::optional<var_int32::potion> potion = std::nullopt;
            std::optional<int32_t> custom_color = std::nullopt;
            list_array<potion_effect> custom_effects = {};
            std::optional<std::string> custom_name;
            bool operator==(const potion_contents& other) const = default;
        };

        struct potion_duration_scale : public enum_item<43> {
            float multiplier;
            bool operator==(const potion_duration_scale& other) const = default;
            using nbt_inline = void;
        };

        struct suspicious_stew_effects : public enum_item<44> {
            struct effect {
                var_int32::potion id;
                var_int32 duration = 160;
                bool operator==(const effect& other) const = default;
            };

            list_array<effect> effects;
            bool operator==(const suspicious_stew_effects& other) const = default;
            using nbt_inline = void;
        };

        struct writable_book_content : public enum_item<45> {
            struct page {
                string_sized<1024> raw;
                std::optional<string_sized<1024>> filtered = std::nullopt;
                bool operator==(const page& other) const = default;
            };

            list_array_sized<page, 100> pages;
            bool operator==(const writable_book_content& other) const = default;
        };

        struct written_book_content : public enum_item<46> {
            struct page {
                string_sized<1024> raw;
                std::optional<string_sized<1024>> filtered = std::nullopt;
                bool operator==(const page& other) const = default;
            };

            string_sized<32> title;
            std::string author;
            var_int32 generation = 0;
            list_array_sized<page, 100> pages = {};
            bool resolved = false;
            bool operator==(const written_book_content& other) const = default;
        };

        struct trim : public enum_item<47> {
            or_<var_int32::trim_material, trim_material> material;
            or_<var_int32::trim_pattern, trim_pattern> pattern;
            bool operator==(const trim& other) const = default;
        };

        struct debug_stick_state : public enum_item<48> {
            std::unordered_map<api::id::block_type, std::string> block_to_property;

            bool operator==(const debug_stick_state& other) const = default;
            using nbt_inline = void;
            using packet_as_nbt = void;
        };

        struct entity_data : public enum_item<49> {
            api::id::entity_type type;
            util::nbt nbt;
            bool operator==(const entity_data& other) const = default;
        };

        struct bucket_entity_data : public enum_item<50> {
            util::nbt data;

            bool operator==(const bucket_entity_data& other) const = default;
            using nbt_inline = void;
        };

        struct block_entity_data : public enum_item<51> {
            api::id::block_entity_type type;
            util::nbt nbt;
            bool operator==(const block_entity_data& other) const = default;
        };

        struct instrument : public enum_item<52> {
            or_<var_int32::instrument, base_objects::instrument> value;
            bool operator==(const instrument& other) const = default;
            using nbt_inline = void;
        };

        struct provides_trim_material : public enum_item<53> {
            struct reference : public default_enum_item<0> {
                identifier name;
                bool operator==(const reference& other) const = default;
                using nbt_inline = void;
            };

            struct direct : public enum_item<1> {
                or_<var_int32::trim_material, trim_material> value;
                bool operator==(const direct& other) const = default;
                using nbt_inline = void;
            };

            enum_switch<uint8_t, reference, direct> material;

            bool operator==(const provides_trim_material& other) const = default;
            using nbt_inline = void;
        };

        struct ominous_bottle_amplifier : public enum_item<54> {
            limited_num<var_int32, 0, 4> amplifier;
            bool operator==(const ominous_bottle_amplifier& other) const = default;
            using nbt_inline = void;
        };

        struct jukebox_playable : public enum_item<55> { //TODO recheck this component

            //would fail to parse in client, use direct one
            struct reference : public default_enum_item<0> {
                identifier name;
                bool operator==(const reference& other) const = default;
                using nbt_inline = void;
            };

            struct direct : public enum_item<1> {
                or_<var_int32::sound_event, jukebox_song> value;
                bool operator==(const direct& other) const = default;
                using nbt_inline = void;
            };

            enum_switch<uint8_t, reference, direct> song;
            bool operator==(const jukebox_playable& other) const = default;
            using nbt_inline = void;
        };

        struct provides_banner_patterns : public enum_item<56> {
            identifier key;
            bool operator==(const provides_banner_patterns& other) const = default;
            using nbt_inline = void;
        };

        struct recipes : public enum_item<57> {
            list_array<var_int32::recipe> recipe_ids;

            bool operator==(const recipes& other) const = default;
            using nbt_inline = void;
        };

        struct lodestone_tracker : public enum_item<58> {
            struct position {
                identifier has_global_position;
                base_objects::position pos;

                bool operator==(const position& other) const = default;
            };

            std::optional<position> target = std::nullopt;
            bool tracked = true;

            bool operator==(const lodestone_tracker& other) const = default;
        };

        struct firework_explosion : public enum_item<59> {
            item_firework_explosion explosion;

            bool operator==(const firework_explosion& other) const = default;
            using nbt_inline = void;
        };

        struct fireworks : public enum_item<60> {
            var_int32 flight_duration = 0;
            list_array_sized<item_firework_explosion, 256> explosions;

            bool operator==(const fireworks& other) const = default;
        };

        struct profile : public enum_item<61> {
            struct property {
                string_sized<64> name;
                std::string value;
                std::optional<string_sized<1024>> signature = std::nullopt;
                bool operator==(const property& other) const = default;
            };

            struct offline_profile {
                base_objects::uuid id;
                string_sized<16> name;
                list_array<property> properties;
                bool operator==(const offline_profile& other) const = default;
                using nbt_inline = void;
            };

            struct online_profile {
                std::optional<string_sized<16>> name;
                std::optional<base_objects::uuid> id;
                list_array<property> properties;
                bool operator==(const online_profile& other) const = default;
                using nbt_inline = void;
            };

            enum class model_e {
                slim = 0,
                wide = 1,
            };

            bool_or<offline_profile, online_profile> profile_;

            std::optional<std::string> texture;
            std::optional<std::string> cape;
            std::optional<std::string> elytra;
            std::optional<enum_as<model_e, var_int32>> model = std::nullopt;

            bool operator==(const profile& other) const = default;
        };

        struct note_block_sound : public enum_item<62> {
            identifier sound;

            bool operator==(const note_block_sound& other) const = default;
            using nbt_inline = void;
        };

        struct banner_patterns : public enum_item<63> {
            struct layer {
                struct decl {
                    identifier asset_id;
                    std::string translation_key;
                    bool operator==(const decl& other) const = default;
                };

                //TODO replace with registry entry
                value_optional<var_int32::banner_pattern, decl> pattern;
                enum_as<dye_color, var_int32> color;
                bool operator==(const layer& other) const = default;
            };

            list_array<layer> layers;

            bool operator==(const banner_patterns& other) const = default;
            using nbt_inline = void;
        };

        struct base_color : public enum_item<64> {
            enum_as<dye_color, var_int32> color;

            bool operator==(const base_color& other) const = default;
            using nbt_inline = void;
        };

        struct pot_decorations : public enum_item<65> {
            std::array<var_int32::item, 4> item_decorations;

            bool operator==(const pot_decorations& other) const = default;
            using nbt_inline = void;
        };

        struct container : public enum_item<66> {
            list_array_sized<slot, 256> items;

            container();
            container(container&&);
            container(const container&);
            container& operator=(container&&);
            container& operator=(const container&);

            std::optional<size_t> get_free_slot();
            //returns count of items that failed to add
            int32_t add(const slot_data& item);
            void set(size_t slot, slot_data&& item);
            void set(size_t slot, const slot_data& item);
            slot& get(size_t slot);
            bool contains(size_t slot);
            std::optional<size_t> contains(const slot_data& item) const;
            list_array<size_t> contains(const std::string& id, size_t count) const;
            bool remove(size_t slot);
            void clear(int32_t id, size_t count);
            void clear();
            size_t count() const;
            size_t size() const;

            bool operator==(const container& other) const;
        };

        struct block_state : public enum_item<67> {
            std::unordered_map<std::string, std::string> properties;

            bool operator==(const block_state& other) const = default;
            using nbt_inline = void;
        };

        struct bees : public enum_item<68> {
            struct bee {
                util::nbt entity_data;
                var_int32 ticks_in_hive;
                var_int32 min_ticks_in_hive;

                bool operator==(const bee& other) const = default;
            };

            list_array<bee> inside;

            bool operator==(const bees& other) const = default;
            using nbt_inline = void;
        };

        struct lock : public enum_item<69> {
            util::nbt key; //TODO replace with predicate

            bool operator==(const lock& other) const = default;
            using nbt_inline = void;
        };

        struct container_loot : public enum_item<70> {
            api::id::loot_table loot_table;
            int64_t seed = 0;

            bool operator==(const container_loot& other) const = default;
            using packet_as_nbt = void;
        };

        struct break_sound : public enum_item<71> {
            or_<var_int32::sound_event, sound_event_t> sound;

            bool operator==(const break_sound& other) const = default;
            using nbt_inline = void;
        };

        struct villager_variant : public enum_item<72> {
            var_int32::villager_variant variant;

            bool operator==(const villager_variant& other) const = default;
            using actual_name = component_custom_name<"villager/variant">;
            using nbt_inline = void;
        };

        struct wolf_variant : public enum_item<73> {
            var_int32::wolf_variant variant;

            bool operator==(const wolf_variant& other) const = default;
            using actual_name = component_custom_name<"wolf/variant">;
            using nbt_inline = void;
        };

        struct wolf_sound_variant : public enum_item<74> {
            var_int32::wolf_sound_variant variant;

            bool operator==(const wolf_sound_variant& other) const = default;
            using actual_name = component_custom_name<"wolf/sound_variant">;
            using nbt_inline = void;
        };

        struct wolf_collar : public enum_item<75> {
            enum_as<dye_color, var_int32> color;

            bool operator==(const wolf_collar& other) const = default;
            using actual_name = component_custom_name<"wolf/collar">;
            using nbt_inline = void;
        };

        struct fox_variant : public enum_item<76> {
            var_int32::fox_variant variant;

            bool operator==(const fox_variant& other) const = default;
            using actual_name = component_custom_name<"fox/variant">;
            using nbt_inline = void;
        };

        struct salmon_size : public enum_item<77> {
            var_int32 size;

            bool operator==(const salmon_size& other) const = default;
            using actual_name = component_custom_name<"salmon/size">;
            using nbt_inline = void;
        };

        struct parrot_variant : public enum_item<78> {
            var_int32::parrot_variant variant;

            bool operator==(const parrot_variant& other) const = default;
            using actual_name = component_custom_name<"parrot/variant">;
            using nbt_inline = void;
        };

        struct tropical_fish_pattern : public enum_item<79> {
            var_int32::tropical_fish_pattern variant;

            bool operator==(const tropical_fish_pattern& other) const = default;
            using actual_name = component_custom_name<"tropical_fish/pattern">;
            using nbt_inline = void;
        };

        struct tropical_fish_base_color : public enum_item<80> {
            enum_as<dye_color, var_int32> color;

            bool operator==(const tropical_fish_base_color& other) const = default;
            using actual_name = component_custom_name<"tropical_fish/base_color">;
            using nbt_inline = void;
        };

        struct tropical_fish_pattern_color : public enum_item<81> {
            enum_as<dye_color, var_int32> color;

            bool operator==(const tropical_fish_pattern_color& other) const = default;
            using actual_name = component_custom_name<"tropical_fish/pattern_color">;
            using nbt_inline = void;
        };

        struct mooshroom_variant : public enum_item<82> {
            var_int32::mooshroom_variant variant;

            bool operator==(const mooshroom_variant& other) const = default;
            using actual_name = component_custom_name<"mooshroom/variant">;
            using nbt_inline = void;
        };

        struct rabbit_variant : public enum_item<83> {
            var_int32::rabbit_variant variant;

            bool operator==(const rabbit_variant& other) const = default;
            using actual_name = component_custom_name<"rabbit/variant">;
            using nbt_inline = void;
        };

        struct pig_variant : public enum_item<84> {
            var_int32::pig_variant variant;

            bool operator==(const pig_variant& other) const = default;
            using actual_name = component_custom_name<"pig/variant">;
            using nbt_inline = void;
        };

        struct cow_variant : public enum_item<85> {
            var_int32::cow_variant variant;

            bool operator==(const cow_variant& other) const = default;
            using actual_name = component_custom_name<"cow/variant">;
            using nbt_inline = void;
        };

        struct chicken_variant : public enum_item<86> {
            struct reference : public default_enum_item<0> {
                identifier name;

                bool operator==(const reference& other) const = default;
                using nbt_inline = void;
            };

            struct direct : public enum_item<1> {
                var_int32::chicken_variant id;

                bool operator==(const direct& other) const = default;
                using nbt_inline = void;
            };

            enum_switch<uint8_t, reference, direct> variant;

            bool operator==(const chicken_variant& other) const = default;
            using actual_name = component_custom_name<"chicken/variant">;
            using nbt_inline = void;
        };

        struct frog_variant : public enum_item<87> {
            var_int32::frog_variant variant;
            bool operator==(const frog_variant& other) const = default;
            using actual_name = component_custom_name<"frog/variant">;
            using nbt_inline = void;
        };

        struct horse_variant : public enum_item<88> {
            var_int32::horse_variant variant;
            bool operator==(const horse_variant& other) const = default;
            using actual_name = component_custom_name<"horse/variant">;
            using nbt_inline = void;
        };

        struct painting_variant : public enum_item<89> {
            var_int32::painting_variant variant;

            bool operator==(const painting_variant& other) const = default;
            using actual_name = component_custom_name<"painting/variant">;
            using nbt_inline = void;
        };

        struct llama_variant : public enum_item<90> {
            var_int32::llama_variant variant;

            bool operator==(const llama_variant& other) const = default;
            using actual_name = component_custom_name<"llama/variant">;
            using nbt_inline = void;
        };

        struct axolotl_variant : public enum_item<91> {
            var_int32::axolotl_variant variant;

            bool operator==(const axolotl_variant& other) const = default;
            using actual_name = component_custom_name<"axolotl/variant">;
            using nbt_inline = void;
        };

        struct cat_variant : public enum_item<92> {
            var_int32::cat_variant variant;

            bool operator==(const cat_variant& other) const = default;
            using actual_name = component_custom_name<"cat/variant">;
            using nbt_inline = void;
        };

        struct cat_collar : public enum_item<93> {
            enum_as<dye_color, var_int32> color;

            bool operator==(const cat_collar& other) const = default;
            using actual_name = component_custom_name<"cat/collar">;
            using nbt_inline = void;
        };

        struct sheep_color : public enum_item<94> {
            enum_as<dye_color, var_int32> color;

            bool operator==(const sheep_color& other) const = default;
            using actual_name = component_custom_name<"sheep/color">;
            using nbt_inline = void;
        };

        struct shulker_color : public enum_item<95> {
            enum_as<dye_color, var_int32> color;

            bool operator==(const shulker_color& other) const = default;
            using actual_name = component_custom_name<"shulker/color">;
            using nbt_inline = void;
        };

        using base = enum_switch<
            var_int32,
            custom_data,
            max_stack_size,
            max_damage,
            damage,
            unbreakable,
            custom_name,
            item_name,
            item_model,
            lore,
            rarity,
            enchantments,
            can_place_on,
            can_break,
            attribute_modifiers,
            custom_model_data,
            tooltip_display,
            repair_cost,
            creative_slot_lock,
            enchantment_glint_override,
            intangible_projectile,
            food,
            consumable,
            use_remainder,
            use_cooldown,
            damage_resistant,
            tool,
            weapon,
            enchantable,
            equippable,
            repairable,
            glider,
            tooltip_style,
            death_protection,
            blocks_attacks,
            stored_enchantments,
            dyed_color,
            map_color,
            map_id,
            map_decorations,
            map_post_processing,
            charged_projectiles,
            bundle_contents,
            potion_contents,
            potion_duration_scale,
            suspicious_stew_effects,
            writable_book_content,
            written_book_content,
            trim,
            debug_stick_state,
            entity_data,
            bucket_entity_data,
            block_entity_data,
            instrument,
            provides_trim_material,
            ominous_bottle_amplifier,
            jukebox_playable,
            provides_banner_patterns,
            recipes,
            lodestone_tracker,
            firework_explosion,
            fireworks,
            profile,
            note_block_sound,
            banner_patterns,
            base_color,
            pot_decorations,
            container,
            block_state,
            bees,
            lock,
            container_loot,
            break_sound,
            villager_variant,
            wolf_variant,
            wolf_sound_variant,
            wolf_collar,
            fox_variant,
            salmon_size,
            parrot_variant,
            tropical_fish_pattern,
            tropical_fish_base_color,
            tropical_fish_pattern_color,
            mooshroom_variant,
            rabbit_variant,
            pig_variant,
            cow_variant,
            chicken_variant,
            frog_variant,
            horse_variant,
            painting_variant,
            llama_variant,
            axolotl_variant,
            cat_variant,
            cat_collar,
            sheep_color,
            shulker_color>;

        base type;

        static void parse_component(component& item, const std::string& name, util::nbt_read_stream& stream);
        static void encode_component(const component& item, util::nbt_write_compound_stream& stream);

        component();
        component(component&& mov) noexcept;
        component(const component& copy);

        template <class T>
        component(T&& mov) noexcept
            requires(std::is_constructible_v<base, T>);

        template <class T>
        component(const T& copy)
            requires(std::is_constructible_v<base, T>);

        component& operator=(component&& mov) noexcept;
        component& operator=(const component& copy);

        template <class T>
        component& operator=(T&& mov)
            requires std::is_constructible_v<base, T>;
        template <class T>
        component& operator=(const T& copy)
            requires std::is_constructible_v<base, T>;
        ~component() noexcept;

        bool operator==(const component& other) const;
        bool operator!=(const component& other) const;
        int32_t get_id() const;
    };

    template <class T>
    component::component(T&& mov) noexcept
        requires(std::is_constructible_v<base, T>)
        : base(std::move(mov)) {
        *this = std::move(mov);
    }

    template <class T>
    component::component(const T& copy)
        requires(std::is_constructible_v<base, T>)
    {
        *this = copy;
    }

    template <class T>
    component& component::operator=(T&& mov)
        requires std::is_constructible_v<base, T>
    {
        type = std::move(mov);
        return *this;
    }

    template <class T>
    component& component::operator=(const T& copy)
        requires std::is_constructible_v<base, T>
    {
        type = copy;
        return *this;
    }
}

#endif /* SRC_BASE_OBJECTS_COMPONENT */
