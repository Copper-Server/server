/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/client.hpp>
#include <src/api/command.hpp>
#include <src/api/configuration.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/players.hpp>
#include <src/api/registers.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::base::play_engine {
    struct player_info : public PluginAutoRegister<"base/play_engine/player_info", player_info> {
        player_info() {}

        ~player_info() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            register_event(api::players::handlers::on_gamemode_changed, [](base_objects::SharedClientData& client) {
                using piu = api::client::play::player_info_update;
                piu current;
                current.actions.push(piu::header{client.data->uuid});
                current.actions.push(piu::set_gamemode{.gamemode = client.player_data.gamemode});
                api::players::iterate_online([&current](base_objects::SharedClientData& client) {
                    if (!client.is_virtual)
                        client << piu{current};
                    return false;
                });
            });
            register_event(api::players::handlers::on_tab_listing_changed, [](base_objects::SharedClientData& client) {
                using piu = api::client::play::player_info_update;
                piu current;
                current.actions.push(piu::header{client.data->uuid});
                current.actions.push(piu::listed{.should = client.enable_tab_listings});
                api::players::iterate_online([&current](base_objects::SharedClientData& client) {
                    if (!client.is_virtual)
                        client << piu{current};
                    return false;
                });
            });
            register_event(api::players::handlers::on_skin_parts_changed, [](base_objects::SharedClientData& client) {
                using piu = api::client::play::player_info_update;
                piu current;
                current.actions.push(piu::header{client.data->uuid});
                current.actions.push(piu::set_hat_visible{.visible = client.skin_parts.data.hat_enabled});
                api::players::iterate_online([&current](base_objects::SharedClientData& client) {
                    if (!client.is_virtual)
                        client << piu{current};
                    return false;
                });
            });
        }

        static void push_player_info_action(api::client::play::player_info_update& res, base_objects::SharedClientData& client_ref) {
            using piu = api::client::play::player_info_update;
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

        void OnPlay_pre_initialize(base_objects::SharedClientData& client_ref) override {
            using piu = api::client::play::player_info_update;
            base_objects::network::response response = base_objects::network::response::empty();

            piu all_players;
            piu new_player;
            push_player_info_action(new_player, client_ref);
            api::players::iterate_online([&new_player, &all_players, &client_ref](base_objects::SharedClientData& client) {
                if (&client != &client_ref && !client.is_virtual) {
                    push_player_info_action(all_players, client);
                    client << piu{new_player};
                }
                return false;
            });
            client_ref << std::move(all_players);
            client_ref << std::move(new_player);
        }

        void PlayerLeave(base_objects::SharedClientData& client_ref) override {
            api::players::iterate_online([&client_ref](base_objects::SharedClientData& client) {
                if (&client != &client_ref && !client.is_virtual)
                    client << api::client::play::player_info_remove{
                        .uuids = {client_ref.data->uuid}
                    };
                return false;
            });
        }
    };
}