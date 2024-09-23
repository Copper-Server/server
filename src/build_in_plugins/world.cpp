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

        void WorldManagementPlugin::add_world_name_suggestion(base_objects::command_browser& browser) {
            browser.set_suggestion_callback([this](const std::string& current, base_objects::client_data_holder& client) {
                auto suggestions = worlds_storage.get_list().convert<std::string>([this](uint64_t id) {
                    return worlds_storage.get_name(id).get();
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
            if (worlds_storage.base_world_id == -1 && !Server::instance().config.allowed_dimensions.empty()) {
                log::info("World", "Base world not set, setting to first world.");
                if (auto id = worlds_storage.get_id("overworld"); id != -1) {
                    worlds_storage.base_world_id = id;
                } else
                    worlds_storage.base_world_id = worlds_storage.get_list()[0];
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
            using pred_string = base_objects::predicates::string;
            using cmd_pred_string = base_objects::predicates::command::string;

            using pred_nbt_compound_tag = base_objects::predicates::nbt_compound_tag;
            using cmd_pred_nbt_compound_tag = base_objects::predicates::command::nbt_compound_tag;
            auto worlds = browser.add_child("worlds");
            {
                auto create = worlds.add_child("create");
                auto world_name = create.add_child("<world_name>");
                auto settings = world_name.set_argument_type(cmd_pred_string()).add_child("[settings]");
                settings.set_argument_type(cmd_pred_nbt_compound_tag())
                    .set_callback("command.world.create", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        const std::string& name = std::get<pred_string>(args[0]).value;
                        auto& settings = std::get<pred_nbt_compound_tag>(args[1]).nbt;
                        if (worlds_storage.exists(name)) {
                            Chat message("Failed to create world, world with this name already exists: " + name);
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            worlds_storage.create(name, [&](storage::world_data& world) {
                                world.load(enbt::compound::make_ref(settings));
                            });
                            Chat message("World created: " + name);
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
            }
            {
                auto remove = worlds.add_child("remove");
                auto world_id = remove.add_child({"<world_id>"});
                auto world_name = remove.add_child({"<world_name>"});


                world_id.set_argument_type(cmd_pred_long())
                    .set_callback("command.world.remove", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to set world id, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else
                            worlds_storage.erase(id);
                    });
                world_name
                    .set_argument_type(cmd_pred_string())
                    .set_callback("command.world.remove", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        const std::string& id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (!worlds_storage.exists(actual_id)) {
                            Chat message("Failed to set world id, world with this id not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else
                            worlds_storage.erase(actual_id);
                    });
                add_world_id_suggestion(world_id);
                add_world_name_suggestion(world_name);
            }
            {
                auto base = worlds.add_child("base");
                {
                    auto set = base.add_child("set");
                    auto world_id = set.add_child("<world_id>");
                    auto world_name = set.add_child("<world_name>");
                    world_id
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
                    world_name
                        .set_argument_type(cmd_pred_string())
                        .set_callback("command.world.base.set", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                            const std::string id = std::get<pred_string>(args[0]).value;
                            auto actual_id = worlds_storage.get_id(id);
                            if (!worlds_storage.exists(actual_id)) {
                                Chat message("Failed to set world id, world with this id not set: " + id);
                                message.SetColor("red");
                                api::players::calls::on_system_message({client, message});
                            } else
                                worlds_storage.base_world_id = actual_id;
                        });
                    add_world_id_suggestion(world_id);
                    add_world_name_suggestion(world_name);
                }
                base.set_callback("command.world.base", [this](const list_array<predicate>&, base_objects::client_data_holder& client) {
                    if (worlds_storage.base_world_id == -1)
                        api::players::calls::on_system_message({client, {"Base world not set."}});
                    else
                        api::players::calls::on_system_message({client, {"Base world is: " + worlds_storage.get(worlds_storage.base_world_id)->world_name.get() + " (" + std::to_string(worlds_storage.base_world_id) + ")"}});
                });
            }
            {
                auto load = worlds.add_child("load");
                auto world_id = load.add_child("<world_id>");
                auto world_name = load.add_child("<world_name>");
                world_id
                    .set_argument_type(cmd_pred_long())
                    .set_callback("command.world.load", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to load world, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            worlds_storage.get(id);
                            Chat message("World loaded: " + worlds_storage.get_name(id).get());
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
                world_name
                    .set_argument_type(cmd_pred_string())
                    .set_callback("command.world.load", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        const std::string id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (!worlds_storage.exists(actual_id)) {
                            Chat message("Failed to load world, world with this id not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            worlds_storage.get(actual_id);
                            Chat message("World loaded: " + worlds_storage.get_name(actual_id).get());
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
                add_world_id_suggestion(world_id);
                add_world_name_suggestion(world_name);
            }
            {
                auto save = worlds.add_child("save");
                auto world_id = save.add_child("<world_id>");
                auto world_name = save.add_child("<world_name>");
                world_id
                    .set_argument_type(cmd_pred_long())
                    .set_callback("command.world.save", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to save world, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            worlds_storage.save(id);
                            Chat message("World saved: " + worlds_storage.get_name(id).get());
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
                world_name
                    .set_argument_type(cmd_pred_string())
                    .set_callback("command.world.save", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        const std::string id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (!worlds_storage.exists(actual_id)) {
                            Chat message("Failed to save world, world with this id not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            worlds_storage.save(actual_id);
                            Chat message("World saved: " + worlds_storage.get_name(actual_id).get());
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
                add_world_id_suggestion(world_id);
                add_world_name_suggestion(world_name);
            }
            {
                auto save_all = worlds.add_child("save_all");
                save_all.set_callback("command.world.save_all", [this](const list_array<predicate>&, base_objects::client_data_holder& client) {
                    worlds_storage.save_all();
                    Chat message("All worlds saved.");
                    message.SetColor("green");
                    api::players::calls::on_system_message({client, message});
                });
            }
            {
                auto list = worlds.add_child("list");
                list.set_callback("command.world.list", [this](const list_array<predicate>&, base_objects::client_data_holder& client) {
                    std::string message = "Worlds: ";
                    worlds_storage.for_each_world([&message](uint64_t id, storage::world_data& world) {
                        message += world.world_name.get() + ", ";
                    });
                    message.erase(message.size() - 2);
                    message[message.size() - 1] = '.';
                    api::players::calls::on_system_message({client, {message}});
                });
            }
            {
                auto unload = worlds.add_child("unload");
                auto world_id = unload.add_child("<world_id>");
                auto world_name = unload.add_child("<world_name>");
                world_id
                    .set_argument_type(cmd_pred_long())
                    .set_callback("command.world.unload", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to unload world, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            worlds_storage.save_and_unload(id);
                            Chat message("World unloaded: " + worlds_storage.get_name(id).get());
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
                world_name
                    .set_argument_type(cmd_pred_string())
                    .set_callback("command.world.unload", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        const std::string id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (!worlds_storage.exists(actual_id)) {
                            Chat message("Failed to unload world, world with this id not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            worlds_storage.save_and_unload(actual_id);
                            Chat message("World unloaded: " + worlds_storage.get_name(actual_id).get());
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
                add_world_id_suggestion(world_id);
                add_world_name_suggestion(world_name);
            }
            {
                auto unload_all = worlds.add_child("unload_all");
                unload_all.set_callback("command.world.unload_all", [this](const list_array<predicate>&, base_objects::client_data_holder& client) {
                    worlds_storage.save_and_unload_all();
                    Chat message("All worlds unloaded.");
                    message.SetColor("green");
                    api::players::calls::on_system_message({client, message});
                });
            }
            {
                auto get_id = worlds.add_child("get_id");
                auto world_name = get_id.add_child("<world_name>");
                world_name
                    .set_argument_type(cmd_pred_string())
                    .set_callback("command.world.get_id", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        const std::string id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (actual_id == -1) {
                            Chat message("Failed to get world id, world with this name not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            Chat message("World id: " + std::to_string(actual_id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
                add_world_name_suggestion(world_name);
            }
            {
                auto get_name = worlds.add_child("get_name");
                auto world_id = get_name.add_child("<world_id>");
                world_id
                    .set_argument_type(cmd_pred_long())
                    .set_callback("command.world.get_name", [this](const list_array<predicate>& args, base_objects::client_data_holder& client) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to get world name, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({client, message});
                        } else {
                            Chat message("World name: " + worlds_storage.get_name(id).get());
                            message.SetColor("green");
                            api::players::calls::on_system_message({client, message});
                        }
                    });
                add_world_id_suggestion(world_id);
            }
        }
    }
}
