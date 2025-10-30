/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef SRC_API_SELECTOR
#define SRC_API_SELECTOR
#include <library/list_array.hpp>
#include <optional>
#include <src/api/ecs.hpp>
#include <string>
#include <unordered_map>

namespace copper_server::base_objects {
    struct command_context;
}
namespace copper_server::api {
    //extended entity selector, supports more selection arguments
    struct selector {
        struct integer_range {
            int min;
            int max;
            bool is_inverted;
        };

        struct double_range {
            double min;
            double max;
            bool is_inverted;
        };

        struct string_t {
            std::string string;
            bool is_inverted;
        };

        enum class sort_t {
            nearest,
            furthest,
            random,
            arbitrary
        };

        struct flags_t {
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
            //@<only_players, only_entities, random, nearest, only_one, self, except_self, select_virtual> custom

            bool only_players : 1 = false;  //
            bool only_entities : 1 = false; //
            bool random : 1 = false;
            bool nearest : 1 = false; //
            bool only_one : 1 = false;
            bool self : 1 = false;           //
            bool except_self : 1 = false;    //
            bool select_virtual : 1 = false; //
        };

        std::string full_string;

        // position selectors
        std::optional<double> x;                //
        std::optional<double> y;                //
        std::optional<double> z;                //
        std::optional<double> dx;               //
        std::optional<double> dy;               //
        std::optional<double> dz;               //
        std::optional<double_range> x_rotation; //
        std::optional<double_range> y_rotation; //
        std::optional<integer_range> distance;  //

        //Scoreboard selectors
        std::unordered_map<std::string, integer_range> scores;
        list_array<string_t> tags;
        list_array<string_t> team;

        //entity species
        list_array<string_t> name;
        list_array<string_t> type;
        list_array<string_t> family;
        list_array<string_t> predicate;

        //entity data
        std::optional<std::string> nbt;
        std::optional<std::string> abilities;
        list_array<string_t> has_item;

        //Player data
        std::optional<std::string> gamemode;
        std::optional<integer_range> level; //
        std::unordered_map<std::string, bool> advancements;

        //Permissions
        std::unordered_map<std::string, bool> has_permission; //
        list_array<string_t> in_group;                        //

        //Traits
        std::optional<uint32_t> limit;     //
        std::optional<sort_t> sort;        //


        flags_t flags;
        bool scores_inverted = false;
        bool tags_inverted = false;
        bool team_inverted = false;

        bool name_inverted = false;
        bool type_inverted = false;
        bool family_inverted = false;
        bool predicate_inverted = false;

        bool nbt_inverted = false;
        bool abilities_inverted = false;

        bool gamemode_inverted = false;

        bool limit_inverted = false; //if inverted then selects entities more than limit, and retuns noting if got less than limit


        void build_selector_parse(std::string_view& selector_string);
        void build_selector(std::string_view selector_string);

        bool select(base_objects::command_context&, std::function<void(api::ecs::entity)>&& fn) const;
    };
}

#endif /* SRC_API_SELECTOR */
