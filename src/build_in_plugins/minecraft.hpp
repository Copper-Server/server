#ifndef SRC_BUILD_IN_PLUGINS_MINECRAFT
#define SRC_BUILD_IN_PLUGINS_MINECRAFT
#include "../log.hpp"
#include "../plugin/registration.hpp"
#include "../protocolHelper/state_configuration.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class MinecraftPlugin : public PluginRegistration {
        public:
            void OnLoad(const PluginRegistrationPtr& self) override {
                TCPClientHandleConfiguration::base_plugins.push_back(self);
                TCPClientHandleConfiguration::plugins_configuration["minecraft:brand"] = self;
                log::info("Minecraft", "Minecraft plugin loaded!");
            }

            void OnReload(const PluginRegistrationPtr& self) override {
                log::info("Minecraft", "Minecraft plugin reloaded!");
            }

            void OnUnload(const PluginRegistrationPtr& self) override {
                TCPClientHandleConfiguration::base_plugins.erase(self);
                TCPClientHandleConfiguration::plugins_configuration.erase("minecraft:brand");
                log::info("Minecraft", "Minecraft plugin unloaded!");
            }

            plugin_response OnConfiguration(base_objects::client_data_holder& client) override {
                list_array<uint8_t> response;
                WriteString(response, "CraftedCraft");
                return PluginResponse(response, "minecraft:brand");
            }

            plugin_response OnConfigurationHandle(const PluginRegistrationPtr& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::client_data_holder& client) override {
                if (chanel == "minecraft:brand") {
                    ArrayStream stream(data.data(), data.size());
                    int32_t len = ReadVar<int32_t>(stream);
                    std::string brand((char*)stream.data_read(), len);
                    stream.r += len;
                    client->client_brand = brand;
                }
                return false;
            }
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_MINECRAFT */
