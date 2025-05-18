#ifndef SRC_BASE_OBJECTS_PACKETS_ENUMS
#define SRC_BASE_OBJECTS_PACKETS_ENUMS
#include <cstdint>

namespace copper_server::base_objects::packets {
    enum class game_event_id : uint8_t {     //[Argument values]
        no_respawn_block_available = 0,      //none
        raining_begin = 1,                   //none
        raining_end = 2,                     //none
        gamemode_change = 3,                 //0 - survival, 1 - creative, 2 - adventure, 3 - spectator
        win_game = 4,                        //0 - respawn, 1 - roll credits and respawn
        demo_event = 5,                      //0 - welcome, 101 - movement, 102 - jump, 103 - inventory, 104 - end screen
        arrow_hit_player = 6,                //none
        rain_level_change = 7,               //from 0 to 1
        thunder_level_change = 8,            //from 0 to 1
        puffer_fish_sting = 9,               //none
        guardian_appear = 10,                //none
        disable_respawn_screen = 11,         //0 and 1
        enable_limited_crafting = 12,        //0 and 1
        start_waiting_for_level_chunks = 13, //none
    };
}

#endif /* SRC_BASE_OBJECTS_PACKETS_ENUMS */
