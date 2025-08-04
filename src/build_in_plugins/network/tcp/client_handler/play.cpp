#include <src/api/client.hpp>
#include <src/api/configuration.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/new_packets.hpp>
#include <src/api/players.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/build_in_plugins/network/tcp/util.hpp>
#include <src/plugin/main.hpp>
#include <src/registers.hpp>
#include <src/storage/players_data.hpp>
namespace copper_server::build_in_plugins::network::tcp::client_handler {
    using accept_teleportation = api::new_packets::server_bound::play::accept_teleportation;
    using block_entity_tag_query = api::new_packets::server_bound::play::block_entity_tag_query;
    using bundle_item_selected = api::new_packets::server_bound::play::bundle_item_selected;
    using change_difficulty = api::new_packets::server_bound::play::change_difficulty;
    using change_gamemode = api::new_packets::server_bound::play::change_gamemode;
    using chat_ack = api::new_packets::server_bound::play::chat_ack;
    using chat_command = api::new_packets::server_bound::play::chat_command;
    using chat_command_signed = api::new_packets::server_bound::play::chat_command_signed;
    using chat = api::new_packets::server_bound::play::chat;
    using chat_session_update = api::new_packets::server_bound::play::chat_session_update;
    using chunk_batch_received = api::new_packets::server_bound::play::chunk_batch_received;
    using client_command = api::new_packets::server_bound::play::client_command;
    using client_tick_end = api::new_packets::server_bound::play::client_tick_end;
    using client_information = api::new_packets::server_bound::play::client_information;
    using command_suggestion = api::new_packets::server_bound::play::command_suggestion;
    using configuration_acknowledged = api::new_packets::server_bound::play::configuration_acknowledged;
    using container_button_click = api::new_packets::server_bound::play::container_button_click;
    using container_click = api::new_packets::server_bound::play::container_click;
    using container_close = api::new_packets::server_bound::play::container_close;
    using container_slot_state_changed = api::new_packets::server_bound::play::container_slot_state_changed;
    using cookie_response = api::new_packets::server_bound::play::cookie_response;
    using custom_payload = api::new_packets::server_bound::play::custom_payload;
    using debug_sample_subscription = api::new_packets::server_bound::play::debug_sample_subscription;
    using edit_book = api::new_packets::server_bound::play::edit_book;
    using entity_tag_query = api::new_packets::server_bound::play::entity_tag_query;
    using interact = api::new_packets::server_bound::play::interact;
    using jigsaw_generate = api::new_packets::server_bound::play::jigsaw_generate;
    using keep_alive = api::new_packets::server_bound::play::keep_alive;
    using lock_difficulty = api::new_packets::server_bound::play::lock_difficulty;
    using move_player_pos = api::new_packets::server_bound::play::move_player_pos;
    using move_player_pos_rot = api::new_packets::server_bound::play::move_player_pos_rot;
    using move_player_rot = api::new_packets::server_bound::play::move_player_rot;
    using move_player_status_only = api::new_packets::server_bound::play::move_player_status_only;
    using move_vehicle = api::new_packets::server_bound::play::move_vehicle;
    using paddle_boat = api::new_packets::server_bound::play::paddle_boat;
    using pick_item_from_block = api::new_packets::server_bound::play::pick_item_from_block;
    using pick_item_from_entity = api::new_packets::server_bound::play::pick_item_from_entity;
    using ping_request = api::new_packets::server_bound::play::ping_request;
    using place_recipe = api::new_packets::server_bound::play::place_recipe;
    using player_abilities = api::new_packets::server_bound::play::player_abilities;
    using player_action = api::new_packets::server_bound::play::player_action;
    using player_command = api::new_packets::server_bound::play::player_command;
    using player_input = api::new_packets::server_bound::play::player_input;
    using player_loaded = api::new_packets::server_bound::play::player_loaded;
    using pong = api::new_packets::server_bound::play::pong;
    using recipe_book_change_settings = api::new_packets::server_bound::play::recipe_book_change_settings;
    using recipe_book_seen_recipe = api::new_packets::server_bound::play::recipe_book_seen_recipe;
    using rename_item = api::new_packets::server_bound::play::rename_item;
    using resource_pack = api::new_packets::server_bound::play::resource_pack;
    using seen_advancements = api::new_packets::server_bound::play::seen_advancements;
    using select_trade = api::new_packets::server_bound::play::select_trade;
    using set_beacon = api::new_packets::server_bound::play::set_beacon;
    using set_carried_item = api::new_packets::server_bound::play::set_carried_item;
    using set_command_block = api::new_packets::server_bound::play::set_command_block;
    using set_command_minecart = api::new_packets::server_bound::play::set_command_minecart;
    using set_creative_mode_slot = api::new_packets::server_bound::play::set_creative_mode_slot;
    using set_jigsaw_block = api::new_packets::server_bound::play::set_jigsaw_block;
    using set_structure_block = api::new_packets::server_bound::play::set_structure_block;
    using set_test_block = api::new_packets::server_bound::play::set_test_block;
    using sign_update = api::new_packets::server_bound::play::sign_update;
    using swing = api::new_packets::server_bound::play::swing;
    using teleport_to_entity = api::new_packets::server_bound::play::teleport_to_entity;
    using test_instance_block_action = api::new_packets::server_bound::play::test_instance_block_action;
    using use_item_on = api::new_packets::server_bound::play::use_item_on;
    using use_item = api::new_packets::server_bound::play::use_item;
    using custom_click_action = api::new_packets::server_bound::play::custom_click_action;

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
                            plugin->OnPlayLeave(*hold);
                        });
                    }

                    auto data = players_data.get_player_data(hold->data->uuid_str);
                    data.player = std::move(hold->player_data);
                    data.save();
                    for (auto& plugin : hold->compatible_plugins)
                        pluginManagement.getPlugin(plugin)->OnPlay_uninitialized_compatible(*hold);

                    pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                        plugin->OnPlay_uninitialized(*hold);
                    });
                    if (hold->packets_state.is_play_fully_initialized) {
                        pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                            plugin->OnPlayLeft(*hold);
                        });
                    }
                }
                return false;
            });
            api::new_packets::register_server_bound_processor<api::new_packets::server_bound::configuration::finish_configuration>([this](api::new_packets::server_bound::configuration::finish_configuration&& packet, base_objects::SharedClientData& client) {
                extra_data_t::get(client).ka_solution.set_callback([](int64_t res, base_objects::SharedClientData& client) {
                    client << api::new_packets::client_bound::configuration::keep_alive{.keep_alive_id = (uint64_t)res};
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

                auto last_death_location = client.player_data.last_death_location ? std::optional(base_objects::packets::death_location_data(client.player_data.last_death_location->world_id, {(int32_t)client.player_data.last_death_location->x, (int32_t)client.player_data.last_death_location->y, (int32_t)client.player_data.last_death_location->z}))
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
                    .dimension_names = api::world::request_names().to_container<std::vector<base_objects::identifier>>(),
                    .max_players = api::configuration::get().server.max_players,
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
                    .portal_cooldown = 0, //TODO add
                    .sea_level = 0,       //TODO add
                    .enforce_secure_chat = api::configuration::get().mojang.enforce_secure_profile
                };
                client << api::client::play::change_difficulty{
                    .difficulty = difficulty,
                    .is_locked = difficulty_locked
                };
                api::client::play::player_abilities::flags_f abilities_f;
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
                    pluginManagement.getPlugin(plugin)->OnPlay_initialize_compatible(client);

                pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                    plugin->OnPlay_initialize(client);
                });

                client.packets_state.is_fully_initialized = false;
                client.packets_state.is_play_fully_initialized = false;
                auto [yaw, pitch] = util::to_yaw_pitch(client.player_data.assigned_entity->rotation);
                client << api::client::play::player_position{
                    .x = client.player_data.assigned_entity->position.x,
                    .y = client.player_data.assigned_entity->position.y,
                    .z = client.player_data.assigned_entity->position.z,
                    .velocity_x = client.player_data.assigned_entity->motion.x,
                    .velocity_y = client.player_data.assigned_entity->motion.y,
                    .velocity_z = client.player_data.assigned_entity->motion.z,
                    .yaw = (float)client.player_data.assigned_entity->rotation.x,
                    .pitch = (float)client.player_data.assigned_entity->rotation.y,
                    .flags = api::new_packets::teleport_flags{}
                };
            });
            api::new_packets::register_server_bound_processor<accept_teleportation>([this](accept_teleportation&& packet, base_objects::SharedClientData& client) {
                if (!client.packets_state.is_fully_initialized) {
                    client.packets_state.is_fully_initialized = true;
                    api::world::sync_settings(client);
                    api::world::register_entity(api::world::resolve_id(client.player_data.world_id), client.player_data.assigned_entity);

                    for (auto& plugin : client.compatible_plugins)
                        pluginManagement.getPlugin(plugin)->OnPlay_post_initialize_compatible(client);

                    pluginManagement.inspect_plugin_registration(PluginManagement::registration_on::play, [&](auto&& plugin) {
                        plugin->OnPlay_post_initialize(client);
                    });
                }
            });


            api::new_packets::register_server_bound_processor<keep_alive>([](keep_alive&& packet, base_objects::SharedClientData& client) {
                auto delay = extra_data_t::get(client).ka_solution.got_valid_keep_alive((int64_t)packet.id);
                client.packets_state.keep_alive_ping_ms = (int32_t)std::min<int64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(delay).count(), INT32_MAX);
            });
        }
    };
}