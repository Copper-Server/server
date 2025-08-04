#ifndef SRC_BASE_OBJECTS_COMPONENT
#define SRC_BASE_OBJECTS_COMPONENT
#include <array>
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <optional>
#include <src/base_objects/box.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/dye_color.hpp>
#include <src/base_objects/packets_help.hpp>
#include <src/base_objects/position.hpp>
#include <src/util/cts.hpp>
#include <src/util/readers.hpp>
#include <string>

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
        std::vector<int32_t> colors;      //rgb
        std::vector<int32_t> fade_colors; //rgb
        bool trail;
        bool twinkle;
        auto operator<=>(const item_firework_explosion& other) const = default;
    };

    struct sound_event {
        identifier name;
        std::optional<float> fixed_range;
        auto operator<=>(const sound_event& other) const = default;
    };

    struct potion_effect {
        struct data_t {
            var_int32 amplifier;
            var_int32 duration;
            bool is_ambient;
            bool show_particles;
            bool show_icon;
            std::optional<box<data_t>> hidden_effect;

            auto operator<=>(const data_t& other) const = default;
            data_t copy() const;
        };

        var_int32::potion type_id;
        box<data_t> data;
        auto operator<=>(const potion_effect& other) const = default;
        potion_effect copy() const;
    };

    struct consume_effect {
        struct apply_effects : public enum_item<0> {
            std::vector<potion_effect> effects;
            float probability;
            auto operator<=>(const apply_effects& other) const = default;
        };

        struct remove_effects : public enum_item<1> {
            id_set<var_int32::mob_effect> effects;
            auto operator<=>(const remove_effects& other) const = default;
        };

        struct clear_all_effects : public enum_item<2> {
            auto operator<=>(const clear_all_effects& other) const = default;
        };

        struct teleport_randomly : public enum_item<3> {
            float diameter;
            auto operator<=>(const teleport_randomly& other) const = default;
        };

        struct play_sound : public enum_item<4> {
            sound_event event;
            auto operator<=>(const play_sound& other) const = default;
        };

        enum_switch<
            var_int32,
            apply_effects,
            remove_effects,
            clear_all_effects,
            teleport_randomly,
            play_sound>
            effect;
        auto operator<=>(const consume_effect& other) const = default;
    };

    struct partial_component {
        var_int32::data_component_type type;
        enbt::value value;
        auto operator<=>(const partial_component& other) const = default;
    };

    struct weak_slot {
        depends_next<var_int32> count;
        var_int32::item id;
        auto operator<=>(const weak_slot& other) const = default;
    };

    struct trim_material {
        struct override_t {
            identifier material_type;
            std::string asset_name;
            auto operator<=>(const override_t& other) const = default;
        };

        std::string suffix;
        std::vector<override_t> overrides;
        Chat description;
        auto operator<=>(const trim_material& other) const = default;
    };

    struct trim_pattern {
        std::string asset_name;
        var_int32::item template_item;
        Chat description;
        bool decal;
        auto operator<=>(const trim_pattern& other) const = default;
    };

    struct instrument {
        or_<var_int32::sound_event, sound_event> sound;
        float sound_range;
        float instrument_range;
        Chat description;
        auto operator<=>(const instrument& other) const = default;
    };

    struct jukebox_song {
        or_<var_int32::sound_event, sound_event> sound;
        Chat description;
        float duration;
        var_int32 output; //redstone
        auto operator<=>(const jukebox_song& other) const = default;
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
            enbt::value data;
            auto operator<=>(const custom_data& other) const = default;
        };

        struct max_stack_size : public enum_item<1> {
            var_int32 size;
            auto operator<=>(const max_stack_size& other) const = default;
        };

        struct max_damage : public enum_item<2> {
            var_int32 dmg;
            auto operator<=>(const max_damage& other) const = default;
        };

        struct damage : public enum_item<3> {
            var_int32 dmg;
            auto operator<=>(const damage& other) const = default;
        };

        struct unbreakable : public enum_item<4> {
            auto operator<=>(const unbreakable& other) const = default;
        };

        struct custom_name : public enum_item<5> {
            Chat name;
            auto operator<=>(const custom_name& other) const = default;
        };

        struct item_model : public enum_item<6> {
            identifier model;
            auto operator<=>(const item_model& other) const = default;
        };

        struct lore : public enum_item<8> {
            std::vector<Chat> lines;
            auto operator<=>(const lore& other) const = default;
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
            auto operator<=>(const struct rarity& other) const = default;
        };

        struct enchantments : public enum_item<10> {
            struct enchantment {
                var_int32::enchantment id;
                var_int32 level;
                auto operator<=>(const enchantment& other) const = default;
            };

            std::vector<enchantment> enchantments;
            auto operator<=>(const struct enchantments& other) const = default;
        };

        struct can_place_on : public enum_item<11> {
            struct property {
                std::string name;

                struct range : public enum_item<false> {
                    std::string min;
                    std::string max;
                    auto operator<=>(const range& other) const = default;
                };

                struct exact : public enum_item<true> {
                    std::string value;
                    auto operator<=>(const exact& other) const = default;
                };

                enum_switch<bool, range, exact> is_exact;
                auto operator<=>(const property& other) const = default;
            };

            std::optional<id_set<var_int32::block_type>> blocks;
            std::optional<std::vector<property>> properties;
            std::optional<enbt::value> nbt;
            std::vector<box<component>> full_components_match;
            std::vector<partial_component> partial_components_match;
            auto operator<=>(const can_place_on& other) const = default;
            can_place_on copy() const;
        };

        struct can_break : public enum_item<12> {
            struct property {
                std::string name;

                struct range : public enum_item<false> {
                    std::string min;
                    std::string max;
                    auto operator<=>(const range& other) const = default;
                };

                struct exact : public enum_item<true> {
                    std::string value;
                    auto operator<=>(const exact& other) const = default;
                };

                enum_switch<bool, range, exact> is_exact;
                auto operator<=>(const property& other) const = default;
            };

            std::optional<id_set<var_int32::block_type>> blocks;
            std::optional<std::vector<property>> properties;
            std::optional<enbt::value> nbt;
            std::vector<box<component>> full_components_match;
            std::vector<partial_component> partial_components_match;
            auto operator<=>(const can_break& other) const = default;
            can_break copy() const;
        };

        struct attribute_modifiers : public enum_item<13> {
            struct attribute {
                enum class operation_e {
                    add = 0,
                    multiply_base = 1,
                    multiply_total = 2,
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


                var_int32::attribute attr_id;
                identifier modifier_id;
                double value;
                enum_as<operation_e, var_int32> operation;
                enum_as<slot_e, var_int32> slot;
                auto operator<=>(const attribute& other) const = default;
            };

            std::vector<attribute> attributes;
            auto operator<=>(const attribute_modifiers& other) const = default;
        };

        struct custom_model_data : public enum_item<14> {
            std::vector<float> floats;
            std::vector<int8_t> flags; //boolean
            std::vector<std::string> strings;
            std::vector<int32_t> colors;
            auto operator<=>(const custom_model_data& other) const = default;
        };

        struct tooltip_display : public enum_item<15> {
            bool hide_tooltip;
            std::vector<var_int32::data_component_type> hidden_components;
            auto operator<=>(const tooltip_display& other) const = default;
        };

        struct repair_cost : public enum_item<16> {
            var_int32 cost;
            auto operator<=>(const repair_cost& other) const = default;
        };

        struct creative_slot_lock : public enum_item<17> {
            auto operator<=>(const creative_slot_lock& other) const = default;
        };

        struct enchantment_glint_override : public enum_item<18> {
            bool has;
            auto operator<=>(const enchantment_glint_override& other) const = default;
        };

        struct intangible_projectile : public enum_item<19> {
            enbt::value value_compound;
            auto operator<=>(const intangible_projectile& other) const = default;
        };

        struct food : public enum_item<20> {
            var_int32 nutrition;
            float saturation_modifier;
            bool can_always_eat;
            auto operator<=>(const food& other) const = default;
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
            float consume_seconds;
            enum_as<animation_e, var_int32> animation;
            or_<var_int32::sound_event, sound_event> sound;
            bool has_particles;
            std::vector<consume_effect> effects;
            auto operator<=>(const consumable& other) const = default;
        };

        struct use_remainder : public enum_item<22> {
            box<slot> remainder;

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

            auto operator<=>(const use_remainder& other) const = default;
        };

        struct use_cooldown : public enum_item<23> {
            float seconds;
            std::optional<identifier> cooldown_group;
            auto operator<=>(const use_cooldown& other) const = default;
        };

        struct damage_resistant : public enum_item<24> {
            identifier types; //tag without #
            auto operator<=>(const damage_resistant& other) const = default;
        };

        struct tool : public enum_item<25> {
            struct rule {
                id_set<var_int32::block_type> blocks;
                std::optional<float> speed;
                std::optional<bool> correct_drop_for_blocks;
                auto operator<=>(const rule& other) const = default;
            };

            std::vector<rule> rules;
            float default_mine_speed;
            var_int32 damage_per_block;
            bool creative_protection; //if true players cannot break blocks holding this item
            auto operator<=>(const tool& other) const = default;
        };

        struct weapon : public enum_item<26> {
            var_int32 dmg_per_attack;
            float disable_shield_for;
            auto operator<=>(const weapon& other) const = default;
        };

        struct enchantable : public enum_item<27> {
            var_int32 value;
            auto operator<=>(const enchantable& other) const = default;
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
            enum_as<equippable_on_e, var_int32> equippable_on;
            or_<var_int32::sound_event, sound_event> equip_sound;
            std::optional<identifier> model;
            std::optional<identifier> overlay;
            std::optional<id_set<var_int32::entity_type>> allowed_entities;
            bool dispensable;
            bool swappable;
            bool reduces_durability_on_damage;
            auto operator<=>(const equippable& other) const = default;
        };

        struct repairable : public enum_item<29> {
            id_set<var_int32::item> items;
            auto operator<=>(const repairable& other) const = default;
        };

        struct glider : public enum_item<30> {
            auto operator<=>(const glider& other) const = default;
        };

        struct tooltip_style : public enum_item<31> {
            identifier style;
            auto operator<=>(const tooltip_style& other) const = default;
        };

        struct death_protection : public enum_item<32> {
            std::vector<consume_effect> effects;
            auto operator<=>(const death_protection& other) const = default;
        };

        struct blocks_attacks : public enum_item<33> {
            struct damage_reductions {
                float horizontal_block_angle;
                std::optional<id_set<var_int32::damage_type>> damage_kind;
                float base;
                float factor;
                auto operator<=>(const damage_reductions& other) const = default;
            };

            float block_delay;
            float disable_cooldown_scale;
            std::vector<damage_reductions> reductions;
            float item_damage_threshold;
            float item_damage_base;
            float item_damage_factor;
            std::optional<identifier> bypassed_by;
            std::optional<or_<var_int32::sound_event, sound_event>> block_sound;
            std::optional<or_<var_int32::sound_event, sound_event>> disable_sound;
            auto operator<=>(const blocks_attacks& other) const = default;
        };

        struct stored_enchantments : public enum_item<34> {
            struct enchantment {
                var_int32::enchantment id;
                var_int32 level;
                auto operator<=>(const enchantment& other) const = default;
            };

            std::vector<enchantment> enchantments;
            auto operator<=>(const stored_enchantments& other) const = default;
        };

        struct dyed_color : public enum_item<35> {
            int32_t rgb;
            auto operator<=>(const dyed_color& other) const = default;
        };

        struct map_color : public enum_item<36> {
            int32_t rgb;
            auto operator<=>(const map_color& other) const = default;
        };

        struct map_id : public enum_item<37> {
            var_int32 id;
            auto operator<=>(const map_id& other) const = default;
        };

        struct map_decorations : public enum_item<38> {
            enbt::value value;
            auto operator<=>(const map_decorations& other) const = default;
        };

        struct map_post_processing : public enum_item<39> {
            enum class type_e : uint8_t {
                lock = 0,
                scale = 1,
            };

            enum_as<type_e, var_int32> type;
            auto operator<=>(const map_post_processing& other) const = default;
        };

        struct charged_projectiles : public enum_item<40> {
            std::vector<box<slot>> projectiles;
            auto operator<=>(const charged_projectiles& other) const = default;
            charged_projectiles copy() const;
        };

        struct bundle_contents : public enum_item<41> {
            std::vector<box<slot>> content;
            auto operator<=>(const bundle_contents& other) const = default;
            bundle_contents copy() const;
        };

        struct potion_contents : public enum_item<42> {
            std::optional<var_int32::potion> id;
            std::optional<int32_t> custom_color;
            std::vector<potion_effect> custom_effects;
            std::string custom_name;
            auto operator<=>(const potion_contents& other) const = default;
        };

        struct potion_duration_scale : public enum_item<43> {
            float multiplier;
            auto operator<=>(const potion_duration_scale& other) const = default;
        };

        struct suspicious_stew_effects : public enum_item<44> {
            struct effect {
                var_int32::potion potion_id;
                var_int32 duration;
                auto operator<=>(const effect& other) const = default;
            };

            std::vector<effect> effects;
            auto operator<=>(const suspicious_stew_effects& other) const = default;
        };

        struct writable_book_content : public enum_item<45> {
            struct page {
                string_sized<1024> raw;
                std::optional<string_sized<1024>> filtered;
            };

            vector_sized<page, 100> pages;
            auto operator<=>(const writable_book_content& other) const = default;
        };

        struct written_book_content : public enum_item<46> {
            struct page {
                string_sized<1024> raw;
                std::optional<string_sized<1024>> filtered;
            };

            string_sized<32> raw_title;
            std::optional<string_sized<32>> filtered_title;
            std::string author;
            var_int32 generation;
            vector_sized<page, 100> pages;
            bool resolved;
            auto operator<=>(const written_book_content& other) const = default;
        };

        struct trim : public enum_item<47> {
            or_<var_int32::trim_material, trim_material> material;
            or_<var_int32::trim_pattern, trim_pattern> pattern;
            auto operator<=>(const trim& other) const = default;
        };

        struct debug_stick_state : public enum_item<48> {
            enbt::value data;
            auto operator<=>(const debug_stick_state& other) const = default;
        };

        struct entity_data : public enum_item<49> {
            enbt::value data;
            auto operator<=>(const entity_data& other) const = default;
        };

        struct bucket_entity_data : public enum_item<50> {
            enbt::value data;
            auto operator<=>(const bucket_entity_data& other) const = default;
        };

        struct block_entity_data : public enum_item<51> {
            enbt::value data;
            auto operator<=>(const block_entity_data& other) const = default;
        };

        struct instrument : public enum_item<52> {
            or_<var_int32::sound_event, base_objects::instrument> value;
            auto operator<=>(const instrument& other) const = default;
        };

        struct provides_trim_material : public enum_item<53> {
            struct reference : public enum_item<0> {
                identifier name;
                auto operator<=>(const reference& other) const = default;
            };

            struct direct : public enum_item<1> {
                or_<var_int32::trim_material, trim_material> value;
                auto operator<=>(const direct& other) const = default;
            };

            enum_switch<uint8_t, reference, direct> material;

            auto operator<=>(const provides_trim_material& other) const = default;
        };

        struct ominous_bottle_amplifier : public enum_item<54> {
            var_int32 amplifier;
            auto operator<=>(const ominous_bottle_amplifier& other) const = default;
        };

        struct jukebox_playable : public enum_item<55> {
            //would fail to parse in client, use direct one
            struct reference : public enum_item<0> {
                identifier name;
                auto operator<=>(const reference& other) const = default;
            };

            struct direct : public enum_item<1> {
                or_<var_int32::sound_event, jukebox_song> value;
                auto operator<=>(const direct& other) const = default;
            };

            enum_switch<uint8_t, reference, direct> material;
            auto operator<=>(const jukebox_playable& other) const = default;
        };

        struct provides_banner_patterns : public enum_item<56> {
            identifier key;
            auto operator<=>(const provides_banner_patterns& other) const = default;
        };

        struct recipes : public enum_item<57> {
            enbt::compound data;
            auto operator<=>(const recipes& other) const = default;
        };

        struct lodestone_tracker : public enum_item<58> {
            struct position {
                identifier has_global_position;
                base_objects::position pos;
                auto operator<=>(const position& other) const = default;
            };

            std::optional<position> global_position;

            auto operator<=>(const lodestone_tracker& other) const = default;
        };

        struct firework_explosion : public enum_item<59> {
            item_firework_explosion explosion;
            auto operator<=>(const firework_explosion& other) const = default;
        };

        struct fireworks : public enum_item<60> {
            var_int32 flight_duration;
            std::vector<item_firework_explosion> explosions;
            auto operator<=>(const fireworks& other) const = default;
        };

        struct profile : public enum_item<61> {
            struct property {
                string_sized<64> name;
                std::string value;
                std::optional<string_sized<1024>> signature;
                auto operator<=>(const property& other) const = default;
            };

            std::optional<string_sized<16>> name;
            std::optional<enbt::raw_uuid> uuid;
            std::vector<property> property;

            auto operator<=>(const profile& other) const = default;
        };

        struct note_block_sound : public enum_item<62> {
            identifier sound;
            auto operator<=>(const note_block_sound& other) const = default;
        };

        struct banner_patterns : public enum_item<63> {
            struct layer {
                struct decl {
                    identifier asset_id;
                    std::string translation_key;
                };

                value_optional<var_int32::banner_pattern, decl> pattern_type;
                enum_as<dye_color, var_int32> color;
                auto operator<=>(const layer& other) const = default;
            };

            std::vector<layer> layers;
            auto operator<=>(const banner_patterns& other) const = default;
        };

        struct base_color : public enum_item<64> {
            enum_as<dye_color, var_int32> color;
            auto operator<=>(const base_color& other) const = default;
        };

        struct pot_decorations : public enum_item<65> {
            std::array<var_int32::item, 4> item_decorations;

            auto operator<=>(const pot_decorations& other) const = default;
        };

        struct container : public enum_item<66> {
            vector_sized<box<slot>, 256> items;

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

            auto operator<=>(const container& other) const = default;
            container copy() const;
        };

        struct block_state : public enum_item<67> {
            struct property {
                std::string name;
                std::string value;
                auto operator<=>(const property& other) const = default;
            };

            std::vector<property> properties;

            auto operator<=>(const block_state& other) const = default;
        };

        struct bees : public enum_item<68> {
            struct bee {
                enbt::compound nbt;
                var_int32 ticks_in_hive;
                var_int32 min_ticks_in_hive;
            };

            std::vector<bee> inside;

            auto operator<=>(const bees& other) const = default;
        };

        struct lock : public enum_item<69> {
            enbt::value key;

            auto operator<=>(const lock& other) const = default;
        };

        struct container_loot : public enum_item<70> {
            enbt::compound loot;
            auto operator<=>(const container_loot& other) const = default;
        };

        struct break_sound : public enum_item<71> {
            or_<var_int32::sound_event, sound_event> sound;
            auto operator<=>(const break_sound& other) const = default;
        };

        struct villager_variant : public enum_item<72> {
            var_int32::villager_variant variant;

            auto operator<=>(const villager_variant& other) const = default;
            using actual_name = component_custom_name<"villager/variant">;
        };

        struct wolf_variant : public enum_item<73> {
            var_int32::wolf_variant variant;

            auto operator<=>(const wolf_variant& other) const = default;
            using actual_name = component_custom_name<"wolf/variant">;
        };

        struct wolf_sound_variant : public enum_item<74> {
            var_int32::wolf_sound_variant variant;

            auto operator<=>(const wolf_sound_variant& other) const = default;
            using actual_name = component_custom_name<"wolf/sound_variant">;
        };

        struct wolf_collar : public enum_item<75> {
            dye_color color;
            auto operator<=>(const wolf_collar& other) const = default;
            using actual_name = component_custom_name<"wolf/collar">;
        };

        struct fox_variant : public enum_item<76> {
            var_int32::fox_variant variant;
            auto operator<=>(const fox_variant& other) const = default;
            using actual_name = component_custom_name<"fox/variant">;
        };

        struct salmon_size : public enum_item<77> {
            var_int32 size;
            auto operator<=>(const salmon_size& other) const = default;
            using actual_name = component_custom_name<"salmon/size">;
        };

        struct parrot_variant : public enum_item<78> {
            var_int32::parrot_variant variant;
            auto operator<=>(const parrot_variant& other) const = default;
            using actual_name = component_custom_name<"parrot/variant">;
        };

        struct tropical_fish_pattern : public enum_item<79> {
            var_int32::tropical_fish_pattern variant;
            auto operator<=>(const tropical_fish_pattern& other) const = default;
            using actual_name = component_custom_name<"tropical_fish/pattern">;
        };

        struct tropical_fish_base_color : public enum_item<80> {
            dye_color color;
            auto operator<=>(const tropical_fish_base_color& other) const = default;
            using actual_name = component_custom_name<"tropical_fish/base_color">;
        };

        struct tropical_fish_pattern_color : public enum_item<81> {
            dye_color color;
            auto operator<=>(const tropical_fish_pattern_color& other) const = default;
            using actual_name = component_custom_name<"tropical_fish/pattern_color">;
        };

        struct mooshroom_variant : public enum_item<82> {
            var_int32::mooshroom_variant variant;
            auto operator<=>(const mooshroom_variant& other) const = default;
            using actual_name = component_custom_name<"mooshroom/variant">;
        };

        struct rabbit_variant : public enum_item<83> {
            var_int32::rabbit_variant variant;
            auto operator<=>(const rabbit_variant& other) const = default;
            using actual_name = component_custom_name<"rabbit/variant">;
        };

        struct pig_variant : public enum_item<84> {
            var_int32::pig_variant variant;
            auto operator<=>(const pig_variant& other) const = default;
            using actual_name = component_custom_name<"pig/variant">;
        };

        struct cow_variant : public enum_item<85> {
            var_int32::cow_variant variant;
            auto operator<=>(const cow_variant& other) const = default;
            using actual_name = component_custom_name<"cow/variant">;
        };

        struct chicken_variant : public enum_item<86> {
            struct reference : public enum_item<0> {
                identifier name;
                auto operator<=>(const reference& other) const = default;
            };

            struct direct : public enum_item<1> {
                var_int32::chicken_variant id;
                auto operator<=>(const direct& other) const = default;
            };

            enum_switch<uint8_t, reference, direct> variant;

            auto operator<=>(const chicken_variant& other) const = default;
            using actual_name = component_custom_name<"chicken/variant">;
        };

        struct frog_variant : public enum_item<87> {
            var_int32::frog_variant variant;
            auto operator<=>(const frog_variant& other) const = default;
            using actual_name = component_custom_name<"frog/variant">;
        };

        struct horse_variant : public enum_item<88> {
            var_int32::horse_variant variant;
            auto operator<=>(const horse_variant& other) const = default;
            using actual_name = component_custom_name<"horse/variant">;
        };

        struct painting_variant : public enum_item<89> {
            var_int32::painting_variant variant;
            auto operator<=>(const painting_variant& other) const = default;
            using actual_name = component_custom_name<"painting/variant">;
        };

        struct llama_variant : public enum_item<90> {
            var_int32::llama_variant variant;
            auto operator<=>(const llama_variant& other) const = default;
            using actual_name = component_custom_name<"llama/variant">;
        };

        struct axolotl_variant : public enum_item<91> {
            var_int32::axolotl_variant variant;
            auto operator<=>(const axolotl_variant& other) const = default;
            using actual_name = component_custom_name<"axolotl/variant">;
        };

        struct cat_variant : public enum_item<92> {
            var_int32::cat_variant variant;
            auto operator<=>(const cat_variant& other) const = default;
            using actual_name = component_custom_name<"cat/variant">;
        };

        struct cat_collar : public enum_item<93> {
            dye_color color;
            auto operator<=>(const cat_collar& other) const = default;
            using actual_name = component_custom_name<"cat/collar">;
        };

        struct sheep_color : public enum_item<94> {
            dye_color color;
            auto operator<=>(const sheep_color& other) const = default;
            using actual_name = component_custom_name<"sheep/color">;
        };

        struct shulker_color : public enum_item<95> {
            dye_color color;
            auto operator<=>(const shulker_color& other) const = default;
            using actual_name = component_custom_name<"shulker/color">;
        };

        using base = enum_switch<
            var_int32,
            custom_data,
            max_stack_size,
            max_damage,
            damage,
            unbreakable,
            custom_name,
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

        static component parse_component(const std::string& name, const enbt::value& item);
        static std::pair<std::string, enbt::value> encode_component(const component& item);

        component() {}
        component(component&& mov) : type(std::move(mov.type)) {}

        component(const component& copy) {
            *this = copy;
        }

        template <class T>
        component(T&& mov) noexcept
            requires(std::is_constructible_v<base, T&&>) : type(std::move(mov)) 
        {}

        template <class T>
        component(const T& copy) 
            requires(std::is_constructible_v<base, T&&>) 
        {
            *this = copy;
        }

        component& operator=(component&& mov) noexcept;
        component& operator=(const component& copy);

        template <class T>
        component& operator=(T&& mov)
            requires std::is_constructible_v<base, T&&> 
        {
            type = base(std::move(mov));
            return *this;
        }

        template <class T>
        component& operator=(const T& copy)
            requires std::is_constructible_v<base, T&&>
        {
            if constexpr (std::is_copy_constructible_v<T>)
                type = base(copy);
            else
                type = base(copy.copy());
            return *this;
        }

        auto operator<=>(const component& other) const = default;
        size_t get_id() const;
    };
}

#endif /* SRC_BASE_OBJECTS_COMPONENT */
