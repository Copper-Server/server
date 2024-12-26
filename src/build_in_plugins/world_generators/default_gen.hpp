#ifndef SRC_BUILD_IN_PLUGINS_WORLD_GENERATORS_DEFAULT_GEN
#define SRC_BUILD_IN_PLUGINS_WORLD_GENERATORS_DEFAULT_GEN
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins::world_generators {
    class DefaultGen : public PluginAutoRegister<"default_world_generator", DefaultGen> {
    public:
        DefaultGen() {}

        void OnRegister(const PluginRegistrationPtr& self) override;
    };
}
#endif /* SRC_BUILD_IN_PLUGINS_WORLD_GENERATORS_DEFAULT_GEN */
