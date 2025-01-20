#include <src/build_in_plugins/minecraft.hpp>
#include <src/log.hpp>
#include <src/plugin/main.hpp>
#include <src/protocolHelper/packets.hpp>
#include <src/protocolHelper/util.hpp>

namespace copper_server::build_in_plugins {

    void MinecraftPlugin::OnLoad(const PluginRegistrationPtr& self) {
        pluginManagement.bindPluginOn("minecraft:brand", self, PluginManagement::registration_on::configuration);
        log::info("Minecraft", "Minecraft plugin loaded!");
    }

    void MinecraftPlugin::OnUnload(const PluginRegistrationPtr& self) {
        log::info("Minecraft", "Minecraft plugin unloaded!");
    }

    MinecraftPlugin::plugin_response MinecraftPlugin::OnConfiguration(base_objects::client_data_holder& client) {
        list_array<uint8_t> response;
        WriteString(response, "CopperServer");
        return packets::configuration::configuration(*client, "minecraft:brand", response);
    }

    MinecraftPlugin::plugin_response MinecraftPlugin::OnConfigurationHandle(const PluginRegistrationPtr& self, const std::string& chanel, const list_array<uint8_t>& data, base_objects::client_data_holder& client) {
        if (chanel == "minecraft:brand") {
            ArrayStream stream(data.data(), data.size());
            int32_t len = stream.read_var<int32_t>();
            std::string brand((char*)stream.data_read(), len);
            stream.r += len;
            client->client_brand = brand;
        }
        return std::nullopt;
    }
}
