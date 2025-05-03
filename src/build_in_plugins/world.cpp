#include <library/enbt/senbt.hpp>
#include <library/fast_task/src/files/files.hpp>
#include <src/api/configuration.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/internal/world.hpp>
#include <src/api/players.hpp>
#include <src/api/protocol.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/build_in_plugins/world.hpp>
#include <src/log.hpp>
#include <src/protocolHelper/packets.hpp>
#include <src/registers.hpp>
#include <src/util/conversions.hpp>
#include <src/util/task_management.hpp>

namespace copper_server::build_in_plugins {
    struct chunk_speed_data {
        int64_t x;
        int64_t z;
        std::chrono::milliseconds tick_time;
    };

    void end_chunk_speed_report_collecting(
        const std::filesystem::path& path,
        const std::string& world_name,
        list_array<list_array<chunk_speed_data>> collected_data,
        base_objects::client_data_holder executor
    ) {
        try {
            enbt::fixed_array enbt_collected_data;
            enbt_collected_data.reserve(collected_data.size());
            for (const auto& tick : collected_data) {
                enbt::fixed_array enbt_tick;
                enbt_tick.reserve(tick.size());
                for (const auto& chunk : tick)
                    enbt_tick.push_back(enbt::fixed_array{chunk.x, chunk.z, chunk.tick_time.count()});
                enbt_collected_data.push_back(std::move(enbt_tick));
            }
            auto current_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            std::tm* timeinfo = std::localtime(&current_time);
            auto report_path = path
                               / "reports"
                               / "chunk_tick_speed"
                               / (std::to_string(timeinfo->tm_year + 1900)
                                  + "_" + std::to_string(timeinfo->tm_mon + 1)
                                  + "_" + std::to_string(timeinfo->tm_mday)
                                  + "_" + std::to_string(timeinfo->tm_hour)
                                  + "_" + std::to_string(timeinfo->tm_min)
                                  + "_" + std::to_string(timeinfo->tm_sec)
                                  + ".senbt");
            std::filesystem::create_directories(report_path.parent_path());
            fast_task::files::async_iofstream report_file(report_path, std::ios::out | std::ios::binary);
            if (!report_file.is_open()) {
                Chat message("Failed to save chunk tick speed report for world: " + world_name + " to: " + report_path.string());
                message.SetColor("red");
                api::players::calls::on_system_message({executor, message});
            }
            report_file << senbt::serialize(enbt_collected_data);
            //enbt::io_helper::write_token(report_file, enbt_collected_data);

            Chat message("Chunk tick speed report for world: " + world_name + " saved to: " + report_path.string());
            message.SetColor("green");
            api::players::calls::on_system_message({executor, message});
        }

        catch (const std::exception& e) {
            log::error("World", "Failed to save chunk speed report: " + std::string(e.what()));
        }
    }

    void start_chunk_speed_report_collecting(storage::world_data& world, const base_objects::command_context& context) {
        world.profiling.chunk_speedometer_callback
            = [start_time = std::chrono::high_resolution_clock::now(),
               collected_data = list_array<list_array<chunk_speed_data>>(),
               current_tick = list_array<chunk_speed_data>(),
               executor = context.executor](storage::world_data& world, int64_t chunk_x, int64_t chunk_z, std::chrono::milliseconds tick_time) mutable {
                  if (chunk_x == INT64_MAX && chunk_z == INT64_MAX && tick_time.count() == 0) {
                      collected_data.push_back(std::move(current_tick));
                      if (start_time + std::chrono::milliseconds(10000) < std::chrono::high_resolution_clock::now()) {
                          Chat message("Saving chunk tick speed report for world: " + world.world_name);
                          message.SetColor("green");
                          api::players::calls::on_system_message({executor, message});
                          Task::start([path = world.get_path(), world_name = world.world_name, collected_data = std::move(collected_data), executor]() mutable {
                              end_chunk_speed_report_collecting(path, world_name, std::move(collected_data), executor);
                          });
                          world.profiling.chunk_speedometer_callback = nullptr;
                      }
                  } else
                      current_tick.emplace_back(chunk_speed_data(chunk_x, chunk_z, tick_time));
              };
    }

    void world_tps_profiling(storage::world_data& world) {
        log::info("World", "Profiling TPS for world " + world.world_name + ": " + std::to_string(world.profiling.tps_for_world));
    }

    void world_slow_chunk_notify_profiling(storage::world_data& world, int64_t chunk_x, int64_t chunk_z, std::chrono::milliseconds tick_time) {
        std::string message = "Got slow chunk tick for world " + world.world_name + " chunk " + std::to_string(chunk_x) + " " + std::to_string(chunk_z) + " in " + std::to_string(tick_time.count()) + "ms";
        log::warn("World", message);
    }

    void slow_world_notify_profiling(storage::world_data& world, std::chrono::milliseconds tick_time) {
        std::string message = "Can't keep up with world " + world.world_name + " in " + std::to_string(tick_time.count()) + "ms";
        log::warn("World", message);
    }

    void WorldManagementPlugin::add_world_id_suggestion(base_objects::command_browser& browser) {
        browser.set_suggestion_callback([this](const std::string& current, base_objects::command_context& context) {
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
        browser.set_suggestion_callback([this](const std::string& current, base_objects::command_context& context) {
            auto suggestions = worlds_storage.get_list().convert<std::string>([this](uint64_t id) {
                return worlds_storage.get_name(id);
            });
            if (current.empty())
                return suggestions;
            else {
                return suggestions.where([&current](const std::string& suggestion) { return suggestion.starts_with(current); });
            }
        });
    }

    WorldManagementPlugin::WorldManagementPlugin()
        : worlds_storage(api::configuration::get(), api::configuration::get().server.get_worlds_path()) {
    }

    void WorldManagementPlugin::OnInitialization(const PluginRegistrationPtr& self) {
        api::world::register_worlds_data(worlds_storage);
    }

    void WorldManagementPlugin::OnLoad(const PluginRegistrationPtr& self) {
        log::info("World", "loading worlds...");
        register_event(worlds_storage.on_world_loaded, [](uint64_t id) {
            log::debug("World", "world id " + std::to_string(id) + " loaded.");
            api::world::get(id, [&](storage::world_data& world) {
                world.profiling.enable_world_profiling = true;
                world.profiling.slow_chunk_tick_callback = world_slow_chunk_notify_profiling;
                world.profiling.slow_world_tick_callback = slow_world_notify_profiling;
            });
            return false;
        });
        register_event(worlds_storage.on_world_unloaded, [](uint64_t id) {
            log::debug("World", "world id " + std::to_string(id) + " unloaded.");
            return false;
        });

        std::list<std::shared_ptr<fast_task::task>> tasks;
        for (auto& it : api::configuration::get().allowed_dimensions) {
            api::world::pre_load_world(it, [&](storage::world_data& world) {
                world.world_type = api::configuration::get().world.type;
                world.world_seed = util::conversions::uuid::from(api::configuration::get().world.seed);
                world.light_processor_id = "default";
                if (world.world_name == "end") {
                    world.generator_id = "end";
                } else if (world.world_name == "nether") {
                    world.generator_id = "nether";
                } else {
                    world.generator_id = api::configuration::get().world.type;
                    for (auto& [name, value] : api::configuration::get().world.generator_settings)
                        world.world_generator_data[name] = value;
                }
                log::info("World", it + " initialized.");
            });

            tasks.push_back(std::make_shared<fast_task::task>([it]() {
                bool complete = false;
                while (!complete) {
                    size_t target = 0;
                    size_t loaded = 0;
                    api::world::get(it, [&](storage::world_data& world) {
                        target = world.profiling.chunk_target_to_load;
                        loaded = world.profiling.chunk_total_loaded;
                    });
                    if (target == loaded && target) {
                        log::info("World", "world " + it + " loaded.");
                        complete = true;
                    } else {
                        log::info("World", "world " + it + " loading... " + std::to_string(loaded) + " / " + std::to_string(target));
                        fast_task::task::sleep(500);
                    }
                }
            }));
        }
        if (worlds_storage.base_world_id == -1 && !api::configuration::get().allowed_dimensions.empty()) {
            log::info("World", "Base world not set, setting to first world.");
            if (auto id = worlds_storage.get_id("overworld"); id != -1) {
                worlds_storage.base_world_id = id;
            } else
                worlds_storage.base_world_id = worlds_storage.get_list()[0];
        }
        log::info("World", "installing ticking task...");
        world_ticking = std::make_shared<fast_task::task>([this]() {
            log::info("World", "load complete.");
            std::chrono::high_resolution_clock::time_point last_tick = std::chrono::high_resolution_clock::now();
            std::chrono::high_resolution_clock::time_point tick_next_awoke = std::chrono::high_resolution_clock::now();
            while (true) {
                if (!worlds_storage.ticks_per_second)
                    return std::chrono::milliseconds(1000);
                constexpr auto second = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1));
                const auto tick_time = second / worlds_storage.ticks_per_second;

                auto sleep_time = std::chrono::high_resolution_clock::now() - tick_next_awoke;
                while (true) {
                    try {
                        fast_task::task::check_cancellation();
                        auto current_time = std::chrono::high_resolution_clock::now();
                        auto elapsed = current_time - last_tick;
                        worlds_storage.apply_tick(current_time, elapsed);
                        tick_next_awoke = std::chrono::high_resolution_clock::now();
                        auto to_tick = tick_next_awoke - current_time;

                        if (to_tick < tick_time + sleep_time) {
                            tick_next_awoke += tick_time - to_tick - sleep_time;
                            fast_task::task::sleep_until(std::chrono::high_resolution_clock::now() + (tick_time - to_tick - sleep_time));
                            sleep_time = std::chrono::high_resolution_clock::now() - tick_next_awoke;
                        }
                        last_tick = current_time;
                    } catch (const std::exception& e) {
                        log::error("World", "Error ticking world: " + std::string(e.what()));
                    } catch (const fast_task::task_cancellation&) {
                        log::debug("World", "ticking task canceled.");
                        throw; //DO not remove, handled by lib!
                    } catch (...) {
                        log::error("World", "Error ticking world. Undefined exception.");
                    }
                }
            }
        });
        fast_task::task::start(world_ticking);
        fast_task::task::await_multiple(tasks);
    }

    void WorldManagementPlugin::OnUnload(const PluginRegistrationPtr& self) {
        log::info("World", "saving worlds...");
        if (world_ticking) {
            fast_task::task::await_notify_cancel(world_ticking);
            world_ticking = nullptr;
        }

        worlds_storage.locked([&](storage::worlds_data& worlds) {
            worlds.for_each_world([&](uint64_t id, storage::world_data& world) {
                log::info("World", "saving world " + world.world_name + "...");
                world.save();
                world.save_chunks(true);
                world.await_save_chunks();
                log::info("World", "world " + world.world_name + " saved.");
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
                log::info("World", "saving world " + world.world_name + "...");
                world.save();
                world.save_chunks(false);
                world.await_save_chunks();
                log::info("World", "world " + world.world_name + " saved.");
            });
        });
    }

    using predicate = base_objects::parser;
    using pred_double = base_objects::parsers::_double;
    using cmd_pred_double = base_objects::parsers::command::_double;
    using pred_long = base_objects::parsers::_long;
    using cmd_pred_long = base_objects::parsers::command::_long;
    using pred_string = base_objects::parsers::string;
    using cmd_pred_string = base_objects::parsers::command::string;

    using pred_nbt_compound_tag = base_objects::parsers::nbt_compound_tag;
    using cmd_pred_nbt_compound_tag = base_objects::parsers::command::nbt_compound_tag;

    using pred_block_pos = base_objects::parsers::block_pos;
    using cmd_pred_block_pos = base_objects::parsers::command::block_pos;

    using pred_angle = base_objects::parsers::angle;
    using cmd_pred_angle = base_objects::parsers::command::angle;

    using pred_block = base_objects::parsers::block;
    using cmd_pred_block = base_objects::parsers::command::block;

    using pred_entity = base_objects::parsers::entity;
    using cmd_pred_entity = base_objects::parsers::command::entity;

    base_objects::full_block_data extract_block(const pred_block& block_data) {
        auto& static_block_data = base_objects::block::get_block(block_data.block_id);
        auto states = static_block_data.assigned_states_to_properties->left.at(static_block_data.default_state);
        for (auto& [state, value] : block_data.states) {
            states.at(state) = value;
        }
        base_objects::block block(static_block_data.assigned_states_to_properties->right.at(states));
        if (block_data.data_tags.is_none())
            return block;
        else
            return base_objects::block_entity(block, block_data.data_tags);
    }

    WorldManagementPlugin::plugin_response WorldManagementPlugin::PlayerJoined(base_objects::client_data_holder& client_ref) {
        auto& client = *client_ref;
        base_objects::network::response response;

        return response;
    }

    void WorldManagementPlugin::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
        {
            auto worlds = browser.add_child("worlds");
            {
                auto create = worlds.add_child("create");
                auto world_name = create.add_child("world_name", cmd_pred_string());
                auto settings = world_name.add_child("settings", cmd_pred_nbt_compound_tag());
                settings
                    .set_callback("command.world.create", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        const std::string& name = std::get<pred_string>(args[0]).value;
                        auto& settings = std::get<pred_nbt_compound_tag>(args[1]).nbt;
                        if (worlds_storage.exists(name)) {
                            Chat message("Failed to create world, world with this name already exists: " + name);
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            worlds_storage.create(name, [&](storage::world_data& world) {
                                world.load(settings.as_compound());
                            });
                            Chat message("World created: " + name);
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                world_name.set_callback("command.world.create", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    const std::string& name = std::get<pred_string>(args[0]).value;
                    if (worlds_storage.exists(name)) {
                        Chat message("Failed to create world, world with this name already exists: " + name);
                        message.SetColor("red");
                        api::players::calls::on_system_message({context.executor, message});
                    } else {
                        worlds_storage.create(name);
                        Chat message("World created: " + name);
                        message.SetColor("green");
                        api::players::calls::on_system_message({context.executor, message});
                    }
                });
            }
            {
                auto remove = worlds.add_child("remove");
                auto world_id = remove.add_child("<world_id>", cmd_pred_long());
                auto world_name = remove.add_child("<world_name>", cmd_pred_string());

                world_id
                    .set_callback("command.world.remove", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to set world id, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else
                            worlds_storage.erase(id);
                    });
                world_name
                    .set_callback("command.world.remove", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        const std::string& id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (!worlds_storage.exists(actual_id)) {
                            Chat message("Failed to set world id, world with this id not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
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
                    auto world_id = set.add_child("<world_id>", cmd_pred_long());
                    auto world_name = set.add_child("<world_name>", cmd_pred_string());
                    world_id
                        .set_callback("command.world.base.set", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            uint64_t id = std::get<pred_long>(args[0]).value;
                            if (!worlds_storage.exists(id)) {
                                Chat message("Failed to set world id, world with this id not set: " + std::to_string(id));
                                message.SetColor("red");
                                api::players::calls::on_system_message({context.executor, message});
                            } else
                                worlds_storage.base_world_id = id;
                        });
                    world_name
                        .set_callback("command.world.base.set", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            const std::string id = std::get<pred_string>(args[0]).value;
                            auto actual_id = worlds_storage.get_id(id);
                            if (!worlds_storage.exists(actual_id)) {
                                Chat message("Failed to set world id, world with this id not set: " + id);
                                message.SetColor("red");
                                api::players::calls::on_system_message({context.executor, message});
                            } else
                                worlds_storage.base_world_id = actual_id;
                        });
                    add_world_id_suggestion(world_id);
                    add_world_name_suggestion(world_name);
                }
                base.set_callback("command.world.base", [this](const list_array<predicate>&, base_objects::command_context& context) {
                    if (worlds_storage.base_world_id == -1)
                        api::players::calls::on_system_message({context.executor, {"Base world not set."}});
                    else
                        api::players::calls::on_system_message({context.executor, {"Base world is: " + worlds_storage.get(worlds_storage.base_world_id)->world_name + " (" + std::to_string(worlds_storage.base_world_id) + ")"}});
                });
            }
            {
                auto load = worlds.add_child("load");
                auto world_id = load.add_child("<world_id>", cmd_pred_long());
                auto world_name = load.add_child("<world_name>", cmd_pred_string());
                world_id
                    .set_callback("command.world.load", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to load world, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            worlds_storage.get(id);
                            Chat message("World loaded: " + worlds_storage.get_name(id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                world_name
                    .set_callback("command.world.load", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        const std::string id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (!worlds_storage.exists(actual_id)) {
                            Chat message("Failed to load world, world with this id not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            worlds_storage.get(actual_id);
                            Chat message("World loaded: " + worlds_storage.get_name(actual_id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                add_world_id_suggestion(world_id);
                add_world_name_suggestion(world_name);
            }
            {
                auto save = worlds.add_child("save");
                auto world_id = save.add_child("<world_id>", cmd_pred_long());
                auto world_name = save.add_child("<world_name>", cmd_pred_string());
                world_id
                    .set_callback("command.world.save", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to save world, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            worlds_storage.save(id);
                            Chat message("World saved: " + worlds_storage.get_name(id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                world_name
                    .set_callback("command.world.save", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        const std::string id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (!worlds_storage.exists(actual_id)) {
                            Chat message("Failed to save world, world with this id not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            worlds_storage.save(actual_id);
                            Chat message("World saved: " + worlds_storage.get_name(actual_id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                add_world_id_suggestion(world_id);
                add_world_name_suggestion(world_name);
            }
            {
                auto save_all = worlds.add_child("save_all");
                save_all.set_callback("command.world.save_all", [this](const list_array<predicate>&, base_objects::command_context& context) {
                    worlds_storage.save_all();
                    Chat message("All worlds saved.");
                    message.SetColor("green");
                    api::players::calls::on_system_message({context.executor, message});
                });
            }
            {
                auto list = worlds.add_child("list");
                list.set_callback("command.world.list", [this](const list_array<predicate>&, base_objects::command_context& context) {
                    std::string message = "Worlds: ";
                    worlds_storage.for_each_world([&message](uint64_t id, storage::world_data& world) {
                        message += world.world_name + ", ";
                    });
                    message.erase(message.size() - 2);
                    message[message.size() - 1] = '.';
                    api::players::calls::on_system_message({context.executor, {message}});
                });
            }
            {
                auto unload = worlds.add_child("unload");
                auto world_id = unload.add_child("<world_id>", cmd_pred_long());
                auto world_name = unload.add_child("<world_name>", cmd_pred_string());
                world_id
                    .set_callback("command.world.unload", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to unload world, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            worlds_storage.save_and_unload(id);
                            Chat message("World unloaded: " + worlds_storage.get_name(id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                world_name
                    .set_callback("command.world.unload", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        const std::string id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (!worlds_storage.exists(actual_id)) {
                            Chat message("Failed to unload world, world with this id not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            worlds_storage.save_and_unload(actual_id);
                            Chat message("World unloaded: " + worlds_storage.get_name(actual_id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                add_world_id_suggestion(world_id);
                add_world_name_suggestion(world_name);
            }
            {
                auto unload_all = worlds.add_child("unload_all");
                unload_all.set_callback("command.world.unload_all", [this](const list_array<predicate>&, base_objects::command_context& context) {
                    worlds_storage.save_and_unload_all();
                    Chat message("All worlds unloaded.");
                    message.SetColor("green");
                    api::players::calls::on_system_message({context.executor, message});
                });
            }
            {
                auto get_id = worlds.add_child("get_id");
                auto world_name = get_id.add_child("<world_name>", cmd_pred_string());
                world_name
                    .set_callback("command.world.get_id", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        const std::string id = std::get<pred_string>(args[0]).value;
                        auto actual_id = worlds_storage.get_id(id);
                        if (actual_id == -1) {
                            Chat message("Failed to get world id, world with this name not set: " + id);
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            Chat message("World id: " + std::to_string(actual_id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                add_world_name_suggestion(world_name);
            }
            {
                auto get_name = worlds.add_child("get_name");
                auto world_id = get_name.add_child("<world_id>", cmd_pred_long());
                world_id
                    .set_callback("command.world.get_name", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                        uint64_t id = std::get<pred_long>(args[0]).value;
                        if (!worlds_storage.exists(id)) {
                            Chat message("Failed to get world name, world with this id not set: " + std::to_string(id));
                            message.SetColor("red");
                            api::players::calls::on_system_message({context.executor, message});
                        } else {
                            Chat message("World name: " + worlds_storage.get_name(id));
                            message.SetColor("green");
                            api::players::calls::on_system_message({context.executor, message});
                        }
                    });
                add_world_id_suggestion(world_id);
            }
            {
                auto setblock = worlds.add_child("setblock");
                auto world = setblock.add_child("world_id", cmd_pred_long());
                auto world_name = setblock.add_child("world_name", cmd_pred_string());
                auto position = world.add_child("xyz", cmd_pred_block_pos());
                world_name.add_child(position);
                auto block = position.add_child("block", cmd_pred_block());
                auto replace = block.add_child("replace");
                auto destroy = block.add_child("destroy");
                auto keep = block.add_child("keep");

                auto command = [&](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto& world = args[0];
                    auto pos = std::get<pred_block_pos>(args[1]);
                    auto block = extract_block(std::get<pred_block>(args[2]));

                    if (pos.x_relative)
                        pos.x += (int32_t)context.other_data["x"];
                    if (pos.y_relative)
                        pos.y += (int32_t)context.other_data["y"];
                    if (pos.z_relative)
                        pos.z += (int32_t)context.other_data["z"];

                    std::visit(
                        [&world, &pos, &block](auto&& arg) {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, pred_long>) {
                                api::world::get(std::get<pred_long>(world).value, [pos, &block](storage::world_data& world) {
                                    world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::replace);
                                });
                            } else {
                                api::world::get(std::get<pred_string>(world).value, [pos, &block](storage::world_data& world) {
                                    world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::replace);
                                });
                            }
                        },
                        world
                    );
                };

                replace.set_callback("command.world.setblock", command);
                block.set_callback("command.world.setblock", command);

                destroy.set_callback("command.world.setblock", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto& world = args[0];
                    auto pos = std::get<pred_block_pos>(args[1]);
                    auto block = extract_block(std::get<pred_block>(args[2]));

                    if (pos.x_relative)
                        pos.x += (int32_t)context.other_data["x"];
                    if (pos.y_relative)
                        pos.y += (int32_t)context.other_data["y"];
                    if (pos.z_relative)
                        pos.z += (int32_t)context.other_data["z"];

                    std::visit(
                        [&world, &pos, &block](auto&& arg) {
                            using T = std::decay_t<decltype(arg)>;
                            if constexpr (std::is_same_v<T, pred_long>) {
                                api::world::get(std::get<pred_long>(world).value, [pos, &block](storage::world_data& world) {
                                    world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::destroy);
                                });
                            } else {
                                api::world::get(std::get<pred_string>(world).value, [pos, &block](storage::world_data& world) {
                                    world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::destroy);
                                });
                            }
                        },
                        world
                    );
                });

                keep.set_callback("command.world.setblock", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto& world = args[0];
                    auto pos = std::get<pred_block_pos>(args[1]);
                    auto block = extract_block(std::get<pred_block>(args[2]));

                    if (pos.x_relative)
                        pos.x += (int32_t)context.other_data["x"];
                    if (pos.y_relative)
                        pos.y += (int32_t)context.other_data["y"];
                    if (pos.z_relative)
                        pos.z += (int32_t)context.other_data["z"];

                    std::visit([&world, &pos, &block](auto&& arg) {
                        using T = std::decay_t<decltype(arg)>;
                        if constexpr (std::is_same_v<T, pred_long>) {
                            api::world::get(std::get<pred_long>(world).value, [pos, &block](storage::world_data& world) {
                                world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::keep);
                            });
                        } else {
                            api::world::get(std::get<pred_string>(world).value, [pos, &block](storage::world_data& world) {
                                world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::keep);
                            });
                        }
                    },
                               world);
                });


                add_world_id_suggestion(world);
                add_world_name_suggestion(world_name);
            }
            {
                auto profile = worlds.add_child("profile");
                auto world_id_ = profile.add_child("world_id", cmd_pred_long());
                auto world_name_ = profile.add_child("world_name", cmd_pred_string());

                add_world_id_suggestion(world_id_);
                add_world_name_suggestion(world_name_);
                {
                    auto world_id = world_id_.add_child("report");
                    auto world_name = world_name_.add_child("report");

                    {
                        auto chunk_tick_speed = world_id.add_child("chunk_tick_speed");
                        chunk_tick_speed.set_callback("command.world.profile.report.chunk_tick_speed", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            std::visit([&](auto&& arg) {
                                using T = std::decay_t<decltype(arg)>;
                                if constexpr (std::is_same_v<T, pred_long> || std::is_same_v<T, pred_string>) {

                                    api::world::get(arg.value, [&](storage::world_data& world) {
                                        if (world.profiling.chunk_speedometer_callback) {
                                            Chat message("Failed to report chunks tick speed, chunk speed profiling already enabled for world: " + world.world_name);
                                            message.SetColor("red");
                                            api::players::calls::on_system_message({context.executor, message});
                                        } else {
                                            start_chunk_speed_report_collecting(world, context);
                                            Chat message("Collecting chunks tick speed report for world: " + world.world_name);
                                            message.SetColor("green");
                                            api::players::calls::on_system_message({context.executor, message});
                                        }
                                    });
                                }
                            },
                                       args[0]);
                        });
                    }
                }

                {
                    auto world_id = world_id_.add_child("enable");
                    auto world_name = world_name_.add_child("enable");
                    world_id
                        .add_child("tps")
                        .set_callback("command.world.profile.enable.tps", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling && world.profiling.got_tps_update) {
                                    Chat message("Profiling already enabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                                world.profiling.enable_world_profiling = true;
                                world.profiling.got_tps_update = world_tps_profiling;
                                Chat message("Profiling enabled for world: " + world.world_name);
                                message.SetColor("green");
                                api::players::calls::on_system_message({context.executor, message});
                            });
                        });

                    world_name
                        .add_child("tps")
                        .set_callback("command.world.profile.enable.tps", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling && world.profiling.got_tps_update) {
                                    Chat message("TPS profiling already enabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                                world.profiling.enable_world_profiling = true;
                                world.profiling.got_tps_update = world_tps_profiling;
                                Chat message("TPS profiling enabled for world: " + world.world_name);
                                message.SetColor("green");
                                api::players::calls::on_system_message({context.executor, message});
                            });
                        });

                    world_id
                        .add_child("slow_chunk")
                        .set_callback("command.world.profile.enable.slow_chunk", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling && world.profiling.slow_chunk_tick_callback) {
                                    Chat message("Chunk profiling already enabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                                world.profiling.enable_world_profiling = true;
                                world.profiling.slow_chunk_tick_callback = world_slow_chunk_notify_profiling;
                                Chat message("Chunk profiling enabled for world: " + world.world_name);
                                message.SetColor("green");
                                api::players::calls::on_system_message({context.executor, message});
                            });
                        });

                    world_name
                        .add_child("slow_chunk")
                        .set_callback("command.world.profile.enable.slow_chunk", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling && world.profiling.slow_chunk_tick_callback) {
                                    Chat message("Chunk profiling already enabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                                world.profiling.enable_world_profiling = true;
                                world.profiling.slow_chunk_tick_callback = world_slow_chunk_notify_profiling;
                                Chat message("Chunk profiling enabled for world: " + world.world_name);
                                message.SetColor("green");
                                api::players::calls::on_system_message({context.executor, message});
                            });
                        });

                    world_id
                        .add_child("slow_world")
                        .set_callback("command.world.profile.enable.slow_world", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling && world.profiling.slow_world_tick_callback) {
                                    Chat message("World profiling already enabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                                world.profiling.enable_world_profiling = true;
                                world.profiling.slow_world_tick_callback = slow_world_notify_profiling;
                                Chat message("World profiling enabled for world: " + world.world_name);
                                message.SetColor("green");
                                api::players::calls::on_system_message({context.executor, message});
                            });
                        });

                    world_name
                        .add_child("slow_world")
                        .set_callback("command.world.profile.enable.slow_world", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling && world.profiling.slow_world_tick_callback) {
                                    Chat message("World profiling already enabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                                world.profiling.enable_world_profiling = true;
                                world.profiling.slow_world_tick_callback = slow_world_notify_profiling;
                                Chat message("World profiling enabled for world: " + world.world_name);
                                message.SetColor("green");
                                api::players::calls::on_system_message({context.executor, message});
                            });
                        });
                }
                {
                    auto world_id = world_id_.add_child("disable");
                    auto world_name = world_name_.add_child("disable");


                    world_id
                        .set_callback("command.world.profile.disable", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling) {
                                    world.profiling.enable_world_profiling = false;
                                    world.profiling.slow_chunk_tick_callback = nullptr;
                                    world.profiling.slow_world_tick_callback = nullptr;
                                    world.profiling.got_tps_update = nullptr;
                                    world.profiling.chunk_speedometer_callback = nullptr;

                                    Chat message("Profiling disabled for world: " + world.world_name);
                                    message.SetColor("green");
                                    api::players::calls::on_system_message({context.executor, message});
                                } else {
                                    Chat message("TPS profiling already disabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                            });
                        });

                    world_name
                        .set_callback("command.world.profile.disable", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling) {
                                    world.profiling.enable_world_profiling = false;
                                    world.profiling.slow_chunk_tick_callback = nullptr;
                                    world.profiling.slow_world_tick_callback = nullptr;
                                    world.profiling.got_tps_update = nullptr;

                                    Chat message("Profiling disabled for world: " + world.world_name);
                                    message.SetColor("green");
                                    api::players::calls::on_system_message({context.executor, message});
                                } else {
                                    Chat message("TPS profiling already disabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                            });
                        });


                    world_id
                        .add_child("tps")
                        .set_callback("command.world.profile.disable.tps", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling) {
                                    if (*world.profiling.got_tps_update.target<void (*)(storage::world_data&)>() == world_tps_profiling) {
                                        world.profiling.got_tps_update = nullptr;
                                        if (!world.profiling.slow_chunk_tick_callback && !world.profiling.slow_world_tick_callback)
                                            world.profiling.enable_world_profiling = false;

                                        Chat message("TPS profiling disabled for world: " + world.world_name);
                                        message.SetColor("green");
                                        api::players::calls::on_system_message({context.executor, message});
                                    } else {
                                        Chat message("TPS profiling enabled by another plugin for world: " + world.world_name);
                                        message.SetColor("red");
                                        api::players::calls::on_system_message({context.executor, message});
                                    }
                                } else {
                                    Chat message("TPS profiling already disabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                            });
                        });

                    world_name
                        .add_child("tps")
                        .set_callback("command.world.profile.disable.tps", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling) {
                                    if (*world.profiling.got_tps_update.target<void (*)(storage::world_data&)>() == world_tps_profiling) {
                                        world.profiling.got_tps_update = nullptr;
                                        if (!world.profiling.slow_chunk_tick_callback && !world.profiling.slow_world_tick_callback)
                                            world.profiling.enable_world_profiling = false;
                                        Chat message("TPS profiling disabled for world: " + world.world_name);
                                        message.SetColor("green");
                                        api::players::calls::on_system_message({context.executor, message});
                                    } else {
                                        Chat message("TPS profiling enabled by another plugin for world: " + world.world_name);
                                        message.SetColor("red");
                                        api::players::calls::on_system_message({context.executor, message});
                                    }
                                } else {
                                    Chat message("TPS profiling already disabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                            });
                        });

                    world_id
                        .add_child("slow_chunk")
                        .set_callback("command.world.profile.disable.slow_chunk", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling) {
                                    if (*world.profiling.slow_chunk_tick_callback.target<void (*)(storage::world_data&, int64_t, int64_t, std::chrono::milliseconds)>() == world_slow_chunk_notify_profiling) {
                                        world.profiling.slow_chunk_tick_callback = nullptr;
                                        Chat message("Slow chunk profiling enabled for world: " + world.world_name);
                                        message.SetColor("green");
                                        api::players::calls::on_system_message({context.executor, message});
                                    } else {
                                        Chat message("Slow chunk profiling enabled by another plugin for world: " + world.world_name);
                                        message.SetColor("red");
                                        api::players::calls::on_system_message({context.executor, message});
                                    }
                                } else {
                                    Chat message("Slow chunk profiling already disabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                            });
                        });

                    world_name
                        .add_child("slow_chunk")
                        .set_callback("command.world.profile.disable.slow_chunk", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling) {
                                    if (*world.profiling.slow_chunk_tick_callback.target<void (*)(storage::world_data&, int64_t, int64_t, std::chrono::milliseconds)>() == world_slow_chunk_notify_profiling) {
                                        world.profiling.slow_chunk_tick_callback = nullptr;
                                        Chat message("Slow chunk profiling enabled for world: " + world.world_name);
                                        message.SetColor("green");
                                        api::players::calls::on_system_message({context.executor, message});
                                    } else {
                                        Chat message("Slow chunk profiling enabled by another plugin for world: " + world.world_name);
                                        message.SetColor("red");
                                        api::players::calls::on_system_message({context.executor, message});
                                    }
                                } else {
                                    Chat message("Slow chunk profiling already disabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                            });
                        });

                    world_id
                        .add_child("slow_world")
                        .set_callback("command.world.profile.disable.slow_world", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling) {
                                    if (*world.profiling.slow_world_tick_callback.target<void (*)(storage::world_data&, std::chrono::milliseconds)>() == slow_world_notify_profiling) {
                                        world.profiling.slow_world_tick_callback = nullptr;
                                        Chat message("Slow chunk profiling enabled for world: " + world.world_name);
                                        message.SetColor("green");
                                        api::players::calls::on_system_message({context.executor, message});
                                    } else {
                                        Chat message("Slow chunk profiling enabled by another plugin for world: " + world.world_name);
                                        message.SetColor("red");
                                        api::players::calls::on_system_message({context.executor, message});
                                    }
                                } else {
                                    Chat message("Slow chunk profiling already disabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                            });
                        });

                    world_name
                        .add_child("slow_world")
                        .set_callback("command.world.profile.disable.slow_world", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);
                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                if (world.profiling.enable_world_profiling) {
                                    if (*world.profiling.slow_world_tick_callback.target<void (*)(storage::world_data&, std::chrono::milliseconds)>() == slow_world_notify_profiling) {
                                        world.profiling.slow_world_tick_callback = nullptr;
                                        Chat message("Slow chunk profiling enabled for world: " + world.world_name);
                                        message.SetColor("green");
                                        api::players::calls::on_system_message({context.executor, message});
                                    } else {
                                        Chat message("Slow chunk profiling enabled by another plugin for world: " + world.world_name);
                                        message.SetColor("red");
                                        api::players::calls::on_system_message({context.executor, message});
                                    }
                                } else {
                                    Chat message("Slow chunk profiling already disabled for world: " + world.world_name);
                                    message.SetColor("red");
                                    api::players::calls::on_system_message({context.executor, message});
                                }
                            });
                        });
                }
                {
                    auto config_id = world_id_.add_child("config");
                    auto config_name = world_name_.add_child("config");

                    config_id
                        .add_child("slow_world_threshold")
                        .set_callback("command.world.profile.config.slow_world_threshold.get", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                api::players::calls::on_system_message({context.executor, "Slow world threshold: " + std::to_string(world.profiling.slow_world_tick_callback_threshold)});
                            });
                        })
                        .add_child("value", cmd_pred_double())
                        .set_callback("command.world.profile.config.slow_world_threshold.set", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);
                            auto& value = std::get<pred_double>(args[1]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                world.profiling.slow_world_tick_callback_threshold = value.value;
                            });
                        });

                    config_name
                        .add_child("slow_world_threshold")
                        .set_callback("command.world.profile.config.slow_world_threshold.get", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                api::players::calls::on_system_message({context.executor, "Slow world threshold: " + std::to_string(world.profiling.slow_world_tick_callback_threshold)});
                            });
                        })
                        .add_child("value", cmd_pred_double())
                        .set_callback("command.world.profile.config.slow_world_tick_threshold.set", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_name = std::get<pred_string>(args[0]);
                            auto& value = std::get<pred_double>(args[1]);

                            api::world::get(world_name.value, [&](storage::world_data& world) {
                                world.profiling.slow_world_tick_callback_threshold = value.value;
                            });
                        });

                    config_id
                        .add_child("slow_chunk_threshold")
                        .set_callback("command.world.profile.config.slow_world_threshold.get", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                api::players::calls::on_system_message({context.executor, "Slow chunk threshold: " + std::to_string(world.profiling.slow_chunk_tick_callback_threshold)});
                            });
                        })
                        .add_child("value", cmd_pred_double())
                        .set_callback("command.world.profile.config.slow_chunk_threshold.set", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_long>(args[0]);
                            auto& value = std::get<pred_double>(args[1]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                world.profiling.slow_chunk_tick_callback_threshold = value.value;
                            });
                        });

                    config_name
                        .add_child("slow_chunk_threshold")
                        .set_callback("command.world.profile.config.slow_world_threshold.get", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_id = std::get<pred_string>(args[0]);

                            api::world::get(world_id.value, [&](storage::world_data& world) {
                                api::players::calls::on_system_message({context.executor, "Slow chunk threshold: " + std::to_string(world.profiling.slow_chunk_tick_callback_threshold)});
                            });
                        })
                        .add_child("value", cmd_pred_double())
                        .set_callback("command.world.profile.config.slow_chunk_threshold.set", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                            auto& world_name = std::get<pred_string>(args[0]);
                            auto& value = std::get<pred_double>(args[1]);

                            api::world::get(world_name.value, [&](storage::world_data& world) {
                                world.profiling.slow_chunk_tick_callback_threshold = value.value;
                            });
                        });
                }
            }
            {
                auto chunks_loaded = worlds.add_child("chunks_loaded");
                chunks_loaded.set_callback("command.chunks_loaded", [this](const list_array<predicate>&, base_objects::command_context& context) {
                    Chat message("Chunks loaded: " + std::to_string(api::world::loaded_chunks_count()));
                    message.SetColor("green");
                    api::players::calls::on_system_message({context.executor, message});
                });
                auto world_id = chunks_loaded.add_child("world_id", cmd_pred_long());
                world_id.set_callback("command.chunks_loaded", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto world_id = std::get<pred_long>(args[0]).value;
                    Chat message("Chunks loaded: " + std::to_string(api::world::loaded_chunks_count(world_id)));
                    message.SetColor("green");
                    api::players::calls::on_system_message({context.executor, message});
                });

                auto world_name = chunks_loaded.add_child("world_name", cmd_pred_string());
                world_name.set_callback("command.chunks_loaded", [this](const list_array<predicate>& args, base_objects::command_context& context) {
                    auto& world_name = std::get<pred_string>(args[0]).value;
                    Chat message("Chunks loaded: " + std::to_string(api::world::loaded_chunks_count(world_name)));
                    message.SetColor("green");
                    api::players::calls::on_system_message({context.executor, message});
                });
                add_world_id_suggestion(world_id);
                add_world_name_suggestion(world_name);
            }
        }
        {
            auto setblock = browser.add_child("setblock");
            auto position = setblock.add_child("xyz", cmd_pred_block_pos());
            auto block = position.add_child("block", cmd_pred_block());
            auto replace = block.add_child("replace");
            auto destroy = block.add_child("destroy");
            auto keep = block.add_child("keep");


            auto command
                = [&](const list_array<predicate>& args, base_objects::command_context& context) {
                      auto pos = std::get<pred_block_pos>(args[0]);
                      auto block = extract_block(std::get<pred_block>(args[1]));

                      if (pos.x_relative)
                          pos.x += (int32_t)context.other_data["x"];
                      if (pos.y_relative)
                          pos.y += (int32_t)context.other_data["y"];
                      if (pos.z_relative)
                          pos.z += (int32_t)context.other_data["z"];

                      api::world::get(context.executor->player_data.world_id, [pos, &block](storage::world_data& world) {
                          world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::replace);
                      });
                  };
            replace.set_callback("command.setblock", command);
            block.set_callback("command.setblock", command);

            destroy.set_callback("command.setblock", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                auto pos = std::get<pred_block_pos>(args[0]);
                auto block = extract_block(std::get<pred_block>(args[1]));

                if (pos.x_relative)
                    pos.x += (int32_t)context.other_data["x"];
                if (pos.y_relative)
                    pos.y += (int32_t)context.other_data["y"];
                if (pos.z_relative)
                    pos.z += (int32_t)context.other_data["z"];

                api::world::get(context.executor->player_data.world_id, [pos, &block](storage::world_data& world) {
                    world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::destroy);
                });
            });

            keep.set_callback("command.setblock", [&](const list_array<predicate>& args, base_objects::command_context& context) {
                auto pos = std::get<pred_block_pos>(args[0]);
                auto block = extract_block(std::get<pred_block>(args[1]));

                if (pos.x_relative)
                    pos.x += (int32_t)context.other_data["x"];
                if (pos.y_relative)
                    pos.y += (int32_t)context.other_data["y"];
                if (pos.z_relative)
                    pos.z += (int32_t)context.other_data["z"];

                api::world::get(context.executor->player_data.world_id, [pos, &block](storage::world_data& world) {
                    world.set_block(std::move(block), pos.x, pos.y, pos.z, storage::block_set_mode::keep);
                });
            });
        }
        {
            auto setbiome = browser.add_child("setbiome");
        }
        {
            auto getworldspawn = browser.add_child("getworldspawn");
            getworldspawn.set_callback("command.getworldspawn", [this](const list_array<predicate>&, base_objects::command_context& context) {
                Chat message("World spawn: x: " + std::to_string(api::configuration::get().world.spawn.x) + " y: " + std::to_string(api::configuration::get().world.spawn.y) + " z: " + std::to_string(api::configuration::get().world.spawn.z) + " yaw: " + std::to_string(api::configuration::get().world.spawn.yaw));
                message.SetColor("green");
                api::players::calls::on_system_message({context.executor, message});
            });
        }
    }
}
