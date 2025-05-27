#include <src/api/command.hpp>
#include <src/api/packets.hpp>
#include <src/api/players.hpp>
#include <src/api/protocol.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    //handles clients with play state, allows players to access world and other things through api
    class PlayEngine : public PluginAutoRegister<"play_engine", PlayEngine> {
        fast_task::task_mutex messages_order;
        list_array<std::array<uint8_t, 256>> lastset_messages;

    public:
        PlayEngine() {}

        void OnLoad(const PluginRegistrationPtr& self) override {
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
        void OnUnload(const PluginRegistrationPtr& self) override{}

        void OnCommandsLoad(const PluginRegistrationPtr& self, base_objects::command_root_browser& browser) override {}

        plugin_response PlayerJoined(base_objects::client_data_holder& client_ref) override {
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
                if (&client != &*client_ref && !client.is_virtual) {
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
                    client.sendPacket(api::packets::play::playerInfoAdd(client, new_player));
                }
                return false;
            });
            response += api::packets::play::playerInfoAdd(*client_ref, all_players);
            response += api::packets::play::playerInfoAdd(*client_ref, new_player);
            return response;
        }
    };
}
