#ifndef SRC_BASE_OBJECTS_PLAYER
#define SRC_BASE_OBJECTS_PLAYER
#include "../library/enbt.hpp"
#include "permissions.hpp"
#include "slot.hpp"
#include <cstdint>
#include <string>

namespace crafted_craft {
    namespace base_objects {
        class player {
        public:
            struct Abilities {
                union Flags {
                    struct {
                        bool invulnerable : 1;
                        bool flying : 1;
                        bool allow_flying : 1;
                        bool creative_mode : 1;
                        bool flying_speed : 1;
                        bool walking_speed : 1;
                    };

                    uint8_t mask = 0;
                } flags;

                float flying_speed = 0.05f;
                float field_of_view_modifier = 0.1f;
            } abilities;

            struct Position {
                double x = 0;
                double y = 0;
                double z = 0;
                float yaw = 0;
                float pitch = 0;
            } position;

            struct Inventory {
                slot crafting_output;
                slot crafting[4];

                struct {
                    slot head;
                    slot chest;
                    slot legs;
                    slot feet;
                } armor;

                slot main_inventory[27];
                slot hotbar[9];
                slot offhand;

                slot carried_item;
            } inventory;

            float health = 20;
            float food = 20;
            float saturation = 5;
            float experience = 0;

            shared_string world_id;
            std::string player_name;

            int32_t fall_distance = 0;

            bool is_died : 1 = false;
            bool is_sleeping : 1 = false;
            bool on_ground : 1 = false;
            bool is_sprinting : 1 = false;
            bool is_sneaking : 1 = false;
            bool hardcore_hearts : 1 = false;
            bool reduced_debug_info : 1 = false;
            bool show_death_screen : 1 = false;


            int8_t op_level = 0;
            uint8_t gamemode = 0;
            int8_t prev_gamemode = -1;

            struct DeathLocation {
                double x = 0;
                double y = 0;
                double z = 0;
                shared_string world_id;
            };

            std::optional<DeathLocation> last_death_location;

            std::optional<enbt::raw_uuid> ride_entity_id;

            list_array<shared_string> permission_groups;

            //[runtime] calculated from permission_groups
            list_array<shared_string> permissions;
            //[runtime] calculated from permission_groups
            list_array<shared_string> instant_granted_actions;

            //sent to client
            enbt::value additional_data = enbt::compound();

            //for server plugins
            enbt::value local_data = enbt::compound();


            player() = default;
            ~player() = default;
        };
    } // namespace base_objects

} // namespace crafted_craft


#endif /* SRC_BASE_OBJECTS_PLAYER */
