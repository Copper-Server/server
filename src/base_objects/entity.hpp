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


        struct block;
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
            int8_t loading_ticket_level = 50; //read about loading tickets, if loading ticket level is higher than 44 then ticket would not be created for this entity

            struct living_entity_data_t {
                int32_t inventory_size = 0;
                float base_health;
                float step_height;
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


            std::function<bool(entity& target_entity, bool force)> pre_death_callback;
            std::function<void(entity& target_entity)> create_callback;
            std::function<void(entity& target_entity, const enbt::compound_const_ref&)> create_callback_with_nbt;
            std::function<void(entity_ref& creating_entity, const enbt::compound_const_ref&)> create_from_enbt_callback;
            std::function<void(entity_ref& checking_entity, entity_data&, util::VECTOR pos)> check_bounds; //if nullptr then used base_bounds, return true if entity is in bounds
            std::function<int32_t(const entity& checking_entity)> get_object_field;                        //optional

            struct world_processor { //used to handle changes applied for entity and implement AI or send changes to client
                void (*entity_init)(entity& self, entity&) = nullptr;

                void (*entity_teleport)(entity& self, entity&, util::VECTOR new_pos) = nullptr;
                void (*entity_move)(entity& self, entity&, util::VECTOR move) = nullptr;
                void (*entity_look_changes)(entity& self, entity&, util::ANGLE_DEG new_rotation) = nullptr;
                void (*entity_rotation_changes)(entity& self, entity&, util::ANGLE_DEG new_rotation) = nullptr;
                void (*entity_motion_changes)(entity& self, entity&, util::VECTOR new_motion) = nullptr;

                void (*entity_rides)(entity& self, entity&, base_objects::entity_ref& other_entity_id) = nullptr;
                void (*entity_leaves_ride)(entity& self, entity&, base_objects::entity_ref& other_entity_id) = nullptr;

                void (*entity_attach)(entity& self, entity&, base_objects::entity_ref& other_entity_id) = nullptr;
                void (*entity_detach)(entity& self, entity&, base_objects::entity_ref& other_entity_id) = nullptr;

                void (*entity_damage)(entity& self, entity&, float health, int32_t type_id, const std::optional<util::VECTOR>& pos);
                void (*entity_damage_with_source)(entity& self, entity&, float health, int32_t type_id, base_objects::entity_ref& source, const std::optional<util::VECTOR>& pos);
                void (*entity_damage_with_sources)(entity& self, entity&, float health, int32_t type_id, base_objects::entity_ref& source, base_objects::entity_ref& source_direct, const std::optional<util::VECTOR>& pos);

                void (*entity_attack)(entity& self, entity&, base_objects::entity_ref& other_entity_id) = nullptr;
                void (*entity_iteract)(entity& self, entity&, base_objects::entity_ref& other_entity_id) = nullptr;
                void (*entity_iteract_block)(entity& self, entity&, int64_t x, int64_t y, int64_t z) = nullptr;

                void (*entity_break)(entity& self, entity&, int64_t x, int64_t y, int64_t z, uint8_t state) = nullptr; //form 0 to 9, other ignored
                void (*entity_cancel_break)(entity& self, entity&, int64_t x, int64_t y, int64_t z) = nullptr;
                void (*entity_finish_break)(entity& self, entity&, int64_t x, int64_t y, int64_t z) = nullptr;
                void (*entity_place_block)(entity& self, entity&, bool is_main_hand, int64_t x, int64_t y, int64_t z, const base_objects::block&) = nullptr;
                void (*entity_place_block_entity)(entity& self, entity&, bool is_main_hand, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref) = nullptr;


                void (*entity_animation)(entity& self, entity&, base_objects::entity_animation animation) = nullptr;
                void (*entity_event)(entity& self, entity&, base_objects::entity_event status) = nullptr;

                void (*entity_add_effect)(entity& self, entity&, uint32_t id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) = nullptr;
                void (*entity_remove_effect)(entity& self, entity&, uint32_t id) = nullptr;

                void (*entity_death)(entity& self, entity&) = nullptr;
                void (*entity_deinit)(entity& self, entity&) = nullptr;


                void (*notify_block_event)(entity& self, const base_objects::world::block_action& action, int64_t x, int64_t y, int64_t z) = nullptr;
                void (*notify_block_change)(entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) = nullptr;
                void (*notify_block_entity_change)(entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block) = nullptr;
                void (*notify_block_destroy_change)(entity& self, int64_t x, int64_t y, int64_t z, const base_objects::block& block) = nullptr;
                void (*notify_block_entity_destroy_change)(entity& self, int64_t x, int64_t y, int64_t z, base_objects::const_block_entity_ref block) = nullptr;
                void (*notify_biome_change)(entity& self, int64_t x, int64_t y, int64_t z, uint32_t) = nullptr;

                void (*notify_sub_chunk)(entity& self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const world::sub_chunk_data&) = nullptr; //used after multiply changes
                void (*notify_chunk)(entity& self, int64_t chunk_x, int64_t chunk_z, const storage::chunk_data&) = nullptr;                        //used after multiply changes

                void (*notify_sub_chunk_light)(entity& self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const world::sub_chunk_data&) = nullptr; //used after multiply changes
                void (*notify_chunk_light)(entity& self, int64_t chunk_x, int64_t chunk_z, const storage::chunk_data&) = nullptr;                        //used after multiply changes

                void (*notify_sub_chunk_blocks)(entity& self, int64_t chunk_x, int64_t chunk_y, int64_t chunk_z, const world::sub_chunk_data&) = nullptr; //used after multiply changes
                void (*notify_chunk_blocks)(entity& self, int64_t chunk_x, int64_t chunk_z, const storage::chunk_data&) = nullptr;                        //used after multiply changes

                void (*on_change_world)(entity& self, storage::world_data& new_world) = nullptr;

                void (*on_tick)(entity& self) = nullptr;
            };

            std::shared_ptr<world_processor> processor;

            struct metadata_sync {
                uint8_t index;

                enum class type_t : uint8_t {
                    byte,
                    varint,
                    varlong,
                    _float,
                    _string,
                    text,
                    opt_text,
                    slot,
                    boolean,
                    euler_angle,
                    postion,
                    opt_position,
                    direction,
                    entity_refrence,
                    block_state,
                    opt_block_state,
                    nbt,
                    particle,
                    particles,
                    villager_data,
                    opt_varint,
                    pose,
                    cat_variant,
                    cow_variant,
                    wolf_variant,
                    wolf_sound_variant,
                    frog_variant,
                    pig_variant,
                    chicken_variant,
                    global_pos,
                    painting_variant,
                    sniffer_state,
                    armadillo_state,
                    vector3,
                    quaternion,
                };
                type_t type;
            };

            std::unordered_map<std::string, metadata_sync> metadata;


            //entity can be added without reload, but it could be removed only by `reset_entities`
            //multi threaded
            static const entity_data& get_entity(uint16_t id);
            static const entity_data& get_entity(const std::string& id);
            static uint16_t register_entity(entity_data);
            static const entity_data& view(const entity& entity);
            static void register_entity_world_processor(std::shared_ptr<world_processor> processor, const std::string& id);

            //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
            static void reset_entities();      //INTERNAL
            static void initialize_entities(); //INTERNAL, used to assign processors


            static uint16_t player_entity_id;
        };

        struct entity {
            struct effect {
                uint32_t duration;
                uint32_t id;
                uint8_t amplifier : 8 = 1;
                bool ambient : 1 = false;
                bool particles : 1 = true;
                bool show_icon : 1 = true;
                bool use_blend : 1 = false; //for darkness
            };

            struct world_syncing {
                bit_list_array<> processed_chunks;
                base_objects::cubic_bounds_chunk_radius processing_region;
                uint64_t assigned_world_id = (uint64_t)-1;
                storage::world_data* world = nullptr;
                uint32_t attached_to_distance = 0;
                uint16_t keep_alive_ticks = 0; //used for handling entity animation
                bool on_ground : 1 = true;
                bool is_sleeping : 1 = false;
                bool is_sneaking : 1 = false;
                bool is_sprinting : 1 = false;

                bool mark_chunk(int64_t pos_x, int64_t pos_z, bool loaded) {
                    if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                        return false;
                    if (!processing_region.in_bounds(pos_x, pos_z))
                        return false;

                    int64_t offset_x = pos_x - (processing_region.center_x - processing_region.radius);
                    int64_t offset_z = pos_z - (processing_region.center_z - processing_region.radius);
                    size_t index = offset_z * (processing_region.radius + processing_region.radius + 1) + offset_x;
                    processed_chunks.set(index, loaded);
                    return true;
                }

                bool chunk_in_bounds(int64_t pos_x, int64_t pos_z) {
                    if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                        return false;
                    return processing_region.in_bounds(pos_x, pos_z);
                }

                bool chunk_processed(int64_t pos_x, int64_t pos_z) const {
                    if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                        return false;

                    if (!processing_region.in_bounds(pos_x, pos_z))
                        return false;
                    uint64_t offset_x = pos_x - (processing_region.center_x - processing_region.radius);
                    uint64_t offset_z = pos_z - (processing_region.center_z - processing_region.radius);


                    size_t index = offset_z * (processing_region.radius + processing_region.radius + 1) + offset_x;
                    return processed_chunks.at(index);
                }

                void update_processing(int32_t center_x, int32_t center_z, uint8_t render_distance) {
                    auto new_processing_diameter = 2 * render_distance + 7;
                    bit_list_array<> new_processing_data(new_processing_diameter * new_processing_diameter);

                    auto processing_diameter = (processing_region.radius + processing_region.radius + 1);
                    int32_t new_radius = new_processing_diameter / 2;

                    for (int32_t dz = 0; dz < new_processing_diameter; ++dz) {
                        for (int32_t dx = 0; dx < new_processing_diameter; ++dx) {
                            // World coordinates for this chunk in the new area
                            int32_t chunk_x = center_x - new_radius + dx;
                            int32_t chunk_z = center_z - new_radius + dz;

                            // Map to old area offsets
                            int64_t old_offset_x = chunk_x - (processing_region.center_x - processing_region.radius);
                            int64_t old_offset_z = chunk_z - (processing_region.center_z - processing_region.radius);

                            // If the chunk was loaded in the old area, copy its bit
                            if (old_offset_x >= 0 && old_offset_x < processing_diameter && old_offset_z >= 0 && old_offset_z < processing_diameter) {
                                size_t old_index = old_offset_z * processing_diameter + old_offset_x;
                                size_t new_index = dz * new_processing_diameter + dx;
                                if (processed_chunks[old_index])
                                    new_processing_data.set(new_index, true);
                            }
                        }
                    }
                    processing_region = {center_x, center_z, new_radius};
                    processed_chunks = std::move(new_processing_data);
                }

                void flush_processing() {
                    auto diameter = processing_region.radius + processing_region.radius + 1;
                    processed_chunks = bit_list_array<>(diameter * diameter);
                }

                template <class FN>
                void for_each_processing(FN&& fn) {
                    auto diameter = processing_region.radius + processing_region.radius + 1;
                    auto x_offset_pre = processing_region.center_x - processing_region.radius;
                    auto z_offset_pre = processing_region.center_z - processing_region.radius;
                    processing_region.enum_points_from_center([&](auto x, auto z) {
                        fn(x, z, processed_chunks.at((z - z_offset_pre) * diameter + (x - x_offset_pre)));
                    });
                }
            };

            enbt::raw_uuid id;
            enbt::compound nbt;
            enbt::compound server_data;
            std::unordered_map<uint32_t, slot_data> inventory;
            std::unordered_map<std::string, std::unordered_map<uint32_t, slot_data>> custom_inventory;
            std::optional<entity_ref> ride_entity;
            list_array<entity_ref> ride_by_entity;                               //used for storing entities, first element is main entity
            std::optional<std::variant<entity_ref, enbt::raw_uuid>> attached_to; //this entity follows entity
            list_array<std::variant<entity_ref, enbt::raw_uuid>> attached;       //entities follows this entity


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

            world_syncing& get_syncing_data() {
                if (!world_syncing_data)
                    throw std::runtime_error("World syncing data is not initialized for entity " + id.to_string());
                return *world_syncing_data;
            }

            //nbt {
            //  float health = 20;
            //  uint8_t food = 20;
            //  float saturation = 5;
            //  float breath = 10;
            //  int32_t level = 0;
            //  int32_t required_experience = 0;
            //  int32_t experience = 0;
            //  int32_t fall_distance = 0;
            //  uint8_t selected_item = 0; //hotbar, 0..8
            //
            //  bool is_sleeping = false;
            //  bool on_ground = false;
            //  bool is_sprinting = false;
            //  bool is_sneaking = false;
            //
            //  calculated_values {
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
            void erase(); //same as force_kill but without animation and other handling(pre_death_callback not called)
            bool is_died();
            const entity_data& const_data();

            entity_ref copy() const;
            enbt::compound copy_to_enbt() const;

            void teleport(util::VECTOR pos);
            void teleport(util::VECTOR pos, float yaw, float pitch);
            void teleport(util::VECTOR pos, float yaw, float pitch, bool on_ground);


            void set_ride_entity(entity_ref entity);
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
            void damage(float health, int32_t type_id, std::optional<util::VECTOR> pos);
            void damage(float health, int32_t type_id, entity_ref& source, std::optional<util::VECTOR> pos);
            void damage(float health, int32_t type_id, entity_ref& source, entity_ref& source_direct, std::optional<util::VECTOR> pos);

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
            void look_at(util::VECTOR pos);

            //looks only if this and another entity registered to same world
            void look_at(const entity_ref& entity);

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

            void breaking_block(int64_t global_x, uint64_t global_y, int64_t global_z, uint32_t time);
            void place_block(int64_t global_x, uint64_t global_y, int64_t global_z, const block&);
            void place_block(int64_t global_x, uint64_t global_y, int64_t global_z, base_objects::const_block_entity_ref);
            void place_block(int64_t global_x, uint64_t global_y, int64_t global_z, block_entity&&);

            static entity_ref create(uint16_t id);
            static entity_ref create(uint16_t id, const enbt::compound_const_ref& nbt);
            static entity_ref create(const std::string& id);
            static entity_ref create(const std::string& id, const enbt::compound_const_ref& nbt);
            static entity_ref load_from_enbt(const enbt::compound_const_ref& file_nbt);

            entity();
            ~entity();

            entity(const entity& other) = delete;
            entity& operator=(const entity& other) = delete;

            entity(entity&& other) = delete;
            entity& operator=(entity&& other) = delete;

            void tick();
            std::optional<int32_t> get_object_field();


            bool is_player(); //returns true if entity_id belongs to player

            uint16_t get_entity_type_id() const {
                return entity_id;
            }

        private:
            friend struct entity_data;
            uint16_t entity_id;
            bool died : 1 = false;
        };
    }
}


#endif /* SRC_BASE_OBJECTS_ENTITY */
