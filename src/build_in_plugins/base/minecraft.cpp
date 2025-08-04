#include <src/api/client.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct MinecraftPlugin : public PluginAutoRegister<"base/minecraft", MinecraftPlugin> {
        void OnLoad(const PluginRegistrationPtr& self) override {
            pluginManagement.bindPluginOn("minecraft:brand", self, PluginManagement::registration_on::configuration);
        }

        bool OnConfiguration(base_objects::SharedClientData& client) override {
            base_objects::network::response::item r;
            r.write_string("CopperServer");
            client << api::client::configuration::custom_payload{
                .channel = "minecraft:brand",
                .payload = r.data.to_container<std::vector>()
            };
            return false;
        }

        bool OnConfigurationHandle(const PluginRegistrationPtr& self, const std::string& chanel, const std::vector<uint8_t>& data, base_objects::SharedClientData& client) override {
            if (chanel == "minecraft:brand") {
                ArrayStream stream(data.data(), data.size());
                int32_t len = stream.read_var<int32_t>();
                std::string brand((char*)stream.data_read(), len);
                stream.r += len;
                client.client_brand = brand;
            }
            return true;
        }
    };
}
