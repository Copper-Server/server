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
            enbt::raw_uuid uid;
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
            static int32_t operation_to_id(const std::string& id);
            static operation_e id_to_operation(int32_t id);
            static operation_e id_to_operation(const std::string& id);
            static std::string id_to_operation_string(int32_t id);

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
                bool ambient = false;
                bool show_particles = true;
                bool show_icon = true;
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
                using block_predicate = enbt::compound;

                struct sound_extended {
                    std::string sound_name;
                    std::optional<float> fixed_range;
                };

                struct apply_effects {
                    struct effect {
                        std::string id;
                        int8_t amplifier = 0;
                        int32_t duration;
                        bool ambient = false;
                        bool show_particles = true;
                        bool show_icon = true;
                    };

                    list_array<effect> effects;
                };

                struct remove_effects {
                    std::variant<std::string, list_array<std::string>> effects;
                };

                struct clear_all_effects {};

                struct teleport_randomly {
                    float diameter;
                };

                struct play_sound {
                    std::variant<std::string, inner::sound_extended> sound;
                };

                struct __custom {
                    std::string type;
                    enbt::value value;
                };

                using application_effect = std::variant<apply_effects, remove_effects, clear_all_effects, teleport_randomly, play_sound, __custom>;
            }

            struct custom_data {
                enbt::compound value;
                static inline std::string component_name = "custom_data";
            };

            struct max_stack_size {
                uint8_t value; // 1..99

                static inline std::string component_name = "max_stack_size";
            };

            struct max_damage {
                int32_t value;
                static inline std::string component_name = "max_damage";
            };

            struct damage {
                int32_t value;
                static inline std::string component_name = "damage";
            };

            struct unbreakable {
                bool value;
                static inline std::string component_name = "unbreakable";
            };

            struct custom_name {
                Chat value;
                static inline std::string component_name = "custom_name";
            };

            struct item_name {
                Chat value;
                static inline std::string component_name = "item_name";
            };

            struct lore {
                list_array<Chat> value;
                static inline std::string component_name = "lore";
            };

            struct rarity {
                enum {
                    common,   //white
                    uncommon, //yellow
                    rare,     //aqua
                    epic,     //pink
                } value;

                static inline std::string component_name = "rarity";
            };

            struct enchantments {
                list_array<std::pair<int32_t, int32_t>> enchants; //type ID, level
                bool show_in_tooltip;
                static inline std::string component_name = "enchantments";
            };

            struct can_place_on {
                list_array<inner::block_predicate> predicates;
                bool show_in_tooltip = true;
                static inline std::string component_name = "can_place_on";
            };

            struct can_break {
                list_array<inner::block_predicate> predicates;
                bool show_in_tooltip = true;
                static inline std::string component_name = "can_break";
            };

            struct attribute_modifiers {
                list_array<item_attribute> attributes;
                bool show_in_tooltip = true;
                static inline std::string component_name = "attribute_modifiers";
            };

            struct banner_patterns {
                struct custom_pattern {
                    std::string asset_id;
                    std::string translation_key;
                };

                struct pattern {
                    std::string color;
                    std::variant<std::string, custom_pattern> pattern;
                };

                list_array<pattern> value;

                static inline std::string component_name = "banner_patterns";
            };

            struct custom_model_data {
                int32_t value;
                static inline std::string component_name = "custom_model_data";
            };

            struct hide_additional_tooltip { //NEED DOCUMENTATION in wiki.vg, what means `additional`
                static inline std::string component_name = "hide_additional_tooltip";
            };

            struct hide_tooltip {
                static inline std::string component_name = "hide_tooltip";
            };

            struct repair_cost {
                int32_t value;
                static inline std::string component_name = "repair_cost";
            };

            struct creative_slot_lock {
                static inline std::string component_name = "creative_slot_lock";
            };

            struct enchantment_glint_override {
                int32_t has_glint;
                static inline std::string component_name = "enchantment_glint_override";
            };

            struct intangible_projectile {
                enbt::value value; //sent empty nbt in notchain server
                static inline std::string component_name = "intangible_projectile";
            };

            struct food {
                int32_t nutrition;
                float saturation;
                bool can_always_eat = false;

                //does noting if `consumable` component not defined
                //float seconds_to_eat; deprecated , since 1.21.2, use consumable.consume_seconds
                //slot_data* consuming_converts_to; deprecated , since 1.21.2, use use_remainder component
                //list_array<std::pair<int32_t, float>> effects; //effect ID, probability     deprecated, since 1.21.2, use consumable.on_consume_effects
                static inline std::string component_name = "food";
            };

            //deprecated
            struct fire_resistant {
                static inline std::string component_name = "fire_resistant";
            };

            struct tool {
                list_array<item_rule> rules;
                float default_mining_speed;
                int32_t damage_per_block;
                static inline std::string component_name = "tool";
            };

            struct stored_enchantments {
                list_array<std::pair<int32_t, int32_t>> enchants; //type ID, level
                bool show_in_tooltip;
                static inline std::string component_name = "stored_enchantments";
            };

            struct dyed_color {
                int32_t rgb;
                bool show_in_tooltip;
                static inline std::string component_name = "dyed_color";
            };

            struct map_color {
                int32_t rgb;
                static inline std::string component_name = "map_color";
            };

            struct map_id {
                int32_t value;
                static inline std::string component_name = "map_id";
            };

            struct map_decorations {
                enbt::value value;
                static inline std::string component_name = "map_decorations";
            };

            struct charged_projectiles {
                list_array<slot_data*> data;
                static inline std::string component_name = "charged_projectiles";
            };

            struct bundle_contents {
                list_array<slot_data*> items;
                static inline std::string component_name = "bundle_contents";
            };

            struct potion_contents {
                std::optional<int32_t> potion_id;
                std::optional<int32_t> color_rgb;
                list_array<item_potion_effect> custom_effects;
                std::optional<std::string> custom_name; //since 1.21.2
                static inline std::string component_name = "potion_contents";
            };

            struct suspicious_stew_effects {
                list_array<std::pair<int32_t, int32_t>> effects; //id, duration
                static inline std::string component_name = "suspicious_stew_effects";
            };

            struct writable_book_content {
                list_array<item_page> pages;
                static inline std::string component_name = "writable_book_content";
            };

            struct written_book_content {
                std::string raw_title;
                std::optional<std::string> filtered_title;
                std::string author;
                list_array<item_page_signed> pages;
                int32_t generation;
                bool resolved;
                static inline std::string component_name = "written_book_content";
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
                bool show_in_tooltip = true;
                static inline std::string component_name = "trim";
            };

            struct debug_stick_state {
                enbt::compound previous_state;
                static inline std::string component_name = "debug_stick_state";
            };

            struct entity_data {
                enbt::value value;
                static inline std::string component_name = "entity_data";
            };

            struct bucket_entity_data {
                enbt::value value;
                static inline std::string component_name = "bucket_entity_data";
            };

            struct block_entity_data {
                enbt::value value;
                static inline std::string component_name = "block_entity_data";
            };

            struct instrument {
                struct type_extended {
                    std::variant<std::string, inner::sound_extended> sound;
                    float duration;
                    float range;
                };

                std::variant<std::string, type_extended> type;
                static inline std::string component_name = "instrument";
            };

            struct ominous_bottle_amplifier {
                int32_t value; //0..4
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
                };

                struct jukebox_compact {
                    int32_t song_type;
                };

                std::variant<std::string, jukebox_extended, jukebox_compact> song; //song name, extended, compact
                bool show_in_tooltip;
                static inline std::string component_name = "jukebox_playable";
            };

            struct recipes {
                enbt::value value;
                static inline std::string component_name = "recipes";
            };

            struct lodestone_tracker {
                struct global_position {
                    std::string dimension;
                    Position position;
                };

                //if not set then the compass will spin randomly
                std::optional<global_position> global_pos;

                bool tracked = true;
                static inline std::string component_name = "lodestone_tracker";
            };

            struct firework_explosion {
                item_firework_explosion value;
                static inline std::string component_name = "firework_explosion";
            };

            struct fireworks {
                list_array<item_firework_explosion> explosions;
                int32_t duration;
                static inline std::string component_name = "fireworks";
            };

            struct profile {
                struct property_t {
                    std::string name; //64
                    std::string value;
                    std::optional<std::string> signature; //1024
                };

                std::optional<std::string> name;
                std::optional<enbt::raw_uuid> uid;
                list_array<property_t> properties;
                static inline std::string component_name = "profile";
            };

            struct note_block_sound {
                std::string sound;
                static inline std::string component_name = "note_block_sound";
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
                static inline std::string component_name = "banner_pattern";
            };

            struct base_color {
                item_color color;
                static inline std::string component_name = "base_color";
            };

            struct pot_decorations {
                std::string decorations[4];
                static inline std::string component_name = "pot_decorations";
            };

            struct container {
                slot_data* items[256] = {nullptr};
                container() = default;
                container(const container&);

                void set(uint8_t slot, slot_data&& item) {
                    if (slot < 256) {
                        if (items[slot])
                            delete items[slot];
                        items[slot] = new slot_data(std::move(item));
                    } else
                        throw std::runtime_error("Slot out of range");
                }

                void set(uint8_t slot, const slot_data& item) {
                    if (slot < 256) {
                        if (items[slot])
                            delete items[slot];
                        items[slot] = new slot_data(item);
                    } else
                        throw std::runtime_error("Slot out of range");
                }

                std::optional<slot_data&> get(uint8_t slot) {
                    if (slot < 256) {
                        if (items[slot])
                            return *items[slot];
                        else
                            return std::nullopt;
                    } else
                        throw std::runtime_error("Slot out of range");
                }

                bool contains(uint8_t slot) {
                    if (slot < 256)
                        return items[slot] != nullptr;
                    else
                        throw std::runtime_error("Slot out of range");
                }

                void remove(uint8_t slot) {
                    if (slot < 256) {
                        if (items[slot])
                            delete items[slot];
                        items[slot] = nullptr;
                    } else
                        throw std::runtime_error("Slot out of range");
                }

                void clear() {
                    for (int i = 0; i < 256; i++) {
                        if (items[i])
                            delete items[i];
                        items[i] = nullptr;
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

                static inline std::string component_name = "container";
            };

            struct block_state {
                struct property_t {
                    std::string name;
                    std::string value;
                };

                list_array<property_t> properties;
                static inline std::string component_name = "block_state";
            };

            struct bees {
                struct bee {
                    enbt::value entity_data;
                    int32_t ticks_in_hive;
                    int32_t min_ticks_in_hive;
                };

                list_array<bee> values;
                static inline std::string component_name = "bees";
            };

            struct lock {
                std::string key; //1.21 in packet as NBT with string tag
                static inline std::string component_name = "lock";
            };

            struct container_loot {
                std::string loot_table;
                int64_t seed;
                static inline std::string component_name = "container_loot";
            };

            struct map_post_processing {
                int32_t value;     //root
                int64_t locked_id = -1; // server side
                static inline std::string component_name = "map_post_processing";
            };

            struct consumable {
                list_array<inner::application_effect> death_effects; //optional
                std::variant<std::string, inner::sound_extended> sound = "entity.generic.eat";
                std::string animation;
                float consumable_seconds;
                bool has_consume_particles = true;
                static inline std::string component_name = "consumable";
            };

            struct damage_resistant { //since 1.21.2, replaces `fire_resistant` component
                std::string types;    //damage type tag tag
                static inline std::string component_name = "damage_resistant";
            };

            struct death_protection {
                list_array<inner::application_effect> death_effects; //optional
                static inline std::string component_name = "death_protection";
            };

            struct enchantable {
                int32_t value; //limit to enchanting cost
                static inline std::string component_name = "enchantable";
            };

            struct equippable {
                struct equip_sound_custom {
                    std::string sound_id;
                    float range;
                };

                std::string slot; //head, chest, legs, feet, body, mainhand, offhand
                std::string camera_overlay;
                std::string model;
                std::variant<std::string, equip_sound_custom> equip_sound;
                std::variant<std::string, std::vector<std::string>> allowed_entities; //if empty then allowed to all entities
                bool dispensable = true;
                bool swappable = true;
                bool damage_on_hurt = true;

                static inline std::string component_name = "equippable";
            };

            struct glider {
                static inline std::string component_name = "glider";
            };

            struct item_model {
                std::string value;
                static inline std::string component_name = "item_model";
            };

            struct repairable {
                std::variant<std::string, std::vector<std::string>> items;
                static inline std::string component_name = "repairable";
            };

            struct tooltip_style {
                std::string value;
                static inline std::string component_name = "tooltip_style";
            };

            struct use_cooldown {
                std::optional<std::string> cooldown_group;
                float seconds;

                static inline std::string component_name = "use_cooldown";
            };

            struct use_remainder {
                slot_data* proxy_value;
                //std::string id;  extracted by `proxy_value->get_slot_data().id`
                //int32_t count;   proxy_value->count
                //components;      proxy_value->components
                use_remainder(slot_data&& consume);
                ~use_remainder();
                static inline std::string component_name = "use_remainder";
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
                    use_remainder>;
        }

        struct slot_data_old {
            std::optional<enbt::value> nbt;
            int32_t id = 0;
            uint8_t count = 0;
            slot_data to_new() const;
        };

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

            list_array<base_objects::shared_string> item_aliases;
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
            const T& get_component() const {
                return std::get<T>(components.at(T::component_name));
            }

            template <class T>
            void remove_component() {
                return components.erase(T::component_name);
            }

            template <class T>
            void add_component(T&& move) {
                components[T::component_name] = std::move(move);
            }

            void add_component(const slot_component::unified& copy) {
                std::visit([this](auto& component) {
                    using T = std::decay_t<decltype(component)>;
                    components[T::component_name] = component;
                },
                           copy);
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

            slot_data_old to_old() const;

            bool operator==(const slot_data& other) const;
            bool operator!=(const slot_data& other) const;


            static slot_data create_item(const base_objects::shared_string& id);
            static slot_data create_item(uint32_t id);
            static static_slot_data& get_slot_data(const base_objects::shared_string& id);
            static static_slot_data& get_slot_data(uint32_t id);

            static void add_slot_data(static_slot_data&& move);

            static_slot_data& get_slot_data();

        private:
            friend class static_slot_data;
            static std::unordered_map<base_objects::shared_string, std::shared_ptr<static_slot_data>> named_full_item_data;
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
