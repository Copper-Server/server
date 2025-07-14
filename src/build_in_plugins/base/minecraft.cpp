#include <src/api/packets.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    class MinecraftPlugin : public PluginAutoRegister<"minecraft", MinecraftPlugin> {
    public:
        void OnLoad(const PluginRegistrationPtr& self) override {
            pluginManagement.bindPluginOn("minecraft:brand", self, PluginManagement::registration_on::configuration);
            log::info("Minecraft", "Minecraft plugin loaded!");
        }

        void OnUnload(const PluginRegistrationPtr& self) override {
            log::info("Minecraft", "Minecraft plugin unloaded!");
        }

        plugin_response OnConfiguration(base_objects::client_data_holder& client) override {
            list_array<uint8_t> response;
            WriteString(response, "CopperServer");
            return api::packets::configuration::configuration(*client, "minecraft:brand", response)
                   += api::packets::configuration::requestCookie(*client, "minecraft:vanilla");
        }

        plugin_response OnConfigurationHandle(const PluginRegistrationPtr& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::client_data_holder& client) override {
            if (chanel == "minecraft:brand") {
                ArrayStream stream(data.data(), data.size());
                int32_t len = stream.read_var<int32_t>();
                std::string brand((char*)stream.data_read(), len);
                stream.r += len;
                client->client_brand = brand;
            }
            return std::nullopt;
        }
    };
}
