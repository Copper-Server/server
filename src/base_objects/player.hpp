#ifndef SRC_BASE_OBJECTS_PLAYER
#define SRC_BASE_OBJECTS_PLAYER
#include <cstdint>
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <string>

namespace copper_server::base_objects {
    struct entity;
    using entity_ref = atomic_holder<entity>;

    class player {
    public:
        struct Abilities {
            struct Flags {
                bool invulnerable : 1;
                bool flying : 1;
                bool allow_flying : 1;
                bool creative_mode : 1;
                bool flying_speed : 1;
                bool walking_speed : 1;

                inline void set(uint8_t raw) {
                    union u_t {
                        Flags flag;
                        uint8_t r;
                    } u{.r = raw};

                    *this = u.flag;
                }

                inline uint8_t get() const {
                    union u_t {
                        Flags flag;
                        uint8_t r;
                    } u{.flag = *this};

                    return u.r;
                }
            } flags;

            float flying_speed = 0.05f;
            float field_of_view_modifier = 0.1f;
        } abilities;

        struct ExperienceData {
            float progress = 0;
            int32_t level = 0, total = 0;
        } experience;

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

        struct OwnRespawnLocation {
            int32_t x = 0;
            int32_t y = 0;
            int32_t z = 0;
            float angle = 0;
            std::string world_id;
        };

        std::optional<DeathLocation> last_death_location;
        std::optional<OwnRespawnLocation> own_respawn_location;

        list_array<std::string> permission_groups;

        //[runtime] calculated from permission_groups
        list_array<std::string> permissions;
        //[runtime] calculated from permission_groups
        list_array<std::string> instant_granted_actions;

        entity_ref assigned_entity;

        //for server plugins
        enbt::compound local_data;

        player& operator=(const player& other) = delete;
        player& operator=(player&& other);

        player();
        ~player();
    };
}

#endif /* SRC_BASE_OBJECTS_PLAYER */
