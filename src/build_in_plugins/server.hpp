#ifndef SRC_BUILD_IN_PLUGINS_WORLD
#define SRC_BUILD_IN_PLUGINS_WORLD
#include "../plugin/registration.hpp"
#include "../protocolHelper/state_play.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class Server : public PluginRegistration {
        public:
            void OnLoad(const PluginRegistrationPtr& self) override {
                std::cout << "World plugin loaded!" << std::endl;
            }

            plugin_response OnPlay_playerEnteredWorld(SharedClientData& client) override {
                //load client stored data
                //prepare associated with client data(world, plugins, etc...)

                return packets::play::joinGame(
                    0,
                    false,
                    {"overworld"},
                    100,
                    2,
                    2,
                    false,
                    true,
                    false,
                    "default",
                    "overworld",
                    0,
                    1,
                    -1,
                    false,
                    false,
                    {},
                    0
                );
            }
        };
    }
}


#endif /* SRC_BUILD_IN_PLUGINS_WORLD */
