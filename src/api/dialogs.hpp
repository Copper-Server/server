#ifndef SRC_API_DIALOGS
#define SRC_API_DIALOGS
#include <library/enbt/enbt.hpp>
#include <src/base_objects/events/event.hpp>
#include <string>
#include <functional>

namespace copper_server::base_objects {
    struct SharedClientData;
}

namespace copper_server::api::dialogs {
    //client could be in two states, configuration and play
    void register_dialog(const std::string& id, std::function<void(base_objects::SharedClientData& client, enbt::value&& payload)>&& fn);
    void pass_dialog(const std::string& id, base_objects::SharedClientData& client, enbt::value&& payload);
    void unload_dialog(const std::string& id);
}

#endif /* SRC_API_DIALOGS */
