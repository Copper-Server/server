
#include <src/api/configuration.hpp>
#include <src/log.hpp>
#include <src/plugin/special.hpp>
#include <src/protocolHelper/client_handler/abstract.hpp>
#include <src/protocolHelper/packets.hpp>
#include <src/protocolHelper/state_handshaking.hpp>
#include <src/protocolHelper/state_status.hpp>

namespace copper_server {

    bool tcp_client_handle_handshaking::AllowProtocolVersion(int proto_version) {
        return api::configuration::get().protocol.allowed_versions_processed.contains(proto_version);
    }

    bool tcp_client_handle_handshaking::AllowServerAddressAndPort(std::string& str, uint16_t port) {
        return true;
    }

    template <class T>
    T readValue(ArrayStream& data) {
        uint8_t tmp[sizeof(T)];
        for (size_t i = 0; i < sizeof(T); i++)
            tmp[i] = data.read();
        return enbt::endian_helpers::convert_endian(std::endian::big, *(T*)tmp);
    }

    base_objects::network::response tcp_client_handle_handshaking::work_packet(ArrayStream& data) {
        log::debug("Handshaking", "Handshaking...");
        uint8_t tmp = data.peek();
        if (tmp != '\0') {
            if (special_handshake) {
                auto [handler, response] = special_handshake->InvalidPacket(tmp, data);
                if (handler) {
                    next_handler = handler;
                    return base_objects::network::response::answer({std::move(response)});
                } else if (!response.empty())
                    return base_objects::network::response::disconnect({std::move(response)});
            }
            return base_objects::network::response::disconnect();
        }
        data.read(); //skip protocol version

        int32_t protocol_version = ReadVar<int32_t>(data);
        session->protocol_version = protocol_version;

        if (!AllowProtocolVersion(protocol_version))
            return base_objects::network::response::disconnect();


        std::string server_address = ReadString(data, 255);
        if (!AllowServerAddressAndPort(server_address, readValue<uint16_t>(data)))
            return base_objects::network::response::disconnect();

        switch (ReadVar<int32_t>(data)) {
        case 1: //status
            log::debug("Handshaking", "Switch to status");
            next_handler = new tcp_client_handle_status(session);
            return base_objects::network::response::empty();
        case 2: //login
            if (!api::configuration::get().protocol.allowed_versions_processed.contains(protocol_version))
                return base_objects::network::response::disconnect();
            log::debug("Handshaking", "Switch to login");
            next_handler = client_handler::abstract::createhandle_login(session);
            return base_objects::network::response::empty();
        case 3: //transfer
            return base_objects::network::response::disconnect();
        default:
            return base_objects::network::response::disconnect();
        }
    }

    base_objects::network::response tcp_client_handle_handshaking::too_large_packet() {
        return base_objects::network::response::disconnect();
    }

    base_objects::network::response tcp_client_handle_handshaking::exception(const std::exception& ex) {
        return base_objects::network::response::disconnect();
    }

    base_objects::network::response tcp_client_handle_handshaking::unexpected_exception() {
        return base_objects::network::response::disconnect();
    }

    tcp_client_handle_handshaking::tcp_client_handle_handshaking(base_objects::network::tcp_session* sock)
        : tcp_client_handle(sock) {}

    tcp_client_handle_handshaking::tcp_client_handle_handshaking()
        : tcp_client_handle(nullptr) {}

    base_objects::network::tcp_client* tcp_client_handle_handshaking::define_ourself(base_objects::network::tcp_session* sock) {
        return new tcp_client_handle_handshaking(sock);
    }

    base_objects::network::tcp_client* tcp_client_handle_handshaking::redefine_handler() {
        return next_handler;
    }
}