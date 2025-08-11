/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/storage/memory/online_player.hpp>
#include <src/base_objects/selector.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>

namespace copper_server::storage::memory {
    void online_player_storage::apply_selector(base_objects::SharedClientData& caller, const std::string& selector, std::function<void(base_objects::SharedClientData&)>&& callback) {
        base_objects::selector sel;
        sel.build_selector(selector);
        base_objects::command_context context(caller, true);
        sel.flags.only_players = true;
        sel.flags.only_entities = false;
        sel.select(context, [&callback](base_objects::entity& entity) {
            auto pl = entity.assigned_player;
            if (pl)
                callback(*pl);
        });
    }
}