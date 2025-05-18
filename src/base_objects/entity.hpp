#ifndef SRC_BASE_OBJECTS_ENTITY
#define SRC_BASE_OBJECTS_ENTITY
#include <chrono>
#include <filesystem>
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <src/base_objects/bounds.hpp>
#include <src/base_objects/entity/animation.hpp>
#include <src/base_objects/entity/event.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/slot.hpp>
#include <src/base_objects/world/block_action.hpp>
#include <src/util/calculations.hpp>
#include <stdint.h>

namespace copper_server {
    namespace storage {
        class world_data;
        class chunk_data;
    }

    namespace base_objects {
        struct SharedClientData;
        using client_data_holder = atomic_holder<SharedClientData>;
        struct entity;
        using entity_ref = atomic_holder<entity>;

        namespace world {
            struct sub_chunk_data;
            class storage;
        }


        union block;
        struct block_entity;
        struct block_entity_ref;
        struct const_block_entity_ref;

        struct entity_data {
            list_array<std::string> entity_aliases; //string entity ids(checks from first to last, if none found in `initialize_entities()` throws) implicitly uses id first
            std::string id;
            std::string name;
            std::string translation_resource_key;
            std::string spawn_group;

            bounding base_bounds;
            float eye_height;

            float acceleration; // block\tick
            float drag_vertical;
            float drag_horizontal;
            float terminal_velocity;

            int32_t max_track_distance = 0;
            int32_t track_tick_interval = 0;

            uint16_t entity_id;

            bool drag_applied_after_acceleration = false;
            bool is_summonable = false;
            bool is_fire_immune = false;
            bool is_saveable = false;
            bool is_spawnable_far_from_player = false;
            uint8_t loading_ticket_level = 50; //read about loading tickets, if loading ticket level is higher than 44 then ticket would not be created for this entity

            struct living_entity_data_t {
                int32_t inventory_size = 0;
                float base_health;
                uint16_t max_air = 300;
                bool can_avoid_traps = false;
                bool can_be_hit_by_projectile = true;
                bool can_freeze;
                bool can_hit;
                bool is_collidable;
                bool is_attackable;

                struct brain_task {
                    std::string name;
                    //TODO implement system
                };

                std::vector<brain_task> brain_tasks;
                std::vector<std::string> brain_sensors;
                std::vector<std::string> brain_memories;
            };

            std::optional<living_entity_data_t> living_entity_data;

            enbt::compound data;


            //"data"{
            //  "loot_table": {table...},
            //  "slot":{
            //      "size": array_size,
            //      "main_hand": id,
            //      "off_hand": id,
            //      "hand": [id, id]
            //      "head": id,
            //      "chest": id,
            //      "legs": id,
            //      "feet": id,
            //      "body": [id, id, id, id, ...],
            //      "storage": [id, id, id, id, ...]
            //  },
            //  "custom_data":{
            //      ...
            //  }
            //}


            std::function<void(entity& target_entity)> tick_callback;
            std::function<bool(entity& target_entity, bool force)> pre_death_callback;
            std::function<void(entity& target_entity)> create_callback;
            std::function<void(entity& target_entity, const enbt::compound_const_ref&)> create_callback_with_nbt;
            std::function<void(entity_ref& creating_entity, const enbt::compound_const_ref&)> create_from_enbt_callback;
            std::function<void(entity_ref& checking_entity, entity_data&, util::VECTOR pos)> check_bounds; //if nullptr then used base_bounds, return true if entity is in bounds
            std::function<int32_t(const entity& checking_entity)> get_object_field;                        //optional

            struct world_processor { //used to handle changes applied for entity and implement AI or send changes to client
                void (*entity_init)(entity& self, base_objects::entity_ref&);

                void (*entity_teleport)(entity& self, base_objects::entity_ref&, util::VECTOR new_pos);
                void (*entity_move)(entity& self, base_objects::entity_ref&, util::VECTOR move);
                void (*entity_look_changes)(entity& self, base_objects::entity_ref&, util::ANGLE_DEG new_rotation);
                void (*entity_rotation_changes)(entity& self, base_objects::entity_ref&, util::ANGLE_DEG new_rotation);
                void (*entity_motion_changes)(entity& self, base_objects::entity_ref&, util::VECTOR new_motion);

                void (*entity_rides)(entity& self, base_objects::entity_ref&, base_objects::entity_ref& other_entity_id);
                void (*entity_leaves_ride)(entity& self, base_objects::entity_ref&);

                void (*entity_attach)(entity& self, base_objects::entity_ref&, base_objects::entity_ref& other_entity_id);
                void (*entity_detach)(entity& self, base_objects::entity_ref&, base_objects::entity_ref& other_entity_id);

                void (*entity_attack)(entity& self, base_objects::entity_ref&, base_objects::entity_ref& other_entity_id);
                void (*entity_iteract)(entity& self, base_objects::entity_ref&, base_objects::entity_ref& other_entity_id);
                void (*entity_iteract_block)(entity& self, base_objects::entity_ref&, int64_t x, int64_t y, int64_t z);

                void (*entity_break)(entity& self, base_objects::entity_ref&, int64_t x, int64_t y, int64_t z, uint8_t state); //form 0 to 9, other ignored
                void (*entity_cancel_break)(entity& self, base_objects::entity_ref&, int64_t x, int64_t y, int64_t z);
                void (*entity_finish_break)(entity& self, base_objects::entity_ref&, int64_t x, int64_t y, int64_t z);
                void (*entity_place_block)(entity& self, base_objects::entity_ref&, int64_t x, int64_t y, int64_t z, const base_objects::block&);
                void (*entity_place_block_entity)(entity& self, base_objects::entity_ref&, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref);


                void (*entity_animation)(entity& self, base_objects::entity_ref&, base_objects::entity_animation animation);
                void (*entity_event)(entity& self, base_objects::entity_ref&, base_objects::entity_event status);

                void (*entity_death)(entity& self, base_objects::entity_ref&);
                void (*entity_deinit)(entity& self, base_objects::entity_ref&);


                void (*notify_block_event)(entity& self, const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z);
                void (*notify_block_change)(entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block);
                void (*notify_block_entity_change)(entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block);
                void (*notify_block_destroy_change)(entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block);
                void (*notify_block_entity_destroy_change)(entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block);
                void (*notify_biome_change)(entity& self, int64_t x, int64_t y, int64_t z, uint32_t);

                void (*notify_sub_chunk)(entity& self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const world::sub_chunk_data&); //used after multiply changes
                void (*notify_chunk)(entity& self, int64_t chunk_x, int64_t chunk_z, const storage::chunk_data&);                        //used after multiply changes

                void (*notify_sub_chunk_light)(entity& self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const world::sub_chunk_data&); //used after multiply changes
                void (*notify_chunk_light)(entity& self, int64_t chunk_x, int64_t chunk_z, const storage::chunk_data&);                        //used after multiply changes

                void (*notify_sub_chunk_blocks)(entity& self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const world::sub_chunk_data&); //used after multiply changes
                void (*notify_chunk_blocks)(entity& self, int64_t chunk_x, int64_t chunk_z, const storage::chunk_data&);                        //used after multiply changes

                void (*on_transfer)(entity& self, storage::world_data& old_world, storage::world_data& new_world);
            };

            std::shared_ptr<world_processor> processor;

            std::unordered_map<uint32_t, uint32_t> internal_entity_aliases; //protocol id -> entity id


            //entity can be added without reload, but it can be removed only by `reset_entities`
            //multi threaded
            static const entity_data& get_entity(uint16_t id);
            static const entity_data& get_entity(const std::string& id);
            static uint16_t register_entity(entity_data);
            static const entity_data& view(const entity& entity);
            //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
            static void reset_entities();      //INTERNAL
            static void initialize_entities(); //INTERNAL, used to assign internal_entity_aliases ids from entity_aliases


            static std::unordered_map<uint32_t, std::unordered_map<std::string, uint32_t>> internal_entity_aliases_protocol;
            static uint16_t player_entity_id;
        };

        struct entity {
            struct effect {
                uint32_t duration;
                uint32_t id;
                uint8_t amplifier : 8 = 1;
                bool ambient : 1 = false;
                bool particles : 1 = true;
            };

            struct world_syncing {
                bit_list_array<> processed_chunks;
                base_objects::cubic_bounds_chunk_radius processing_region;
                uint64_t assigned_world_id = -1;
                storage::world_data* world = nullptr;
                uint32_t attached_to_distance = 0;
                bool on_ground = true; //local calculation
            };

            enbt::raw_uuid id;
            enbt::compound nbt;
            enbt::compound server_data;
            std::unordered_map<uint32_t, slot_data> inventory;
            std::unordered_map<std::string, std::unordered_map<uint32_t, slot_data>> custom_inventory;
            std::optional<enbt::raw_uuid> ride_entity_id;
            std::optional<enbt::raw_uuid> attached_to; //this entity follows entity
            list_array<enbt::raw_uuid> attached;       //entities follows this entity


            std::unordered_map<uint32_t, list_array<effect>> hidden_effects; //effects with lower amplifier than active effect but longer duration
            std::unordered_map<uint32_t, effect> active_effects;

            util::VECTOR position;
            util::VECTOR motion;
            util::ANGLE_DEG head_rotation;
            util::ANGLE_DEG rotation;

            std::optional<world_syncing> world_syncing_data;
            list_array<client_data_holder> spectating_players;
            client_data_holder assigned_player;

            uint32_t protocol_id;

            storage::world_data* current_world() const;

            //nbt {
            //  float health = 20;
            //  uint8_t food = 20;
            //  float saturation = 5;
            //  float breath = 10;
            //  float experience = 0;
            //  int32_t fall_distance = 0;
            //  uint8_t selected_item = 0; //hotbar, 0..8
            //
            //  bool is_sleeping = false;
            //  bool on_ground = false;
            //  bool is_sprinting = false;
            //  bool is_sneaking = false;
            //
            //  calculated_values { //does not trigger `changes_event` event, calculated every tick
            //      float absorption_health;
            //      float max_health;
            //      float acceleration; //falling speed
            //      float block_reach;
            //      float attack_reach;
            //      float jump_height;
            //      float falling_damage;
            //      float walking_speed;
            //      float mining_attack_speed;
            //      float damage;
            //      float protection;
            //      float fire_protection;
            //      float hunger_consumption;
            //      float luck_level;
            //      float swimming_speed;
            //
            //      bool calculate_fall_damage;
            //      bool calculate_drowning;
            //      bool calculate_effect_infested;
            //  }
            //}

            bool kill();
            void force_kill();
            bool is_died();
            const entity_data& const_data();

            entity_ref copy() const;
            enbt::compound copy_to_enbt() const;

            void teleport(util::VECTOR pos, float yaw, float pitch, bool on_ground);
            void teleport(int32_t x, int32_t y, int32_t z, float yaw, float pitch, bool on_ground);


            void set_ride_entity(enbt::raw_uuid entity);
            void remove_ride_entity();

            void add_effect(uint32_t id, uint32_t duration, uint8_t amplifier = 1, bool ambient = false, bool show_particles = true);
            void remove_effect(uint32_t id);
            void remove_all_effects();

            bool is_sleeping() const;
            bool is_on_ground() const;
            bool is_sneaking() const;
            bool is_sprinting() const;

            void set_sleeping(bool sleeping);
            void set_on_ground(bool on_ground);
            void set_sneaking(bool sneaking);
            void set_sprinting(bool sprinting);

            float get_health() const;
            void set_health(float health);
            void add_health(float health);
            void damage(float health);
            void reduce_health(float health);

            uint8_t get_food() const;
            void set_food(uint8_t food);
            void add_food(uint8_t food);
            void reduce_food(uint8_t food);

            float get_saturation() const;
            void set_saturation(float saturation);
            void add_saturation(float saturation);
            void reduce_saturation(float saturation);

            float get_breath() const;
            void set_breath(float breath);
            void add_breath(float breath);
            void reduce_breath(float breath);

            float get_experience() const;
            void set_experience(float experience);
            void add_experience(float experience);
            void reduce_experience(float experience);

            int32_t get_fall_distance() const;
            void set_fall_distance(int32_t fall_distance);
            void add_fall_distance(int32_t fall_distance);
            void reduce_fall_distance(int32_t fall_distance);

            uint8_t get_selected_item() const;
            void set_selected_item(uint8_t selected_item);


            void set_position(util::VECTOR pos);
            void move(float side, float forward, bool jump = false, bool sneaking = false);
            void look(float yaw, float pitch);
            void look_at(float x, float y, float z);
            void look_at(util::VECTOR pos);

            //looks only if this and another entity registered to same world
            void look_at(const entity_ref& entity);
            void look_at(uint64_t world_entity_id);
            void look_at(enbt::raw_uuid id);

            util::VECTOR get_motion() const;
            void set_motion(util::VECTOR mot);
            void add_motion(util::VECTOR mot);

            util::ANGLE_DEG get_rotation() const;
            void set_rotation(util::ANGLE_DEG mot);
            void add_rotation(util::ANGLE_DEG mot);

            util::ANGLE_DEG get_head_rotation() const;
            void set_head_rotation(util::ANGLE_DEG rot);
            void add_head_rotation(util::ANGLE_DEG rot);

            //attack passes only if this and another entity registered to same world
            void attack_from_this(const entity_ref& entity);
            void attack_from_this(uint64_t world_entity_id);
            void attack_from_this(enbt::raw_uuid id);

            void breaking_block(int64_t global_x, uint64_t global_y, int64_t global_z, uint32_t time);
            void place_block(int64_t global_x, uint64_t global_y, int64_t global_z, const block&);
            void place_block(int64_t global_x, uint64_t global_y, int64_t global_z, base_objects::const_block_entity_ref);
            void place_block(int64_t global_x, uint64_t global_y, int64_t global_z, block_entity&&);

            static entity_ref create(uint16_t id);
            static entity_ref create(uint16_t id, const enbt::compound_const_ref& nbt);
            static entity_ref create(const std::string& id);
            static entity_ref create(const std::string& id, const enbt::compound_const_ref& nbt);
            static entity_ref load_from_enbt(const enbt::compound_const_ref& file_nbt);

            entity() {}

            entity(const entity& other) = delete;
            entity& operator=(const entity& other) = delete;

            entity(entity&& other) = delete;
            entity& operator=(entity&& other) = delete;

            void tick();
            std::optional<int32_t> get_object_field();


            bool is_player(); //returns true if entity_id belongs to player
        private:
            friend struct entity_data;
            uint16_t entity_id;
            bool died : 1 = false;
        };
    }
}


#endif /* SRC_BASE_OBJECTS_ENTITY */
