#include <src/api/dialogs.hpp>
#include <unordered_map>
#include <library/fast_task.hpp>

namespace copper_server::api::dialogs {
    fast_task::protected_value<std::unordered_map<std::string, std::function<void(base_objects::SharedClientData& client, enbt::value&& payload)>>> value;

    void register_dialog(const std::string& id, std::function<void(base_objects::SharedClientData& client, enbt::value&& payload)>&& fn){
        value.set([&](auto& map) {
            map[id] = std::move(fn);
        });
    }

    void pass_dialog(const std::string& id, base_objects::SharedClientData& client, enbt::value&& payload) {
        auto fn = value.set([&](auto& map) {
            return map[id];
        });
        if (fn)
            fn(client, std::move(payload));
    }

    void unload_dialog(const std::string& id){
        value.set([&](auto& map) {
            map.erase(id);
        });
    }
}
