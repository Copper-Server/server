#include <src/log.hpp>
#include <src/storage/scoreboard.hpp>
#include <src/util/json_helpers.hpp>

namespace copper_server::storage {
    scoreboard::scoreboard(const std::filesystem::path& path)
        : path(path) {}

    using namespace util;

    base_objects::team::color form_string_color(const std::string& str) {
        static std::unordered_map<std::string, base_objects::team::color> fast_select{
            {"black", base_objects::team::color::black},
            {"dark_blue", base_objects::team::color::dark_blue},
            {"dark_green", base_objects::team::color::dark_green},
            {"dark_aqua", base_objects::team::color::dark_aqua},
            {"dark_red", base_objects::team::color::dark_red},
            {"dark_purple", base_objects::team::color::dark_purple},
            {"gold", base_objects::team::color::gold},
            {"gray", base_objects::team::color::gray},
            {"dark_gray", base_objects::team::color::dark_gray},
            {"blue", base_objects::team::color::blue},
            {"green", base_objects::team::color::green},
            {"aqua", base_objects::team::color::aqua},
            {"red", base_objects::team::color::red},
            {"light_purple", base_objects::team::color::light_purple},
            {"yellow", base_objects::team::color::yellow},
            {"white", base_objects::team::color::white},
        };
        auto it = fast_select.find(str);
        if (it != fast_select.end())
            return it->second;
        else
            return base_objects::team::color::black;
    }

    base_objects::team::collision_rule form_string_collision_rule(const std::string& str) {
        static std::unordered_map<std::string, base_objects::team::collision_rule> fast_select{
            {"always", base_objects::team::collision_rule::always},
            {"never", base_objects::team::collision_rule::never},
            {"push_other_teams", base_objects::team::collision_rule::push_other_teams},
            {"push_own_team", base_objects::team::collision_rule::push_own_team},
        };
        auto it = fast_select.find(str);
        if (it != fast_select.end())
            return it->second;
        else
            return base_objects::team::collision_rule::always;
    }

    base_objects::team::death_message_visibility form_string_death_message_visibility(const std::string& str) {
        static std::unordered_map<std::string, base_objects::team::death_message_visibility> fast_select{
            {"always", base_objects::team::death_message_visibility::always},
            {"hide_for_other_teams", base_objects::team::death_message_visibility::hide_for_other_teams},
            {"hide_for_own_team", base_objects::team::death_message_visibility::hide_for_own_team},
            {"hide_for_own_members", base_objects::team::death_message_visibility::hide_for_own_members},
        };
        auto it = fast_select.find(str);
        if (it != fast_select.end())
            return it->second;
        else
            return base_objects::team::death_message_visibility::always;
    }

    base_objects::team::name_tag_visibility form_string_name_tag_visibility(const std::string& str) {
        static std::unordered_map<std::string, base_objects::team::name_tag_visibility> fast_select{
            {"always", base_objects::team::name_tag_visibility::always},
            {"hide_for_other_teams", base_objects::team::name_tag_visibility::hide_for_other_teams},
            {"hide_for_own_team", base_objects::team::name_tag_visibility::hide_for_own_team},
            {"hide_for_own_members", base_objects::team::name_tag_visibility::hide_for_own_members},
        };
        auto it = fast_select.find(str);
        if (it != fast_select.end())
            return it->second;
        else
            return base_objects::team::name_tag_visibility::always;
    }

    std::string to_string(base_objects::team::color color) {
        switch (color) {
        case base_objects::team::color::black:
            return "black";
        case base_objects::team::color::dark_blue:
            return "dark_blue";
        case base_objects::team::color::dark_green:
            return "dark_green";
        case base_objects::team::color::dark_aqua:
            return "dark_aqua";
        case base_objects::team::color::dark_red:
            return "dark_red";
        case base_objects::team::color::dark_purple:
            return "dark_purple";
        case base_objects::team::color::gold:
            return "gold";
        case base_objects::team::color::gray:
            return "gray";
        case base_objects::team::color::dark_gray:
            return "dark_gray";
        case base_objects::team::color::blue:
            return "blue";
        case base_objects::team::color::green:
            return "green";
        case base_objects::team::color::aqua:
            return "aqua";
        case base_objects::team::color::red:
            return "red";
        case base_objects::team::color::light_purple:
            return "light_purple";
        case base_objects::team::color::yellow:
            return "yellow";
        case base_objects::team::color::white:
            return "white";
        default:
            return "undefined";
        }
    }

    std::string to_string(base_objects::team::collision_rule color) {
        switch (color) {
        case base_objects::team::collision_rule::always:
            return "always";
        case base_objects::team::collision_rule::never:
            return "never";
        case base_objects::team::collision_rule::push_other_teams:
            return "push_other_teams";
        case base_objects::team::collision_rule::push_own_team:
            return "push_own_team";
        default:
            return "undefined";
        }
    }

    std::string to_string(base_objects::team::death_message_visibility color) {
        switch (color) {
        case base_objects::team::death_message_visibility::always:
            return "always";
        case base_objects::team::death_message_visibility::hide_for_other_teams:
            return "hide_for_other_teams";
        case base_objects::team::death_message_visibility::hide_for_own_team:
            return "hide_for_own_team";
        case base_objects::team::death_message_visibility::hide_for_own_members:
            return "hide_for_own_members";
        default:
            return "undefined";
        }
    }

    std::string to_string(base_objects::team::name_tag_visibility color) {
        switch (color) {
        case base_objects::team::name_tag_visibility::always:
            return "always";
        case base_objects::team::name_tag_visibility::hide_for_other_teams:
            return "hide_for_other_teams";
        case base_objects::team::name_tag_visibility::hide_for_own_team:
            return "hide_for_own_team";
        case base_objects::team::name_tag_visibility::hide_for_own_members:
            return "hide_for_own_members";
        default:
            return "undefined";
        }
    }

    void scoreboard::load() {
        auto config_holder = try_read_json_file(path);
        if (!config_holder)
            return;
        auto entry = js_object::get_object(*config_holder);
        auto json_objectives = js_object::get_object(entry["objectives"]);
        for (auto&& [key, obj] : json_objectives) {
            auto json_objective = js_object::get_object(obj);
            std::string display_name = json_objective["display_name"];
            std::string criteria = json_objective["criteria"];
            data.add_objective((std::string)key, display_name, criteria);
            auto objective = data.get_objective((std::string)key);

            for (auto&& [name, value] : js_object::get_object(json_objective["scores"]))
                objective->set_score((std::string)name, (int64_t)value);
        }

        auto json_teams = js_object::get_object(entry["teams"]);
        for (auto&& [key, tea] : json_teams) {
            auto json_team = js_object::get_object(tea);
            std::string display_name = json_team["display_name"];
            std::string prefix = json_team["prefix"];
            std::string suffix = json_team["suffix"];
            data.add_team((std::string)key, display_name, prefix, suffix);
            auto team = data.get_team((std::string)key);
            team->set_color(form_string_color(json_team["color"]));
            team->set_collision_rule(form_string_collision_rule(json_team["collision_rule"]));
            team->set_death_message_visibility(form_string_death_message_visibility(json_team["death_message_visibility"]));
            team->set_name_tag_visibility(form_string_name_tag_visibility(json_team["name_tag_visibility"]));
            team->set_friendly_fire(json_team["friendly_fire"]);
            team->set_see_invisible(json_team["see_invisible"]);

            for (auto item : js_array::get_array(json_team["players"]))
                team->add_player((std::string)item);
        }
    }

    void scoreboard::save() {
        boost::json::object entry;
        auto json_objectives = js_object::get_object(entry["objectives"]);
        data.inspect_each_objective([&json_objectives](const base_objects::objective& objective) {
            boost::json::object json_objective;
            json_objective["display_name"] = objective.get_display_name();
            json_objective["criteria"] = objective.get_criteria();
            auto json_scores = (json_objective["scores"] = boost::json::object()).get_object();
            json_scores.reserve(objective.scores_count());
            objective.inspect_each_score([&json_scores](const std::string& name, int64_t value) {
                json_scores[name] = value;
            });
            json_objectives[objective.get_name()] = std::move(json_objective);
        });
        auto json_teams = js_object::get_object(entry["teams"]);
        data.inspect_each_team([&json_teams](const base_objects::team& team) {
            boost::json::object json_team;
            json_team["display_name"] = team.get_display_name();
            json_team["color"] = to_string(team.get_color());
            json_team["collision_rule"] = to_string(team.get_collision_rule());
            json_team["death_message_visibility"] = to_string(team.get_death_message_visibility());
            json_team["name_tag_visibility"] = to_string(team.get_name_tag_visibility());
            json_team["friendly_fire"] = team.get_friendly_fire();
            json_team["see_invisible"] = team.get_see_invisible();
            auto json_players = (json_team["players"] = boost::json::array()).get_array();
            json_players.reserve(team.player_count());
            team.inspect_each_player([&json_players](const std::string& name) {
                json_players.push_back((boost::json::string)name);
            });
            json_teams[team.get_name()] = std::move(json_team);
        });


        std::ofstream file(path, std::ofstream::trunc);
        if (!file.is_open()) {
            log::warn("server", "Failed to save permissions file. Can not open file.");
            return;
        }
        util::pretty_print(file, entry);
    }
}