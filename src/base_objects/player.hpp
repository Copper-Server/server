#ifndef SRC_BASE_OBJECTS_PLAYER
#define SRC_BASE_OBJECTS_PLAYER
#include <cstdint>
#include <library/enbt/enbt.hpp>
#include <src/base_objects/entity.hpp>
#include <string>

namespace copper_server::base_objects {
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

        std::string world_id;
        std::string player_name;

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
            std::string world_id;
        };

        std::optional<DeathLocation> last_death_location;

        list_array<std::string> permission_groups;

        //[runtime] calculated from permission_groups
        list_array<std::string> permissions;
        //[runtime] calculated from permission_groups
        list_array<std::string> instant_granted_actions;

        entity_ref assigned_entity;

        //for server plugins
        enbt::compound local_data;

        player& operator=(const player& other) = delete;

        player& operator=(player&& other) {
            abilities = std::move(other.abilities);
            world_id = std::move(other.world_id);
            player_name = std::move(other.player_name);
            hardcore_hearts = other.hardcore_hearts;
            reduced_debug_info = other.reduced_debug_info;
            show_death_screen = other.show_death_screen;
            op_level = other.op_level;
            gamemode = other.gamemode;
            prev_gamemode = other.prev_gamemode;
            last_death_location = other.last_death_location;
            permission_groups = std::move(other.permission_groups);
            permissions = std::move(other.permissions);
            instant_granted_actions = std::move(other.instant_granted_actions);
            assigned_entity = std::move(other.assigned_entity);
            local_data = std::move(other.local_data);
            return *this;
        }

        player() = default;
        ~player() = default;
    };
}

#endif /* SRC_BASE_OBJECTS_PLAYER */
