#ifndef SRC_BUILD_IN_PLUGINS_WORLD_GENERATORS_DEFAULT_GEN
#define SRC_BUILD_IN_PLUGINS_WORLD_GENERATORS_DEFAULT_GEN
#include "../../plugin/main.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        namespace world_generators {
            class DefaultGen : public PluginAutoRegister<"default_world_generator", DefaultGen> {
            public:
                DefaultGen(){}
                void OnRegister(const PluginRegistrationPtr& self) override;
            };
        }
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_WORLD_GENERATORS_DEFAULT_GEN */
