#ifndef SRC_BASE_OBJECTS_SCOREBOARD
#define SRC_BASE_OBJECTS_SCOREBOARD
#include "../library/fast_task.hpp"
#include "../library/list_array.hpp"
#include "atomic_holder.hpp"
#include "event.hpp"
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace crafted_craft {
    namespace base_objects {
        class objective {
            struct event_auto_cleanup_t {
                base_objects::base_event* event_obj;
                base_objects::event_register_id id;
                base_objects::event_priority priority;
                bool async_mode;
            };

            struct protected_vals_t {
                std::string name;
                std::string display_name;
                std::string criteria;
                std::unordered_map<std::string, int64_t> scores;


                size_t next_id = 0;
                std::unordered_map<size_t, event_auto_cleanup_t> cleanup_list;

                protected_vals_t() {}

                protected_vals_t(std::string name, std::string display_name, std::string criteria)
                    : name(name), display_name(display_name), criteria(criteria) {}

                protected_vals_t(protected_vals_t&& other) noexcept
                    : name(std::move(other.name)), display_name(std::move(other.display_name)), criteria(std::move(other.criteria)), scores(std::move(other.scores)), next_id(other.next_id), cleanup_list(std::move(other.cleanup_list)) {}
            };

            fast_task::protected_value<protected_vals_t> protected_vals;

        public:
            event<std::string> on_score_change;
            event<std::string> on_score_remove;

            objective(std::string name, std::string display_name, std::string criteria)
                : protected_vals(name, display_name, criteria) {}

            objective(const objective&) = delete;

            objective(objective&& other) noexcept
                : protected_vals(std::move(other.protected_vals)) {
                other.clean_up_registered_events();
            }

            ~objective() {
                clean_up_registered_events();
            }

            std::string get_name() const {
                return protected_vals.get([](const protected_vals_t& vals) -> std::string {
                    return vals.name;
                });
            }

            std::string get_display_name() const {
                return protected_vals.get([](const protected_vals_t& vals) -> std::string {
                    return vals.display_name;
                });
            }

            std::string get_criteria() const {
                return protected_vals.get([](const protected_vals_t& vals) -> std::string {
                    return vals.criteria;
                });
            }

            void set_display_name(std::string display_name) {
                protected_vals.set([display_name, this](protected_vals_t& vals) {
                    vals.display_name = display_name;
                });
            }

            void set_criteria(std::string criteria) {
                protected_vals.set([criteria, this](protected_vals_t& vals) {
                    vals.criteria = criteria;
                });
            }

            void set_score(std::string player, int64_t score) {
                protected_vals.set([player, score, this](protected_vals_t& vals) {
                    vals.scores[player] = score;
                    on_score_change(player);
                });
            }

            template <class FN>
            void inspect_each_score(FN&& fn) const {
                protected_vals.get([&fn](const protected_vals_t& vals) {
                    for (auto& [name, value] : vals.scores)
                        fn(name, value);
                });
            }

            template <class FN>
            void for_each_score(FN&& fn) {
                protected_vals.set([&fn](protected_vals_t& vals) {
                    for (auto& [name, value] : vals.scores)
                        fn(name, value);
                });
            }

            size_t scores_count() const {
                return protected_vals.get([](const protected_vals_t& vals) {
                    return vals.scores.size();
                });
            }

            void reset_score(std::string player) {
                protected_vals.set([player, this](protected_vals_t& vals) {
                    auto it = vals.scores.find(player);
                    if (it == vals.scores.end())
                        return;
                    on_score_change(player);
                    it->second = 0;
                });
            }

            std::optional<int64_t> get_score(std::string player) const {
                return protected_vals.get([player](const protected_vals_t& vals) -> std::optional<int64_t> {
                    auto it = vals.scores.find(player);
                    if (it == vals.scores.end())
                        return std::nullopt;
                    return it->second;
                });
            }

            void remove_score(std::string player) {
                protected_vals.set([player, this](protected_vals_t& vals) {
                    on_score_remove(player);
                    vals.scores.erase(player);
                });
            }

            void clear() {
                protected_vals.set([this](protected_vals_t& vals) {
                    for (auto& [player, score] : vals.scores)
                        on_score_remove(player);
                    vals.scores.clear();
                });
            }

            void clean_up_registered_events() {
                protected_vals.set([](protected_vals_t& vals) {
                    for (auto& [id, leave_data] : vals.cleanup_list)
                        leave_data.event_obj->leave(leave_data.id, leave_data.priority, leave_data.async_mode);
                    vals.cleanup_list.clear();
                });
            }

            template <class T>
            size_t register_event(base_objects::event<T>& event_ref, base_objects::event<T>::function&& fn) {
                return protected_vals.set([&](protected_vals_t& vals) {
                    vals.cleanup_list[vals.next_id++] = {event_ref.join(base_objects::event_priority::avg, false, fn), event_ref, base_objects::event_priority::avg, false};
                    return vals.next_id - 1;
                });
            }

            template <class T>
            size_t register_event(base_objects::event<T>& event_ref, base_objects::event_priority priority, base_objects::event<T>::function&& fn) {
                return protected_vals.set([&](protected_vals_t& vals) {
                    vals.cleanup_list[vals.next_id++] = {event_ref.join(priority, false, fn), event_ref, priority, false};
                    return vals.next_id - 1;
                });
            }

            template <class T>
            size_t register_event(base_objects::event<T>& event_ref, base_objects::event_priority priority, bool async_mode, base_objects::event<T>::function&& fn) {
                return protected_vals.set([&](protected_vals_t& vals) {
                    vals.cleanup_list[vals.next_id++] = {event_ref.join(priority, async_mode, fn), event_ref, priority, async_mode};
                    return vals.next_id - 1;
                });
            }

            void leave_event(size_t id) {
                protected_vals.set([id](protected_vals_t& vals) {
                    auto it = vals.cleanup_list.find(id);
                    if (it == vals.cleanup_list.end())
                        return;
                    it->second.event_obj->leave(it->second.id, it->second.priority, it->second.async_mode);
                    vals.cleanup_list.erase(it);
                });
            }
        };

        struct team {
            enum class collision_rule : uint8_t {
                always,
                never,
                push_other_teams,
                push_own_team
            };

            enum class death_message_visibility : uint8_t {
                always,
                hide_for_other_teams,
                hide_for_own_team,
                hide_for_own_members
            };

            enum class name_tag_visibility : uint8_t {
                always,
                hide_for_other_teams,
                hide_for_own_team,
                hide_for_own_members
            };

            enum class color : uint8_t {
                black,
                dark_blue,
                dark_green,
                dark_aqua,
                dark_red,
                dark_purple,
                gold,
                gray,
                dark_gray,
                blue,
                green,
                aqua,
                red,
                light_purple,
                yellow,
                white
            };

            team(std::string name, std::string display_name, std::string prefix, std::string suffix)
                : protected_vals(name, display_name, prefix, suffix) {}

            std::string get_name() const {
                return protected_vals.get([](const protected_vals_t& vals) -> std::string {
                    return vals.name;
                });
            }

            std::string get_display_name() const {
                return protected_vals.get([](const protected_vals_t& vals) -> std::string {
                    return vals.display_name;
                });
            }

            std::string get_prefix() const {
                return protected_vals.get([](const protected_vals_t& vals) -> std::string {
                    return vals.prefix;
                });
            }

            std::string get_suffix() const {
                return protected_vals.get([](const protected_vals_t& vals) -> std::string {
                    return vals.suffix;
                });
            }

            color get_color() const {
                return protected_vals.get([](const protected_vals_t& vals) -> color {
                    return vals.color;
                });
            }

            collision_rule get_collision_rule() const {
                return protected_vals.get([](const protected_vals_t& vals) -> collision_rule {
                    return vals.collision_rule;
                });
            }

            death_message_visibility get_death_message_visibility() const {
                return protected_vals.get([](const protected_vals_t& vals) -> death_message_visibility {
                    return vals.death_message_visibility;
                });
            }

            name_tag_visibility get_name_tag_visibility() const {
                return protected_vals.get([](const protected_vals_t& vals) -> name_tag_visibility {
                    return vals.name_tag_visibility;
                });
            }

            bool get_friendly_fire() const {
                return protected_vals.get([](const protected_vals_t& vals) -> bool {
                    return vals.friendly_fire;
                });
            }

            bool get_see_invisible() const {
                return protected_vals.get([](const protected_vals_t& vals) -> bool {
                    return vals.see_invisible;
                });
            }

            void set_display_name(std::string display_name) {
                protected_vals.set([display_name, this](protected_vals_t& vals) {
                    vals.display_name = display_name;
                });
            }

            void set_prefix(std::string prefix) {
                protected_vals.set([prefix, this](protected_vals_t& vals) {
                    vals.prefix = prefix;
                });
            }

            void set_suffix(std::string suffix) {
                protected_vals.set([suffix, this](protected_vals_t& vals) {
                    vals.suffix = suffix;
                });
            }

            void set_color(color color) {
                protected_vals.set([color, this](protected_vals_t& vals) {
                    vals.color = color;
                });
            }

            void set_collision_rule(collision_rule rule) {
                protected_vals.set([rule, this](protected_vals_t& vals) {
                    vals.collision_rule = rule;
                });
            }

            void set_death_message_visibility(death_message_visibility visibility) {
                protected_vals.set([visibility, this](protected_vals_t& vals) {
                    vals.death_message_visibility = visibility;
                });
            }

            void set_name_tag_visibility(name_tag_visibility visibility) {
                protected_vals.set([visibility, this](protected_vals_t& vals) {
                    vals.name_tag_visibility = visibility;
                });
            }

            void set_friendly_fire(bool friendly_fire) {
                protected_vals.set([friendly_fire, this](protected_vals_t& vals) {
                    vals.friendly_fire = friendly_fire;
                });
            }

            void set_see_invisible(bool see_invisible) {
                protected_vals.set([see_invisible, this](protected_vals_t& vals) {
                    vals.see_invisible = see_invisible;
                });
            }

            void add_player(std::string player) {
                protected_vals.set([player, this](protected_vals_t& vals) {
                    vals.players.insert(player);
                });
            }

            void remove_player(std::string player) {
                protected_vals.set([player, this](protected_vals_t& vals) {
                    vals.players.erase(player);
                });
            }

            bool has_player(std::string player) const {
                return protected_vals.get([player](const protected_vals_t& vals) -> bool {
                    return vals.players.find(player) != vals.players.end();
                });
            }

            void clear_players() {
                protected_vals.set([this](protected_vals_t& vals) {
                    vals.players.clear();
                });
            }

            template <class FN>
            void inspect_each_player(FN&& fn) const {
                protected_vals.get([&fn](const protected_vals_t& vals) {
                    for (auto& name : vals.players)
                        fn(name);
                });
            }

            size_t player_count() const {
                return protected_vals.get([](const protected_vals_t& vals) {
                    return vals.players.size();
                });
            }

        private:
            struct protected_vals_t {
                std::string name;
                std::string display_name;
                std::string prefix;
                std::string suffix;

                color color = color::white;
                collision_rule collision_rule = collision_rule::always;
                death_message_visibility death_message_visibility = death_message_visibility::always;
                name_tag_visibility name_tag_visibility = name_tag_visibility::always;
                bool friendly_fire = true;
                bool see_invisible = true;
                std::unordered_set<std::string> players;

                protected_vals_t(std::string name, std::string display_name, std::string prefix, std::string suffix)
                    : name(name), display_name(display_name), prefix(prefix), suffix(suffix) {}
            };

            fast_task::protected_value<protected_vals_t> protected_vals;
        };

        class scoreboard {
            struct protected_vals_t {
                std::unordered_map<std::string, atomic_holder<objective>> objectives;
                std::unordered_map<std::string, atomic_holder<team>> teams;
            };

            fast_task::protected_value<protected_vals_t> protected_vals;

        public:
            scoreboard() {}

            template <class FN>
            void inspect_each_objective(FN&& fn) const& {
                protected_vals.get([&fn](const protected_vals_t& vals) {
                    for (auto& [name, obj] : vals.objectives)
                        fn(*obj);
                });
            }

            template <class FN>
            void inspect_each_team(FN&& fn) const& {
                protected_vals.get([fn](const protected_vals_t& vals) {
                    for (auto& [name, team] : vals.teams)
                        fn(*team);
                });
            }

            template <class FN>
            void for_each_objective(FN&& fn) & {
                protected_vals.set([fn](protected_vals_t& vals) {
                    for (auto& [name, obj] : vals.objectives)
                        fn(*obj);
                });
            }

            template <class FN>
            void for_each_team(FN&& fn) & {
                protected_vals.set([fn](protected_vals_t& vals) {
                    for (auto& [name, team] : vals.teams)
                        fn(*team);
                });
            }

            void add_objective(std::string name, std::string display_name, std::string criteria) {
                protected_vals.set([name, display_name, criteria, this](protected_vals_t& vals) {
                    auto it = vals.objectives.find(name);
                    if (it != vals.objectives.end())
                        throw std::runtime_error("Objective already exists");
                    vals.objectives[name] = new objective(name, display_name, criteria);
                });
            }

            void add_team(std::string name, std::string display_name, std::string prefix, std::string suffix) {
                protected_vals.set([name, display_name, prefix, suffix, this](protected_vals_t& vals) {
                    auto it = vals.teams.find(name);
                    if (it != vals.teams.end())
                        throw std::runtime_error("Team already exists");
                    vals.teams[name] = new team(name, display_name, prefix, suffix);
                });
            }

            atomic_holder<objective> get_objective(std::string name) const {
                return protected_vals.get([name](const protected_vals_t& vals) {
                    return vals.objectives.at(name);
                });
            }

            atomic_holder<team> get_team(std::string name) const {
                return protected_vals.get([name](const protected_vals_t& vals) {
                    return vals.teams.at(name);
                });
            }

            void remove_objective(std::string name) {
                protected_vals.set([name, this](protected_vals_t& vals) {
                    vals.objectives.erase(name);
                });
            }

            void remove_team(std::string name) {
                protected_vals.set([name, this](protected_vals_t& vals) {
                    vals.teams.erase(name);
                });
            }

            void clear() {
                protected_vals.set([this](protected_vals_t& vals) {
                    vals.objectives.clear();
                    vals.teams.clear();
                });
            }

            size_t teams_count() const {
                return protected_vals.get([](const protected_vals_t& vals) {
                    return vals.teams.size();
                });
            }

            size_t objectives_count() const {
                return protected_vals.get([](const protected_vals_t& vals) {
                    return vals.objectives.size();
                });
            }

            bool has_objective(std::string name) const {
                return protected_vals.get([name](const protected_vals_t& vals) -> bool {
                    return vals.objectives.find(name) != vals.objectives.end();
                });
            }

            bool has_team(std::string name) const {
                return protected_vals.get([name](const protected_vals_t& vals) -> bool {
                    return vals.teams.find(name) != vals.teams.end();
                });
            }

            void clear_teams() {
                protected_vals.set([this](protected_vals_t& vals) {
                    vals.teams.clear();
                });
            }

            void clear_objectives() {
                protected_vals.set([this](protected_vals_t& vals) {
                    vals.objectives.clear();
                });
            }
        };
    }
}

#endif /* SRC_BASE_OBJECTS_SCOREBOARD */
