#ifndef SRC_BASE_OBJECTS_COMPONENT
#define SRC_BASE_OBJECTS_COMPONENT
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <optional>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/dye_color.hpp>
#include <src/base_objects/position.hpp>
#include <src/util/readers.hpp>
#include <string>


namespace copper_server::base_objects {
    struct item_attribute {
        std::string type;
        std::string id;
        double amount;
        enum class operation_e {
            add,
            multiply_base,
            multiply_total,
        } operation;
        enum class slot_filter {
            any = 0,
            main_hand = 1,
            off_hand = 2,
            any_hand = 3,
            feet = 4,
            legs = 5,
            chest = 6,
            head = 7,
            armor = 8,
            body = 9,
        } slot;


        static std::string id_to_attribute_name(int32_t id);
        static int32_t attribute_name_to_id(const std::string& id);

        static int32_t operation_to_id(operation_e id);
        static int32_t operation_to_id(const std::string& id);
        static operation_e id_to_operation(int32_t id);
        static operation_e id_to_operation(const std::string& id);
        static std::string id_to_operation_string(int32_t id);

        static std::string slot_to_name(slot_filter id);
        static slot_filter name_to_slot(const std::string& id);

        auto operator<=>(const item_attribute& other) const = default;
    };

    struct item_rule {
        //represents list of blocks
        std::variant<list_array<std::string>, std::string> value;
        std::optional<float> speed;
        std::optional<bool> correct_for_drops;
        auto operator<=>(const item_rule& other) const = default;
    };

    struct item_potion_effect {
        int32_t potion_id = 0;

        struct effect_data {
            int32_t amplifier = 0;
            int32_t duration = 0; //-1 infinite

            bool ambient = false;
            bool show_particles = true;
            bool show_icon = true;
            effect_data* hidden_effect = nullptr; //store state of previous weaker effect when it lasts longer than this,

            bool operator==(const effect_data& other) const;

            bool operator!=(const effect_data& other) const {
                return !operator==(other);
            }
        };

        effect_data data;
        auto operator<=>(const item_potion_effect& other) const = default;
    };

    struct item_page {
        std::string text;
        std::optional<std::string> filtered;
        auto operator<=>(const item_page& other) const = default;
    };

    struct item_page_signed {
        Chat text;
        std::optional<Chat> filtered;
        auto operator<=>(const item_page_signed& other) const = default;
    };

    struct item_firework_explosion {
        enum class shape_e {
            small_ball = 0,
            large_ball = 1,
            star = 2,
            creeper = 3,
            burst = 4,
        } shape;
        list_array<int32_t> colors;      //rgb
        list_array<int32_t> fade_colors; //rgb
        bool trail;
        bool twinkle;
        auto operator<=>(const item_firework_explosion& other) const = default;
    };

    struct weak_slot_data {
        std::string id;
        int32_t count;
        auto operator<=>(const weak_slot_data& other) const = default;
    };

    struct slot_data;

    struct component_item;

    namespace component {
        namespace inner {
            struct block_predicate {
                struct property {
                    struct ranged {
                        std::string min;
                        std::string max;
                        auto operator<=>(const ranged& other) const = default;
                    };

                    struct exact {
                        std::string value;
                        auto operator<=>(const exact& other) const = default;
                    };

                    std::string name;
                    std::variant<exact, ranged> match;
                    auto operator<=>(const property& other) const = default;
                };

                std::optional<std::variant<std::string, std::vector<int32_t>>> blocks;//block tag or list of general ids
                std::optional<std::vector<property>> properties;
                std::optional<enbt::value> nbt;
                std::vector<std::shared_ptr<component_item>> data_components;
                std::vector<std::pair<std::string, enbt::value>> partial_data_components;
                auto operator<=>(const block_predicate& other) const = default;
            };

            struct sound_extended {
                std::string sound_name;
                std::optional<float> fixed_range;
                auto operator<=>(const sound_extended& other) const = default;
            };

            struct apply_effects {
                list_array<item_potion_effect> effects;
                float probability = 1;
                auto operator<=>(const apply_effects& other) const = default;
            };

            struct remove_effects {
                std::variant<std::string, list_array<std::string>> effects;
                auto operator<=>(const remove_effects& other) const = default;
            };

            struct clear_all_effects {
                auto operator<=>(const clear_all_effects& other) const = default;
                static inline std::string component_name = "minecraft:clear_all_effects";
            };

            struct teleport_randomly {
                float diameter = 16;
                auto operator<=>(const teleport_randomly& other) const = default;
            };

            struct play_sound {
                inner::sound_extended sound;
                auto operator<=>(const play_sound& other) const = default;
            };

            struct __custom { //DO NOT SEND TO CLIENT, PROCESSED BY SERVER
                std::string type;
                enbt::value value;

                auto operator<=>(const __custom& other) const = default;
            };

            using application_effect = std::variant<apply_effects, remove_effects, clear_all_effects, teleport_randomly, play_sound, __custom>;
        }

        struct attribute_modifiers {
            list_array<item_attribute> value;

            auto operator<=>(const attribute_modifiers& other) const = default;
            static inline std::string component_name = "minecraft:attribute_modifiers";
        };

        struct axolotl_variant {
            std::string value; //lucy, wild, gold, cyan, blue

            auto operator<=>(const axolotl_variant& other) const = default;
            static inline std::string component_name = "minecraft:axolotl/variant";
        };

        struct banner_patterns {
            struct custom_pattern {
                std::string asset_id;
                std::string translation_key;
                auto operator<=>(const custom_pattern& other) const = default;
            };

            struct pattern {
                dye_color color;
                std::variant<std::string, custom_pattern> pattern;
                auto operator<=>(const banner_patterns::pattern& other) const = default;
            };

            list_array<pattern> value;

            auto operator<=>(const banner_patterns& other) const = default;
            static inline std::string component_name = "minecraft:banner_patterns";
        };

        struct base_color {
            dye_color color;
            auto operator<=>(const base_color& other) const = default;
            static inline std::string component_name = "minecraft:base_color";
        };

        struct bees {
            struct bee {
                enbt::value entity_data;
                int32_t ticks_in_hive;
                int32_t min_ticks_in_hive;
                auto operator<=>(const bee& other) const = default;
            };

            list_array<bee> values;

            auto operator<=>(const bees& other) const = default;
            static inline std::string component_name = "minecraft:bees";
        };

        struct block_entity_data {
            enbt::compound value;
            auto operator<=>(const block_entity_data& other) const = default;
            static inline std::string component_name = "minecraft:block_entity_data";
        };

        struct block_state {
            struct property_t {
                std::string name;
                std::string value;
                auto operator<=>(const property_t& other) const = default;
            };

            list_array<property_t> properties;

            auto operator<=>(const block_state& other) const = default;
            static inline std::string component_name = "minecraft:block_state";
        };

        struct blocks_attacks {
            float block_delay_seconds = 0;
            float disable_cooldown_scale = 1;

            struct damage_reduction_t {
                std::string type;
                float base;
                float factor;
                float horizontal_blocking_angle;

                auto operator<=>(const damage_reduction_t& other) const = default;
            };

            std::vector<damage_reduction_t> damage_reductions;

            struct __item_damage {
                float threshold;
                float base;
                float factor;

                auto operator<=>(const __item_damage& other) const = default;
            } item_damage;

            std::optional<std::variant<std::string, inner::sound_extended>> block_sound;
            std::optional<std::variant<std::string, inner::sound_extended>> disabled_sound;

            std::optional<std::string> bypassed_by;

            auto operator<=>(const blocks_attacks& other) const = default;
            static inline std::string component_name = "minecraft:blocks_attacks";
        };

        struct break_sound {
            std::variant<std::string, inner::sound_extended> value;
            auto operator<=>(const break_sound& other) const = default;
            static inline std::string component_name = "minecraft:break_sound";
        };

        struct bucket_entity_data {
            enbt::compound value;
            auto operator<=>(const bucket_entity_data& other) const = default;
            static inline std::string component_name = "minecraft:bucket_entity_data";
        };

        struct bundle_contents {
            list_array<slot_data*> items;

            bool operator==(const bundle_contents& other) const;

            bool operator!=(const bundle_contents& other) const {
                return !operator==(other);
            }

            static inline std::string component_name = "minecraft:bundle_contents";
        };

        struct can_break {
            list_array<inner::block_predicate> value;

            auto operator<=>(const can_break& other) const = default;
            static inline std::string component_name = "minecraft:can_break";
        };

        struct can_place_on {
            list_array<inner::block_predicate> value;

            auto operator<=>(const can_place_on& other) const = default;
            static inline std::string component_name = "minecraft:can_place_on";
        };

        struct cat_collar {
            dye_color value; //dye color

            auto operator<=>(const cat_collar& other) const = default;
            static inline std::string component_name = "minecraft:cat/collar";
        };

        struct cat_variant {
            std::string value; //tabby, tuxedo, red, siamese, british, calico, black, ragdoll, white, jellie, brendan

            auto operator<=>(const cat_variant& other) const = default;
            static inline std::string component_name = "minecraft:cat/variant";
        };

        struct charged_projectiles {
            list_array<slot_data*> data;

            bool operator==(const charged_projectiles& other) const;

            bool operator!=(const charged_projectiles& other) const {
                return !operator==(other);
            }

            static inline std::string component_name = "minecraft:charged_projectiles";
        };

        struct chicken_variant {
            std::string value;

            auto operator<=>(const chicken_variant& other) const = default;
            static inline std::string component_name = "minecraft:chicken/variant";
        };

        struct consumable {
            list_array<inner::application_effect> on_consume_effects; //optional
            std::variant<std::string, inner::sound_extended> sound = "entity.generic.eat";
            std::string animation = "eat";
            float consume_seconds = 1.6f;
            bool has_consume_particles = true;

            auto operator<=>(const consumable& other) const = default;
            static inline std::string component_name = "minecraft:consumable";
        };

        struct container {
            container() = default;
            container(container&&);
            container(const container&);
            ~container();

            container& operator=(container&&);
            container& operator=(const container&);


            std::optional<uint8_t> get_free_slot();
            //returns count of added items
            int32_t add(const slot_data& item);
            void set(uint8_t slot, slot_data&& item);
            void set(uint8_t slot, const slot_data& item);
            slot_data& get(uint8_t slot);
            bool contains(uint8_t slot);
            list_array<uint8_t> contains(const std::string& id, size_t count = 1) const;
            std::optional<uint8_t> contains(const slot_data& item) const;
            

            void remove(uint8_t slot);

            void clear(int32_t id, size_t count = (size_t)-1);
            void clear();

            template <class _FN>
            void clear(_FN selector) {
                list_array<uint8_t> remove_slots;
                for_each(
                    [&remove_slots](slot_data& item, int32_t slot) {
                        if (selector(item))
                            remove_slots.push_back(slot);
                    }
                );
                for (auto i : remove_slots)
                    remove(i);
            }

            uint8_t count() const;

            template <class FN>
            void for_each(FN&& fn) const {
                for (int i = 0; i < 256; i++) 
                    if (items[i])
                        fn(*items[i], i);
            }

            template <class FN>
            void for_each(FN&& fn) {
                for (int i = 0; i < 256; i++) 
                    if (items[i])
                        fn(*items[i], i);
            }

            template <class FN>
            void for_each_all(FN&& fn) const {
                for (int i = 0; i < 256; i++) 
                    fn(items[i], i);
            }

            template <class FN>
            void for_each_all(FN&& fn) {
                for (int i = 0; i < 256; i++) 
                    fn(items[i], i);
            }
            bool operator==(const container& other) const;

            bool operator!=(const container& other) const {
                return !operator==(other);
            }

            static inline std::string component_name = "minecraft:container";
        private:
            //optimized to reduce memory consumption, may be extended to map{item:slot_data, slot:int32} in future to support custom containers
            slot_data* items[256] = {nullptr};
        };

        struct container_loot {
            std::string loot_table;
            int64_t seed;

            auto operator<=>(const container_loot& other) const = default;
            static inline std::string component_name = "minecraft:container_loot";
        };

        struct cow_variant {
            std::string value;

            auto operator<=>(const cow_variant& other) const = default;
            static inline std::string component_name = "minecraft:cow/variant";
        };

        struct creative_slot_lock {
            auto operator<=>(const creative_slot_lock& other) const = default;
            static inline std::string component_name = "minecraft:creative_slot_lock";
        };

        struct custom_data {
            enbt::compound value;

            auto operator<=>(const custom_data& other) const = default;
            static inline std::string component_name = "minecraft:custom_data";
        };

        struct custom_model_data {
            int32_t value;

            auto operator<=>(const custom_model_data& other) const = default;
            static inline std::string component_name = "minecraft:custom_model_data";
        };

        struct custom_name {
            Chat value;

            auto operator<=>(const custom_name& other) const = default;
            static inline std::string component_name = "minecraft:custom_name";
        };

        struct damage {
            int32_t value;

            auto operator<=>(const damage& other) const = default;
            static inline std::string component_name = "minecraft:damage";
        };

        struct damage_resistant {
            std::string types; //damage type tag tag

            auto operator<=>(const damage_resistant& other) const = default;
            static inline std::string component_name = "minecraft:damage_resistant";
        };

        struct debug_stick_state {
            enbt::compound previous_state;
            auto operator<=>(const debug_stick_state& other) const = default;
            static inline std::string component_name = "minecraft:debug_stick_state";
        };

        struct death_protection {
            list_array<inner::application_effect> death_effects; //optional

            auto operator<=>(const death_protection& other) const = default;
            static inline std::string component_name = "minecraft:death_protection";
        };

        struct dyed_color {
            int32_t rgb;

            auto operator<=>(const dyed_color& other) const = default;
            static inline std::string component_name = "minecraft:dyed_color";
        };

        struct enchantable {
            int32_t value; //limit to enchanting cost

            auto operator<=>(const enchantable& other) const = default;
            static inline std::string component_name = "minecraft:enchantable";
        };

        struct enchantment_glint_override {
            int32_t has_glint;

            auto operator<=>(const enchantment_glint_override& other) const = default;
            static inline std::string component_name = "minecraft:enchantment_glint_override";
        };

        struct enchantments {
            list_array<std::pair<int32_t, int32_t>> value; //type ID, level

            auto operator<=>(const enchantments& other) const = default;
            static inline std::string component_name = "minecraft:enchantments";
        };

        struct entity_data {
            enbt::compound value;
            auto operator<=>(const entity_data& other) const = default;
            static inline std::string component_name = "minecraft:entity_data";
        };

        struct equippable {
            struct equip_sound_custom {
                std::string sound_name;
                std::optional<float> fixed_range;
                auto operator<=>(const equip_sound_custom& other) const = default;
            };

            std::string slot; //head, chest, legs, feet, body, mainhand, offhand
            std::optional<std::string> camera_overlay;
            std::optional<std::string> model;
            std::variant<std::string, inner::sound_extended> equip_sound;
            std::variant<std::string, std::vector<std::string>, std::nullptr_t> allowed_entities; //if empty then allowed to all entities
            bool dispensable = true;
            bool swappable = true;
            bool damage_on_hurt = true;
            bool equip_on_interact = true;

            auto operator<=>(const equippable& other) const = default;
            static inline std::string component_name = "minecraft:equippable";
        };

        struct firework_explosion {
            item_firework_explosion value;
            auto operator<=>(const firework_explosion& other) const = default;
            static inline std::string component_name = "minecraft:firework_explosion";
        };

        struct fireworks {
            list_array<item_firework_explosion> explosions;
            int32_t duration;
            auto operator<=>(const fireworks& other) const = default;
            static inline std::string component_name = "minecraft:fireworks";
        };

        struct food {
            int32_t nutrition;
            float saturation;
            bool can_always_eat = false;

            auto operator<=>(const food& other) const = default;
            static inline std::string component_name = "minecraft:food";
        };

        struct fox_variant {
            std::string value; //red, snow

            auto operator<=>(const fox_variant& other) const = default;
            static inline std::string component_name = "minecraft:fox/variant";
        };

        struct frog_variant {
            std::string value;

            auto operator<=>(const frog_variant& other) const = default;
            static inline std::string component_name = "minecraft:frog/variant";
        };

        struct glider {
            auto operator<=>(const glider& other) const = default;
            static inline std::string component_name = "minecraft:glider";
        };

        struct horse_variant {
            std::string value;

            auto operator<=>(const horse_variant& other) const = default;
            static inline std::string component_name = "minecraft:horse/variant";
        };

        struct instrument {
            struct type_extended {
                std::variant<std::string, inner::sound_extended> sound;
                float duration;
                float range;
                Chat description;
                auto operator<=>(const type_extended& other) const = default;
            };

            std::variant<std::string, type_extended> type;
            auto operator<=>(const instrument& other) const = default;
            static inline std::string component_name = "minecraft:instrument";
        };

        struct intangible_projectile {
            enbt::value value; //sent empty nbt in notchain server
            auto operator<=>(const intangible_projectile& other) const = default;
            static inline std::string component_name = "minecraft:intangible_projectile";
        };

        struct item_model {
            std::string value;

            auto operator<=>(const item_model& other) const = default;
            static inline std::string component_name = "minecraft:item_model";
        };

        struct item_name {

            Chat value;
            auto operator<=>(const item_name& other) const = default;
            static inline std::string component_name = "minecraft:item_name";
        };

        struct jukebox_playable {
            //WARN jukebox_extended will cause the client to fail to parse it
            // and subsequently disconnect, which is likely an unintended bug.
            // Currently use only direct song_name or jukebox_compact
            struct jukebox_extended {
                std::variant<std::string, inner::sound_extended> sound_event;
                Chat description;
                float length_in_seconds;
                int32_t comparator_output;
                auto operator<=>(const jukebox_extended& other) const = default;
            };

            struct jukebox_compact {
                int32_t song_type;
                auto operator<=>(const jukebox_compact& other) const = default;
            };

            std::variant<std::string, jukebox_extended, jukebox_compact> song; //song name, extended, compact

            auto operator<=>(const jukebox_playable& other) const = default;
            static inline std::string component_name = "minecraft:jukebox_playable";
        };

        struct llama_variant {
            std::string value;

            auto operator<=>(const llama_variant& other) const = default;
            static inline std::string component_name = "minecraft:llama/variant";
        };

        struct lock {
            std::string key; //1.21 in packet as NBT with string tag

            auto operator<=>(const lock& other) const = default;
            static inline std::string component_name = "minecraft:lock";
        };

        struct lodestone_tracker {
            struct global_position {
                std::string dimension;
                base_objects::position position;
                auto operator<=>(const global_position& other) const = default;
            };

            //if not set then the compass will spin randomly
            std::optional<global_position> global_pos;
            bool tracked = true;

            auto operator<=>(const lodestone_tracker& other) const = default;
            static inline std::string component_name = "minecraft:lodestone_tracker";
        };

        struct lore {
            list_array<Chat> value;
            auto operator<=>(const lore& other) const = default;
            static inline std::string component_name = "minecraft:lore";
        };

        struct map_color {
            int32_t rgb;

            auto operator<=>(const map_color& other) const = default;
            static inline std::string component_name = "minecraft:map_color";
        };

        struct map_decorations {
            enbt::value value;

            auto operator<=>(const map_decorations& other) const = default;
            static inline std::string component_name = "minecraft:map_decorations";
        };

        struct map_id {
            int32_t value;

            auto operator<=>(const map_id& other) const = default;
            static inline std::string component_name = "minecraft:map_id";
        };

        struct map_post_processing {
            int32_t value;          //root
            int64_t locked_id = -1; // server side

            auto operator<=>(const map_post_processing& other) const = default;
            static inline std::string component_name = "minecraft:map_post_processing";
        };

        struct max_damage {
            int32_t value;

            auto operator<=>(const max_damage& other) const = default;
            static inline std::string component_name = "minecraft:max_damage";
        };

        struct max_stack_size {
            uint8_t value; // 1..99

            auto operator<=>(const max_stack_size& other) const = default;

            static inline std::string component_name = "minecraft:max_stack_size";
        };

        struct mooshroom_variant {
            std::string value;

            auto operator<=>(const mooshroom_variant& other) const = default;
            static inline std::string component_name = "minecraft:mooshroom/variant";
        };

        struct note_block_sound {
            std::string sound;
            auto operator<=>(const note_block_sound& other) const = default;
            static inline std::string component_name = "minecraft:note_block_sound";
        };

        struct ominous_bottle_amplifier {
            int32_t amplifier; //0..4
            auto operator<=>(const ominous_bottle_amplifier& other) const = default;
            static inline std::string component_name = "minecraft:ominous_bottle_amplifier";
        };

        struct painting_variant {
            std::string value;

            auto operator<=>(const painting_variant& other) const = default;
            static inline std::string component_name = "minecraft:painting/variant";
        };

        struct parrot_variant {
            std::string value;

            auto operator<=>(const parrot_variant& other) const = default;
            static inline std::string component_name = "minecraft:parrot/variant";
        };

        struct pig_variant {
            std::string value;

            auto operator<=>(const pig_variant& other) const = default;
            static inline std::string component_name = "minecraft:pig/variant";
        };

        struct pot_decorations {
            std::string decorations[4];
            auto operator<=>(const pot_decorations& other) const = default;
            static inline std::string component_name = "minecraft:pot_decorations";
        };

        struct potion_contents {
            struct full {
                std::optional<int32_t> potion_id;
                std::optional<int32_t> color_rgb;
                list_array<item_potion_effect> custom_effects;
                std::optional<std::string> custom_name; //since 1.21.2
                auto operator<=>(const full& other) const = default;
            };

            std::variant<int32_t, full> value;

            void set_potion_id(const std::string& id);
            void set_potion_id(int32_t id);

            void set_custom_color(int32_t rgb);
            void clear_custom_color();

            void set_custom_name(const std::string& name);
            void clear_custom_name();

            void add_custom_effect(item_potion_effect&&);
            void add_custom_effect(const item_potion_effect&);
            void clear_custom_effects();


            void iterate_custom_effects(std::function<void(const item_potion_effect&)> fn) const;
            void iterate_custom_effects(std::function<void(size_t)> size_fn, std::function<void(const item_potion_effect&)> fn) const;
            std::optional<int32_t> get_potion_id() const;
            std::optional<int32_t> get_custom_color() const;
            std::optional<std::string> get_custom_name() const;

            auto operator<=>(const potion_contents& other) const = default;
            static inline std::string component_name = "minecraft:potion_contents";
        };

        struct potion_duration_scale {
            float value = 1.0f;
            auto operator<=>(const potion_duration_scale& other) const = default;
            static inline std::string component_name = "minecraft:potion_duration_scale";
        };

        struct profile {
            struct property_t {
                std::string name; //64
                std::string value;
                std::optional<std::string> signature; //1024
                auto operator<=>(const property_t& other) const = default;
            };

            std::optional<std::string> name;
            std::optional<enbt::raw_uuid> uid;
            list_array<property_t> properties;

            auto operator<=>(const profile& other) const = default;
            static inline std::string component_name = "minecraft:profile";
        };

        struct provides_banner_patterns {
            std::string patterns; //tag
            auto operator<=>(const provides_banner_patterns& other) const = default;
            static inline std::string component_name = "minecraft:provides_banner_patterns";
        };

        struct provides_trim_material {
            std::string patterns; //tag
            auto operator<=>(const provides_trim_material& other) const = default;
            static inline std::string component_name = "minecraft:provides_trim_material";
        };

        struct rabbit_variant {
            std::string value;

            auto operator<=>(const rabbit_variant& other) const = default;
            static inline std::string component_name = "minecraft:rabbit/variant";
        };

        struct rarity {
            enum _internal : uint8_t {
                common,   //white
                uncommon, //yellow
                rare,     //aqua
                epic,     //pink
            } value;

            rarity()
                : value(common) {}

            rarity(_internal value)
                : value(value) {}

            rarity(const rarity& value)
                : value(value.value) {}

            explicit rarity(uint8_t value)
                : value((_internal)value) {}

            std::string to_string() const;
            static rarity from_string(const std::string&);

            auto operator<=>(const rarity& other) const
                = default;
            static inline std::string component_name = "minecraft:rarity";
        };

        struct recipes {
            std::vector<std::string> value;
            auto operator<=>(const recipes& other) const = default;
            static inline std::string component_name = "minecraft:recipes";
        };

        struct repairable {
            std::variant<std::string, std::vector<std::string>> items;
            auto operator<=>(const repairable& other) const = default;
            static inline std::string component_name = "minecraft:repairable";
        };

        struct repair_cost {
            int32_t value;
            auto operator<=>(const repair_cost& other) const = default;
            static inline std::string component_name = "minecraft:repair_cost";
        };

        struct salmon_size {
            std::string value;

            auto operator<=>(const salmon_size& other) const = default;
            static inline std::string component_name = "minecraft:salmon/size";
        };

        struct sheep_color {
            dye_color value;

            auto operator<=>(const sheep_color& other) const = default;
            static inline std::string component_name = "minecraft:sheep/color";
        };

        struct shulker_color {
            dye_color value;

            auto operator<=>(const shulker_color& other) const = default;
            static inline std::string component_name = "minecraft:shulker/color";
        };

        struct stored_enchantments {
            list_array<std::pair<int32_t, int32_t>> enchants; //type ID, level

            auto operator<=>(const stored_enchantments& other) const = default;
            static inline std::string component_name = "minecraft:stored_enchantments";
        };

        struct suspicious_stew_effects {
            list_array<std::pair<int32_t, int32_t>> effects; //id, duration
            auto operator<=>(const suspicious_stew_effects& other) const = default;
            static inline std::string component_name = "minecraft:suspicious_stew_effects";
        };

        struct tool {
            list_array<item_rule> rules;
            float default_mining_speed = 1.0;
            int32_t damage_per_block = 1;

            auto operator<=>(const tool& other) const = default;
            static inline std::string component_name = "minecraft:tool";
        };

        struct tooltip_display {
            bool hide_tooltip = false;
            std::vector<std::string> hidden_components;

            auto operator<=>(const tooltip_display& other) const = default;
            static inline std::string component_name = "minecraft:tooltip_display";
        };

        struct tooltip_style {
            std::string value;

            auto operator<=>(const tooltip_style& other) const = default;
            static inline std::string component_name = "minecraft:tooltip_style";
        };

        struct trim {
            struct material_extended {
                std::string asset_name;
                list_array<std::pair<int32_t, std::string>> overrides; //armor material type, asset name
                Chat description;
                int32_t ingredient;
                float item_model_index;
                auto operator<=>(const material_extended& other) const = default;
            };

            struct pattern_extended {
                std::string asset_name;
                Chat description;
                int32_t template_item;
                bool decal;
                auto operator<=>(const pattern_extended& other) const = default;
            };

            std::variant<uint32_t, material_extended> material; //id or extended
            std::variant<uint32_t, pattern_extended> pattern;   //id or extended

            auto operator<=>(const trim& other) const = default;
            static inline std::string component_name = "minecraft:trim";
        };

        struct tropical_fish_base_color {
            dye_color color;
            auto operator<=>(const tropical_fish_base_color& other) const = default;
            static inline std::string component_name = "minecraft:tropical_fish/base_color";
        };

        struct tropical_fish_pattern {
            std::string pattern;
            auto operator<=>(const tropical_fish_pattern& other) const = default;
            static inline std::string component_name = "minecraft:tropical_fish/pattern";
        };

        struct tropical_fish_pattern_color {
            dye_color color;
            auto operator<=>(const tropical_fish_pattern_color& other) const = default;
            static inline std::string component_name = "minecraft:tropical_fish/pattern_color";
        };

        struct unbreakable {
            auto operator<=>(const unbreakable& other) const = default;
            static inline std::string component_name = "minecraft:unbreakable";
        };

        struct use_cooldown {
            std::optional<std::string> cooldown_group;
            float seconds;

            auto operator<=>(const use_cooldown& other) const = default;
            static inline std::string component_name = "minecraft:use_cooldown";
        };

        struct use_remainder {
            std::variant<slot_data*, weak_slot_data> proxy_value;
            //std::string id;  extracted by `proxy_value->get_slot_data().id`
            //int32_t count;   proxy_value->count
            //components;      proxy_value->components
            use_remainder(slot_data&& consume);
            use_remainder(weak_slot_data&& consume);

            ~use_remainder();

            bool operator==(const use_remainder& other) const;

            bool operator!=(const use_remainder& other) const {
                return !operator==(other);
            }

            static inline std::string component_name = "minecraft:use_remainder";
        };

        struct villager_variant {
            std::string value;

            auto operator<=>(const villager_variant& other) const = default;
            static inline std::string component_name = "minecraft:villager/variant";
        };

        struct weapon {
            int32_t item_damage_per_attack = 1;
            float disable_blocking_for_seconds = 0;

            auto operator<=>(const weapon& other) const = default;
            static inline std::string component_name = "minecraft:weapon";
        };

        struct wolf_collar {
            dye_color value;

            auto operator<=>(const wolf_collar& other) const = default;
            static inline std::string component_name = "minecraft:wolf/collar";
        };

        struct wolf_sound_variant {
            std::string value;

            auto operator<=>(const wolf_sound_variant& other) const = default;
            static inline std::string component_name = "minecraft:wolf/sound_variant";
        };

        struct wolf_variant {
            std::string value;

            auto operator<=>(const wolf_variant& other) const = default;
            static inline std::string component_name = "minecraft:wolf/variant";
        };

        struct writable_book_content {
            list_array<item_page> pages;
            auto operator<=>(const writable_book_content& other) const = default;
            static inline std::string component_name = "minecraft:writable_book_content";
        };

        struct written_book_content {
            std::string raw_title;
            std::optional<std::string> filtered_title;
            std::string author;
            list_array<item_page_signed> pages;
            int32_t generation = 0;
            bool resolved = false;

            auto operator<=>(const written_book_content& other) const = default;
            static inline std::string component_name = "minecraft:written_book_content";
        };

        using unified
            = std::variant<
                attribute_modifiers,
                axolotl_variant,
                banner_patterns,
                base_color,
                bees,
                block_entity_data,
                block_state,
                blocks_attacks,
                break_sound,
                bucket_entity_data,
                bundle_contents,
                can_break,
                can_place_on,
                cat_collar,
                cat_variant,
                charged_projectiles,
                chicken_variant,
                consumable,
                container,
                container_loot,
                cow_variant,
                creative_slot_lock,
                custom_data,
                custom_model_data,
                custom_name,
                damage,
                damage_resistant,
                debug_stick_state,
                death_protection,
                dyed_color,
                enchantable,
                enchantment_glint_override,
                enchantments,
                entity_data,
                equippable,
                firework_explosion,
                fireworks,
                food,
                fox_variant,
                frog_variant,
                glider,
                horse_variant,
                instrument,
                intangible_projectile,
                item_model,
                item_name,
                jukebox_playable,
                llama_variant,
                lock,
                lodestone_tracker,
                lore,
                map_color,
                map_decorations,
                map_id,
                map_post_processing,
                max_damage,
                max_stack_size,
                mooshroom_variant,
                note_block_sound,
                ominous_bottle_amplifier,
                painting_variant,
                parrot_variant,
                pig_variant,
                pot_decorations,
                potion_contents,
                potion_duration_scale,
                profile,
                provides_banner_patterns,
                provides_trim_material,
                rabbit_variant,
                rarity,
                recipes,
                repairable,
                repair_cost,
                salmon_size,
                sheep_color,
                shulker_color,
                stored_enchantments,
                suspicious_stew_effects,
                tool,
                tooltip_display,
                tooltip_style,
                trim,
                tropical_fish_base_color,
                tropical_fish_pattern,
                tropical_fish_pattern_color,
                unbreakable,
                use_cooldown,
                use_remainder,
                villager_variant,
                weapon,
                wolf_collar,
                wolf_sound_variant,
                wolf_variant,
                writable_book_content,
                written_book_content

                >;

        unified parse_component(const std::string& name, const enbt::value& item);
        std::pair<std::string, enbt::value> encode_component(const unified& item);
    }

    struct component_item : public component::unified{
        component_item(const component::unified& self): component::unified(self){}
        component_item(component::unified&& self): component::unified(std::move(self)){}
    };
}

#endif /* SRC_BASE_OBJECTS_COMPONENT */
