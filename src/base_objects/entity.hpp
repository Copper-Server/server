#ifndef SRC_BASE_OBJECTS_ENTITY
#define SRC_BASE_OBJECTS_ENTITY
#include "../calculations.hpp"
#include "../library/enbt.hpp"
#include "../registers.hpp"
#include "atomic_holder.hpp"
#include "float16.hpp"
#include <filesystem>
#include <fstream>
#include <stdint.h>

namespace crafted_craft {
    namespace base_objects {
        class entity_data;
        struct entity;
        using entity_ref = atomic_holder<entity>;


        class block;
    }

    namespace storage {
        class world_data;
    }

    namespace base_objects {


        class entity_data {
        public:
            enum class action : uint64_t {
                stay,
                eat,
                feed,
                attack,
                random_run,
                run,
                random_walk,
                walk,
                random_sneak_walk,
                sneak_walk,
                random_look,
                look,
                custom_call
            };

            //call as event
            struct entity_reaction {
                uint64_t can_aborted : 1;
                uint64_t action_priority : 8;
                uint64_t if_in_view : 1; //reaction_distance filter
                uint64_t if_hurt : 1;
                uint64_t if_hear : 1;
                uint64_t reaction_distance_as_radius : 1; //if 0 then reaction distance will be as cube
                uint64_t max_reaction_distance : 8;
                uint64_t min_reaction_distance : 8;
                uint64_t reaction_keep_alive_ticks : 16;
                action action : 4;
                //call next action if keep_alive_end and not aborted, from vector {reactions}
                uint64_t next_action : 15; //start from 1, 0 used for undefined
                //call this function if {action} equal {action::custom_call}, if nullptr, behavior will be equal {action::stay}
                std::function<void(entity& target_entity, entity& extern_entity, bool in_view, bool hurted, bool hear)> custom_call = nullptr;
            };

            list_array<entity_reaction> entity_reactions;

            struct block_reaction {
                uint64_t can_aborted : 1;
                uint64_t action_priority : 8;
                uint64_t if_in_view : 1; //reaction_distance filter
                uint64_t if_hurt : 1;
                uint64_t if_hear : 1;
                uint64_t reaction_distance_as_radius : 1; //if 0 then reaction distance will be as cube
                uint64_t max_reaction_distance : 8;
                uint64_t min_reaction_distance : 8;
                uint64_t reaction_keep_alive_ticks : 16;
                action action : 4;
                //call next action if keep_alive_end and not aborted, from vector {reactions}
                uint64_t next_action : 15; //start from 1, 0 used for undefined
                base_objects::block block;
                //call this function if {action} equal {action::custom_call}, if nullptr, behavior will be equal {action::stay}
                std::function<void(entity& target_entity, int64_t x, uint64_t y, int64_t z, const base_objects::block& block, bool in_view, bool hurted, bool hear)> custom_call = nullptr;
            };

            list_array<block_reaction> block_reactions;

            struct environment_reaction {
                uint64_t can_aborted : 1;
                uint64_t required_condution : 1;
                uint64_t in_next_required_condution : 1;
                uint64_t action_priority : 8;
                uint64_t reaction_keep_alive_ticks : 34;
                action action : 4;
                uint64_t next_action : 15; //start from 1, 0 used for undefined
                std::function<bool(entity& target_entity)> reaction_condution;
                //return false if keep alive is need aborted
                std::function<bool(entity& target_entity, uint8_t data_buffer[40])>* action_func = nullptr;
            };

            list_array<environment_reaction> environment_reactions;


            //entity
            void work_reaction(entity& target_entity, entity& extern_entity, bool in_view, bool hurted, bool hear);

            // block
            void work_reaction(entity& target_entity, int64_t x, uint64_t y, int64_t z, const base_objects::block& block, bool in_view, bool hurted, bool hear);

            // environment
            void work_reaction(entity& target_entity);

            void keep_reaction(entity& target_entity);

            entity_data() {
                sizeof(entity_reaction);
                sizeof(block_reaction);
                sizeof(environment_reaction);
                sizeof(hunger_exhaustion);
                sizeof(entity_data);
            }

            uint64_t can_despawn : 1;
            uint64_t idle_tick_to_despawn : 39;
            uint64_t send_client : 1;
            uint64_t reactions_enabled : 1;
            uint64_t invisible_for_events : 1;
            uint64_t invisible_for_selector : 1;
            uint64_t hunger_enabled : 1;
            uint64_t default_attack_damage : 16;
            uint64_t has_health : 1;
            uint64_t can_death : 1;
            uint64_t can_respawn : 1;
            float default_damage;
            float apply_action_on_hunger_above;

            struct {
                //all per tick
                float16_t default_hunger;
                float16_t walk;
                float16_t break_block;
                float16_t swim;
                float16_t jump;
                float16_t sprint;
                float16_t hunger_effect; //per level
                float16_t armor_protect; //used while entity got hit in armor and taken damage
                float16_t sprint_jump;
                float16_t per_regen;           //for natural regen
                float16_t attack;              //for future
                float16_t teleport;            //
                float16_t as_rider_distance;   //per block move in ridded entity
                float16_t as_vehicle_distance; //per block move as ridded entity
            } hunger_exhaustion;

            std::function<void(entity& target_entity, float current_hunger)> hunger_action;
            std::function<void(entity& target_entity)> death_action;
        };

        struct entity {
            enum class move_type {
                walk,
                sprint,
                sneak
            };
            ENBT nbt;
            calc::VECTOR position;
            calc::VECTOR rotation;
            calc::VECTOR head_rotation;
            calc::VECTOR motion;
            ENBT::UUID id;
            uint16_t entity_id;
            uint16_t keep_reaction;
            int32_t data; // varies
            storage::world_data* world = nullptr;
            uint8_t keep_reaction_data[20];
            bool died;

            void move(double new_x, double new_y, double new_z, move_type);
            void transfer(storage::world_data* new_world, double new_x, double new_y, double new_z);

            inline void move(calc::VECTOR new_pos, move_type move_type) {
                move(new_pos.x, new_pos.y, new_pos.z, move_type);
            }

            inline void transfer(storage::world_data* new_world, calc::VECTOR new_pos) {
                transfer(new_world, new_pos.x, new_pos.y, new_pos.z);
            }

            entity() {
                sizeof(entity);
            }

            entity(const entity& other) = delete;
            entity& operator=(const entity& other) = delete;

            entity(entity&& other) = delete;
            entity& operator=(entity&& other) = delete;

            entity_ref copy() const;
            ENBT copy_to_enbt() const;
            void tick();

            void kill();

            static entity_ref load_from_enbt(const ENBT& nbt);
        };
    }
}


#endif /* SRC_BASE_OBJECTS_ENTITY */
