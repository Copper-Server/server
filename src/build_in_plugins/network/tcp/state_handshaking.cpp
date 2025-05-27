
#include <src/api/configuration.hpp>
#include <src/api/packets.hpp>
#include <src/build_in_plugins/network/tcp/client_handler/abstract.hpp>
#include <src/build_in_plugins/network/tcp/special_plugin_handshake.hpp>
#include <src/build_in_plugins/network/tcp/state_handshaking.hpp>
#include <src/build_in_plugins/network/tcp/state_status.hpp>
#include <src/log.hpp>
#include <src/plugin/special.hpp>

namespace copper_server::build_in_plugins::network::tcp {
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
            base_objects::network::response result;
            pluginManagement.requestPlugin<SpecialPluginHandshakeRegistration>()->enum_handles([&](SpecialPluginHandshake& special_handshake) -> bool {
                auto [handler, response] = special_handshake.InvalidPacket(tmp, data);
                if (handler) {
                    next_handler = handler;
                    result = base_objects::network::response::answer({std::move(response)});
                    return true;
                } else if (!response.empty()) {
                    result = base_objects::network::response::disconnect({std::move(response)});
                    return true;
                }
                return false;
            });
            if (result.has_data())
                return result;
            else
                return base_objects::network::response::disconnect();
        }
        data.read(); //skip packet id

        int32_t protocol_version = data.read_var<int32_t>();
        session->protocol_version = protocol_version;

        std::string server_address = data.read_string();
        if (!AllowServerAddressAndPort(server_address, data.read_value<uint16_t>()))
            return base_objects::network::response::disconnect();

        switch (auto next_state = data.read_var<int32_t>()) { //next state
        case 1:                                               //status
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
        default: {
            base_objects::network::response result;
            pluginManagement.requestPlugin<SpecialPluginHandshakeRegistration>()->enum_handles([&](SpecialPluginHandshake& special_handshake) -> bool {
                auto [handler, response] = special_handshake.InvalidNextOperation(next_state, data);
                if (handler) {
                    next_handler = handler;
                    result = base_objects::network::response::answer({std::move(response)});
                    return true;
                } else if (!response.empty()) {
                    result = base_objects::network::response::disconnect({std::move(response)});
                    return true;
                }
                return false;
            });
            if (result.has_data())
                return result;
            else
                return base_objects::network::response::disconnect();
        }
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

    tcp_client_handle_handshaking::tcp_client_handle_handshaking(api::network::tcp::session* sock)
        : tcp_client_handle(sock) {}

    tcp_client_handle_handshaking::tcp_client_handle_handshaking()
        : tcp_client_handle(nullptr) {}

    base_objects::network::tcp::client* tcp_client_handle_handshaking::define_ourself(api::network::tcp::session* sock) {
        return new tcp_client_handle_handshaking(sock);
    }

    base_objects::network::tcp::client* tcp_client_handle_handshaking::redefine_handler() {
        return next_handler;
    }
}