#ifndef SRC_BASE_OBJECTS_SELECTOR
#define SRC_BASE_OBJECTS_SELECTOR
#include "../library/list_array.hpp"
#include "shared_client_data.hpp"
#include "shared_string.hpp"
#include <optional>
#include <string>
#include <unordered_map>

namespace crafted_craft {
    namespace base_objects {
        struct integer_range {
            std::optional<int> min;
            std::optional<int> max;
            bool is_inverted;
        };

        struct selector_string {
            shared_string string;
            bool is_inverted;
        };

        enum class selector_sort {
            nearest,
            furthest,
            random,
            arbitrary
        };

        union selector_flags {
            struct {
                //@p == only_players & nearest & only_one & except_self
                //@r == only_players & random
                //@a == only_players
                //@e == 0
                //@s == self
                //@R == random
                //@E == only_entities
                //@n == nearest & except_self
                //@o == random & only_one
                //@t == only_entities & random
                //@<only_players, only_entities, random, nearest, only_one, self, except_self> custom

                bool only_players : 1;
                bool only_entities : 1;
                bool random : 1;
                bool nearest : 1;
                bool only_one : 1;
                bool self : 1;
                bool except_self : 1;
            };

            uint64_t raw = 0;
        };

        struct selector {
            std::string full_string;

            //Position selectors
            std::optional<double> x;
            std::optional<double> y;
            std::optional<double> z;
            std::optional<double> dx;
            std::optional<double> dy;
            std::optional<double> dz;
            std::optional<double> x_rotation;
            std::optional<double> y_rotation;
            integer_range distance;

            //Scoreboard selectors
            std::unordered_map<shared_string, integer_range> scores;
            list_array<selector_string> tags;
            list_array<selector_string> team;

            //Entity species
            list_array<selector_string> name;
            list_array<selector_string> type;
            list_array<selector_string> family;
            list_array<selector_string> predicate;

            //Entity data
            std::optional<std::string> nbt;
            std::optional<std::string> abilities;
            list_array<selector_string> has_item;


            //Player data
            std::optional<shared_string> gamemode;
            std::optional<double> level;
            std::unordered_map<shared_string, bool> advancements;

            //Permissions
            std::unordered_map<shared_string, bool> has_permission;
            list_array<selector_string> in_group;

            //Traits
            std::optional<uint32_t> limit;
            std::optional<selector_sort> sort;


            selector_flags flags;

            //Position selectors
            bool x_inverted = false;
            bool y_inverted = false;
            bool z_inverted = false;
            bool dx_inverted = false;
            bool dy_inverted = false;
            bool dz_inverted = false;
            bool x_rotation_inverted = false;
            bool y_rotation_inverted = false;

            //Scoreboard selectors
            bool scores_inverted = false;
            bool tags_inverted = false;
            bool team_inverted = false;

            //Entity species
            bool name_inverted = false;
            bool type_inverted = false;
            bool family_inverted = false;
            bool predicate_inverted = false;


            //Entity data
            bool nbt_inverted = false;
            bool abilities_inverted = false;
            bool has_item_inverted = false;

            //Traits
            bool limit_inverted = false; //if inverted then selects entities more than limit, and retuns noting if got less than limit


            //parses and builds selector
            selector build_selector(const std::string& selector_string);

            //from noting
            bool select(class Entity& entity) const;
            bool select(client_data_holder& player) const;

            //entity selects
            bool select(Entity& caller, class Entity& entity) const;
            bool select(Entity& caller, client_data_holder& player) const;

            //player selects
            bool select(client_data_holder& caller, class Entity& entity) const;
            bool select(client_data_holder& caller, client_data_holder& player) const;
        };

    } // namespace base_objects


} // namespace crafted_craft


#endif /* SRC_BASE_OBJECTS_SELECTOR */
