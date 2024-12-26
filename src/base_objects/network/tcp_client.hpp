#ifndef SRC_BASE_OBJECTS_NETWORK_TCP_CLIENT
#define SRC_BASE_OBJECTS_NETWORK_TCP_CLIENT
#include <src/base_objects/network/response.hpp>
#include <string>

namespace copper_server::base_objects::network {
    class tcp_session;

    class tcp_client {
    protected:
    public:
        virtual response work_client(list_array<uint8_t>&) = 0;

        virtual response on_switch();

        //will return nullptr if Redefine not needed
        virtual tcp_client* redefine_handler() = 0;
        virtual tcp_client* define_ourself(tcp_session* session) = 0;

        virtual ~tcp_client();

        static void log_console(const std::string& prefix, const list_array<uint8_t>& data, size_t size);
    };
}

#endif /* SRC_BASE_OBJECTS_NETWORK_TCP_CLIENT */
