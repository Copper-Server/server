#ifndef SRC_BASE_OBJECTS_COMMANDS
#define SRC_BASE_OBJECTS_COMMANDS
#include "chat.hpp"
#include "packets.hpp"
#include "shared_client_data.hpp"
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace crafted_craft {
    namespace base_objects {


        struct suggestion {
            std::string insertion;
            std::optional<Chat> tooltip;
        };

        using command_callback = std::function<void(const list_array<std::string>&, client_data_holder&)>;
        using command_redirect = std::function<void(const list_array<std::string>&, const std::string&, client_data_holder&)>;

        struct command {
            using parsers = packets::command_node::parsers;
            using properties_t = packets::command_node::properties_t;
            packets::command_node node;
            command_callback callback;
            command_redirect redirect_command;
            std::function<std::vector<suggestion>(const list_array<std::string>&)> suggestions;
            std::string description;
            std::string usage;


            std::unordered_map<std::string, int32_t> childs_cache;
            uint32_t links = 0;

            command() = default;
            command(const std::string& name, const std::string& description = "", const std::string& usage = "");


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
            std::vector<suggestion> request_suggestions(const std::string& command);

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
            std::list<command_browser> get_childs();

            command_browser& set_redirect(const std::string& path, const command_redirect& redirect);
            command_browser& remove_redirect();

            command_browser& set_callback(const command_callback& callback);
            command_browser& set_callback(const command_callback& callback, packets::command_node::parsers parser, packets::command_node::properties_t properties = {});
            command_browser& remove_callback();

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
            std::list<command_browser> get_childs();

            command_browser open(const std::string& path) const;
            std::string get_documentation() const;

            command_manager& get_manager()const;
        };

    } // namespace base_objects

} // namespace crafted_craft


#endif /* SRC_BASE_OBJECTS_COMMANDS */
