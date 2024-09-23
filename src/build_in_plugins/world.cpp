#include "world.hpp"
#include "../ClientHandleHelper.hpp"
#include "../api/players.hpp"
#include "../api/world.hpp"
#include "../log.hpp"
#include "../util/conversions.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        void WorldManagementPlugin::add_world_id_suggestion(base_objects::command_browser& browser) {
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

        WorldManagementPlugin::WorldManagementPlugin()
            : worlds_storage(Server::instance().config, Server::instance().config.server.get_worlds_path()) {
        }

        void WorldManagementPlugin::OnLoad(const PluginRegistrationPtr& self) {
            log::info("World", "loading worlds...");
            api::world::register_worlds_data(worlds_storage);

            for (auto& it : Server::instance().config.allowed_dimensions) {
                api::world::pre_load_world(it.get(), [&](storage::world_data& world) {
                    world.world_type = Server::instance().config.world.type;
                    world.world_seed = util::conversions::uuid::from(Server::instance().config.world.seed.get());
                    world.light_processor_id = "default"_ss;
                    if (world.world_name.get() == "end") {
                        world.generator_id = "end"_ss;
                    } else if (world.world_name.get() == "nether") {
                        world.generator_id = "nether"_ss;
                    } else {
                        world.generator_id = Server::instance().config.world.type;
                        for (auto& [name, value] : Server::instance().config.world.generator_settings)
                            world.world_generator_data[name.get()] = value;
                        world.load_points.push_back({10, 10, -10, -10});
                    }
                });
                log::info("World", it.get() + " loaded.");
            }
            log::info("World", "installing ticking task...");
            world_ticking = std::make_shared<fast_task::task>([this]() {
                log::info("World", "load complete.");
                std::random_device rd;
                std::mt19937 gen(rd());
                while (true) {
                    std::chrono::nanoseconds time(0);
                    worlds_storage.locked([&](storage::worlds_data& worlds) {
                        fast_task::task::check_cancellation();
                        time = worlds_storage.apply_tick(gen);
                    });
                    fast_task::task::sleep_until(std::chrono::high_resolution_clock::now() + time);
                }
            });
            fast_task::task::start(world_ticking);
        }

        void WorldManagementPlugin::OnUnload(const PluginRegistrationPtr& self) {
            log::info("World", "saving worlds...");
            worlds_storage.locked([&](storage::worlds_data& worlds) {
                fast_task::task::notify_cancel(world_ticking);
                worlds.for_each_world([&](uint64_t id, storage::world_data& world) {
                    log::info("World", "saving world " + world.world_name.get() + "...");
                    world.save();
                    world.save_and_unload_chunks();
                    world.await_save_chunks();
                    log::info("World", "world " + world.world_name.get() + " saved.");
                });
                worlds.unload_all();
            });
        }

        //do not release memory, just save
        void WorldManagementPlugin::OnFaultUnload(const PluginRegistrationPtr& self) {
            log::info("World", "saving worlds...");
            worlds_storage.locked([&](storage::worlds_data& worlds) {
                fast_task::task::notify_cancel(world_ticking);
                worlds.for_each_world([&](uint64_t id, storage::world_data& world) {
                    log::info("World", "saving world " + world.world_name.get() + "...");
                    world.save();
                    world.save_chunks();
                    world.await_save_chunks();
                    log::info("World", "world " + world.world_name.get() + " saved.");
                });
            });
        }


        void WorldManagementPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
            using predicate = base_objects::predicate;
            using pred_long = base_objects::predicates::_long;
            using cmd_pred_long = base_objects::predicates::command::_long;
            auto worlds = browser.add_child("worlds");
            {
                auto create = worlds.add_child("create");
            }
            {
                auto world_id = worlds.add_child("remove").add_child({"<world_id>"});
                world_id
                    .set_argument_type(cmd_pred_long())
                    .set_callback("command.world.remove", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to set world id, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else
                            worlds_storage.erase(id);
                    });
                add_world_id_suggestion(world_id);
            }
            {
                auto base = worlds.add_child("base");
                {
                    auto set = base.add_child("set").add_child("<world_id>");
                    set
                        .set_argument_type(cmd_pred_long())
                        .set_callback("command.world.base.set", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                            uint64_t id = std::get<pred_long>(args[0]).value;
                            if (!worlds_storage.exists(id)) {
                                Chat message("Failed to set world id, world with this id not set: " + std::to_string(id));
                                message.SetColor("red");
                                api::players::calls::on_system_message({client, message});
                            } else
                                worlds_storage.base_world_id = id;
                        });
                    add_world_id_suggestion(set);
                }
                base.set_callback("command.world.base", [this](const list_array<predicate>&, base_objects::client_data_holder& client) {
                    if (worlds_storage.base_world_id == -1)
                        api::players::calls::on_system_message({client, {"Base world not set."}});
                    else
                        api::players::calls::on_system_message({client, {"Base world is: " + worlds_storage.get(worlds_storage.base_world_id)->world_name.get()}});
                });
            }
        }
    }
}