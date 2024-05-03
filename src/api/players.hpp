#ifndef SRC_API_PLAYERS
#define SRC_API_PLAYERS
#include "../base_objects/chat.hpp"
#include "../base_objects/event.hpp"
#include "../base_objects/position.hpp"
#include "../library/list_array.hpp"
#include <array>
#include <optional>
#include <string>

namespace crafted_craft {
    namespace api {
        namespace players {
            namespace handlers {
                extern base_objects::event<std::string> on_player_join;
                extern base_objects::event<std::string> on_player_leave;
            }

            template <typename T>
            struct personal {
                std::string player;
                T data;
            };

            struct titles_times {
                //all in ticks
                int32_t fade_in;
                int32_t stay;
                int32_t fade_out;
            };

            struct unsigned_chat {
                Chat message;
                int32_t chat_type_id; //get id from registry
                Chat sender_name;
                std::optional<Chat> receiver_name;
            };

            struct player_chat {
                std::string sender;
                std::string message;

                std::optional<std::array<uint8_t, 256>> signature;
                uint64_t salt;
                list_array<std::array<uint8_t, 256>> previous_messages;
                int32_t chat_type_id; //get id from registry
            };

            struct player_personal_chat : public player_chat {
                std::string receiver;
            };

            namespace calls {

                struct teleport_request {
                    std::string player;
                    Position position;
                };

                extern base_objects::event<teleport_request> on_teleport_request;


                extern base_objects::event<player_chat> on_player_chat;
                extern base_objects::event<player_personal_chat> on_player_personal_chat;

                extern base_objects::event<Chat> on_system_message_broadcast;
                extern base_objects::event<personal<Chat>> on_system_message;

                extern base_objects::event<Chat> on_system_message_overlay_broadcast;
                extern base_objects::event<personal<Chat>> on_system_message_overlay;

                extern base_objects::event<personal<Chat>> on_player_kick;
                extern base_objects::event<personal<Chat>> on_player_ban;

                extern base_objects::event<Chat> on_action_bar_message_broadcast;
                extern base_objects::event<personal<Chat>> on_action_bar_message;

                extern base_objects::event<Chat> on_title_message_broadcast;
                extern base_objects::event<personal<Chat>> on_title_message;

                extern base_objects::event<Chat> on_subtitle_message_broadcast;
                extern base_objects::event<personal<Chat>> on_subtitle_message;

                extern base_objects::event<titles_times> on_title_times_broadcast;
                extern base_objects::event<personal<titles_times>> on_title_times;

                extern base_objects::event<unsigned_chat> on_unsigned_message_broadcast;
                extern base_objects::event<personal<unsigned_chat>> on_unsigned_message;
            }
        }
    }
}

#endif /* SRC_API_PLAYERS */
