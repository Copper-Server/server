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

                struct namespace_ {
                    loot_table_ loot_table;
                };
            }


        }
    }
}

#endif /* SRC_STORAGE_MEMORY_WHOLE_TINGS */
