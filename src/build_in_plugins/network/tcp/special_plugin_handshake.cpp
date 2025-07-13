#include <src/build_in_plugins/network/tcp/special_plugin_handshake.hpp>

namespace copper_server::build_in_plugins::network::tcp {


    SpecialPluginHandshakeRegistration::SpecialPluginHandshakeRegistration() {}

    void SpecialPluginHandshakeRegistration::register_handle(std::unique_ptr<SpecialPluginHandshake>&& self) {
        list.set([&](auto& arr) {
            arr.push_back(std::move(self));
        });
    }

    void SpecialPluginHandshakeRegistration::unregister_handle(SpecialPluginHandshake* self) {
        list.set([&](auto& arr) {
            arr.remove_if([&](auto& it) {
                return it.get() == self;
            });
        });
    }

    //return true to break
    void SpecialPluginHandshakeRegistration::enum_handles(const std::function<bool(SpecialPluginHandshake&)>& callback) {
        list.get([&](auto& arr) {
            for (auto& item : arr)
                if (callback(*item))
                    break;
        });
    }
}
