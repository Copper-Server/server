#include "commands.hpp"
#include "../library/list_array.hpp"
#include "../protocolHelper/util.hpp"
#include "packets.hpp"


namespace crafted_craft {
    namespace base_objects {
        action_provider::action_provider(const std::string& tag)
            : action_tag(tag) {}

        action_provider::action_provider(const std::string&& tag)
            : action_tag(std::move(tag)) {}

        action_provider::action_provider(const std::string& tag, const list_array<shared_string>& requirement)
            : action_tag(tag), required_permissions_tag(requirement) {}

        action_provider::action_provider(const std::string&& tag, const list_array<shared_string>& requirement)
            : action_tag(std::move(tag)), required_permissions_tag(requirement) {}

        action_provider::action_provider(const std::string& tag, list_array<shared_string>&& requirement)
            : action_tag(tag), required_permissions_tag(std::move(requirement)) {}

        action_provider::action_provider(const std::string&& tag, list_array<shared_string>&& requirement)
            : action_tag(std::move(tag)), required_permissions_tag(std::move(requirement)) {}

        int32_t command::get_child(list_array<command>& command_nodes, const std::string& name) {
            if (childs_cache.size() == node.children.size()) {
                auto it = childs_cache.find(name);
                if (it == childs_cache.end())
                    return -1;
                if (command_nodes[it->second].node.name != name) {
                    childs_cache.clear();
                    return get_child(command_nodes, name);
                } else
                    return it->second;
            } else {
                childs_cache.clear();
                for (size_t i = 0; i < node.children.size(); i++) {
                    auto& check_name = command_nodes[node.children[i]].node.name;
                    if (!check_name)
                        throw std::invalid_argument("invalid command children tree, empty name found");
                    if (childs_cache.find(*check_name) != childs_cache.end())
                        throw std::invalid_argument("invalid command children tree, duplicate name found");
                    childs_cache[*check_name] = i;
                }
                return get_child(command_nodes, name);
            }
        }

        command::command(const std::string& name, const std::string& description, const std::string& usage, const std::string& action_name)
            : description(description), usage(usage), action_name(action_name) {
            node.name = name;
            node.flags.node_type = packets::command_node::node_type::literal;
        }

        int32_t get_index(list_array<command>& command_nodes, const std::string& path, int32_t current_ = 0) {
            if (current_ < 0)
                throw std::invalid_argument("invalid command id");
            auto real_path =
                list_array<char>(path)
                    .split_by(' ')
                    .convert<std::string>([](const list_array<char>& a) {
                        return std::string(a.data(), a.size());
                    });

            if (real_path.size() == 0) {
                return current_;
            } else {
                int32_t current = current_;
                for (auto& part : real_path) {
                    current = command_nodes[current].get_child(command_nodes, part);
                    if (current == -1)
                        throw std::invalid_argument("invalid command path");
                }
                return current;
            }
        }

        void apply_options(packets::command_node::parsers parser, packets::command_node::properties_t& parser_data) {
            switch (parser) {
            case packets::command_node::parsers::brigadier_float:
            case packets::command_node::parsers::brigadier_double:
            case packets::command_node::parsers::brigadier_integer:
            case packets::command_node::parsers::brigadier_long: {
                parser_data.flags = 0;
                if (parser_data.min)
                    *parser_data.flags |= 1;
                if (parser_data.max)
                    *parser_data.flags |= 2;
                parser_data.registry = std::nullopt;
                break;
            }
            case packets::command_node::parsers::brigadier_string: {
                if (!parser_data.flags)
                    parser_data.flags = 0;
                parser_data.min = std::nullopt;
                parser_data.max = std::nullopt;
                parser_data.registry = std::nullopt;
                break;
            }
            case packets::command_node::parsers::minecraft_entity: {
                if (!parser_data.flags)
                    parser_data.flags = 0;
                parser_data.min = std::nullopt;
                parser_data.max = std::nullopt;
                parser_data.registry = std::nullopt;
                break;
            }
            case packets::command_node::parsers::minecraft_score_holder: {
                if (!parser_data.flags)
                    parser_data.flags = 0;
                parser_data.min = std::nullopt;
                parser_data.max = std::nullopt;
                parser_data.registry = std::nullopt;
                break;
            }
            case packets::command_node::parsers::minecraft_time: {
                if (!parser_data.min)
                    parser_data.min = std::numeric_limits<int32_t>::max();
                parser_data.flags = std::nullopt;
                parser_data.max = std::nullopt;
                parser_data.registry = std::nullopt;
                break;
            }
            case packets::command_node::parsers::minecraft_resource_or_tag:
            case packets::command_node::parsers::minecraft_resource_or_tag_key:
            case packets::command_node::parsers::minecraft_resource:
            case packets::command_node::parsers::minecraft_resource_key: {
                if (!parser_data.registry)
                    throw std::invalid_argument("this parser should have registry");
                parser_data.flags = std::nullopt;
                parser_data.min = std::nullopt;
                parser_data.max = std::nullopt;
                break;
            }
            default:
                parser_data.flags = std::nullopt;
                parser_data.min = std::nullopt;
                parser_data.max = std::nullopt;
                parser_data.registry = std::nullopt;
                break;
            }
        }


        int32_t get_index(const list_array<command>& command_nodes, const std::string& path, int32_t current_ = 0) {
            return get_index(const_cast<list_array<command>&>(command_nodes), path, current_);
        }

        std::string parse_quoted_string(std::string& part, std::string& path) {
            if (part[0] == '"') {
                //allow \" in string
                std::string phrase = part + ' ' + path;
                auto pos = phrase.find('"', 1);
                while (pos != std::string::npos && phrase[pos - 1] == '\\')
                    pos = phrase.find('"', pos + 1);

                if (pos == std::string::npos)
                    throw std::invalid_argument("invalid quoted string");
                part = phrase.substr(1, pos - 1);
                bool has_space = phrase.find(' ', pos + 1) != std::string::npos;
                path = phrase.substr(pos + 1 + has_space);
                return part;
            } else
                return part;
        }

        std::optional<std::string> parse_argument(command& current, std::string& part, std::string& path) {
            auto node = current.node;

            auto parser = *node.parser_id;
            packets::command_node::properties_t default_prop;
            auto& parser_data = node.properties ? *node.properties : default_prop;
            switch (parser) {
            case packets::command_node::parsers::brigadier_bool: {
                if (part == "true" || part == "false")
                    return part;
                else
                    return std::nullopt;
            }
            case packets::command_node::parsers::brigadier_float: {
                float value = std::stof(part);
                uint8_t flags = parser_data.flags.value_or(0);
                if (flags & 1)
                    if (value < std::get<float>(parser_data.min.value_or(std::numeric_limits<float>::min())))
                        return std::nullopt;
                if (flags & 2)
                    if (value > std::get<float>(parser_data.max.value_or(std::numeric_limits<float>::max())))
                        return std::nullopt;
                return std::to_string(value);
            }
            case packets::command_node::parsers::brigadier_double: {
                double value = std::stod(part);
                uint8_t flags = parser_data.flags.value_or(0);
                if (flags & 1)
                    if (value < std::get<double>(parser_data.min.value_or(std::numeric_limits<double>::min())))
                        return std::nullopt;
                if (flags & 2)
                    if (value > std::get<double>(parser_data.max.value_or(std::numeric_limits<double>::max())))
                        return std::nullopt;
                return std::to_string(value);
            }
            case packets::command_node::parsers::brigadier_integer: {
                int32_t value = std::stoi(part);
                uint8_t flags = parser_data.flags.value_or(0);
                if (flags & 1)
                    if (value < std::get<int32_t>(parser_data.min.value_or(std::numeric_limits<int32_t>::min())))
                        return std::nullopt;
                if (flags & 2)
                    if (value > std::get<int32_t>(parser_data.max.value_or(std::numeric_limits<int32_t>::max())))
                        return std::nullopt;
                return std::to_string(value);
            }
            case packets::command_node::parsers::brigadier_long: {
                int64_t value = std::stoll(part);
                uint8_t flags = parser_data.flags.value_or(0);
                if (flags & 1)
                    if (value < std::get<int64_t>(parser_data.min.value_or(std::numeric_limits<int64_t>::min())))
                        return std::nullopt;
                if (flags & 2)
                    if (value > std::get<int64_t>(parser_data.max.value_or(std::numeric_limits<int64_t>::max())))
                        return std::nullopt;
                return std::to_string(value);
            }
            case packets::command_node::parsers::brigadier_string: {
                switch (parser_data.flags.value_or(0)) {
                default:
                case 0: //SINGLE_WORD
                    return part;
                    break;
                case 1: //QUOTABLE_PHRASE
                    if (part[0] == '"')
                        return parse_quoted_string(part, path);
                    else
                        return part;
                    break;
                case 2: { //GREEDY_PHRASE
                    auto phrase = part + ' ' + path;
                    path.clear();
                    return phrase;
                }
                }
            }
            default:
                return std::nullopt;
            }
        }

        std::optional<std::string> find_argument_no_except(list_array<command>& command_nodes, command*& command, std::string& string, std::string& rest) {
            for (auto child_id : command->node.children) {
                auto& child = command_nodes[child_id];
                if (child.node.flags.node_type == packets::command_node::node_type::argument) {
                    auto res = parse_argument(child, string, rest);
                    if (res) {
                        command = &child;
                        return res;
                    }
                }
            }
            return std::nullopt;
        }

        std::string find_argument(list_array<command>& command_nodes, command*& command, std::string& string, std::string& rest) {
            auto res = find_argument_no_except(command_nodes, command, string, rest);
            if (res)
                return *res;
            else
                throw std::invalid_argument("invalid argument");
        }

        command_manager::command_manager() {
            command_nodes.push_back({});
            graph_ready = false;
        }

        void command_manager::execute_command(const std::string& command_string, client_data_holder& data) {
            std::string path = command_string;
            list_array<std::string> args;
            command* current = &command_nodes[0];

            while (path.size() > 0 || !current->node.flags.has_redirect) {
                auto split = path.find(' ');
                std::string part;
                if (split == std::string::npos) {
                    std::swap(part, path);
                } else {
                    part = path.substr(0, split);
                    path = path.substr(split + 1);
                }

                if (part.empty())
                    break;

                int32_t child = current->get_child(command_nodes, part);
                if (child == -1)
                    args.push_back(find_argument(command_nodes, current, part, path));
                else {
                    auto& command = command_nodes[child];
                    if (command.node.flags.node_type == packets::command_node::node_type::argument)
                        args.push_back(find_argument(command_nodes, current, part, path));
                    else
                        current = &command_nodes[child];

                    if (!current->node.flags.has_redirect)
                        continue;
                }
            }
            auto& callback = current->callback;
            if (current->node.flags.has_redirect) {
                if (current->redirect_command)
                    return current->redirect_command(args, path, data);
                else
                    throw std::invalid_argument("internal server error, invalid command structure, report to admin, NO_REDIRECT");
            } else {
                if (current->node.flags.is_executable) {
                    if (callback) {
                        if (Server::instance().permissions_manager.has_rights(current->action_name, data))
                            return callback(args, data);
                        else
                            throw std::exception("Not enough permissions for this.");
                    } else
                        throw std::invalid_argument("internal server error, invalid command structure, report to admin, NO_CALLBACK");
                } else
                    throw std::invalid_argument("command is not executable");
            }
        }

        bool has_accessible_callbacks_child(command* current, list_array<command>& command_nodes, client_data_holder& data) {
            return current->node.children.contains_one([&](int32_t id) {
                auto& command = command_nodes[id];
                if (command.callback)
                    return Server::instance().permissions_manager.has_rights(command.action_name, data);
                else
                    return has_accessible_callbacks_child(&command, command_nodes, data);
            });
        }

        list_array<std::string> extract_suggestions(command* current, list_array<command>& command_nodes, const std::string& part, client_data_holder& data) {
            list_array<std::string> suggestions;
            current->node.children.for_each([&](int32_t id) {
                auto& command = command_nodes[id];
                if (!has_accessible_callbacks_child(&command, command_nodes, data))
                    return;
                if (command.callback)
                    if (!Server::instance().permissions_manager.has_rights(command.action_name, data))
                        return;

                auto res = command.node.name.value_or("");
                if (res.starts_with(part)) {
                    if (command.suggestions)
                        suggestions.push_back(command.suggestions(part, data));
                    else
                        suggestions.push_back(res);
                }
            });
            return suggestions;
        }

        list_array<std::string> command_manager::request_suggestions(const std::string& command_string, client_data_holder& data) {
            bool ends_with_space = command_string.ends_with(' ');
            std::string path = command_string;
            list_array<std::string> args;
            command* current = &command_nodes[0];

            while (path.size() > 0 || !current->node.flags.has_redirect) {
                auto split = path.find(' ');
                std::string part;
                if (split == std::string::npos) {
                    std::swap(part, path);
                } else {
                    part = path.substr(0, split);
                    path = path.substr(split + 1);
                }

                try {
                    if (part.empty())
                        return extract_suggestions(current, command_nodes, part, data);

                    int32_t child = current->get_child(command_nodes, part);
                    if (child == -1) {
                        auto res = find_argument_no_except(command_nodes, current, part, path);
                        if (res)
                            args.push_back(std::move(*res));
                        else
                            return extract_suggestions(current, command_nodes, part, data);
                    } else {
                        auto& command = command_nodes[child];
                        if (command.node.flags.node_type == packets::command_node::node_type::argument) {
                            auto res = find_argument_no_except(command_nodes, current, part, path);
                            if (res)
                                args.push_back(std::move(*res));
                            else
                                return extract_suggestions(current, command_nodes, part, data);
                        } else {
                            current = &command_nodes[child];
                            if (path.empty() && !ends_with_space)
                                return extract_suggestions(current, command_nodes, path, data).transform([&](std::string&& s) {
                                    s = part + ' ' + s;
                                    return s;
                                });
                        }
                        if (!current->node.flags.has_redirect)
                            continue;
                    }
                } catch (...) {
                    return {};
                }
            }
            return {};
        }

        void check_properties(const command& command) {
            auto& node = command.node;
            auto& parser = *node.parser_id;
            auto& parser_data = node.properties;
            switch (parser) {
            case packets::command_node::parsers::brigadier_bool: {
                if (parser_data)
                    throw std::invalid_argument("this parser should not have properties");
                break;
            }
            case packets::command_node::parsers::brigadier_float:
            case packets::command_node::parsers::brigadier_double:
            case packets::command_node::parsers::brigadier_integer:
            case packets::command_node::parsers::brigadier_long: {
                if (!parser_data)
                    throw std::invalid_argument("this parser should have properties");
                if (!parser_data->flags)
                    throw std::invalid_argument("this parser should have flags");

                if (*parser_data->flags & 1) {
                    if (!parser_data->min)
                        throw std::invalid_argument("this parser should have min");
                } else if (parser_data->min)
                    throw std::invalid_argument("this parser should not have min");

                if (*parser_data->flags & 2) {
                    if (!parser_data->max)
                        throw std::invalid_argument("this parser should have max");
                } else if (parser_data->max)
                    throw std::invalid_argument("this parser should not have max");
                if (parser_data->registry)
                    throw std::invalid_argument("this parser should not have registry");
                break;
            }
            case packets::command_node::parsers::brigadier_string: {
                if (!parser_data)
                    throw std::invalid_argument("this parser should have properties");
                if (!parser_data->flags)
                    throw std::invalid_argument("this parser should have flags");
                if (parser_data->max || parser_data->min || parser_data->registry)
                    throw std::invalid_argument("this parser should not have max, min or registry");
                break;
            }
            case packets::command_node::parsers::minecraft_entity: {
                if (!parser_data)
                    throw std::invalid_argument("this parser should have properties");
                if (!parser_data->flags)
                    throw std::invalid_argument("this parser should have flags");
                if (parser_data->max || parser_data->min || parser_data->registry)
                    throw std::invalid_argument("this parser should not have max, min or registry");
                break;
            }
            case packets::command_node::parsers::minecraft_score_holder: {
                if (!parser_data)
                    throw std::invalid_argument("this parser should have properties");
                if (!parser_data->flags)
                    throw std::invalid_argument("this parser should have flags");
                if (parser_data->max || parser_data->min || parser_data->registry)
                    throw std::invalid_argument("this parser should not have max, min or registry");
                break;
            }
            case packets::command_node::parsers::minecraft_time: {
                if (!parser_data)
                    throw std::invalid_argument("this parser should have properties");
                if (!parser_data->min)
                    throw std::invalid_argument("this parser should have min");
                if (parser_data->max || parser_data->registry || parser_data->flags)
                    throw std::invalid_argument("this parser should not have max, registry or flags");
                break;
            }
            case packets::command_node::parsers::minecraft_resource_or_tag:
            case packets::command_node::parsers::minecraft_resource_or_tag_key:
            case packets::command_node::parsers::minecraft_resource:
            case packets::command_node::parsers::minecraft_resource_key: {
                if (!parser_data)
                    throw std::invalid_argument("this parser should have properties");
                if (!parser_data->registry)
                    throw std::invalid_argument("this parser should have registry");
                if (parser_data->max || parser_data->min || parser_data->flags)
                    throw std::invalid_argument("this parser should not have max, min or flags");
                break;
            }
            default:
                if (parser_data)
                    throw std::invalid_argument("this parser should not have properties");
                break;
            }
        }

        const list_array<uint8_t>& command_manager::compile_to_graph() {
            if (graph_ready)
                return graph_cache;
            list_array<uint8_t> res;
            for (auto& command : command_nodes) {
                auto& node = command.node;
                res.push_back((uint8_t)node.flags.raw);
                res.push_back((uint8_t)node.children.size());
                for (auto& child : node.children)
                    WriteVar<int32_t>(child, res);
                if (node.flags.has_redirect)
                    WriteVar<int32_t>(*node.redirect_node, res);
                if (node.flags.node_type != packets::command_node::node_type::root)
                    WriteString(res, *node.name, 32767);
                if (node.flags.node_type == packets::command_node::node_type::argument) {
                    check_properties(command);
                    WriteVar<int32_t>((int32_t)*node.parser_id, res);
                    if (node.properties) {
                        auto& parser_data = *node.properties;
                        if (parser_data.flags)
                            WriteVar<int32_t>(*parser_data.flags, res);
                        if (parser_data.min)
                            std::visit([&res](auto val) { WriteValue<decltype(val)>(val, res); }, *parser_data.min);
                        if (parser_data.max)
                            std::visit([&res](auto val) { WriteValue<decltype(val)>(val, res); }, *parser_data.max);
                        if (parser_data.registry)
                            WriteIdentifier(res, *parser_data.registry);
                    }
                }
                if (node.flags.has_suggestion)
                    WriteIdentifier(res, *node.suggestion_type);
            }
            res.commit();
            graph_ready = true;
            return graph_cache = res;
        }

        bool command_manager::is_graph_fresh() const {
            return graph_ready;
        }

        bool command_manager::belongs(command* _command) {
            return command_nodes.contains_one([_command](const command& c) { return &c == _command; });
        }

        void command_manager::reload_commands() {
            command_nodes.clear();
            graph_ready = false;
            command_nodes.push_back({});
            command_root_browser browser(*this);
            for (auto& plugin : pluginManagement.registeredPlugins())
                plugin->OnCommandsLoad(plugin, browser);

            for (auto& plugin : pluginManagement.registeredPlugins())
                plugin->OnCommandsLoadComplete(plugin, browser);

            command_nodes.commit();
        }

        command_root_browser::command_root_browser(command_manager& manager)
            : manager(manager) {}

        command_browser command_root_browser::add_child(command&& command) {
            if (manager.command_nodes.size() >= INT32_MAX)
                throw std::runtime_error("command nodes limit reached");


            auto& root = manager.command_nodes[0];

            if (root.childs_cache.find(*command.node.name) != root.childs_cache.end())
                throw std::invalid_argument("This command already defined");

            root.node.children.push_back(manager.command_nodes.size());
            root.childs_cache[*command.node.name] = manager.command_nodes.size();
            manager.command_nodes.push_back(command);
            manager.graph_ready = false;
            return command_browser(manager, int32_t(manager.command_nodes.size() - 1));
        }

        list_array<command_browser> command_root_browser::get_childs() {
            list_array<command_browser> res;
            for (auto& child : manager.command_nodes[0].node.children)
                res.push_back(command_browser(manager, child));
            return res;
        }

        command_browser command_root_browser::open(const std::string& path) const {
            return command_browser(manager, path);
        }

        std::string command_root_browser::get_documentation() const {
            std::string res;
            for (auto& child : manager.command_nodes[0].node.children)
                res += command_browser(manager, child).get_documentation() + "\n\n";
            return res;
        }

        command_manager& command_root_browser::get_manager() const {
            return manager;
        }

        command_browser command_browser::open(const std::string& path) const {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");

            return command_browser(manager, get_index(manager.command_nodes, path, current_id));
        }

        std::string command_browser::get_documentation() const {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            std::string res = *current_command.node.name;
            if (current_command.description.size() > 0)
                res += " - " + current_command.description;

            for (auto& child : current_command.node.children) {
                command_browser browser(manager, child);
                std::string child_modify = browser.get_documentation();
                if (child_modify.size() > 0) {
                    auto cmd = browser.look_up();
                    res.reserve(child_modify.size() + 4 + cmd.node.name->size());
                    res.push_back('\n');
                    res.push_back('\t');
                    for (auto& ch : child_modify) {
                        res.push_back(ch);
                        if (ch == '\n')
                            res.push_back('\t');
                    }
                    if (res.back() == '\t')
                        res.pop_back();
                    if (res.back() == '\n')
                        res.pop_back();
                }
            }
            return res;
        }

        const command& command_browser::look_up() {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            return current_command;
        }

        command_browser::command_browser(command_manager& manager, int32_t id)
            : manager(manager), current_id(id), current_command(manager.command_nodes[id]) {}

        command_browser::command_browser(command_manager& manager, const std::string& path)
            : manager(manager),
              current_command(manager.command_nodes[current_id = get_index(manager.command_nodes, path)]) {}

        command_browser::command_browser(command_browser& browser, const std::string& path)
            : manager(manager),
              current_command(manager.command_nodes[current_id = get_index(manager.command_nodes, path, browser.current_id)]) {}

        command_browser::command_browser(command_browser&& browser) noexcept
            : manager(browser.manager),
              current_id(browser.current_id),
              current_command(browser.current_command) {}

        command_browser command_browser::add_child(command&& command, packets::command_node::parsers parser, packets::command_node::properties_t properties) {
            command_browser browser = add_child(std::move(command));
            browser.current_command.node.flags.node_type = packets::command_node::node_type::argument;
            browser.current_command.node.parser_id = parser;
            apply_options(parser, properties);
            browser.current_command.node.properties = properties;
            return browser;
        }

        command_browser command_browser::add_child(command&& command) {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");

            if (manager.command_nodes.size() >= INT32_MAX)
                throw std::runtime_error("command nodes limit reached");

            if (current_command.childs_cache.find(*command.node.name) != current_command.childs_cache.end())
                throw std::invalid_argument("This command already defined");

            current_command.node.children.push_back(manager.command_nodes.size());
            current_command.childs_cache[*command.node.name] = manager.command_nodes.size();
            manager.command_nodes.push_back(command);
            manager.graph_ready = false;


            return command_browser(manager, int32_t(manager.command_nodes.size() - 1));
        }

        command_browser command_browser::add_child(command_browser& command) {
            if (&command.manager != &manager)
                throw std::invalid_argument("command_browser from different manager");
            if (!is_valid() && !command.is_valid())
                throw std::runtime_error("command has been deleted");

            if (manager.command_nodes.size() >= INT32_MAX)
                throw std::runtime_error("command nodes limit reached");

            if (current_command.childs_cache.find(*command.current_command.node.name) != current_command.childs_cache.end())
                throw std::invalid_argument("This command already defined");

            current_command.node.children.push_back(command.current_id);
            current_command.childs_cache[*command.current_command.node.name] = command.current_id;
            manager.graph_ready = false;


            return command_browser(manager, int32_t(manager.command_nodes.size() - 1));
        }

        list_array<command_browser> command_browser::get_childs() {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");

            list_array<command_browser> res;
            for (auto& child : current_command.node.children)
                res.push_back(command_browser(manager, child));
            return res;
        }

        command_browser& command_browser::set_redirect(const std::string& path, const command_redirect& redirect) {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");

            auto redirect_index = get_index(manager.command_nodes, path);
            if (current_id == redirect_index)
                throw std::invalid_argument("cannot redirect to itself");

            auto& command_ = manager.command_nodes[current_id];
            auto& target_node = command_.node;
            target_node.flags.has_redirect = true;
            target_node.redirect_node = redirect_index;
            command_.redirect_command = redirect;
            manager.graph_ready = false;
            return *this;
        }

        command_browser& command_browser::remove_redirect() {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");

            auto& target_node = current_command.node;
            target_node.flags.has_redirect = false;
            target_node.redirect_node = std::nullopt;
            current_command.redirect_command = {};
            manager.graph_ready = false;
            return *this;
        }

        void apply_action_command(command& current_command, const action_provider& action) {
            if (!action.action_tag.empty()) {
                current_command.action_name = action.action_tag;
                auto& permissions_manager =
                    Server::instance()
                        .permissions_manager;
                if (!permissions_manager.has_action(action.action_tag))
                    permissions_manager.register_action(action.action_tag, action.required_permissions_tag);
            }
        }

        void apply_action_command(command& current_command, action_provider&& action) {
            if (!action.action_tag.empty()) {
                current_command.action_name = action.action_tag;
                auto& permissions_manager =
                    Server::instance()
                        .permissions_manager;
                if (!permissions_manager.has_action(action.action_tag))
                    permissions_manager.register_action(action.action_tag, std::move(action.required_permissions_tag));
            }
        }

        command_browser& command_browser::set_callback(const std::string& action, const command_callback& callback) {
            return set_callback(action_provider{action}, callback);
        }

        command_browser& command_browser::set_callback(const std::string& action, const command_callback& callback, packets::command_node::parsers parser, packets::command_node::properties_t properties) {
            return set_callback(action_provider{action}, callback, parser, properties);
        }

        command_browser& command_browser::set_callback(const action_provider& action, const command_callback& callback) {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            if (current_command.callback)
                throw std::runtime_error("This command already has callback");


            apply_action_command(current_command, action);
            current_command.node.flags.is_executable = true;
            current_command.callback = callback;
            manager.graph_ready = false;
            return *this;
        }

        command_browser& command_browser::set_callback(const action_provider& action, const command_callback& callback, packets::command_node::parsers parser, packets::command_node::properties_t properties) {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            if (current_command.callback)
                throw std::runtime_error("This command already has callback");
            apply_action_command(current_command, action);

            current_command.node.flags.is_executable = true;
            current_command.callback = callback;
            current_command.node.parser_id = parser;
            apply_options(parser, properties);
            current_command.node.properties = properties.flags || properties.max || properties.min || properties.registry ? std::optional(properties) : std::nullopt;
            manager.graph_ready = false;
            return *this;
        }

        command_browser& command_browser::set_callback(action_provider&& action, const command_callback& callback) {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            if (current_command.callback)
                throw std::runtime_error("This command already has callback");


            apply_action_command(current_command, std::move(action));
            current_command.node.flags.is_executable = true;
            current_command.callback = callback;
            manager.graph_ready = false;
            return *this;
        }

        command_browser& command_browser::set_callback(action_provider&& action, const command_callback& callback, packets::command_node::parsers parser, packets::command_node::properties_t properties) {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            if (current_command.callback)
                throw std::runtime_error("This command already has callback");
            apply_action_command(current_command, std::move(action));

            current_command.node.flags.is_executable = true;
            current_command.callback = callback;
            current_command.node.parser_id = parser;
            apply_options(parser, properties);
            current_command.node.properties = properties.flags || properties.max || properties.min || properties.registry ? std::optional(properties) : std::nullopt;
            manager.graph_ready = false;
            return *this;
        }

        command_browser& command_browser::remove_callback() {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            current_command.node.flags.is_executable = false;
            current_command.callback = {};
            manager.graph_ready = false;
            return *this;
        }

        command_browser& command_browser::set_suggestion(const std::string& suggestion_type) {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            current_command.node.flags.has_suggestion = true;
            current_command.node.suggestion_type = suggestion_type;
            current_command.suggestions = {};
            manager.graph_ready = false;
            return *this;
        }

        command_browser& command_browser::set_suggestion_callback(const command_suggestion& suggestions) {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            current_command.node.flags.has_suggestion = true;
            current_command.node.suggestion_type = "minecraft:ask_server";
            current_command.suggestions = suggestions;
            manager.graph_ready = false;
            return *this;
        }

        command_browser& command_browser::remove_suggestion() {
            if (!is_valid())
                throw std::runtime_error("command has been deleted");
            current_command.node.flags.has_suggestion = false;
            current_command.node.suggestion_type = std::nullopt;
            current_command.suggestions = {};
            manager.graph_ready = false;
            return *this;
        }


        command_browser& command_browser::modify_command(const command& _command) {
            return modify_command(command(_command));
        }

        command_browser& command_browser::modify_command(command&& _command) {
            if (&current_command == &_command)
                return *this;
            if (!is_valid())
                throw std::runtime_error("command has been deleted");


            auto old_childs = manager.command_nodes[current_id].node.children;
            auto& new_command = manager.command_nodes[current_id] = std::move(_command);
            new_command.node.children = old_childs;
            manager.graph_ready = false;

            *reinterpret_cast<command**>(&current_command) = &new_command;
            return *this;
        }

        bool command_browser::is_valid() const {
            if (current_id == -1)
                return false;
            if (manager.command_nodes.size() < current_id)
                return false;
            return manager.belongs(&current_command);
        }

    }

} // namespace crafted_craft
