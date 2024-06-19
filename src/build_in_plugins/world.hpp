#ifndef SRC_BUILD_IN_PLUGINS_WORLD
#define SRC_BUILD_IN_PLUGINS_WORLD
#include "../base_objects/commands.hpp"
#include "../log.hpp"
#include "../plugin/registration.hpp"
#include "../storage/world_data.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class WorldManagementPlugin : public PluginRegistration {
            storage::worlds_data worlds_storage;

            void add_world_id_suggestion(base_objects::command_browser& browser) {
                browser.set_suggestion_callback([this](const list_array<std::string>& args, base_objects::client_data_holder& client) {
                    auto suggestions = worlds_storage.get_list().convert<base_objects::suggestion>([](uint64_t id) {
                        return base_objects::suggestion{std::to_string(id)};
                    });
                    if (args.empty())
                        return suggestions;
                    else {
                        auto& input = args[0];
                        return suggestions.where([&input](const base_objects::suggestion& suggestion) { return suggestion.insertion.starts_with(input); });
                    }
                });
            }


        public:
            WorldManagementPlugin(const std::string& storage_path)
                : worlds_storage(TCPserver::get_global_instance().server_config, storage_path) {
            }

            void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {
                auto worlds = browser.add_child({"worlds"});
                {
                    auto create = worlds.add_child({"create"});
                }
                {
                    auto world_id = worlds.add_child({"remove"}).add_child({"[world_id]"});
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
                        auto set = base.add_child({"set"}).add_child({"[world_id]"});
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
