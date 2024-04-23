#ifndef SRC_BUILD_IN_PLUGINS_MINECRAFT
#define SRC_BUILD_IN_PLUGINS_MINECRAFT
#include "../plugin/registration.hpp"
#include "../protocolHelper/state_configuration.hpp"

namespace crafted_craft {
    namespace build_in_plugins {
        class Minecraft : public PluginRegistration {
        public:
            void OnLoad(const PluginRegistrationPtr& self) override {
                std::cout << "Minecraft plugin loaded!" << std::endl;
                TCPClientHandleConfiguration::plugins_configuration["minecraft:brand"] = self;
            }

            plugin_response OnConfigurationHandle(const std::string& chanel, const list_array<uint8_t>& data, SharedClientData& client) override {
                if (chanel == "minecraft:brand") {
                    ArrayStream stream(data.data(), data.size());
                    int32_t len = ReadVar<int32_t>(stream);
                    std::string brand((char*)stream.data_read(), len);
                    stream.r += len;
                    client.client_brand = brand;
                    std::cout << "Client brand: " << brand << std::endl;


                    list_array<uint8_t> response;
                    WriteString(response, "CraftedCraft");
                    return PluginResponse(response, "minecraft:brand");
                }
                return false;
            }
        };
    }
}

#endif /* SRC_BUILD_IN_PLUGINS_MINECRAFT */
