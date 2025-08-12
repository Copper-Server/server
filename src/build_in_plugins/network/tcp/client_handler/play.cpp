/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/client.hpp>
#include <src/api/command.hpp>
#include <src/api/configuration.hpp>
#include <src/api/dialogs.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/packets.hpp>
#include <src/api/players.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>
#include <src/plugin/main.hpp>
#include <src/registers.hpp>
#include <src/storage/players_data.hpp>
#include <src/util/calculations.hpp>

namespace copper_server::build_in_plugins::network::tcp::client_handler {
    using accept_teleportation = api::packets::server_bound::play::accept_teleportation;
    using block_entity_tag_query = api::packets::server_bound::play::block_entity_tag_query;
    using bundle_item_selected = api::packets::server_bound::play::bundle_item_selected;
    using change_difficulty = api::packets::server_bound::play::change_difficulty;
    using change_gamemode = api::packets::server_bound::play::change_gamemode;
    using chat_ack = api::packets::server_bound::play::chat_ack;
    using chat_command = api::packets::server_bound::play::chat_command;
    using chat_command_signed = api::packets::server_bound::play::chat_command_signed;
    using chat = api::packets::server_bound::play::chat;
    using chat_session_update = api::packets::server_bound::play::chat_session_update;
    using chunk_batch_received = api::packets::server_bound::play::chunk_batch_received;
    using client_command = api::packets::server_bound::play::client_command;
    using client_tick_end = api::packets::server_bound::play::client_tick_end;
    using client_information = api::packets::server_bound::play::client_information;
    using command_suggestion = api::packets::server_bound::play::command_suggestion;
    using configuration_acknowledged = api::packets::server_bound::play::configuration_acknowledged;
    using container_button_click = api::packets::server_bound::play::container_button_click;
    using container_click = api::packets::server_bound::play::container_click;
    using container_close = api::packets::server_bound::play::container_close;
    using container_slot_state_changed = api::packets::server_bound::play::container_slot_state_changed;
    using cookie_response = api::packets::server_bound::play::cookie_response;
    using custom_payload = api::packets::server_bound::play::custom_payload;
    using debug_sample_subscription = api::packets::server_bound::play::debug_sample_subscription;
    using edit_book = api::packets::server_bound::play::edit_book;
    using entity_tag_query = api::packets::server_bound::play::entity_tag_query;
    using interact = api::packets::server_bound::play::interact;
    using jigsaw_generate = api::packets::server_bound::play::jigsaw_generate;
    using keep_alive = api::packets::server_bound::play::keep_alive;
    using lock_difficulty = api::packets::server_bound::play::lock_difficulty;
    using move_player_pos = api::packets::server_bound::play::move_player_pos;
    using move_player_pos_rot = api::packets::server_bound::play::move_player_pos_rot;
    using move_player_rot = api::packets::server_bound::play::move_player_rot;
    using move_player_status_only = api::packets::server_bound::play::move_player_status_only;
    using move_vehicle = api::packets::server_bound::play::move_vehicle;
    using paddle_boat = api::packets::server_bound::play::paddle_boat;
    using pick_item_from_block = api::packets::server_bound::play::pick_item_from_block;
    using pick_item_from_entity = api::packets::server_bound::play::pick_item_from_entity;
    using ping_request = api::packets::server_bound::play::ping_request;
    using place_recipe = api::packets::server_bound::play::place_recipe;
    using player_abilities = api::packets::server_bound::play::player_abilities;
    using player_action = api::packets::server_bound::play::player_action;
    using player_command = api::packets::server_bound::play::player_command;
    using player_input = api::packets::server_bound::play::player_input;
    using player_loaded = api::packets::server_bound::play::player_loaded;
    using pong = api::packets::server_bound::play::pong;
    using recipe_book_change_settings = api::packets::server_bound::play::recipe_book_change_settings;
    using recipe_book_seen_recipe = api::packets::server_bound::play::recipe_book_seen_recipe;
    using rename_item = api::packets::server_bound::play::rename_item;
    using resource_pack = api::packets::server_bound::play::resource_pack;
    using seen_advancements = api::packets::server_bound::play::seen_advancements;
    using select_trade = api::packets::server_bound::play::select_trade;
    using set_beacon = api::packets::server_bound::play::set_beacon;
    using set_carried_item = api::packets::server_bound::play::set_carried_item;
    using set_command_block = api::packets::server_bound::play::set_command_block;
    using set_command_minecart = api::packets::server_bound::play::set_command_minecart;
    using set_creative_mode_slot = api::packets::server_bound::play::set_creative_mode_slot;
    using set_jigsaw_block = api::packets::server_bound::play::set_jigsaw_block;
    using set_structure_block = api::packets::server_bound::play::set_structure_block;
    using set_test_block = api::packets::server_bound::play::set_test_block;
    using sign_update = api::packets::server_bound::play::sign_update;
    using swing = api::packets::server_bound::play::swing;
    using teleport_to_entity = api::packets::server_bound::play::teleport_to_entity;
    using test_instance_block_action = api::packets::server_bound::play::test_instance_block_action;
    using use_item_on = api::packets::server_bound::play::use_item_on;
    using use_item = api::packets::server_bound::play::use_item;
    using custom_click_action = api::packets::server_bound::play::custom_click_action;

    struct tcp_play : public PluginAutoRegister<"network/tcp_play", tcp_play> {
        storage::players_data players_data;

        tcp_play()
            : players_data(api::configuration::get().server.get_storage_path() / "players") {}

        struct extra_data_t {
            keep_alive_solution ka_solution;

            static extra_data_t& get(base_objects::SharedClientData& client) {
                if (!client.packets_state.extra_data) {
                    auto allocated = new extra_data_t{.ka_solution = client.get_session()};
                    client.packets_state.extra_data = std::shared_ptr<void>((void*)allocated, [](void* d) { delete reinterpret_cast<extra_data_t*>(d); });
                }
                return *reinterpret_cast<extra_data_t*>(client.packets_state.extra_data.get());
            }
        };

        void OnRegister(const PluginRegistrationPtr&) override {
            register_event(api::players::handlers::on_disconnect, base_objects::events::priority::high, [this](const base_objects::client_data_holder& hold) {
                if (hold->packets_state.state == base_objects::SharedClientData::packets_state_t::protocol_state::play) {
                    if (hold->packets_state.is_play_fully_initialized) {
                        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                            plugin->PlayerLeave(*hold);
                        });
                    }

                    auto data = players_data.get_player_data(hold->data->uuid_str);
                    data.player = std::move(hold->player_data);
                    data.save();
                    for (auto& plugin : hold->compatible_plugins)
                        plugin->OnPlay_uninitialized_compatible(*hold);

                    pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                        plugin->OnPlay_uninitialized(*hold);
                    });
                    if (hold->packets_state.is_play_fully_initialized) {
                        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                            plugin->PlayerLeft(*hold);
                        });
                    }
                    [[maybe_unused]] auto _ = api::entity_id_map::remove_id(hold->data->uuid);
                }
                return false;
            });
            api::packets::register_server_bound_processor<api::packets::server_bound::configuration::finish_configuration>([this](api::packets::server_bound::configuration::finish_configuration&&, base_objects::SharedClientData& client) {
                extra_data_t::get(client).ka_solution.set_callback([](int64_t res, base_objects::SharedClientData& client) {
                    client << api::packets::client_bound::configuration::keep_alive{.keep_alive_id = (uint64_t)res};
                });
                extra_data_t::get(client).ka_solution.make_keep_alive_packet();

                auto client_ref = api::players::get_player(client);
                {
                    auto data = players_data.get_player_data(client.data->uuid_str);
                    data.load();
                    data.player.assigned_entity->id = client.data->uuid;
                    data.player.assigned_entity->assigned_player = client_ref;
                    client.player_data = std::move(data.player);
                }
                bool world_debug = false;
                bool world_flat = false;
                bool enable_respawn_screen = false;
                bool reduced_debug_info = false;
                bool do_limited_crafting = false;
                int8_t difficulty = 0;
                bool difficulty_locked = false;
                int32_t world_type = 0;
                int64_t hashed_seed = 0;

                auto last_death_location = client.player_data.last_death_location
                                               ? std::optional(
                                                     api::client::play::login::death_location_t(
                                                         client.player_data.last_death_location->world_id,
                                                         {(int32_t)client.player_data.last_death_location->x, (int32_t)client.player_data.last_death_location->y, (int32_t)client.player_data.last_death_location->z}
                                                     )
                                                 )
                                               : std::nullopt;


                auto [world_id, world_name] = api::world::prepare_world(client);

                api::world::get(world_id, [&](storage::world_data& data) {
                    world_debug = data.world_generator_data.contains("debug") ? (bool)data.world_generator_data["debug"] : false;
                    world_flat = data.world_generator_data.contains("flat") ? (bool)data.world_generator_data["flat"] : false;
                    enable_respawn_screen = !(data.world_game_rules.contains("doImmediateRespawn") ? (bool)data.world_game_rules["doImmediateRespawn"] : false);
                    reduced_debug_info = data.world_game_rules.contains("reducedDebugInfo") ? (bool)data.world_game_rules["reducedDebugInfo"] : false;
                    do_limited_crafting = data.world_game_rules.contains("doLimitedCrafting") ? (bool)data.world_game_rules["doLimitedCrafting"] : false;
                    difficulty = data.difficulty;
                    difficulty_locked = data.difficulty_locked;
                    world_type = registers::dimensionTypes.at(data.world_type).id;
                });
                auto player_entity_id = api::entity_id_map::allocate_id(client.data->uuid);
                api::entity_id_map::assign_entity(player_entity_id, client.player_data.assigned_entity);
                client.player_data.assigned_entity->protocol_id = player_entity_id;

                client << api::client::play::login{
                    .entity_id = player_entity_id,
                    .is_hardcore = client.player_data.hardcore_hearts,
                    .dimension_names = api::world::request_names().convert<base_objects::identifier>(),
                    .max_players = (int32_t)api::configuration::get().server.max_players,
                    .view_distance = client.view_distance,
                    .simulation_distance = client.simulation_distance,
                    .reduced_debug_info = reduced_debug_info,
                    .respawn_screen = enable_respawn_screen,
                    .limited_crafting_enabled = do_limited_crafting,
                    .dimension_type = world_type,
                    .dimension_name = world_name,
                    .seed_hashed = hashed_seed,
                    .gamemode = client.player_data.gamemode,
                    .prev_gamemode = client.player_data.prev_gamemode,
                    .world_is_debug = world_debug,
                    .world_is_flat = world_flat,
                    .death_location = last_death_location,
                    .portal_cooldown = 0, //TODO add
                    .sea_level = 0,       //TODO add
                    .enforce_secure_chat = api::configuration::get().mojang.enforce_secure_profile
                };
                client << api::client::play::change_difficulty{
                    .difficulty = (api::packets::difficulty_e)difficulty,
                    .is_locked = difficulty_locked
                };
                api::client::play::player_abilities::flags_f abilities_f{};
                if (client.player_data.abilities.flags.allow_flying)
                    abilities_f = abilities_f | api::client::play::player_abilities::allow_flying;
                if (client.player_data.abilities.flags.creative_mode)
                    abilities_f = abilities_f | api::client::play::player_abilities::creative_mode;
                if (client.player_data.abilities.flags.flying)
                    abilities_f = abilities_f | api::client::play::player_abilities::flying;
                if (client.player_data.abilities.flags.invulnerable)
                    abilities_f = abilities_f | api::client::play::player_abilities::invulnerable;
                client << api::client::play::player_abilities{
                    .flags = abilities_f,
                    .flying_speed = client.player_data.abilities.flying_speed,
                    .fov_modifier = client.player_data.abilities.field_of_view_modifier
                };
                client << api::client::play::set_held_slot{.slot = client.player_data.assigned_entity->get_selected_item()};
                client.player_data.assigned_entity->set_experience(client.player_data.assigned_entity->get_experience());

                for (auto& plugin : client.compatible_plugins)
                    plugin->OnPlay_initialize_compatible(client);

                pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                    plugin->OnPlay_initialize(client);
                });

                client.packets_state.is_fully_initialized = false;
                client.packets_state.is_play_fully_initialized = false;
                auto [yaw, pitch] = util::to_yaw_pitch(client.player_data.assigned_entity->rotation);
                client << api::client::play::player_position{
                    .teleport_id = 0, //TODO replace with automatic id
                    .x = client.player_data.assigned_entity->position.x,
                    .y = client.player_data.assigned_entity->position.y,
                    .z = client.player_data.assigned_entity->position.z,
                    .velocity_x = client.player_data.assigned_entity->motion.x,
                    .velocity_y = client.player_data.assigned_entity->motion.y,
                    .velocity_z = client.player_data.assigned_entity->motion.z,
                    .yaw = (float)yaw,
                    .pitch = (float)pitch,
                    .flags = api::packets::teleport_flags{}
                };
            });

            api::packets::register_server_bound_processor<accept_teleportation>([]([[maybe_unused]] accept_teleportation&& packet, base_objects::SharedClientData& client) {
                if (!client.packets_state.is_fully_initialized) {
                    client.packets_state.is_fully_initialized = true;
                    api::world::sync_settings(client);
                    api::world::register_entity(api::world::resolve_id(client.player_data.world_id), client.player_data.assigned_entity);

                    for (auto& plugin : client.compatible_plugins)
                        plugin->OnPlay_post_initialize_compatible(client);

                    pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                        plugin->OnPlay_post_initialize(client);
                    });
                }
            });

            api::packets::register_server_bound_processor<block_entity_tag_query>([](block_entity_tag_query&& packet, base_objects::SharedClientData& client) {
                if (client.player_data.assigned_entity)
                    if (client.player_data.assigned_entity->current_world())
                        client.player_data.assigned_entity->current_world()->get_block(
                            packet.location.x,
                            packet.location.y,
                            packet.location.z,
                            [](auto&) {},
                            [&](auto&, auto& enbt) {
                                client << api::client::play::tag_query{
                                    .tag_query_id = packet.tag_query_id,
                                    .nbt = enbt
                                };
                            }
                        );
            });

            api::packets::register_server_bound_processor<bundle_item_selected>([]([[maybe_unused]] bundle_item_selected&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<change_difficulty>([]([[maybe_unused]] change_difficulty&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<change_gamemode>([]([[maybe_unused]] change_gamemode&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<chat_ack>([]([[maybe_unused]] chat_ack&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            //api::packets::register_server_bound_processor<chat_command>([](chat_command&& packet, base_objects::SharedClientData& client) { //processed by play_engine
            //});

            api::packets::register_server_bound_processor<chat_command_signed>([]([[maybe_unused]] chat_command_signed&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<chat>([]([[maybe_unused]] chat&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<chat_session_update>([]([[maybe_unused]] chat_session_update&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<chunk_batch_received>([]([[maybe_unused]] chunk_batch_received&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<client_command>([](client_command&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                if (packet.action_id == client_command::action_id_e::perform_respawn) {
                    //TODO client << api::client::play::respawn{};
                } else if (packet.action_id == client_command::action_id_e::request_stats) {
                    //TODO client << api::client::play::award_stats{
                    //    .
                    //};
                }
            });

            api::packets::register_server_bound_processor<client_tick_end>([](client_tick_end&&, base_objects::SharedClientData& client) {
                client << api::client::play::ticking_step{};
            });

            api::packets::register_server_bound_processor<client_information>([](client_information&& packet, base_objects::SharedClientData& client) {
                client.locale = packet.locale.value;
                client.view_distance = (uint8_t)std::min<uint32_t>(packet.view_distance, api::configuration::get().game_play.view_distance);
                client.chat_mode = (base_objects::SharedClientData::ChatMode)packet.chat_mode.get();
                client.enable_chat_colors = packet.enable_chat_colors;
                client.skin_parts.mask = packet.displayer_skin_parts.get();
                client.main_hand = (base_objects::SharedClientData::MainHand)packet.main_hand.get();
                client.enable_filtering = packet.enable_text_filtering;
                client.allow_server_listings = packet.allow_server_listings;
                client.particle_status = (base_objects::SharedClientData::ParticleStatus)packet.particle_status.get();

                if (client.player_data.assigned_entity)
                    client.player_data.assigned_entity->get_syncing_data().update_render_distance(client.view_distance);
            });

            api::packets::register_server_bound_processor<command_suggestion>([](command_suggestion&& packet, base_objects::SharedClientData& client) {
                base_objects::command_context context(client, true);
                auto suggestions = api::command::get_manager().request_suggestions(packet.command_text.value, context);
                auto pos = packet.command_text.value.find_last_of(" /");
                if (pos == std::string::npos)
                    pos = 0;
                client << api::client::play::command_suggestions{
                    .suggestion_transaction_id = packet.suggestion_transaction_id,
                    .start = (int32_t)pos,
                    .length = int32_t(packet.command_text.value.size() - pos),
                    .matches = suggestions
                                   .convert_fn([](auto& it) {
                                       return api::client::play::command_suggestions::match{.set = it};
                                   })
                };
            });

            api::packets::register_server_bound_processor<configuration_acknowledged>([]([[maybe_unused]] configuration_acknowledged&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {});

            api::packets::register_server_bound_processor<container_button_click>([]([[maybe_unused]] container_button_click&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<container_click>([]([[maybe_unused]] container_click&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<container_close>([]([[maybe_unused]] container_close&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<container_slot_state_changed>([]([[maybe_unused]] container_slot_state_changed&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<cookie_response>([](cookie_response&& packet, base_objects::SharedClientData& client) {
                if (auto plugin = pluginManagement.get_bind_cookies(PluginManagement::registration_on::play, packet.key); plugin)
                    plugin->OnPlayCookie(plugin, packet.key, packet.payload ? *packet.payload : list_array<uint8_t>{}, client);
            });

            api::packets::register_server_bound_processor<custom_payload>([](custom_payload&& packet, base_objects::SharedClientData& client) {
                if (auto plugin = pluginManagement.get_bind_plugin(PluginManagement::registration_on::configuration, packet.channel); plugin)
                    plugin->OnPlayHandle(plugin, packet.channel, packet.payload, client);
            });

            api::packets::register_server_bound_processor<debug_sample_subscription>([]([[maybe_unused]] debug_sample_subscription&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<edit_book>([]([[maybe_unused]] edit_book&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<entity_tag_query>([]([[maybe_unused]] entity_tag_query&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                auto entity = api::entity_id_map::get_entity(packet.entity_id);
                if (entity)
                    client << api::client::play::tag_query{
                        .tag_query_id = packet.tag_query_id,
                        .nbt = entity->nbt //TODO check if required adding more info to nbt
                    };
            });

            api::packets::register_server_bound_processor<interact>([]([[maybe_unused]] interact&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<jigsaw_generate>([]([[maybe_unused]] jigsaw_generate&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<keep_alive>([](keep_alive&& packet, base_objects::SharedClientData& client) {
                auto delay = extra_data_t::get(client).ka_solution.got_valid_keep_alive((int64_t)packet.id);
                client.packets_state.keep_alive_ping_ms = (int32_t)std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(delay).count(), INT32_MAX);
            });

            api::packets::register_server_bound_processor<lock_difficulty>([]([[maybe_unused]] lock_difficulty&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<move_player_pos>([]([[maybe_unused]] move_player_pos&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<move_player_pos_rot>([]([[maybe_unused]] move_player_pos_rot&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<move_player_rot>([]([[maybe_unused]] move_player_rot&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<move_player_status_only>([]([[maybe_unused]] move_player_status_only&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });

            api::packets::register_server_bound_processor<move_vehicle>([]([[maybe_unused]] move_vehicle&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<paddle_boat>([]([[maybe_unused]] paddle_boat&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<pick_item_from_block>([]([[maybe_unused]] pick_item_from_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<pick_item_from_entity>([]([[maybe_unused]] pick_item_from_entity&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<ping_request>([](ping_request&& packet, base_objects::SharedClientData& client) {
                client << api::client::play::pong_response{.id = packet.payload};
            });
            api::packets::register_server_bound_processor<place_recipe>([]([[maybe_unused]] place_recipe&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<player_abilities>([]([[maybe_unused]] player_abilities&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<player_action>([]([[maybe_unused]] player_action&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<player_command>([]([[maybe_unused]] player_command&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<player_input>([]([[maybe_unused]] player_input&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<player_loaded>([]([[maybe_unused]] player_loaded&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<pong>([]([[maybe_unused]] pong&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<recipe_book_change_settings>([]([[maybe_unused]] recipe_book_change_settings&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<recipe_book_seen_recipe>([]([[maybe_unused]] recipe_book_seen_recipe&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<rename_item>([]([[maybe_unused]] rename_item&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<resource_pack>([]([[maybe_unused]] resource_pack&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<seen_advancements>([]([[maybe_unused]] seen_advancements&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<select_trade>([]([[maybe_unused]] select_trade&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<set_beacon>([]([[maybe_unused]] set_beacon&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<set_carried_item>([]([[maybe_unused]] set_carried_item&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<set_command_block>([]([[maybe_unused]] set_command_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<set_command_minecart>([]([[maybe_unused]] set_command_minecart&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<set_creative_mode_slot>([]([[maybe_unused]] set_creative_mode_slot&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<set_jigsaw_block>([]([[maybe_unused]] set_jigsaw_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<set_structure_block>([]([[maybe_unused]] set_structure_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<set_test_block>([]([[maybe_unused]] set_test_block&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<sign_update>([]([[maybe_unused]] sign_update&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<swing>([](swing&& packet, base_objects::SharedClientData& client) {
                if (client.player_data.assigned_entity)
                    if (client.player_data.assigned_entity->current_world())
                        client.player_data.assigned_entity->current_world()->locked([&](auto& world) {
                            world.entity_animation(
                                *client.player_data.assigned_entity,
                                packet.hand == swing::hand_e::main
                                    ? base_objects::entity_animation::swing_main_arm
                                    : base_objects::entity_animation::swing_offhand
                            );
                        });
            });
            api::packets::register_server_bound_processor<teleport_to_entity>([]([[maybe_unused]] teleport_to_entity&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<test_instance_block_action>([]([[maybe_unused]] test_instance_block_action&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<use_item_on>([]([[maybe_unused]] use_item_on&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<use_item>([]([[maybe_unused]] use_item&& packet, [[maybe_unused]] base_objects::SharedClientData& client) {
                //TODO
            });
            api::packets::register_server_bound_processor<custom_click_action>([](custom_click_action&& packet, base_objects::SharedClientData& client) {
                api::dialogs::pass_dialog(packet.id, client, std::move(packet.payload));
            });
        }
    };
}