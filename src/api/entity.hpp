/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_ENTITY
#define SRC_API_ENTITY
#include <chrono>
#include <filesystem>
#include <library/list_array.hpp>
#include <memory>
#include <src/api/ecs.hpp>
#include <src/base_objects/bounds.hpp>
#include <src/base_objects/entity/animation.hpp>
#include <src/base_objects/entity/event.hpp>
#include <src/base_objects/entity/metadata.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/weather.hpp>
#include <src/base_objects/world/block_action.hpp>
#include <src/util/calculations.hpp>
#include <src/util/nbt.hpp>
#include <stdint.h>

namespace copper_server {
    namespace storage {
        class world_data;
    }

    namespace base_objects {
        struct shared_client_data;
        using client_data_holder = std::shared_ptr<shared_client_data>;

        namespace world {
            struct sub_chunk_data;
            struct chunk_data;
        }

        struct block;
        struct block_entity;
    }
}

namespace copper_server::api {
    struct entity_data {
        list_array<std::string> entity_aliases; //string entity ids(checks from first to last, if none found in `initialize_entities()` throws) implicitly uses id first
        std::string id;
        std::string name;
        std::string translation_resource_key;
        std::string spawn_group;

        base_objects::bounding base_bounds{0.0, 0.0};
        float eye_height = 0.0f;

        float acceleration = 0.0f; // block\tick
        float drag_vertical = 0.0f;
        float drag_horizontal = 0.0f;
        float terminal_velocity = 0.0f;

        int32_t max_track_distance = 0;
        int32_t track_tick_interval = 0;

        int32_t entity_id = 0;

        bool drag_applied_after_acceleration = false;
        bool is_summonable = false;
        bool is_fire_immune = false;
        bool is_saveable = false;
        bool is_spawnable_far_from_player = false;
        int8_t loading_ticket_level = 50; //read about loading tickets, if loading ticket level is higher than 44 then ticket would not be created for this entity

        struct living_entity_data_t {
            int32_t inventory_size = 0;
            float base_health = 0.0f;
            float step_height = 0.0f;
            uint16_t max_air = 300;
            bool can_avoid_traps = false;
            bool can_be_hit_by_projectile = true;
            bool can_freeze = false;
            bool can_hit = false;
            bool is_collidable = false;
            bool is_attackable = false;

            struct brain_task {
                std::string name;
                //TODO implement system
            };

            std::vector<brain_task> brain_tasks;
            std::vector<std::string> brain_sensors;
            std::vector<std::string> brain_memories;
        };

        struct spawn_restriction_t {
            enum class location_e {
                in_lava,
                in_water,
                on_ground,
                unrestricted,
            } location
                = location_e::unrestricted;
            enum class heightmap_e {
                surface,
                ocean_floor,
                motion_blocking,
                motion_blocking_no_leaves,
            } heightmap
                = heightmap_e::motion_blocking_no_leaves;
        } spawn_restriction;

        std::optional<living_entity_data_t> living_entity_data;
        std::optional<int32_t> spawn_egg;

        util::nbt_compound data;

        std::function<bool(ecs::entity target_entity, bool force)> pre_death_callback;
        std::function<void(ecs::entity target_entity)> create_callback;
        std::function<void(ecs::entity target_entity)> load_callback;
        std::function<void(ecs::entity checking_entity, entity_data&, util::vector pos)> check_bounds; //if nullptr then used base_bounds, return true if entity is in bounds
        std::function<int32_t(ecs::entity checking_entity)> get_object_field;                          //optional

        struct world_processor { //used to handle changes applied for entity and implement AI or send changes to client
            void (*entity_init)(ecs::entity self, ecs::entity) = nullptr;

            void (*entity_teleport)(ecs::entity self, ecs::entity, util::vector new_pos) = nullptr;
            void (*entity_move)(ecs::entity self, ecs::entity, util::vector move) = nullptr;
            void (*entity_look_changes)(ecs::entity self, ecs::entity, util::angle_deg new_rotation) = nullptr;
            void (*entity_rotation_changes)(ecs::entity self, ecs::entity, util::angle_deg new_rotation) = nullptr;
            void (*entity_motion_changes)(ecs::entity self, ecs::entity, util::vector new_motion) = nullptr;

            void (*entity_rides)(ecs::entity self, ecs::entity, ecs::entity other_entity_id) = nullptr;
            void (*entity_leaves_ride)(ecs::entity self, ecs::entity, ecs::entity other_entity_id) = nullptr;

            void (*entity_attach)(ecs::entity self, ecs::entity, ecs::entity other_entity_id) = nullptr;
            void (*entity_detach)(ecs::entity self, ecs::entity, ecs::entity other_entity_id) = nullptr;

            void (*entity_damage)(ecs::entity self, ecs::entity, float health, int32_t type_id, const std::optional<util::vector>& pos) = nullptr;
            void (*entity_damage_with_source)(ecs::entity self, ecs::entity, float health, int32_t type_id, std::optional<ecs::entity> source, const std::optional<util::vector>& pos) = nullptr;
            void (*entity_damage_with_sources)(ecs::entity self, ecs::entity, float health, int32_t type_id, std::optional<ecs::entity> source, std::optional<ecs::entity> source_direct, const std::optional<util::vector>& pos) = nullptr;

            void (*entity_attack)(ecs::entity self, ecs::entity, ecs::entity other_entity_id) = nullptr;
            void (*entity_iteract)(ecs::entity self, ecs::entity, ecs::entity other_entity_id) = nullptr;
            void (*entity_iteract_block)(ecs::entity self, ecs::entity, int64_t x, int64_t y, int64_t z) = nullptr;

            void (*entity_break)(ecs::entity self, ecs::entity, int64_t x, int64_t y, int64_t z, uint8_t state) = nullptr; //form 0 to 9, other ignored
            void (*entity_cancel_break)(ecs::entity self, ecs::entity, int64_t x, int64_t y, int64_t z) = nullptr;
            void (*entity_finish_break)(ecs::entity self, ecs::entity, int64_t x, int64_t y, int64_t z) = nullptr;
            void (*entity_place_block)(ecs::entity self, ecs::entity, bool is_main_hand, int64_t x, int64_t y, int64_t z, const base_objects::block&) = nullptr;
            void (*entity_place_block_entity)(ecs::entity self, ecs::entity, bool is_main_hand, int64_t x, int64_t y, int64_t z, ecs::entity) = nullptr;


            void (*entity_animation)(ecs::entity self, ecs::entity, base_objects::entity_animation animation) = nullptr;
            void (*entity_event)(ecs::entity self, ecs::entity, base_objects::entity_event status) = nullptr;

            void (*entity_add_effect)(ecs::entity self, ecs::entity, uint32_t id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) = nullptr;
            void (*entity_remove_effect)(ecs::entity self, ecs::entity, uint32_t id) = nullptr;

            void (*entity_death)(ecs::entity self, ecs::entity) = nullptr;
            void (*entity_deinit)(ecs::entity self, ecs::entity) = nullptr;


            void (*notify_block_event)(ecs::entity self, const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z) = nullptr;
            void (*notify_block_change)(ecs::entity self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) = nullptr;
            void (*notify_block_entity_change)(ecs::entity self, int64_t x, int64_t y, int64_t z, ecs::entity) = nullptr;
            void (*notify_block_destroy_change)(ecs::entity self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) = nullptr;
            void (*notify_block_entity_destroy_change)(ecs::entity self, int64_t x, int64_t y, int64_t z, ecs::entity) = nullptr;
            void (*notify_biome_change)(ecs::entity self, int64_t x, int64_t y, int64_t z, uint32_t) = nullptr;

            void (*notify_sub_chunk)(ecs::entity self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const base_objects::world::sub_chunk_data&) = nullptr; //used after multiply changes
            void (*notify_chunk)(ecs::entity self, int64_t chunk_x, int64_t chunk_z, const base_objects::world::chunk_data&) = nullptr;                          //used after multiply changes

            void (*notify_sub_chunk_light)(ecs::entity self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const base_objects::world::sub_chunk_data&) = nullptr; //used after multiply changes
            void (*notify_chunk_light)(ecs::entity self, int64_t chunk_x, int64_t chunk_z, const base_objects::world::chunk_data&) = nullptr;                          //used after multiply changes

            void (*notify_sub_chunk_blocks)(ecs::entity self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const base_objects::world::sub_chunk_data&) = nullptr; //used after multiply changes
            void (*notify_chunk_blocks)(ecs::entity self, int64_t chunk_x, int64_t chunk_z, const base_objects::world::chunk_data&) = nullptr;                          //used after multiply changes

            void (*on_change_world)(ecs::entity self, storage::world_data& new_world) = nullptr;

            void (*on_tick)(ecs::entity self) = nullptr;

            void (*sync_time)(ecs::entity self, uint32_t time, int64_t day_time) = nullptr;
            void (*weather_change)(ecs::entity self, uint32_t weather_time, base_objects::weather) = nullptr;
        };

        std::shared_ptr<world_processor> processor;

        std::vector<double> eye_height_in_each_pose;


        ecs::entity_recipe recipe;

        //entity can be added without reload, but it could be removed only by `reset_entities`
        //multi threaded
        static const entity_data& get_entity(int32_t id);
        static const entity_data& get_entity(const std::string& id);
        static list_array<int32_t> get_entity_ids();
        static int32_t register_entity(entity_data);
        static const entity_data& view(ecs::entity entity);
        static void register_entity_world_processor(std::shared_ptr<world_processor> processor, const std::string& id);

        //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
        static void reset_entities();      //INTERNAL
        static void initialize_entities(); //INTERNAL, used to assign processors


        static entity_data& initialization_get(int32_t id);
        static int32_t player_entity_id;
    };

    struct entity {
        ecs::entity handle;

        storage::world_data* current_world() const; 
        const entity_data& const_data() const;

        base_objects::entity_metadata::entity_pose get_pose() const;
        void set_pose(base_objects::entity_metadata::entity_pose pose);
        double eye_height() const;

        bool kill();
        void force_kill();
        void erase(); //same as force_kill but without animation and other handling(pre_death_callback not called)
        bool is_died() const;
        bool is_player() const;

        std::optional<ecs::entity> copy() const;


        int32_t get_protocol_id() const;
        util::vector get_position() const;


        void moved(util::vector pos);
        void moved(util::vector pos, float yaw, float pitch);
        void moved(util::vector pos, float yaw, float pitch, bool on_ground);

        void rotated(float yaw, float pitch);
        void rotated(float yaw, float pitch, bool on_ground);


        void teleport(util::vector pos);
        void teleport(util::vector pos, float yaw, float pitch);
        void teleport(util::vector pos, float yaw, float pitch, bool on_ground);

        void set_ride_entity(ecs::entity entity);
        void remove_ride_entity();

        void add_effect(uint32_t id, uint32_t duration, uint8_t amplifier = 1, bool ambient = false, bool show_particles = true, bool show_icon = true, bool use_blend = false);
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
        void reduce_health(float health);
        void damage(float health, int32_t type_id, std::optional<util::vector> pos);
        void damage(float health, int32_t type_id, std::optional<ecs::entity> source, std::optional<util::vector> pos);
        void damage(float health, int32_t type_id, std::optional<ecs::entity> source, std::optional<ecs::entity> source_direct, std::optional<util::vector> pos);

        int32_t get_food() const;
        void set_food(int32_t food);
        void add_food(int32_t food);
        void reduce_food(int32_t food);

        float get_saturation() const;
        void set_saturation(float saturation);
        void add_saturation(float saturation);
        void reduce_saturation(float saturation);

        int32_t get_breath() const;
        void set_breath(int32_t breath);
        void add_breath(int32_t breath);
        void reduce_breath(int32_t breath);

        int32_t get_level() const;
        void set_level(int32_t level);
        void add_level(int32_t level);
        void reduce_level(int32_t level);

        int32_t get_experience() const;
        void set_experience(int32_t experience);
        void add_experience(int32_t experience);
        void reduce_experience(int32_t experience);

        int32_t get_fall_distance() const;
        void set_fall_distance(int32_t fall_distance);

        uint8_t get_selected_item() const;
        void set_selected_item(uint8_t selected_item);

        void move(float side, float forward, bool jump = false, bool sneaking = false);
        void look(float yaw, float pitch);
        void look_at(float x, float y, float z);
        void look_at(util::vector pos);

        //looks only if this and another entity registered to same world
        void look_at(ecs::entity entity);

        util::vector get_motion() const;
        void set_motion(util::vector mot);
        void add_motion(util::vector mot);

        util::angle_deg get_rotation() const;
        void set_rotation(util::angle_deg mot);
        void add_rotation(util::angle_deg mot);

        util::angle_deg get_head_rotation() const;
        void set_head_rotation(util::angle_deg rot);
        void add_head_rotation(util::angle_deg rot);

        //attack passes only if this and another entity registered to same world
        void attack_from_this(ecs::entity entity);

        void breaking_block(int64_t global_x, uint64_t global_y, int64_t global_z, uint32_t time);
        void place_block(int64_t global_x, uint64_t global_y, int64_t global_z, const base_objects::block&);
        void place_block(int64_t global_x, uint64_t global_y, int64_t global_z, ecs::entity block_entity);

        static ecs::entity create(int32_t id);
        static ecs::entity create(int32_t id, const util::nbt_compound& nbt);
        static ecs::entity create(const std::string& id);
        static ecs::entity create(const std::string& id, const util::nbt_compound& nbt);

        bool hitboxes_touching_x(double min, double max);
        bool hitboxes_touching_y(double min, double max);
        bool hitboxes_touching_z(double min, double max);

        std::optional<int32_t> get_object_field() const;

        static void store_to_file(ecs::entity entity, util::nbt_write_stream& w);
        static ecs::entity load_from_file(util::nbt_read_stream& w);
        static void store_to_nbt(ecs::entity entity, util::nbt_compound& w);
        static ecs::entity load_from_nbt(const util::nbt_compound& nbt);

        void tick();
    };
}

#endif /* SRC_API_ENTITY */
