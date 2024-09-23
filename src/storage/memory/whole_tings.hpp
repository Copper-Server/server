#ifndef SRC_STORAGE_MEMORY_WHOLE_TINGS
#define SRC_STORAGE_MEMORY_WHOLE_TINGS
#include "../../base_objects/chat.hpp"
#include "../../base_objects/particle_data.hpp"

#include "../../base_objects/entity.hpp"
#include "../../base_objects/shared_string.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace crafted_craft {
    namespace storage {
        namespace memory {
            namespace whole_things {
                template <class T>
                struct range {
                    T min;
                    T max;
                };

                template <class T>
                using condition_value = std::variant<T, range<T>>;

                struct float_provider_constant {
                    float value;
                };

                struct float_provider_uniform {
                    float min_inclusive;
                    float max_exclusive;
                };

                struct float_provider_clamped_normal {
                    float mean;
                    float deviation;
                    float min;
                    float max;
                };

                struct float_provider_trapezoid {
                    float min;
                    float max;
                    float plateau;
                };

                using float_provider = std::variant<float_provider_constant, float_provider_uniform, float_provider_clamped_normal, float_provider_trapezoid>;

                struct tags_ {
                    using tag = std::vector<std::string>;
                    using tags_folder = std::unordered_map<std::string, tag>;

                    struct banner_pattern_ {
                        tag no_item_required;
                        tags_folder pattern_item;
                    };

                    struct cat_variant_ {
                        tag default_spawns;
                        tag full_moon_spawns;
                    };

                    struct enchantment_ {
                        tags_folder self_;
                        tags_folder exclusive_set;
                    };

                    struct world_gen_ {
                        struct biome_ {
                            tags_folder self_;
                            tags_folder has_structure;
                        } biome;

                        tags_folder flat_level_generator_preset;
                        tags_folder structure;
                        tags_folder world_preset;
                    };

                    banner_pattern_ banner_pattern;
                    tags_folder blocks;
                    cat_variant_ cat_variant;
                    tags_folder damage_type;
                    enchantment_ enchantment;
                    tags_folder entity_type;
                    tags_folder fluid;
                    tags_folder game_event;
                    tags_folder instrument;
                    tags_folder item;
                    tags_folder painting_variant;
                    tags_folder point_of_interest_type;
                    tags_folder point_of_interest_type;
                };

                struct predicate {
                    struct all_off;
                    struct any_off;

                    struct block_state_property {                                                                 //requires loot context
                        base_objects::shared_string block;                                                        //id
                        std::unordered_map<base_objects::shared_string, condition_value<std::string>> properties; //optional
                    };

                    struct damage_source_properties {
                        struct entity_condition {
                            struct distance_t {
                                std::optional<condition_value<float>> absolute;   //xyz
                                std::optional<condition_value<float>> horizontal; //xz
                                std::optional<condition_value<float>> x;
                                std::optional<condition_value<float>> y;
                                std::optional<condition_value<float>> z;
                            };

                            struct effect_t {
                                std::optional<condition_value<int32_t>> amplifier;
                                std::optional<condition_value<int32_t>> duration;
                                std::optional<bool> ambient;
                                std::optional<bool> visible;
                            };

                            struct equipment_t {
                                std::optional<std::variant<base_objects::shared_string, std::vector<base_objects::shared_string>, std::string>> items; //ID, list of IDs or tag
                                std::optional<condition_value<int32_t>> count;
                                std::optional<enbt::value> components;
                                std::optional<enbt::value> predicates; //sub predicates
                            };

                            struct flags_t {
                                bool is_baby : 1;
                                bool is_on_fire : 1;
                                bool is_sneaking : 1;
                                bool is_sprinting : 1;
                                bool is_swimming : 1;
                                bool is_on_ground : 1;
                                bool is_flying : 1;
                            };

                            std::variant<base_objects::shared_string, std::vector<base_objects::shared_string>, std::string> type; //ID, list of IDs or tag
                            std::optional<distance_t> distance;
                            std::unordered_map<base_objects::shared_string, effect_t> effects;
                            std::unordered_map<base_objects::shared_string, equipment_t> equipment; //mainhand, offhand, head, chest, legs, feet, body
                        };
                    };

                    //struct effect_type_hold {
                    //    std::variant<set*, add*, multiply*, remove_binomial*, _custom*, all_off*> effect_type;
                    //
                    //    std::string type();
                    //    set& as_set();
                    //    add& as_add();
                    //    multiply& as_multiply();
                    //    remove_binomial& as_remove_binomial();
                    //    _custom& as_custom();
                    //    all_off& as_all_off();
                    //
                    //    std::variant<
                    //        set&,
                    //        add&,
                    //        multiply&,
                    //        remove_binomial&,
                    //        _custom&,
                    //        all_off&>
                    //    get_variant();
                    //
                    //    effect_type_hold(set*);
                    //    effect_type_hold(add*);
                    //    effect_type_hold(multiply*);
                    //    effect_type_hold(remove_binomial*);
                    //    effect_type_hold(_custom*);
                    //    effect_type_hold(all_off*);
                    //
                    //    effect_type_hold(const effect_type_hold&);
                    //    effect_type_hold(effect_type_hold&&);
                    //
                    //    ~effect_type_hold();
                    //};
                    //
                    //struct all_off {
                    //    std::vector<effect_type_hold> effects;
                    //};
                };

                struct loot_table_ {
                    struct entry {
                        std::string type;
                        std::string name;
                    };

                    struct pool {
                        float rolls;
                        float bonus_rolls;
                        std::vector<entry> entries;
                    };

                    struct loot_table_item {
                        std::vector<pool> pools;
                        std::string type;
                        std::string random_sequence;
                    };

                    std::unordered_map<std::string, loot_table_item> loot_table_items;
                };

                struct banner_pattern_ {
                    struct pattern {
                        std::string asset_id;
                        std::string translation_key;
                    };

                    std::unordered_map<std::string, pattern> patterns;
                };

                struct damage_type_ {
                    struct type_ {
                        std::string message_id;
                        std::string scaling;
                        std::string death_message_type;
                        std::string effects;
                        float exhaustion;
                    };

                    std::unordered_map<std::string, type_> types;
                };

                struct enchantment {
                    struct value_effects {
                        struct _custom {
                            std::string type;
                            std::function<void(
                                base_objects::entity& attacker,
                                base_objects::entity& victim,
                                enbt::value& effect_data
                            )>
                                callback;
                            enbt::value effect_data;
                        };

                        struct set {
                            float value;
                        };

                        struct add {
                            float value;
                        };

                        struct multiply {
                            float factor;
                        };

                        struct remove_binomial {
                            float chance;
                        };

                        struct all_off;

                        struct effect_type_hold {
                            std::variant<set*, add*, multiply*, remove_binomial*, _custom*, all_off*> effect_type;

                            std::string type();
                            set& as_set();
                            add& as_add();
                            multiply& as_multiply();
                            remove_binomial& as_remove_binomial();
                            _custom& as_custom();
                            all_off& as_all_off();

                            std::variant<
                                set&,
                                add&,
                                multiply&,
                                remove_binomial&,
                                _custom&,
                                all_off&>
                            get_variant();

                            effect_type_hold(set*);
                            effect_type_hold(add*);
                            effect_type_hold(multiply*);
                            effect_type_hold(remove_binomial*);
                            effect_type_hold(_custom*);
                            effect_type_hold(all_off*);

                            effect_type_hold(const effect_type_hold&);
                            effect_type_hold(effect_type_hold&&);

                            ~effect_type_hold();
                        };

                        struct all_off {
                            std::vector<effect_type_hold> effects;
                        };

                        std::vector<effect_type_hold> effects;
                        std::optional<std::string> predicate;
                        std::string enchanted; //attacker or victim
                    };

                    struct entity_effects {
                        struct _custom {
                            std::string type;
                            std::function<void(
                                base_objects::entity& attacker,
                                base_objects::entity& victim,
                                base_objects::entity& damaging_entity,
                                enbt::value& effect_data
                            )>
                                callback;
                            enbt::value effect_data;
                        };

                        //only in location based effect
                        struct attribute {
                            struct level_based_value {
                                struct linear {
                                    float base;
                                    float per_level_above_first;
                                };

                                struct levels_squared {
                                    float added;
                                };

                                struct clamped {
                                    level_based_value* value;
                                    float min;
                                    float max;
                                };

                                struct fraction {
                                    float numerator;
                                    float denominator;
                                };

                                struct lookup {
                                    std::vector<float> values; //values[level - 1] == level value
                                    float fallback;            //out of range
                                };

                                std::variant<float, linear, levels_squared, clamped, fraction, lookup> value;
                            } amount;

                            std::string attribute; //id
                            std::string operation; //add_value, add_multiplied_base, add_multiplied_total
                            std::string id;        //resource location, must be unique. WHY? FOR WHAT?
                        };

                        struct apply_mob_effect {
                            std::variant<std::string, std::vector<std::string>> to_apply;
                            float min_duration;
                            float max_duration;
                            float min_amplifier;
                            float max_amplifier;
                        };

                        struct damage_entity {
                            std::string damage_type;
                            float min_damage;
                            float max_damage;
                        };

                        struct damage_item {
                            float amount;
                        };

                        struct explode {
                            struct {
                                std::string type;
                                base_objects::particle_data data;
                            } small_particle;

                            struct {
                                std::string type;
                                base_objects::particle_data data;
                            } large_particle;

                            std::variant<std::string, std::vector<std::string>> immune_blocks;
                            std::optional<int32_t[3]> offset;
                            float knockback_multiplier;
                            std::string damage_type;
                            std::string block_interaction; //none, block, mob, tnt, trigger
                            std::string sound;
                            bool attribute_to_user;
                            bool create_fire;
                        };

                        struct ignite {
                            float duration; //seconds
                        };

                        struct play_sound {
                            std::string sound;
                            float_provider volume;
                            float_provider pitch;
                        };

                        struct replace_block {
                            std::optional<int32_t[3]> offset;
                            std::string trigger_game_event;
                            //TO-DO
                        };

                        struct replace_disk {
                            std::optional<int32_t[3]> offset;
                            std::string trigger_game_event;
                            //TO-DO
                        };

                        struct run_function {
                            std::string function_id;
                        };

                        struct set_block_properties {
                            std::optional<int32_t[3]> offset;
                            std::unordered_map<std::string, std::string> properties;
                        };

                        struct spawn_particles {
                            struct {
                                std::string type;
                                base_objects::particle_data data;
                            } particle;

                            struct position_data {
                                std::string type; //entity_position, in_bounding_box
                                float offset;
                                std::optional<float> scale; //in_bounding_box
                            };

                            struct velocity_data {
                                float_provider base;
                                float movement_scale;
                            };

                            position_data horizontal_position;
                            position_data vertical_position;
                            velocity_data horizontal_velocity;
                            velocity_data vertical_velocity;
                        };

                        struct summon_entity {
                            std::variant<std::string, std::vector<std::string>> tag_or_entity_ids;
                            bool join_owner_team;
                        };

                        struct all_off;

                        struct effect_type_hold {
                            std::variant<
                                attribute*,
                                apply_mob_effect*,
                                damage_entity*,
                                explode*,
                                ignite*,
                                play_sound*,
                                replace_block*,
                                replace_disk*,
                                run_function*,
                                set_block_properties*,
                                spawn_particles*,
                                _custom*,
                                all_off*>
                                effect_type;

                            std::string type();

                            attribute& as_attribute();
                            apply_mob_effect& as_apply_mob_effect();
                            damage_entity& as_damage_entity();
                            explode& as_explode();
                            ignite& as_ignite();
                            play_sound& as_play_sound();
                            replace_block& as_replace_block();
                            replace_disk& as_replace_disk();
                            run_function& as_run_function();
                            set_block_properties& as_set_block_properties();
                            spawn_particles& as_spawn_particles();
                            _custom& as_custom();
                            all_off& as_all_off();

                            std::variant<
                                attribute&,
                                apply_mob_effect&,
                                damage_entity&,
                                explode&,
                                ignite&,
                                play_sound&,
                                replace_block&,
                                replace_disk&,
                                run_function&,
                                set_block_properties&,
                                spawn_particles&,
                                _custom&,
                                all_off&>
                            get_variant();

                            effect_type_hold(attribute*);
                            effect_type_hold(apply_mob_effect*);
                            effect_type_hold(damage_entity*);
                            effect_type_hold(explode*);
                            effect_type_hold(ignite*);
                            effect_type_hold(play_sound*);
                            effect_type_hold(replace_block*);
                            effect_type_hold(replace_disk*);
                            effect_type_hold(run_function*);
                            effect_type_hold(set_block_properties*);
                            effect_type_hold(spawn_particles*);
                            effect_type_hold(_custom*);
                            effect_type_hold(all_off*);

                            effect_type_hold(const effect_type_hold&);
                            effect_type_hold(effect_type_hold&&);

                            ~effect_type_hold();
                        };

                        struct all_off {
                            std::vector<effect_type_hold> effects;
                        };

                        std::vector<effect_type_hold> effects;
                        //std::optional<base_objects::predicate> predicate; //or requirements if location besed
                        std::string enchanted; //attacker, victim or damaging_entity
                        std::string affected;  //attacker, victim or damaging_entity
                    };

                    struct effect_minecraft {
                        struct armor_effectiveness {
                            list_array<value_effects> effects;
                        };

                        struct damage {
                            list_array<value_effects> effects;
                        };

                        struct damage_protection {
                            list_array<value_effects> effects;
                        };

                        struct smash_damage_per_fallen_block {
                            list_array<value_effects> effects;
                        };

                        struct knockback {
                            list_array<value_effects> effects;
                        };

                        struct equipment_drops {
                            list_array<value_effects> effects; //uses enchanted
                        };

                        struct ammo_use {
                            list_array<value_effects> effects;
                        };

                        struct projectile_piercing {
                            list_array<value_effects> effects;
                        };

                        struct block_experience {
                            list_array<value_effects> effects;
                        };

                        struct repair_with_xp {
                            list_array<value_effects> effects;
                        };

                        struct item_damage {
                            list_array<value_effects> effects;
                        };

                        struct projectile_count {
                            list_array<value_effects> effects;
                        };

                        struct trident_return_acceleration {
                            list_array<value_effects> effects;
                        };

                        struct projectile_spread {
                            list_array<value_effects> effects;
                        };

                        struct fishing_time_reduction {
                            list_array<value_effects> effects;
                        };

                        struct fishing_luck_bonus {
                            list_array<value_effects> effects;
                        };

                        struct mob_experience {
                            list_array<value_effects> effects;
                        };

                        struct crossbow_charge_time {
                            list_array<value_effects> effects; //ignores predicate
                        };

                        struct trident_spin_attack_strength {
                            list_array<value_effects> effects; //ignores predicate
                        };

                        struct hit_block {
                            list_array<entity_effects> effects;
                        };

                        struct tick {
                            list_array<entity_effects> effects;
                        };

                        struct tick {
                            list_array<entity_effects> effects;
                        };

                        struct projectile_spawned {
                            list_array<entity_effects> effects;
                        };

                        struct post_attack {
                            list_array<entity_effects> effects; // uses affected and enchanted
                        };

                        struct location_changed {
                            list_array<entity_effects> effects; //allows attributes
                        };

                        struct damage_immunity {
                            //list_array<std::optional<entity_effects>> effects; //allows attributes
                        };
                    };

                    Chat description;
                    std::variant<std::string, std::vector<std::string>> exclusive_set;
                    std::variant<std::string, std::vector<std::string>> supported_items;
                    std::variant<std::string, std::vector<std::string>> primary_items;
                    std::vector<std::string> slots;

                    //std::unordered_map<std::string, effect> effects;

                    struct {
                        int32_t base;
                        int32_t per_level_above_first;
                    } min_cost;

                    struct {
                        int32_t base;
                        int32_t per_level_above_first;
                    } max_cost;

                    int32_t anvil_cost;
                    int32_t weight;
                    uint8_t max_level;
                };

                struct namespace_ {
                    tags_ tags;
                    loot_table_ loot_table;
                    banner_pattern_ banner_pattern;
                    damage_type_ damage_type;
                };
            }


        }
    }
}

#endif /* SRC_STORAGE_MEMORY_WHOLE_TINGS */
