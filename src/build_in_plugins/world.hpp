#ifndef SRC_BUILD_IN_PLUGINS_WORLD
#define SRC_BUILD_IN_PLUGINS_WORLD
#include "../api/players.hpp"
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/registration.hpp"
#include "../storage/world_data.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class WorldManagementPlugin : public PluginRegistration {
            storage::worlds_data worlds_storage;

            void add_world_id_suggestion(base_objects::command_browser& browser) {
                browser.set_suggestion_callback([this](const std::string& current, base_objects::client_data_holder& client) {
                    auto suggestions = worlds_storage.get_list().convert<std::string>([](uint64_t id) {
                        return std::to_string(id);
                    });
                    if (current.empty())
                        return suggestions;
                    else {
                        return suggestions.where([&current](const std::string& suggestion) { return suggestion.starts_with(current); });
                    }
                });
            }


        public:
            WorldManagementPlugin()
                : worlds_storage(Server::instance().config, Server::instance().config.server.base_path / Server::instance().config.server.storage_folder) {
            }

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
                auto worlds = browser.add_child({"worlds"});
                {
                    auto create = worlds.add_child({"create"});
                }
                {
                    auto world_id = worlds.add_child({"remove"}).add_child({"<world_id>"});
                    world_id.set_callback("command.world.remove", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                        uint64_t id;
                        try {
                            id = std::stoull(args[0]);
                        } catch (...) {
                            Chat message("Failed to set world id, invalid argument: " + args[0]);
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        }
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to set world id, world with this id not set: " + args[0]);
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else
                            worlds_storage.erase(id);
                    });
                    add_world_id_suggestion(world_id);
                }
                {
                    auto base = worlds.add_child({"base"});
                    {
                        auto set = base.add_child({"set"}).add_child({"<world_id>"});
                        set.set_callback("command.world.base.set", [this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                            uint64_t id;
                            try {
                                id = std::stoull(args[0]);
                            } catch (...) {
                                Chat message("Failed to set world id, invalid argument: " + args[0]);
                                message.SetColor("red");
                                api::players::calls::on_system_message({client, message});
                            }
                            if (!worlds_storage.exists(id)) {
                                Chat message("Failed to set world id, world with this id not set: " + args[0]);
                                message.SetColor("red");
                                api::players::calls::on_system_message({client, message});
                            } else
                                worlds_storage.base_world_id = id;
                        });
                        add_world_id_suggestion(set);
                    }
                    base.set_callback("command.world.base", [this](const list_array<std::string>&, base_objects::client_data_holder& client) {
                        if (worlds_storage.base_world_id == -1)
                            api::players::calls::on_system_message({client, {"Base world not set."}});
                        else
                            api::players::calls::on_system_message({client, {"Base world is: " + worlds_storage.get(worlds_storage.base_world_id)->world_name}});
                    });
                }
            }
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_WORLD */
