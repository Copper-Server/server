#include <src/api/command.hpp>
#include <src/api/players.hpp>
#include <src/api/protocol.hpp>
#include <src/base_objects/player.hpp>
#include <src/build_in_plugins/play_engine.hpp>
#include <src/protocolHelper/packets/abstract.hpp>

namespace copper_server::build_in_plugins {
    PlayEngine::PlayEngine() {}

    void PlayEngine::OnLoad(const PluginRegistrationPtr& self) {
        register_event(api::protocol::on_chat_command, [this](const auto& event) {
            base_objects::command_context context(event.client_data, true);
            try {
                api::command::get_manager().execute_command(event.data, context);
            } catch (base_objects::command_exception& ex) {
                try {
                    std::rethrow_exception(ex.exception);
                } catch (const std::exception& inner_ex) {
                    std::string error_message = event.data;
                    std::string error_place(event.data.size() + 4, ' ');
                    error_place[0] = '\n';
                    error_place[error_place.size() - 2] = '\n';
                    error_place[error_place.size() - 1] = '\t';
                    if (ex.pos != -1)
                        error_place[ex.pos] = '^';
                    api::players::calls::on_system_message({event.client_data, error_message + error_place + inner_ex.what()});
                }
            }
            return false;
        });
    }

    void PlayEngine::OnUnload(const PluginRegistrationPtr& self) {
    }

    void PlayEngine::OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) {
    }

    PlayEngine::plugin_response PlayEngine::PlayerJoined(base_objects::client_data_holder& client_ref) {
        base_objects::network::response response = base_objects::network::response::empty();
        list_array<base_objects::packets::player_actions_add> all_players;
        list_array<base_objects::packets::player_actions_add> new_player;
        new_player.push_back(
            base_objects::packets::player_actions_add{
                .player_id = client_ref->data->uuid,
                .name = client_ref->name,
                .properties = to_list_array(client_ref->data->properties).convert_fn([](auto&& mojang) {
                    return base_objects::packets::player_actions_add::property{
                        .name = std::move(mojang.name),
                        .value = std::move(mojang.value),
                        .signature = std::move(mojang.signature)
                    };
                })
            }
        );
        api::players::iterate_online([&new_player, &all_players, &client_ref](base_objects::SharedClientData& client) {
            if (&client != &*client_ref) {
                all_players.push_back(
                    base_objects::packets::player_actions_add{
                        .player_id = client.data->uuid,
                        .name = client.name,
                        .properties = to_list_array(client.data->properties).convert_fn([](auto&& mojang) {
                            return base_objects::packets::player_actions_add::property{
                                .name = std::move(mojang.name),
                                .value = std::move(mojang.value),
                                .signature = std::move(mojang.signature)
                            };
                        })
                    }
                );
                client.sendPacket(packets::play::playerInfoAdd(client, new_player));
            }
            return false;
        });
        response += packets::play::playerInfoAdd(*client_ref, all_players);
        response += packets::play::playerInfoAdd(*client_ref, new_player);
        return response;
    }
}
