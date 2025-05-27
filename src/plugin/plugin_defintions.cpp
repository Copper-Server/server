#include <src/plugin/main.hpp>
#include <src/plugin/special.hpp>

namespace copper_server {
    SpecialPluginStatus* special_status;

    namespace __internal__ {
        std::vector<std::pair<std::string, std::shared_ptr<delayed_construct_base>>>& registration_list() {
            static std::vector<std::pair<std::string, std::shared_ptr<delayed_construct_base>>> list;
            return list;
        }

        void register_configuration(const PluginRegistrationPtr& self) {
            pluginManagement.registerPluginOn(self, PluginManagement::registration_on::configuration);
        }

        void register_play(const PluginRegistrationPtr& self) {
            pluginManagement.registerPluginOn(self, PluginManagement::registration_on::play);
        }
    }
}