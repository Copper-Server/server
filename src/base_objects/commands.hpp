#ifndef SRC_BASE_OBJECTS_COMMANDS
#define SRC_BASE_OBJECTS_COMMANDS
#include <functional>
#include <library/enbt.hpp>
#include <library/list_array.hpp>
#include <optional>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/parsers.hpp>
#include <src/base_objects/permissions.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <string>
#include <unordered_map>
#include <variant>

namespace copper_server {
    namespace base_objects {
        struct command_exception {
            std::exception_ptr exception;
            size_t pos = 0;
            command_exception(std::exception_ptr exception, size_t pos)
                : exception(exception), pos(pos) {}
        };

        struct command_context {
            client_data_holder& executor;
            enbt::compound other_data;
            //for player, position, rotation, motion, and world_id automatically copied to other_data
            //command result must be set in other_data at "result" value

            void apply_executor_data();

            command_context(client_data_holder& executor, bool apply_data = true)
                : executor(executor) {
                if (apply_data)
                    apply_executor_data();
            }
        };

        struct command;
        using command_callback = std::function<void(const list_array<parser>&, command_context&)>;

        using command_redirect = std::function<void(command& target, const list_array<parser>&, const std::string&, command_context&)>;


        using command_suggestion = std::function<list_array<std::string>(const std::string& current, command_context&)>;

        struct action_provider {
            std::string action_tag;
            list_array<std::string> required_permissions_tag;
            action_provider(const char* tag);
            action_provider(const std::string& tag);
            action_provider(std::string&& tag);
            action_provider(const std::string& tag, const list_array<std::string>& requirement);
            action_provider(std::string&& tag, const list_array<std::string>& requirement);
            action_provider(const std::string& tag, list_array<std::string>&& requirement);
            action_provider(std::string&& tag, list_array<std::string>&& requirement);
        };

        struct redirect_command {
            command_redirect redirect_routine;
            int32_t target_command;
        };

        struct command {
            std::string name;
            std::string description;
            std::string usage;
            std::optional<command_parser> argument_predicate;
            std::optional<command_callback> executable;
            std::optional<redirect_command> redirect;
            std::variant<command_suggestion, std::string> suggestions;

            list_array<int32_t> childs;
            std::unordered_map<std::string, int32_t> childs_cache;
            std::string action_name;
            uint32_t links = 0;

            using parsers = packets::command_node::parsers;

            bool is_named_suggestion() const {
                return std::visit(
                    [&](auto& it) -> bool {
                        return std::is_same_v<std::decay_t<decltype(it)>, std::string>;
                    },
                    suggestions
                );
            }

            bool is_custom_suggestion() const {
                return std::visit(
                    [&](auto& it) {
                        return std::is_same_v<std::decay_t<decltype(it)>, command_suggestion>;
                    },
                    suggestions
                );
            }

            bool has_suggestion() const {
                return std::visit(
                    [&](auto& it) {
                        if constexpr (std::is_same_v<std::decay_t<decltype(it)>, command_suggestion>)
                            return (bool)it;
                        else
                            return true;
                    },
                    suggestions
                );
            }

            const std::string& get_named_suggestion() const {
                return std::visit(
                    [&](auto& it) -> const std::string& {
                        if constexpr (std::is_same_v<std::decay_t<decltype(it)>, std::string>)
                            return it;
                        else
                            throw std::runtime_error("Invalid type");
                    },
                    suggestions
                );
            }

            const command_suggestion& get_custom_suggestion() const {
                return std::visit(
                    [&](auto& it) -> const command_suggestion& {
                        if constexpr (std::is_same_v<std::decay_t<decltype(it)>, command_suggestion>)
                            return it;
                        else
                            throw std::runtime_error("Invalid type");
                    },
                    suggestions
                );
            }

            int32_t get_child(list_array<command>& commands_nodes, const std::string& name);

            command(const char* name)
                : name(name), suggestions("") {}

            command(
                const std::string& name = "",
                const std::string& description = "",
                const std::string& usage = "",
                const std::optional<command_parser>& argument_predicate = std::nullopt,
                const std::optional<command_callback>& executable = std::nullopt,
                const std::optional<redirect_command>& redirect = std::nullopt,
                const std::variant<command_suggestion, std::string>& suggestions = ""
            )
                : name(name),
                  description(description),
                  usage(usage),
                  argument_predicate(argument_predicate),
                  executable(executable),
                  redirect(redirect),
                  suggestions(suggestions) {}
        };

        class command_custom_parser {
        public:
            std::vector<parser> native_predicates;
            command_suggestion suggestions_provider;

            virtual parsers::custom_virtual parse(parsers::command::custom_virtual& cfg, std::string& part, std::string& path) = 0;
            virtual std::string name() = 0;
        };

        using named_suggestion_provider = std::function<list_array<std::string>(command& cmd, const std::string& name, const std::string& current, command_context&)>;

        class command_manager {
            std::unordered_map<std::string, std::shared_ptr<command_custom_parser>> custom_parsers;
            std::unordered_map<std::string, named_suggestion_provider> named_suggestion_providers = {
                {"minecraft:ask_server",
                 named_suggestion_provider([](command& cmd, const std::string&, const std::string&, command_context&) -> list_array<std::string> {
                     return {cmd.name};
                 })}
            };
            list_array<command> command_nodes;
            list_array<uint8_t> graph_cache;
            bool graph_ready;
            void remove(size_t id);

        public:
            friend class command_browser;
            friend class command_root_browser;
            command_manager();
            command_manager(const command_manager&) = delete;
            command_manager(command_manager&&) = delete;

            //minecraft:ask_server already registered to prevent stack overflow and cannot be redefined
            void register_named_suggestion_provider(const std::string& name, const named_suggestion_provider& provider);
            void remove_named_suggestion_provider(const std::string& name);


            void register_parser(const std::shared_ptr<command_custom_parser>& parser);
            command_custom_parser& get_parser(const std::string& name);
            void unregister_parser(const std::string& name);

            void execute_command(const std::string& command_string, command_context&);
            void execute_command_from(const std::string& command_string, command& cmd, command_context&);
            list_array<std::string> request_suggestions(const std::string& command, command_context&);

            const list_array<uint8_t>& compile_to_graph();
            bool is_graph_fresh() const;

            bool belongs(command* command);

            //every command refrence after this command become invalid, even when created by plugins in OnCommandsLoad, bc. of the commit command for optimization
            void reload_commands();


            std::optional<parser> parse_string(command_parser&& config, const std::string& string);
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
            command_browser add_child(command&& command, command_parser pred);
            command_browser add_child(command_browser& command);
            bool remove_child(const std::string& name);
            list_array<command_browser> get_childs();

            command_browser& set_redirect(const std::string& path, command_redirect redirect);
            command_browser& remove_redirect();

            command_browser& set_argument_type(const command_parser& pred);
            command_browser& remove_argument();

            command_browser& set_callback(const action_provider& action, const command_callback& callback);

            command_browser& set_callback(action_provider&& action, const command_callback& callback) {
                return set_callback(action, callback);
            }

            command_browser& remove_callback();


            command_browser& set_suggestion(const std::string& suggestion_type);
            command_browser& set_suggestion_callback(const command_suggestion& suggestion);
            command_browser& remove_suggestion();

            command_browser& modify_command(const command& _command);
            command_browser& modify_command(command&& _command);

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
            bool remove_child(const std::string& name);

            list_array<command_browser> get_childs();

            command_browser open(const std::string& path) const;
            std::string get_documentation() const;

            command_manager& get_manager() const;
        };

    } // namespace base_objects

} // namespace copper_server


#endif /* SRC_BASE_OBJECTS_COMMANDS */
