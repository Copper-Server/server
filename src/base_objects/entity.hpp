#ifndef SRC_BASE_OBJECTS_ENTITY
#define SRC_BASE_OBJECTS_ENTITY
#include "../calculations.hpp"
#include "../chunk_core.hpp"
#include "../library/enbt.hpp"
#include "../registers.hpp"
#include "float16.hpp"
#include <filesystem>
#include <fstream>
#include <stdint.h>

namespace crafted_craft {


    class Entity_data {
    public:
        enum class Action : uint64_t {
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
        struct EntityReaction {
            uint64_t can_aborted : 1;
            uint64_t action_priority : 8;
            uint64_t if_in_view : 1; //reaction_distance filter
            uint64_t if_hurted : 1;
            uint64_t if_hear : 1;
            uint64_t reaction_distance_as_radius : 1; //if 0 then reaction distance will be as cube
            uint64_t max_reaction_distance : 8;
            uint64_t min_reaction_distance : 8;
            uint64_t reaction_keep_alive_ticks : 16;
            Action action : 4;
            //call next action if keep_alive_end and not aborted, from vector {reactions}
            uint64_t next_action : 15; //start from 1, 0 used for UNDEFINED
            //call this function if {action} equal {Action::custom_call}, if nullptr, behavior will be equal {Action::stay}
            std::function<void(class Entity& target_entity, class Entity& extern_entity, bool in_view, bool hurted, bool hear)>* custom_call = nullptr;
        };

        list_array<EntityReaction> entity_reacions;

        struct BlockReaction {
            uint64_t can_aborted : 1;
            uint64_t action_priority : 8;
            uint64_t if_in_view : 1; //reaction_distance filter
            uint64_t if_hurted : 1;
            uint64_t if_hear : 1;
            uint64_t reaction_distance_as_radius : 1; //if 0 then reaction distance will be as cube
            uint64_t max_reaction_distance : 8;
            uint64_t min_reaction_distance : 8;
            uint64_t reaction_keep_alive_ticks : 16;
            Action action : 4;
            //call next action if keep_alive_end and not aborted, from vector {reactions}
            uint64_t next_action : 15; //start from 1, 0 used for UNDEFINED
            //call this function if {action} equal {Action::custom_call}, if nullptr, behavior will be equal {Action::stay}
            std::function<void(class Entity& target_entity, block_pos_t x, uint16_t y, block_pos_t z, const Block& block, bool in_view, bool hurted, bool hear)>* custom_call = nullptr;
        };

        list_array<BlockReaction> block_reacions;

        struct EnvironmentReaction {
            uint64_t can_aborted : 1;
            uint64_t required_condution : 1;
            uint64_t in_next_required_condution : 1;
            uint64_t action_priority : 8;
            uint64_t reaction_keep_alive_ticks : 34;
            Action action : 4;
            uint64_t next_action : 15; //start from 1, 0 used for UNDEFINED
            std::function<bool(class Entity& target_entity)> reaction_condution;
            //return false if keep alive is need aborted
            std::function<bool(class Entity& target_entity, uint8_t data_buffer[40])>* action_func = nullptr;
        };

        list_array<EnvironmentReaction> enviropement_reactions;


        //entity
        void WorkReaction(class Entity& target_entity, class Entity& extern_entity, bool in_view, bool hurted, bool hear);

        // block
        void WorkReaction(class Entity& target_entity, block_pos_t x, uint16_t y, block_pos_t z, const Block& block, bool in_view, bool hurted, bool hear);

        // environment
        void WorkReaction(class Entity& target_entity);

        void KeepReaction(class Entity& target_entity);

        Entity_data() {
            sizeof(EntityReaction);
            sizeof(BlockReaction);
            sizeof(EnvironmentReaction);
            sizeof(hunger_exhaustion);
            sizeof(Entity_data);
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
        float def_damage;

        struct {
            //all per tick
            float16_t def;
            float16_t walk;
            float16_t break_block;
            float16_t swim;
            float16_t jump;
            float16_t sprint;
            float16_t hunger_effect; //per level
            float16_t armor_protect; //used while entity got hit in armor and taken damage
            float16_t sprint_jump;
            float16_t per_rengen;    //for natural rengen
            float16_t attack;        //for future
            float16_t teleport;      //
            float16_t ride_distance; //per block move in rided entity
        } hunger_exhaustion;

        std::function<void(class Entity& target_entity, float current_hunger)> hunger_action;
    };

    struct Entity {
        enum class MoveType {
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
        WorldClusters* world;
        uint8_t keep_reaction_data[20];
        bool Save(std::filesystem::path path);
        bool Load(std::filesystem::path path);
        bool Remove(WorldClusters& world, std::filesystem::path path);
        void Move(double new_x, double new_y, double new_z, MoveType);

        inline void Move(calc::VECTOR new_pos, MoveType move_type) {
            Move(new_pos.x, new_pos.y, new_pos.z, move_type);
        }

        Entity() {
            sizeof(Entity);
        }
    };
}


#endif /* SRC_BASE_OBJECTS_ENTITY */
