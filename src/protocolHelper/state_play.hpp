#ifndef SRC_PROTOCOLHELPER_STATE_PLAY
#define SRC_PROTOCOLHELPER_STATE_PLAY
#include "packets.hpp"
#include "util.hpp"

namespace crafted_craft {
    class TCPClientHandlePlay : public TCPClientHandle {
        Response SendKeepAlive() {
            keep_alive_packet = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
            list_array<uint8_t> response;
            response.push_back(0x24);
            WriteValue<int64_t>(keep_alive_packet, response);
            timer.expires_from_now(boost::posix_time::seconds(session->serverData().timeout_seconds));
            timer.async_wait([this](const boost::system::error_code& ec) {
                if (ec == boost::asio::error::operation_aborted)
                    return;
                session->disconnect();
            });
            keep_alive_wait = true;
            return Response::Answer({std::move(response)});
        }

        bool inited = false;
        bool keep_alive_wait = false;
        boost::asio::deadline_timer timer;
        std::list<PluginRegistration::plugin_response> queriedPackets;
        int64_t keep_alive_packet = 0;
        int32_t excepted_pong = 0;
        std::chrono::time_point<std::chrono::system_clock> pong_timer;

        Response IdleActions() {
            std::list<PluginRegistration::plugin_response> load_next_packets;
            for (size_t i = 0; i < 20 && !queriedPackets.empty(); i++) {
                load_next_packets.push_back(std::move(queriedPackets.front()));
                queriedPackets.pop_front();
            }
            Response response(Response::Empty());
            response.reserve(load_next_packets.size());
            for (auto& it : load_next_packets) {
                if (std::holds_alternative<PluginRegistration::PluginResponse>(it)) {
                    auto& plugin = std::get<PluginRegistration::PluginResponse>(it);
                    response += packets::configuration::configuration(plugin.plugin_chanel, plugin.data);
                } else if (std::holds_alternative<Response>(it))
                    response += std::get<Response>(it);
            }
            if (!keep_alive_wait)
                response += SendKeepAlive();
            return response;
        }

        Response WorkPacket(ArrayStream& packet) override {
            uint8_t packet_id = packet.read();
            switch (packet_id) {
            case 0x00: {
                int32_t teleport_id = ReadVar<int32_t>(packet);
                if (session->sharedData().packets_state.pending_teleport_ids.front() != teleport_id) {
                    return packets::play::kick({"Invalid teleport id"});
                } else
                    session->sharedData().packets_state.pending_teleport_ids.pop_front();
                break;
            }
            case 0x01: {
                break;
            }
            case 0x02: {

                break;
            }

            case 0x03: {
                break;
            }
            case 0x04: {
                break;
            }
            case 0x05: {
                break;
            }
            case 0x06: {
                break;
            }
            case 0x07: {
                break;
            }
            case 0x08: {
                break;
            }
            case 0x09: {
                break;
            }
            case 0x0A: {
                break;
            }
            case 0x0B: {
                break;
            }
            case 0x0C: {
                break;
            }
            case 0x0D: {
                break;
            }
            case 0x0E: {
                break;
            }
            case 0x0F: {
                break;
            }
            case 0x10: {
                break;
            }
            case 0x11: {
                break;
            }
            case 0x12: {
                break;
            }
            case 0x13: {
                break;
            }
            case 0x14: {
                break;
            }
            case 0x15: { //keep alive
                int64_t keep_alive_packet_response = ReadValue<int64_t>(packet);
                if (keep_alive_packet == keep_alive_packet_response) {
                    timer.cancel();
                    keep_alive_wait = false;
                }
                break;
            }
            case 0x16: {
                break;
            }
            case 0x17: {
                break;
            }
            case 0x18: {
                break;
            }
            case 0x19: {
                break;
            }
            case 0x1A: {
                break;
            }
            case 0x1B: {
                break;
            }
            case 0x1C: {
                break;
            }
            case 0x1D: {
                break;
            }
            case 0x1E: { //ping request
                int64_t ping = ReadValue<int64_t>(packet);
                break;
            }
            case 0x1F: { // place recipe
                int32_t window_id = ReadVar<int32_t>(packet);
                std::string recipe_id = ReadString(packet, 32767);
                bool make_all = ReadValue<bool>(packet);
                break;
            }
            case 0x20: { //player abilities
                int8_t flags = ReadValue<int8_t>(packet);
                bool flying = flags & 0x02;
                break;
            }
            case 0x21: { //player action
                int32_t status = ReadVar<int32_t>(packet);
                Position pos;
                pos.raw = ReadValue<int64_t>(packet);
                int8_t face = ReadValue<int8_t>(packet);
                int32_t sequence = ReadVar<int32_t>(packet);
                break;
            }
            case 0x22: { //player command
                int32_t entity_id = ReadVar<int32_t>(packet);
                int32_t action_id = ReadVar<int32_t>(packet);
                int32_t jump_boost = ReadVar<int32_t>(packet);
                break;
            }
            case 0x23: { //player input
                float sideways = ReadValue<float>(packet);
                float forward = ReadValue<float>(packet);
                int8_t flags = ReadValue<int8_t>(packet);
                break;
            }
            case 0x24: { //pong
                int32_t pong = ReadValue<int32_t>(packet);
                break;
            }
            case 0x25: { //change recipe book settings
                int32_t book_id = ReadVar<int32_t>(packet);
                bool book_open = ReadValue<bool>(packet);
                bool filter_active = ReadValue<bool>(packet);
                break;
            }
            case 0x26: { //set seen recipe
                std::string recipe_id = ReadString(packet, 32767);
                break;
            }
            case 0x27: { //rename item
                std::string name = ReadString(packet, 32767);
                break;
            }
            case 0x28: { //resource pack response
                ENBT::UUID uuid = ReadUUID(packet);
                int32_t result = ReadVar<int32_t>(packet);
                break;
            }
            case 0x29: { //seen advancments
                int32_t action = ReadVar<int32_t>(packet);
                if (action == 1) {
                    std::string tab_id = ReadString(packet, 32767);
                }
                break;
            }
            case 0x2A: { //select trade
                int32_t selected_slot = ReadVar<int32_t>(packet);
                break;
            }
            case 0x2B: { //set beacon effect
                std::optional<int32_t> primary_effect;
                std::optional<int32_t> secondary_effect;
                if (ReadValue<bool>(packet))
                    primary_effect = ReadVar<int32_t>(packet);
                if (ReadValue<bool>(packet))
                    secondary_effect = ReadVar<int32_t>(packet);
                break;
            }
            case 0x2C: { //set held item
                int16_t slot = ReadVar<int16_t>(packet);
                break;
            }
            case 0x2D: { //program command block
                Position pos;
                pos.raw = ReadValue<int64_t>(packet);
                std::string command = ReadString(packet, 32767);
                int32_t mode = ReadVar<int32_t>(packet);
                int8_t flags = ReadValue<int8_t>(packet);

                break;
            }
            case 0x2E: { //program command block mineCart
                int32_t entity_id = ReadVar<int32_t>(packet);
                std::string command = ReadString(packet, 32767);
                bool track_output = ReadValue<bool>(packet);
                break;
            }
            case 0x2F: { //set creative slot
                int16_t slot = ReadVar<int16_t>(packet);
                base_objects::packets::slot item = ReadSlot(packet);
                break;
            }
            case 0x30: { //program jigsaw block
                Position pos;
                pos.raw = ReadValue<int64_t>(packet);
                std::string name = ReadString(packet, 32767);
                std::string target = ReadString(packet, 32767);
                std::string pool = ReadString(packet, 32767);
                std::string final_state = ReadString(packet, 32767);
                std::string joint_type = ReadString(packet, 32767);
                int32_t selection_priority = ReadVar<int32_t>(packet);
                int32_t placement_priority = ReadVar<int32_t>(packet);

                break;
            }
            case 0x31: { //program structure block
                Position pos;
                pos.raw = ReadValue<int64_t>(packet);
                int32_t action = ReadVar<int32_t>(packet);
                int32_t mode = ReadVar<int32_t>(packet);
                std::string name = ReadString(packet, 32767);
                int8_t offset_x = ReadValue<int8_t>(packet);
                int8_t offset_y = ReadValue<int8_t>(packet);
                int8_t offset_z = ReadValue<int8_t>(packet);
                int8_t size_x = ReadVar<int8_t>(packet);
                int8_t size_y = ReadVar<int8_t>(packet);
                int8_t size_z = ReadVar<int8_t>(packet);
                int32_t mirror = ReadVar<int32_t>(packet);
                int32_t rotation = ReadVar<int32_t>(packet);
                std::string metadata = ReadString(packet, 128);
                float integrity = ReadValue<float>(packet);
                int64_t seed = ReadValue<int64_t>(packet);
                int8_t flags = ReadValue<int8_t>(packet);
                break;
            }
            case 0x32: { //update sign
                Position pos;
                pos.raw = ReadValue<int64_t>(packet);
                bool is_front_text = ReadValue<bool>(packet);
                std::string line1 = ReadString(packet, 384);
                std::string line2 = ReadString(packet, 384);
                std::string line3 = ReadString(packet, 384);
                std::string line4 = ReadString(packet, 384);
                break;
            }
            case 0x33: { //swing arm
                int32_t hand = ReadVar<int32_t>(packet);

                break;
            }
            case 0x34: { //spectator request to teleport
                ENBT::UUID target_entity = ReadUUID(packet);
                break;
            }
            case 0x35: { //Use item On
                int32_t hand = ReadVar<int32_t>(packet);
                Position pos;
                pos.raw = ReadValue<int64_t>(packet);
                int32_t face = ReadVar<int32_t>(packet);
                float cursor_x = ReadValue<float>(packet);
                float cursor_y = ReadValue<float>(packet);
                float cursor_z = ReadValue<float>(packet);
                bool inside_block = ReadValue<bool>(packet);
                int32_t sequence = ReadVar<int32_t>(packet);

                break;
            }
            case 0x36: { //Use item
                int32_t hand = ReadVar<int32_t>(packet);
                int32_t sequence = ReadVar<int32_t>(packet);


                break;
            }
            default:
                break;
            }
            return IdleActions();
        }

        Response TooLargePacket() override {
            return packets::play::kick("Packet too large");
        }

        Response Exception(const std::exception& ex) override {
            return packets::play::kick("Internal server error: " + std::string(ex.what()) + "\nPlease report this to the server owner!");
        }

        Response UnexpectedException() override {
            return packets::play::kick("Internal server error\nPlease report this to the server owner!");
        }

        Response OnSwitch() override {
            return IdleActions();
        }

    public:
        TCPClientHandlePlay(TCPsession* session)
            : TCPClientHandle(session), timer(session->sock.get_executor()) {}

        ~TCPClientHandlePlay() override {
        }

        TCPclient* DefineOurself(TCPsession* sock) override {
            return nullptr;
        }
    };
}

#endif /* SRC_PROTOCOLHELPER_STATE_PLAY */
