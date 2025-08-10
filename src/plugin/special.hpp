/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_PLUGIN_SPECIAL
#define SRC_PLUGIN_SPECIAL
#include <library/enbt/enbt.hpp>
#include <optional>
#include <src/base_objects/chat.hpp>
#include <string>

namespace copper_server {
    class SpecialPluginStatus {
    public:
        SpecialPluginStatus() {}

        virtual std::string StatusResponseVersionName() {
            return "Copper Server";
        }

        virtual bool ShowConnectionStatus() {
            return true;
        }

        virtual size_t MaxPlayers() {
            return 20;
        }

        virtual size_t OnlinePlayers() {
            return 0;
        }

        virtual std::optional<bool> PreventsChatReports() {
            return std::nullopt;
        }

        virtual std::string CustomJson() {
            return "";
        }

        //string also can be Chat json object!
        virtual list_array<std::pair<std::string, enbt::raw_uuid>> OnlinePlayersSample() {
            return {};
        }

        virtual Chat Description() {
            return "A server running in Copper Server";
        }

        virtual bool ConnectionAvailable(int32_t protocol_version) {
            return true;
        }

        //return empty string if no icon, icon must be 64x64 and png format in base64
        virtual std::string ServerIcon() {
            return "";
        }

        virtual ~SpecialPluginStatus() noexcept {}
    };

    extern SpecialPluginStatus* special_status;
}

#endif /* SRC_PLUGIN_SPECIAL */
