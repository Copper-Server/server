#ifndef SRC_BUILD_IN_PLUGINS_LIGHT_PROCESSORS_DEFAULT_PROC
#define SRC_BUILD_IN_PLUGINS_LIGHT_PROCESSORS_DEFAULT_PROC
#include "../../plugin/main.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        namespace light_processors {
            class DefaultProc : public PluginAutoRegister<"default_light_processor", DefaultProc> {
            public:
                DefaultProc() {}

                void OnRegister(const PluginRegistrationPtr& self) override;
            };
        }
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_LIGHT_PROCESSORS_DEFAULT_PROC */
