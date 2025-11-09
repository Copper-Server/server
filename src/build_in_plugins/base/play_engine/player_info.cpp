/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/command.hpp>
#include <src/api/configuration.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/api/packets/server_bound/play.hpp>
#include <src/api/players.hpp>
#include <src/api/registers.hpp>
#include <src/api/selector.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/uuid.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::base::play_engine {
    struct player_info : public plugin_auto_register<"base/play_engine/player_info", player_info> {
        player_info() {}

        ~player_info() noexcept {}

        void on_initialization(const plugin_registration_ptr& _) override {
            register_event(api::players::handlers::on_gamemode_changed, [](base_objects::shared_client_data& client) {
                using piu = api::packets::client_bound::play::player_info_update;
                piu current;
                current.actions.push(piu::header{client.data->uuid});
                current.actions.push(piu::set_gamemode{.gamemode = client.player_data.gamemode});
                api::players::iterate_online([&current](base_objects::shared_client_data& client) {
                    if (!client.is_virtual)
                        client << piu{current};
                    return false;
                });
            });
            register_event(api::players::handlers::on_tab_listing_changed, [](base_objects::shared_client_data& client) {
                using piu = api::packets::client_bound::play::player_info_update;
                piu current;
                current.actions.push(piu::header{client.data->uuid});
                current.actions.push(piu::listed{.should = client.enable_tab_listings});
                api::players::iterate_online([&current](base_objects::shared_client_data& client) {
                    if (!client.is_virtual)
                        client << piu{current};
                    return false;
                });
            });
            register_event(api::players::handlers::on_skin_parts_changed, [](base_objects::shared_client_data& client) {
                using piu = api::packets::client_bound::play::player_info_update;
                piu current;
                current.actions.push(piu::header{client.data->uuid});
                current.actions.push(piu::set_hat_visible{.visible = client.skin_parts.data.hat_enabled});
                api::players::iterate_online([&current](base_objects::shared_client_data& client) {
                    if (!client.is_virtual)
                        client << piu{current};
                    return false;
                });
            });
        }

        static void push_player_info_action(api::packets::client_bound::play::player_info_update& res, base_objects::shared_client_data& client_ref) {
            using piu = api::packets::client_bound::play::player_info_update;
            res.actions.push(piu::header{client_ref.data->uuid});
            res.actions.push(
                piu::add_player{
                    .name = client_ref.name,
                    .properties
                    = to_list_array(client_ref.data->properties)
                          .convert_fn([](auto&& mojang) {
                              return piu::add_player::property{
                                  .name = std::move(mojang.name),
                                  .value = std::move(mojang.value),
                                  .signature = std::move(mojang.signature)
                              };
                          })
                }
            );
            res.actions.push(piu::listed{.should = client_ref.enable_tab_listings});
            res.actions.push(piu::set_gamemode{.gamemode = client_ref.player_data.gamemode});
            res.actions.push(piu::set_hat_visible{.visible = client_ref.skin_parts.data.hat_enabled});
            res.actions.push(piu::set_ping{.milliseconds = (int32_t)client_ref.ping.count()});
        }

        void on_play_pre_initialize(base_objects::shared_client_data& client_ref) override {
            using piu = api::packets::client_bound::play::player_info_update;
            piu all_players;
            piu new_player;
            push_player_info_action(new_player, client_ref);
            api::players::iterate_online([&new_player, &all_players, &client_ref](base_objects::shared_client_data& client) {
                if (&client != &client_ref && !client.is_virtual) {
                    push_player_info_action(all_players, client);
                    client << piu{new_player};
                }
                return false;
            });
            client_ref << std::move(all_players);
            client_ref << std::move(new_player);
        }

        void player_leave(base_objects::shared_client_data& client_ref) override {
            api::players::iterate_online([&client_ref](base_objects::shared_client_data& client) {
                if (&client != &client_ref && !client.is_virtual)
                    client << api::packets::client_bound::play::player_info_remove{
                        .uuids = {client_ref.data->uuid}
                    };
                return false;
            });
        }

        static bool change_player_gamemode(base_objects::shared_client_data& client, uint8_t gamemode) {
            if (client.player_data.gamemode == gamemode)
                return false;
            client.player_data.prev_gamemode = client.player_data.gamemode;
            client.player_data.gamemode = gamemode;
            using piu = api::packets::client_bound::play::player_info_update;
            piu data;
            data.actions.push(piu::header{client.data->uuid});
            data.actions.push(piu::set_gamemode{.gamemode = client.player_data.gamemode});
            api::players::iterate_online([&data](base_objects::shared_client_data& client) {
                client << piu{data};
                return false;
            });

            client << api::packets::client_bound::play::game_event{
                .event = api::packets::client_bound::play::game_event::gamemode_change{
                    .gamemode = (float)gamemode
                }
            };
            return true;
        }

        void on_commands_load(const plugin_registration_ptr& _, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using cmd_pred_gamemode = base_objects::parsers::command::gamemode;
            using cmd_pred_entity = base_objects::parsers::command::entity;
            using pred_string = base_objects::parsers::string;
            using pred_gamemode = base_objects::parsers::gamemode;
            using pred_entity = base_objects::parsers::entity;

            {
                browser
                    .add_child("gamemode")
                    .add_child("gamemode", cmd_pred_gamemode{})
                    .set_callback("command.gamemode", [](const list_array<predicate>& args, base_objects::command_context& context) {
                        return (uint32_t)change_player_gamemode(context.executor, (uint8_t)std::get<pred_gamemode>(args[0]));
                    })
                    .add_child("target", cmd_pred_entity{.flag = cmd_pred_entity::only_player_entity})
                    .set_callback("command.gamemode.target", [](const list_array<predicate>& args, base_objects::command_context& context) -> uint32_t {
                        auto gamemode = (uint8_t)std::get<pred_gamemode>(args[0]);
                        auto selector = std::get<pred_entity>(args[1]);
                        switch (selector.type) {
                        case pred_entity::type_t::name: {
                            auto player_ref = api::players::get_player(base_objects::shared_client_data::packets_state_t::protocol_state::play, selector.value);
                            if (player_ref)
                                return change_player_gamemode(*player_ref, gamemode);
                            break;
                        }
                        case pred_entity::type_t::selector: {
                            uint32_t count = 0;
                            api::selector(selector.value).select(context, [&count, gamemode](api::ecs::entity entity) {
                                if (entity.has<api::ecs::com::assigned_player>()) {
                                    auto player_hold = entity.modify<api::ecs::com::assigned_player>();
                                    auto player_ref = player_hold->player;
                                    if (player_ref)
                                        count += change_player_gamemode(*player_ref, gamemode);
                                }
                            });
                            return count;
                        }
                        case pred_entity::type_t::uuid: {
                            base_objects::uuid uuid;
                            base_objects::uuid::from_uuid_string(uuid, selector.value);
                            auto entity = api::entity_id_map::get_entity(uuid);
                            if (entity) {
                                if (entity->has<api::ecs::com::assigned_player>()) {
                                    auto player_hold = entity->modify<api::ecs::com::assigned_player>();
                                    auto player_ref = player_hold->player;
                                    if (player_ref)
                                        return change_player_gamemode(*player_ref, gamemode);
                                }
                            }
                            break;
                        }
                        }
                        return 0;
                    });
            }
        }
    };
}