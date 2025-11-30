#ifndef SRC_BASE_OBJECTS_BLOCK_ENTITY
#define SRC_BASE_OBJECTS_BLOCK_ENTITY
#include <atomic>
#include <src/api/id.hpp>
#include <src/base_objects/block.hpp>
#include <src/base_objects/uuid.hpp>

#include <src/base_objects/chat.hpp>
#include <src/util/calculations.hpp>
#include <src/util/nbt.hpp>

namespace copper_server::util {
    class nbt_read_stream;
    class nbt_write_stream;

    namespace nbt_collection {
        class compound_flex;
    }

    class nbt_write_compound_stream;
}

namespace copper_server::base_objects {
    namespace block_entity {
        namespace test_instance {
            struct error {
                util::xyz<int32_t> pos;
                chat text;
            };
        };

        namespace sculk_catalyst {
            struct listener {
                enum class facing_e {
                    down = 0,
                    up = 1,
                    north = 2,
                    south = 3,
                    west = 4,
                    east = 5,
                };

                int32_t charge;         //the vanilla limits it up to 1000, idk would it break backward compatibility if there would be value greater than 1000, TODO check
                list_array<double> pos; //x,y,z
                int32_t decay_delay = 0;
                int32_t update_delay = 0;
                list_array<facing_e> facings;
            };
        };

        struct mob_spawner_entry {
            struct rules_t {
                int32_t block_light_limit;
                int32_t sky_light_limit;
            };

            struct equipment_t {
                struct chances_t {
                    std::optional<float> feet;
                    std::optional<float> legs;
                    std::optional<float> chest;
                    std::optional<float> head;
                    std::optional<float> body;
                    std::optional<float> mainhand;
                    std::optional<float> offhand;
                };

                api::id::loot_table loot_table;
                std::optional<std::variant<float, chances_t>> slot_drop_chances;


                void from_nbt_base_data(util::nbt_collection::compound_flex& collector);
                void to_nbt_base_data(util::nbt_write_compound_stream& collector);
                void from_nbt(util::nbt_read_stream& stream);
                void to_nbt(util::nbt_write_stream& stream);
            };

            util::nbt entity;
            std::optional<rules_t> custom_spawn_rules;
            std::optional<equipment_t> equipment;
        };
    };

    struct viewer_count_manager {
        std::atomic_uint32_t users;

        uint32_t use() {
            return ++users;
        }

        void unuse() {
            --users;
        }

        uint32_t current() {
            return users;
        }
    };

    struct vibration_listener {
        struct event_t {
            float distance;
            api::id::game_event game_event;
            list_array<double> pos; //x,y,z
            std::optional<base_objects::uuid> projectile_owner;
            std::optional<base_objects::uuid> source;
        };

        struct selector_t {
            int64_t tick;
            event_t event;
        };

        std::optional<event_t> event;
        int32_t event_delay = 0;
        selector_t selector;
    };
}

#endif /* SRC_BASE_OBJECTS_BLOCK_ENTITY */
