#ifndef SRC_BASE_OBJECTS_COMMANDS
#define SRC_BASE_OBJECTS_COMMANDS
#include "../library/list_array.hpp"
#include "chat.hpp"
#include "packets.hpp"
#include "permissions.hpp"
#include "shared_client_data.hpp"
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

namespace crafted_craft {
    namespace base_objects {
        using command_callback = std::function<void(const list_array<std::string>&, client_data_holder&)>;
        using command_redirect = std::function<void(const list_array<std::string>&, const std::string&, client_data_holder&)>;
        using command_suggestion = std::function<list_array<std::string>(const std::string& current, client_data_holder&)>;

        struct action_provider {
            std::string action_tag;
            list_array<base_objects::shared_string> required_permissions_tag;
            action_provider(const std::string& tag);
            action_provider(const std::string&& tag);
            action_provider(const std::string& tag, const list_array<shared_string>& requirement);
            action_provider(const std::string&& tag, const list_array<shared_string>& requirement);
            action_provider(const std::string& tag, list_array<shared_string>&& requirement);
            action_provider(const std::string&& tag, list_array<shared_string>&& requirement);
        };

        struct command {
            using parsers = packets::command_node::parsers;
            using properties_t = packets::command_node::properties_t;
            packets::command_node node;
            command_callback callback;
            command_redirect redirect_command;
            command_suggestion suggestions;
            std::string description;
            std::string usage;
            //permission
            std::string action_name;
            //mapped permission for native clients
            int8_t permission_level = 0;

            std::unordered_map<std::string, int32_t> childs_cache;
            uint32_t links = 0;

            command() = default;
            command(const std::string& name, const std::string& description = "", const std::string& usage = "", const std::string& action_name = "");


            int32_t get_child(list_array<command>& commands_nodes, const std::string& name);
        };

        class command_manager {
            list_array<command> command_nodes;
            list_array<uint8_t> graph_cache;
            bool graph_ready;

        public:
            friend class command_browser;
            friend class command_root_browser;
            command_manager();
            command_manager(const command_manager&) = delete;
            command_manager(command_manager&&) = delete;

            void execute_command(const std::string& command, client_data_holder&);
            list_array<std::string> request_suggestions(const std::string& command, client_data_holder&);

            const list_array<uint8_t>& compile_to_graph();
            bool is_graph_fresh() const;

            bool belongs(command* command);


            //every command refrence after this command become invalid, even when created by plugins in OnCommandsLoad, bc. of the commit command for optimization
            void reload_commands();
        };

        class command_browser {
            command_manager& manager;
            command& current_command;
            int32_t current_id;


            command_browser(command_manager& manager, int32_t id);
            friend class command_root_browser;

        public:
            command_browser(command_manager& manager, const std::string& path);
            command_browser(command_browser& browser, const std::string& path);
            command_browser(command_browser&& browser) noexcept;

            command_browser add_child(command&& command);
            command_browser add_child(command&& command, packets::command_node::parsers parser, packets::command_node::properties_t properties = {});
            command_browser add_child(command_browser& command);
            list_array<command_browser> get_childs();

            command_browser& set_redirect(const std::string& path, const command_redirect& redirect);
            command_browser& remove_redirect();

            command_browser& set_callback(const std::string& action, const command_callback& callback);
            command_browser& set_callback(const std::string& action, const command_callback& callback, packets::command_node::parsers parser, packets::command_node::properties_t properties = {});
            command_browser& set_callback(const action_provider& action, const command_callback& callback);
            command_browser& set_callback(const action_provider& action, const command_callback& callback, packets::command_node::parsers parser, packets::command_node::properties_t properties = {});
            command_browser& set_callback(action_provider&& action, const command_callback& callback);
            command_browser& set_callback(action_provider&& action, const command_callback& callback, packets::command_node::parsers parser, packets::command_node::properties_t properties = {});
            command_browser& remove_callback();

            command_browser& set_suggestion(const std::string& suggestion_type);
            command_browser& set_suggestion_callback(const command_suggestion& suggestion);
            command_browser& remove_suggestion();

            command_browser& modify_command(const command& command);
            command_browser& modify_command(command&& command);

            command_browser open(const std::string& path) const;
            std::string get_documentation() const;
            const command& look_up();

            bool is_valid() const;
        };

        class command_root_browser {
            command_manager& manager;

        public:
            command_root_browser(command_manager& manager);

            command_root_browser(command_root_browser&& browser) noexcept
                : manager(browser.manager) {}

            command_root_browser(const command_root_browser& browser) noexcept
                : manager(browser.manager) {}

            command_browser add_child(command&& command);
            list_array<command_browser> get_childs();

            command_browser open(const std::string& path) const;
            std::string get_documentation() const;

            command_manager& get_manager()const;
        };

    } // namespace base_objects

} // namespace crafted_craft


#endif /* SRC_BASE_OBJECTS_COMMANDS */
