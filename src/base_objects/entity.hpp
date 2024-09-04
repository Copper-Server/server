#ifndef SRC_BASE_OBJECTS_ENTITY
#define SRC_BASE_OBJECTS_ENTITY
#include "../calculations.hpp"
#include "../library/enbt.hpp"
#include "../registers.hpp"
#include "atomic_holder.hpp"
#include "bounds.hpp"
#include "shared_client_data.hpp"
#include "sync_event.hpp"
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


        struct entity_data {
            list_array<std::string> entity_aliases; //string entity ids(checks from first to last, if none found in `initialize_entities()` throws)
            std::string id;
            std::string name;
            std::string translation_resource_key;
            std::string kind_name;
            bounding base_bounds;
            float base_health; //NaN == infinity
            enum class bounds_mode_t {
                solid,
                solid_except_self_type,
                solid_for_vehicles,
                none
            } bounds_mode;
            bool living_entity;
            float acceleration; // block\tick


            std::function<void(entity& target_entity)> tick_callback;
            std::function<bool(entity& target_entity, bool force)> pre_death_callback;
            std::function<entity_ref()> create_callback;
            std::function<entity_ref(const enbt::compound_ref&)> create_callback_with_nbt;
            std::function<void(entity_ref& creating_entity, const enbt::compound_ref&)> create_from_enbt_callback;
            std::function<void(entity_ref& checking_entity, entity_data&, calc::VECTOR pos)> check_bounds; //if nullptr then used base_bounds, return true if entity is in bounds


            std::unordered_map<uint32_t, uint32_t> internal_entity_aliases; //protocol id -> entity id


            //entity can be added without reload, but it can be removed only by `reset_entities`
            //multi threaded
            static const entity_data& get_entity(uint16_t id);
            static uint16_t register_entity(entity_data);
            static const entity_data& view(const entity& entity);
            //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
            static void reset_entities();      //INTERNAL
            static void initialize_entities(); //INTERNAL, used to assign internal_entity_aliases ids from entity_aliases


            static std::unordered_map<uint32_t, std::unordered_map<std::string, uint32_t>> internal_entity_aliases_protocol;
        };

        struct entity {
            ENBT::UUID id;
            ENBT nbt;
            ENBT server_data;
            calc::VECTOR position;
            calc::VECTOR motion;
            calc::VECTOR rotation;
            calc::VECTOR head_rotation;
            client_data_holder assigned_player = nullptr; //if not nullptr, player controls this entity. Entity can be player_entity or any other like in bedrock edition. No need for subscription to changes_event, everything handled automaticaly
            storage::world_data* world = nullptr;


            sync_event<entity&> changes_event;
            bool kill();
            void force_kill();
            bool is_died();
            const entity_data& const_data();

            entity_ref copy() const;
            ENBT copy_to_enbt() const;


            static entity_ref create(uint16_t id);
            static entity_ref create(uint16_t id, const enbt::compound_ref& nbt);
            static entity_ref load_from_enbt(const enbt::compound_ref& file_nbt);

            entity() {}

            entity(const entity& other) = delete;
            entity& operator=(const entity& other) = delete;

            entity(entity&& other) = delete;
            entity& operator=(entity&& other) = delete;


            void tick();

        private:
            friend struct entity_data;
            uint16_t entity_id;
            bool died;
        };
    }
}


#endif /* SRC_BASE_OBJECTS_ENTITY */
