#ifndef SRC_BASE_OBJECTS_SLOT
#define SRC_BASE_OBJECTS_SLOT
#include "../library/enbt.hpp"
#include "../library/list_array.hpp"
#include "../util/readers.hpp"
#include "chat.hpp"
#include "position.hpp"
#include <optional>
#include <string>

namespace crafted_craft {
    namespace base_objects {
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
            int32_t potion_id;

            struct effect_data {
                int32_t amplifier;
                int32_t duration; //-1 infinite

                bool ambient = false;
                bool show_particles = true;
                bool show_icon = true;
                effect_data* hidden_effect = nullptr; //store state of previous weaker effect when it lasts longer than this,

                bool operator==(const effect_data& other) const;
                bool operator!=(const effect_data& other) const{
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

        struct item_color {
            enum __internal : uint8_t {
                white = 0,
                orange = 1,
                magenta = 2,
                light_blue = 3,
                yellow = 4,
                lime = 5,
                pink = 6,
                gray = 7,
                light_gray = 8,
                cyan = 9,
                purple = 10,
                blue = 11,
                brown = 12,
                green = 13,
                red = 14,
                black = 15,
            } value;

            item_color()
                : value(white) {}

            item_color(__internal value)
                : value(value) {}

            item_color(const item_color& value)
                : value(value.value) {}

            explicit item_color(uint8_t value)
                : value((__internal)value) {}

            std::string to_string() const;
            static item_color from_string(const std::string&);

            operator __internal() const {
                return value;
            }

            auto operator<=>(const item_color& other) const = default;
        };

        struct slot_data;

        namespace slot_component {
            namespace inner {
                using block_predicate = enbt::compound;

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
                };

                struct teleport_randomly {
                    float diameter = 16;
                    auto operator<=>(const teleport_randomly& other) const = default;
                };

                struct play_sound {
                    std::variant<std::string, inner::sound_extended> sound;
                    auto operator<=>(const play_sound& other) const = default;
                };

                struct __custom {
                    std::string type;
                    enbt::value value;
                    auto operator<=>(const __custom& other) const = default;
                };

                using application_effect = std::variant<apply_effects, remove_effects, clear_all_effects, teleport_randomly, play_sound, __custom>;
            }

            struct custom_data {
                enbt::compound value;

                auto operator<=>(const custom_data& other) const = default;
                static inline std::string component_name = "custom_data";
            };

            struct max_stack_size {
                uint8_t value; // 1..99

                auto operator<=>(const max_stack_size& other) const = default;

                static inline std::string component_name = "max_stack_size";
            };

            struct max_damage {
                int32_t value;

                auto operator<=>(const max_damage& other) const = default;
                static inline std::string component_name = "max_damage";
            };

            struct damage {
                int32_t value;

                auto operator<=>(const damage& other) const = default;
                static inline std::string component_name = "damage";
            };

            struct unbreakable {
                bool value = true;

                auto operator<=>(const unbreakable& other) const = default;
                static inline std::string component_name = "unbreakable";
            };

            struct custom_name {
                Chat value;

                auto operator<=>(const custom_name& other) const = default;
                static inline std::string component_name = "custom_name";
            };

            struct item_name {
                Chat value;
                auto operator<=>(const item_name& other) const = default;
                static inline std::string component_name = "item_name";
            };

            struct lore {
                list_array<Chat> value;
                auto operator<=>(const lore& other) const = default;
                static inline std::string component_name = "lore";
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
                static inline std::string component_name = "rarity";
            };

            struct enchantments {
                list_array<std::pair<int32_t, int32_t>> enchants; //type ID, level
                bool show_in_tooltip = true;

                auto operator<=>(const enchantments& other) const = default;
                static inline std::string component_name = "enchantments";
            };

            struct can_place_on {
                list_array<inner::block_predicate> predicates;
                bool show_in_tooltip = true;

                auto operator<=>(const can_place_on& other) const = default;
                static inline std::string component_name = "can_place_on";
            };

            struct can_break {
                list_array<inner::block_predicate> predicates;
                bool show_in_tooltip = true;

                auto operator<=>(const can_break& other) const = default;
                static inline std::string component_name = "can_break";
            };

            struct attribute_modifiers {
                list_array<item_attribute> attributes;
                bool show_in_tooltip = true;

                auto operator<=>(const attribute_modifiers& other) const = default;
                static inline std::string component_name = "attribute_modifiers";
            };

            struct banner_patterns {
                struct custom_pattern {
                    std::string asset_id;
                    std::string translation_key;
                    auto operator<=>(const custom_pattern& other) const = default;
                };

                struct pattern {
                    item_color color;
                    std::variant<std::string, custom_pattern> pattern;
                    auto operator<=>(const banner_patterns::pattern& other) const = default;
                };

                list_array<pattern> value;

                auto operator<=>(const banner_patterns& other) const = default;
                static inline std::string component_name = "banner_patterns";
            };

            struct custom_model_data {
                int32_t value;

                auto operator<=>(const custom_model_data& other) const = default;
                static inline std::string component_name = "custom_model_data";
            };

            struct hide_additional_tooltip { //NEED DOCUMENTATION in wiki.vg, what means `additional`
                auto operator<=>(const hide_additional_tooltip& other) const = default;
                static inline std::string component_name = "hide_additional_tooltip";
            };

            struct hide_tooltip {
                auto operator<=>(const hide_tooltip& other) const = default;
                static inline std::string component_name = "hide_tooltip";
            };

            struct repair_cost {
                int32_t value;
                auto operator<=>(const repair_cost& other) const = default;
                static inline std::string component_name = "repair_cost";
            };

            struct creative_slot_lock {
                auto operator<=>(const creative_slot_lock& other) const = default;
                static inline std::string component_name = "creative_slot_lock";
            };

            struct enchantment_glint_override {
                int32_t has_glint;
                auto operator<=>(const enchantment_glint_override& other) const = default;
                static inline std::string component_name = "enchantment_glint_override";
            };

            struct intangible_projectile {
                enbt::value value; //sent empty nbt in notchain server
                auto operator<=>(const intangible_projectile& other) const = default;
                static inline std::string component_name = "intangible_projectile";
            };

            struct food {
                int32_t nutrition;
                float saturation;
                bool can_always_eat = false;

                auto operator<=>(const food& other) const = default;
                //does noting if `consumable` component not defined
                //float seconds_to_eat; deprecated , since 1.21.2, use consumable.consume_seconds
                //slot_data* consuming_converts_to; deprecated , since 1.21.2, use use_remainder component
                //list_array<std::pair<int32_t, float>> effects; //effect ID, probability     deprecated, since 1.21.2, use consumable.on_consume_effects
                static inline std::string component_name = "food";
            };

            struct tool {
                list_array<item_rule> rules;
                float default_mining_speed = 1.0;
                int32_t damage_per_block = 1;

                auto operator<=>(const tool& other) const = default;
                static inline std::string component_name = "tool";
            };

            struct stored_enchantments {
                list_array<std::pair<int32_t, int32_t>> enchants; //type ID, level
                bool show_in_tooltip = true;

                auto operator<=>(const stored_enchantments& other) const = default;
                static inline std::string component_name = "stored_enchantments";
            };

            struct dyed_color {
                int32_t rgb;
                bool show_in_tooltip = true;

                auto operator<=>(const dyed_color& other) const = default;
                static inline std::string component_name = "dyed_color";
            };

            struct map_color {
                int32_t rgb;

                auto operator<=>(const map_color& other) const = default;
                static inline std::string component_name = "map_color";
            };

            struct map_id {
                int32_t value;

                auto operator<=>(const map_id& other) const = default;
                static inline std::string component_name = "map_id";
            };

            struct map_decorations {
                enbt::value value;

                auto operator<=>(const map_decorations& other) const = default;
                static inline std::string component_name = "map_decorations";
            };

            struct charged_projectiles {
                list_array<slot_data*> data;

                bool operator==(const charged_projectiles& other) const;

                bool operator!=(const charged_projectiles& other) const {
                    return !operator==(other);
                }
                static inline std::string component_name = "charged_projectiles";
            };

            struct bundle_contents {
                list_array<slot_data*> items;

                bool operator==(const bundle_contents& other) const;
                bool operator!=(const bundle_contents& other) const{
                    return !operator==(other);
                }
                static inline std::string component_name = "bundle_contents";
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
                std::optional<int32_t> get_potion_id() const;
                std::optional<int32_t> get_custom_color() const;
                std::optional<std::string> get_custom_name() const;

                auto operator<=>(const potion_contents& other) const = default;
                static inline std::string component_name = "potion_contents";
            };

            struct suspicious_stew_effects {
                list_array<std::pair<int32_t, int32_t>> effects; //id, duration
                auto operator<=>(const suspicious_stew_effects& other) const = default;
                static inline std::string component_name = "suspicious_stew_effects";
            };

            struct writable_book_content {
                list_array<item_page> pages;
                auto operator<=>(const writable_book_content& other) const = default;
                static inline std::string component_name = "writable_book_content";
            };

            struct written_book_content {
                std::string raw_title;
                std::optional<std::string> filtered_title;
                std::string author;
                list_array<item_page_signed> pages;
                int32_t generation = 0;
                bool resolved = false;

                auto operator<=>(const written_book_content& other) const = default;
                static inline std::string component_name = "written_book_content";
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
                bool show_in_tooltip = true;

                auto operator<=>(const trim& other) const = default;
                static inline std::string component_name = "trim";
            };

            struct debug_stick_state {
                enbt::compound previous_state;
                auto operator<=>(const debug_stick_state& other) const = default;
                static inline std::string component_name = "debug_stick_state";
            };

            struct entity_data {
                enbt::compound value;
                auto operator<=>(const entity_data& other) const = default;
                static inline std::string component_name = "entity_data";
            };

            struct bucket_entity_data {
                enbt::compound value;
                auto operator<=>(const bucket_entity_data& other) const = default;
                static inline std::string component_name = "bucket_entity_data";
            };

            struct block_entity_data {
                enbt::compound value;
                auto operator<=>(const block_entity_data& other) const = default;
                static inline std::string component_name = "block_entity_data";
            };

            struct instrument {
                struct type_extended {
                    std::variant<std::string, inner::sound_extended> sound;
                    float duration;
                    float range;
                    auto operator<=>(const type_extended& other) const = default;
                };

                std::variant<std::string, type_extended> type;
                auto operator<=>(const instrument& other) const = default;
                static inline std::string component_name = "instrument";
            };

            struct ominous_bottle_amplifier {
                int32_t amplifier; //0..4
                auto operator<=>(const ominous_bottle_amplifier& other) const = default;
                static inline std::string component_name = "ominous_bottle_amplifier";
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
                bool show_in_tooltip = true;

                auto operator<=>(const jukebox_playable& other) const = default;
                static inline std::string component_name = "jukebox_playable";
            };

            struct recipes {
                std::vector<std::string> value;
                auto operator<=>(const recipes& other) const = default;
                static inline std::string component_name = "recipes";
            };

            struct lodestone_tracker {
                struct global_position {
                    std::string dimension;
                    Position position;
                    auto operator<=>(const global_position& other) const = default;
                };

                //if not set then the compass will spin randomly
                std::optional<global_position> global_pos;
                bool tracked = true;

                auto operator<=>(const lodestone_tracker& other) const = default;
                static inline std::string component_name = "lodestone_tracker";
            };

            struct firework_explosion {
                item_firework_explosion value;
                auto operator<=>(const firework_explosion& other) const = default;
                static inline std::string component_name = "firework_explosion";
            };

            struct fireworks {
                list_array<item_firework_explosion> explosions;
                int32_t duration;
                auto operator<=>(const fireworks& other) const = default;
                static inline std::string component_name = "fireworks";
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
                static inline std::string component_name = "profile";
            };

            struct note_block_sound {
                std::string sound;
                auto operator<=>(const note_block_sound& other) const = default;
                static inline std::string component_name = "note_block_sound";
            };

            struct base_color {
                item_color color;
                auto operator<=>(const base_color& other) const = default;
                static inline std::string component_name = "base_color";
            };

            struct pot_decorations {
                std::string decorations[4];
                auto operator<=>(const pot_decorations& other) const = default;
                static inline std::string component_name = "pot_decorations";
            };

            struct container {
                //optimized to reduce memory consumption, may be extended to map{item:slot_data, slot:int32} in future to support custom containers
                slot_data* items[256] = {nullptr};
                container() = default;
                container(const container&);

                void set(uint8_t slot, slot_data&& item);
                void set(uint8_t slot, const slot_data& item);
                slot_data* get(uint8_t slot);
                bool contains(uint8_t slot);
                list_array<uint8_t> contains(const std::string& id, size_t count = 1) const;
                std::optional<uint8_t> contains(const slot_data& item) const;

                void remove(uint8_t slot);
                void clear();

                uint8_t count() const {
                    uint8_t i = 0;
                    for (int i = 0; i < 256; i++)
                        i += (bool)items[i];
                    return i;
                }

                template <class FN>
                void for_each(FN&& fn) const {
                    for (int i = 0; i < 256; i++) {
                        if (items[i])
                            fn(*items[i], i);
                    }
                }

                container(container&& other) {
                    for (int i = 0; i < 256; i++) {
                        items[i] = other.items[i];
                        other.items[i] = nullptr;
                    }
                }

                ~container();

                container& operator=(const container&);

                container& operator=(container&& other) {
                    for (int i = 0; i < 256; i++) {
                        items[i] = other.items[i];
                        other.items[i] = nullptr;
                    }
                    return *this;
                }

                bool operator==(const container& other) const;

                bool operator!=(const container& other) const {
                    return !operator==(other);
                }

                static inline std::string component_name = "container";
            };

            struct block_state {
                struct property_t {
                    std::string name;
                    std::string value;
                    auto operator<=>(const property_t& other) const = default;
                };

                list_array<property_t> properties;

                auto operator<=>(const block_state& other) const = default;
                static inline std::string component_name = "block_state";
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
                static inline std::string component_name = "bees";
            };

            struct lock {
                std::string key; //1.21 in packet as NBT with string tag

                auto operator<=>(const lock& other) const = default;
                static inline std::string component_name = "lock";
            };

            struct container_loot {
                std::string loot_table;
                int64_t seed;

                auto operator<=>(const container_loot& other) const = default;
                static inline std::string component_name = "container_loot";
            };

            struct map_post_processing {
                int32_t value;     //root
                int64_t locked_id = -1; // server side

                auto operator<=>(const map_post_processing& other) const = default;
                static inline std::string component_name = "map_post_processing";
            };

            struct consumable {
                list_array<inner::application_effect> on_consume_effects; //optional
                std::variant<std::string, inner::sound_extended> sound = "entity.generic.eat";
                std::string animation = "eat";
                float consume_seconds = 1.6;
                bool has_consume_particles = true;

                auto operator<=>(const consumable& other) const = default;
                static inline std::string component_name = "consumable";
            };

            struct damage_resistant { //since 1.21.2, replaces `fire_resistant` component
                std::string types;    //damage type tag tag

                auto operator<=>(const damage_resistant& other) const = default;
                static inline std::string component_name = "damage_resistant";
            };

            struct death_protection {
                list_array<inner::application_effect> death_effects; //optional

                auto operator<=>(const death_protection& other) const = default;
                static inline std::string component_name = "death_protection";
            };

            struct enchantable {
                int32_t value; //limit to enchanting cost

                auto operator<=>(const enchantable& other) const = default;
                static inline std::string component_name = "enchantable";
            };

            struct equippable {
                struct equip_sound_custom {
                    std::string sound_id;
                    float range;
                    auto operator<=>(const equip_sound_custom& other) const = default;
                };

                std::string slot; //head, chest, legs, feet, body, mainhand, offhand
                std::optional<std::string> camera_overlay;
                std::optional<std::string> model;
                std::variant<std::string, equip_sound_custom, std::nullptr_t> equip_sound;
                std::variant<std::string, std::vector<std::string>, std::nullptr_t> allowed_entities; //if empty then allowed to all entities
                bool dispensable = true;
                bool swappable = true;
                bool damage_on_hurt = true;

                auto operator<=>(const equippable& other) const = default;
                static inline std::string component_name = "equippable";
            };

            struct glider {
                auto operator<=>(const glider& other) const = default;
                static inline std::string component_name = "glider";
            };

            struct item_model {
                std::string value;
                auto operator<=>(const item_model& other) const = default;
                static inline std::string component_name = "item_model";
            };

            struct repairable {
                std::variant<std::string, std::vector<std::string>> items;
                auto operator<=>(const repairable& other) const = default;
                static inline std::string component_name = "repairable";
            };

            struct tooltip_style {
                std::string value;

                auto operator<=>(const tooltip_style& other) const = default;
                static inline std::string component_name = "tooltip_style";
            };

            struct use_cooldown {
                std::optional<std::string> cooldown_group;
                float seconds;

                auto operator<=>(const use_cooldown& other) const = default;
                static inline std::string component_name = "use_cooldown";
            };

            struct use_remainder {
                slot_data* proxy_value;
                //std::string id;  extracted by `proxy_value->get_slot_data().id`
                //int32_t count;   proxy_value->count
                //components;      proxy_value->components
                use_remainder(slot_data&& consume);
                ~use_remainder();

                bool operator==(const use_remainder& other) const;
                bool operator!=(const use_remainder& other) const{
                    return !operator==(other);
                }
                static inline std::string component_name = "use_remainder";
            };

            struct use_remainder____weak {
                size_t count;
                std::string id;

                auto operator<=>(const use_remainder____weak& other) const = default;
                static inline std::string component_name = "use_remainder____weak";
            };

            using unified
                = std::variant<
                    custom_data,
                    max_stack_size,
                    max_damage,
                    damage,
                    unbreakable,
                    custom_name,
                    item_name,
                    lore,
                    rarity,
                    enchantments,
                    can_place_on,
                    can_break,
                    attribute_modifiers,
                    banner_patterns,
                    custom_model_data,
                    hide_additional_tooltip,
                    hide_tooltip,
                    repair_cost,
                    creative_slot_lock,
                    enchantment_glint_override,
                    intangible_projectile,
                    food,
                    tool,
                    stored_enchantments,
                    dyed_color,
                    map_color,
                    map_id,
                    map_decorations,
                    charged_projectiles,
                    bundle_contents,
                    potion_contents,
                    suspicious_stew_effects,
                    writable_book_content,
                    written_book_content,
                    trim,
                    debug_stick_state,
                    entity_data,
                    bucket_entity_data,
                    block_entity_data,
                    instrument,
                    ominous_bottle_amplifier,
                    jukebox_playable,
                    recipes,
                    lodestone_tracker,
                    firework_explosion,
                    fireworks,
                    profile,
                    note_block_sound,
                    base_color,
                    pot_decorations,
                    container,
                    block_state,
                    bees,
                    lock,
                    container_loot,
                    map_post_processing,
                    consumable,
                    damage_resistant,
                    death_protection,
                    enchantable,
                    equippable,
                    glider,
                    item_model,
                    repairable,
                    tooltip_style,
                    use_cooldown,
                    use_remainder,
                    use_remainder____weak>;

            unified parse_component(const std::string& name, const enbt::value& item);
        }

        struct static_slot_data {
            std::string id;
            std::unordered_map<std::string, slot_component::unified> default_components;
            int32_t internal_id;

            enbt::compound server_side;
            //{
            //  "pot_decoration_aliases": ["...",...]
            //}


            //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
            static void reset_items();      //INTERNAL
            static void initialize_items(); //INTERNAL, used to assign internal_item_aliases ids from item_aliases

            list_array<std::string> item_aliases;
            std::unordered_map<uint32_t, uint32_t> internal_item_aliases; //protocol id -> block id
            static std::unordered_map<uint32_t, std::unordered_map<std::string, uint32_t>> internal_item_aliases_protocol;
        };

        struct slot_data {
            std::unordered_map<std::string, slot_component::unified> components;
            int32_t count = 0;
            int32_t id = 0;

            template <class T>
            T& get_component() {
                return std::get<T>(components.at(T::component_name));
            }

            template <class T>
            T& access_component() {
                if (components.contains(T::component_name))
                    return std::get<T>(components[T::component_name]);
                else
                    return std::get<T>(components[T::component_name] = T{});
            }

            template <class T>
            const T& get_component() const {
                return std::get<T>(components.at(T::component_name));
            }

            template <class T>
            void remove_component() {
                return components.erase(T::component_name);
            }

            void add_component(slot_component::unified&& copy) {
                std::visit(
                    [this](auto& component) {
                        using T = std::decay_t<decltype(component)>;
                        components[T::component_name] = std::move(component);
                    },
                    copy
                );
            }

            template <class T>
            void add_component(T&& move) {
                components[T::component_name] = std::move(move);
            }

            void add_component(const slot_component::unified& copy) {
                std::visit(
                    [this](auto& component) {
                        using T = std::decay_t<decltype(component)>;
                        components[T::component_name] = component;
                    },
                    copy
                );
            }

            template <class T>
            void add_component(const T& copy) {
                components[T::component_name] = copy;
            }

            template <class T>
            bool has_component() const {
                return components.contains(T::component_name);
            }

            //any data
            void remove_component(const std::string& name) {
                components.erase(name);
            }

            //any data
            bool has_component(const std::string& name) const {
                return components.contains(name);
            }

            enbt::compound to_enbt() const;
            static slot_data from_enbt(enbt::compound_const_ref compound);

            bool operator==(const slot_data& other) const;
            bool operator!=(const slot_data& other) const;


            static slot_data create_item(const std::string& id);
            static slot_data create_item(uint32_t id);
            static static_slot_data& get_slot_data(const std::string& id);
            static static_slot_data& get_slot_data(uint32_t id);

            static void add_slot_data(static_slot_data&& move);

            static_slot_data& get_slot_data();

        private:
            friend class static_slot_data;
            static std::unordered_map<std::string, std::shared_ptr<static_slot_data>> named_full_item_data;
            static std::vector<std::shared_ptr<static_slot_data>> full_item_data_;

            template <typename T, typename = void>
            struct has_component_name : std::false_type {};

            template <typename T>
            struct has_component_name<T, decltype((void)T::component_name, void())> : std::true_type {};
        };

        using slot = std::optional<slot_data>;
    }
}

#endif /* SRC_BASE_OBJECTS_SLOT */
