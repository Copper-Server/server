#ifndef SRC_BASE_OBJECTS_SLOT
#define SRC_BASE_OBJECTS_SLOT

#include "../library/enbt.hpp"
#include "../util/readers.hpp"
#include "chat.hpp"
#include "position.hpp"
#include <optional>

namespace crafted_craft {
    namespace base_objects {
        struct item_attribute {
            int32_t id;
            ENBT::UUID uid;
            std::string name;
            double value;
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
            static operation_e id_to_operation(int32_t id);

            static std::string slot_to_name(slot_filter id);
            static slot_filter name_to_slot(const std::string& id);
        };

        struct item_rule {
            //represents list of blocks
            std::variant<list_array<int32_t>, std::string> value;
            std::optional<float> speed;
            std::optional<bool> correct_drop_for_blocks;
        };

        struct item_potion_effect {
            int32_t potion_id;

            struct effect_data {
                int32_t duration; //-1 infinite
                bool ambient;
                bool show_particles;
                bool show_icon;
                effect_data* hidden_effect = nullptr; //store state of previous weaker effect when it lasts longer than this,
            };

            effect_data data;
        };

        struct item_page {
            std::string text;
            std::optional<std::string> filtered;
        };

        struct item_page_signed {
            Chat text;
            std::optional<Chat> filtered;
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
        };

        enum class item_color : uint8_t {
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
        };

        struct slot_data;

        namespace slot_component {
            namespace inner {
                struct block_predicate {
                };

                struct sound_extended {
                    std::string sound_name;
                    std::optional<float> fixed_range;
                };
            }

            struct custom_data {
                ENBT value;
            };

            struct max_stack_size {
                uint8_t value; // 1..99
            };

            struct max_damage {
                int32_t value;
            };

            struct damage {
                int32_t value;
            };

            struct unbreakable {
                bool value;
            };

            struct custom_name {
                Chat value;
            };

            struct item_name {
                Chat value;
            };

            struct lore {
                list_array<Chat> value;
            };

            enum class rarity {
                common,   //white
                uncommon, //yellow
                rare,     //aqua
                epic,     //pink
            };

            struct enchantments {
                list_array<std::pair<int32_t, int32_t>> enchants; //type ID, level
                bool show_in_tooltip;
            };

            struct can_place_on {
                list_array<inner::block_predicate> predicates;
                bool show_in_tooltip;
            };

            struct can_break {
                list_array<inner::block_predicate> predicates;
                bool show_in_tooltip;
            };

            struct attribute_modifiers {
                list_array<item_attribute> attributes;
                bool show_in_tooltip;
            };

            struct custom_model_data {
                int32_t value;
            };

            struct hide_additional_tooltip { //NEED DOCUMENTATION in wiki.vg, what means `additional`
            };

            struct hide_tooltip {
            };

            struct repair_cost {
                int32_t value;
            };

            struct creative_slot_lock {
            };

            struct enchantment_glint_override {
                int32_t has_glint; //HUH? confusing name
            };

            struct intangible_projectile {
                ENBT value; //sent empty nbt in notchain server
            };

            struct food {
                int32_t nutrition;
                float saturation_modifier;
                float can_always_eat;
                float seconds_to_eat;
                //must contain value
                slot_data* consuming_converts_to;
                list_array<std::pair<int32_t, float>> effects; //effect ID, probability
            };

            struct fire_resistant {
            };

            struct tool {
                list_array<item_rule> rules;
                float default_mining_speed;
                int32_t damage_per_block;
            };

            struct stored_enchantments {
                list_array<std::pair<int32_t, int32_t>> enchants; //type ID, level
                bool show_in_tooltip;
            };

            struct dyed_color {
                int32_t rgb;
            };

            struct map_color {
                int32_t rgb;
            };

            struct map_id {
                int32_t value;
            };

            struct map_decorations {
                ENBT value;
            };

            enum class map_post_processing {
                lock = 0,
                scale = 1,
            };

            struct charged_projectiles {
                list_array<slot_data*> data;
            };

            struct bundle_contents {
                list_array<slot_data*> items;
            };

            struct potion_contents {
                std::optional<int32_t> potion_id;
                std::optional<int32_t> color_rgb;
                list_array<item_potion_effect> custom_effects;
            };

            struct suspicious_stew_effects {
                list_array<std::pair<int32_t, int32_t>> effects; //id, duration
            };

            struct writable_book_content {
                list_array<item_page> pages;
            };

            struct written_book_content {
                std::string raw_title;
                std::optional<std::string> filtered_title;
                std::string author;
                list_array<item_page_signed> pages;
                int32_t generation;
                bool resolved;
            };

            struct trim {
                struct material_extended {
                    std::string asset_name;
                    list_array<std::pair<int32_t, std::string>> overrides; //armor material type, asset name
                    Chat description;
                    int32_t ingredient;
                    float item_model_index;
                };

                struct pattern_extended {
                    std::string asset_name;
                    Chat description;
                    int32_t template_item;
                    bool decal;
                };

                std::variant<int32_t, material_extended> material; //id or extended
                std::variant<int32_t, pattern_extended> pattern;   //id or extended
                bool show_in_tooltip;
            };

            struct debug_stick_state {
                ENBT previous_state;
            };

            struct entity_data {
                ENBT value;
            };

            struct bucket_entity_data {
                ENBT value;
            };

            struct block_entity_data {
                ENBT value;
            };

            struct instrument {
                struct type_extended {
                    std::variant<int32_t, inner::sound_extended> sound;
                    float duration;
                    float range;
                };

                std::variant<int32_t, type_extended> type;
            };

            struct ominous_bottle_amplifier {
                int32_t value; //0..4
            };

            struct jukebox_playable {
                //WARN jukebox_extended will cause the client to fail to parse it
                // and subsequently disconnect, which is likely an unintended bug.
                // Currently use only direct song_name or jukebox_compact
                struct jukebox_extended {
                    inner::sound_extended sound;
                    Chat description;
                    float duration;
                    int32_t redstone_output;
                };

                struct jukebox_compact {
                    int32_t song_type;
                };

                std::variant<std::string, jukebox_extended, jukebox_compact> song; //song name, extended, compact
                bool show_in_tooltip;
            };

            struct recipes {
                ENBT value;
            };

            struct lodestone_tracker {
                struct global_position {
                    std::string dimension;
                    Position position;
                };

                //if not set then the compass will spin randomly
                std::optional<global_position> global_pos;

                bool tracked;
            };

            struct firework_explosion {
                item_firework_explosion value;
            };

            struct fireworks {
                list_array<item_firework_explosion> explosions;
                int32_t duration;
            };

            struct profile {
                struct property_t {
                    std::string name; //64
                    std::string value;
                    std::optional<std::string> signature; //1024
                };

                std::optional<std::string> name;
                std::optional<ENBT::UUID> uid;
                list_array<property_t> properties;
            };

            struct note_block_sound {
                std::string sound;
            };

            struct banner_pattern {
                struct layer_direct {
                    std::string id;
                    std::string translation_key;
                    item_color color;
                };

                struct layer {
                    int32_t pattern;
                    item_color color;
                };

                list_array<std::variant<layer, layer_direct>> layers;
            };

            struct base_color {
                item_color color;
            };

            struct pot_decorations {
                int32_t decorations[4];
            };

            struct container {
                slot_data* items[256] = {nullptr};
                container() = default;
                container(const container&);

                container(container&& other) {
                    for (int i = 0; i < 256; i++) {
                        items[i] = other.items[i];
                        other.items[i] = nullptr;
                    }
                }

                ~container() {
                    for (int i = 0; i < 256; i++) {
                        delete items[i];
                    }
                }

                container& operator=(const container&);

                container& operator=(container&& other) {
                    for (int i = 0; i < 256; i++) {
                        items[i] = other.items[i];
                        other.items[i] = nullptr;
                    }
                    return *this;
                }
            };

            struct block_state {
                struct property_t {
                    std::string name;
                    std::string value;
                };

                list_array<property_t> properties;
            };

            struct bees {
                struct bee {
                    ENBT entity_data;
                    int32_t ticks_in_hive;
                    int32_t min_ticks_in_hive;
                };

                list_array<bee> values;
            };

            struct lock {
                std::string key; //1.21 in packet as NBT with string tag
            };

            struct container_loot {
                ENBT data;
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
                    custom_model_data,
                    hide_additional_tooltip,
                    hide_tooltip,
                    repair_cost,
                    creative_slot_lock,
                    enchantment_glint_override,
                    intangible_projectile,
                    food,
                    fire_resistant,
                    tool,
                    stored_enchantments,
                    dyed_color,
                    map_color,
                    map_id,
                    map_decorations,
                    map_post_processing,
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
                    banner_pattern,
                    base_color,
                    pot_decorations,
                    container,
                    block_state,
                    bees,
                    lock,
                    container_loot>;
        }

        struct slot_data_storage {
            std::optional<ENBT> nbt;
            int32_t id = 0;
            uint8_t count = 0;
            slot_data unpack() const;
        };

        struct slot_data {
            std::unordered_map<std::string, slot_component::unified> components;
            int32_t count = 0;
            int32_t id = 0;


            slot_data_storage pack() const;

            bool operator==(const slot_data& other) const;
            bool operator!=(const slot_data& other) const;
        };


        using slot = std::optional<slot_data>;
    }
}

#endif /* SRC_BASE_OBJECTS_SLOT */
