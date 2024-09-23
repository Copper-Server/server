#ifndef SRC_PLUGIN_SPECIAL
#define SRC_PLUGIN_SPECIAL
#include "../base_objects/server_configuaration.hpp"
#include "../protocolHelper/util.hpp"

namespace crafted_craft {
    class SpecialPluginHandshake {
    public:
        //allows to handle custom packets from the client, like send a packet from server to server or another use case.
        //if TCPClientHandle nullptr then connection will be closed,
        //if list_array<uint8_t> empty then client will be disconnected
        virtual std::pair<TCPClientHandle*, list_array<uint8_t>> InvalidPacket(uint8_t packet_id, ArrayStream& data) {
            return {nullptr, {}};
        }

        //allows to set custom handler for this protocol, packet will be again parsed by this new handler
        virtual TCPClientHandle* UnsupportedProtocolVersion(int protocol_version) {
            return nullptr;
        }

        virtual ~SpecialPluginHandshake() {}
    };

    class SpecialPluginStatus {
    public:
        const base_objects::ServerConfiguration& config;

        SpecialPluginStatus(const base_objects::ServerConfiguration& config)
            : config(config) {}


        virtual std::string StatusResponseVersionName() {
            return "CraftedCraft";
        }

        virtual bool ShowConnectionStatus() {
            return true;
        }

        virtual size_t MaxPlayers() {
            return 20;
        }

        virtual size_t OnlinePlayers() {
            return 0;
        }

        //string also can be Chat json object!
        virtual list_array<std::pair<std::string, enbt::raw_uuid>> OnlinePlayersSample() {
            return {};
        }

        virtual Chat Description() {
            return "A CraftedCraft server";
        }

        virtual bool ConnectionAvailable(int32_t protocol_version) {
            return true;
        }

        //return empty string if no icon, icon must be 64x64 and png format in base64
        virtual std::string ServerIcon() {
            return "";
        }

        virtual ~SpecialPluginStatus() {}
    };

    extern SpecialPluginHandshake* special_handshake;
    extern SpecialPluginStatus* special_status;
}

#endif /* SRC_PLUGIN_SPECIAL */
