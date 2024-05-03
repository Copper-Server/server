#include "allowlist.hpp"
#include "ban.hpp"
#include "players.hpp"

namespace crafted_craft {
    namespace api {
        namespace allowlist {
            base_objects::event<allowlist_mode> on_mode_change;
            base_objects::event<std::string> on_kick;
            base_objects::event<std::string> on_add;
            base_objects::event<std::string> on_remove;
        }

        namespace ban {
            base_objects::event<ban_data> on_ban;
            base_objects::event<ban_data> on_pardon;
            base_objects::event<ban_data> on_ban_ip;
            base_objects::event<ban_data> on_pardon_ip;
        }

        namespace players {
            namespace handlers {
                base_objects::event<std::string> on_player_join;
                base_objects::event<std::string> on_player_leave;
            }

            namespace calls {
                base_objects::event<teleport_request> on_teleport_request;
                base_objects::event<player_chat> on_player_chat;
                base_objects::event<player_personal_chat> on_player_personal_chat;
                base_objects::event<Chat> on_system_message_broadcast;
                base_objects::event<personal<Chat>> on_system_message;
                base_objects::event<Chat> on_system_message_overlay_broadcast;
                base_objects::event<personal<Chat>> on_system_message_overlay;
                base_objects::event<personal<Chat>> on_player_kick;
                base_objects::event<personal<Chat>> on_player_ban;
                base_objects::event<Chat> on_action_bar_message_broadcast;
                base_objects::event<personal<Chat>> on_action_bar_message;
                base_objects::event<Chat> on_title_message_broadcast;
                base_objects::event<personal<Chat>> on_title_message;
                base_objects::event<Chat> on_subtitle_message_broadcast;
                base_objects::event<personal<Chat>> on_subtitle_message;
                base_objects::event<titles_times> on_title_times_broadcast;
                base_objects::event<personal<titles_times>> on_title_times;
                base_objects::event<unsigned_chat> on_unsigned_message_broadcast;
                base_objects::event<personal<unsigned_chat>> on_unsigned_message;
            }
        }
    }
}
