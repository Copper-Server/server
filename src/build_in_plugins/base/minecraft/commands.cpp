/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/log.hpp>
#include <src/api/packets/client_bound/play.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/player.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/plugin/main.hpp>
#include <src/util/readers.hpp>

namespace copper_server::build_in_plugins::base::minecraft {
    struct commands : public plugin_auto_register<"base/minecraft/commands", commands> {
        void on_commands_load(const plugin_registration_ptr& _, base_objects::command_root_browser& browser) override {
            using predicate = base_objects::parser;
            using cmd_pred_gamemode = base_objects::parsers::command::gamemode;
            using cmd_pred_entity = base_objects::parsers::command::entity;
            using pred_string = base_objects::parsers::string;
            using pred_gamemode = base_objects::parsers::gamemode;
            using pred_entity = base_objects::parsers::entity;

            ///advancement
            ///attribute
            ///ban DONE
            ///ban-ip DONE
            ///banlist DONE
            ///bossbar
            ///clear
            ///clone
            ///damage
            ///data
            ///datapack
            ///debug
            ///defaultgamemode
            ///deop  DONE
            ///dialog
            ///difficulty
            ///effect
            ///enchant
            ///execute
            ///experience
            ///fetchprofile
            ///fill
            ///fillbiome
            ///forceload
            ///function
            ///gamemode DONE

            ///gamerule
            ///give
            ///help DONE
            ///item
            ///kick
            ///kill
            ///list
            ///locate
            ///loot
            ///me
            ///msg DONE
            ///op DONE
            ///pardon DONE
            ///pardon-ip DONE
            ///particle
            ///perf
            ///place
            ///playsound
            ///random
            ///recipe
            ///reload
            ///return
            ///ride
            ///rotate
            ///save-all
            ///save-off
            ///save-on
            ///say
            ///schedule
            ///scoreboard
            ///seed
            ///setblock
            ///setidletimeout
            ///setworldspawn
            ///spawnpoint
            ///spectate
            ///spreadplayers
            ///stop
            ///stopsound
            ///summon
            ///tag
            ///team
            ///teammsg
            ///teleport
            ///tell
            ///tellraw
            ///test
            ///tick
            ///time
            ///title
            ///tm
            ///tp
            ///transfer
            ///trigger
            ///version DONE
            ///w
            ///waypoint
            ///weather
            ///whitelist PARTIAL the implementation doesn't support the selectors done as alias for white list
            ///worldborder
            ///xp
        }
    };
}
