#include <src/base_objects/network/tcp_client.hpp>
#include <src/log.hpp>

namespace copper_server::base_objects::network {
    response tcp_client::on_switch() {
        return response::empty();
    }

    //will return nullptr if Redefine not needed
    tcp_client::~tcp_client() {}

    void tcp_client::log_console(const std::string& prefix, const list_array<uint8_t>& data, size_t size) {
        std::string output = prefix + " ";
        output.reserve(size * 2);
        static const char hex_chars[] = "0123456789ABCDEF";
        for (size_t i = 0; i < size; i++) {
            output.push_back(hex_chars[data[i] >> 4]);
            output.push_back(hex_chars[data[i] & 0xF]);
        }
        log::debug("Debug tools", output);
    }
}