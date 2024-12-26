#ifndef SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_LOGIN
#define SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_LOGIN
#include <src/plugin/registration.hpp>
#include <src/protocolHelper/util.hpp>

namespace copper_server::client_handler::release_767 {
    class handle_login : public tcp_client_handle {
    protected:
        base_objects::network::response encryptionRequest();
        base_objects::network::response loginSuccess();
        base_objects::network::response setCompression(int32_t threshold);
        base_objects::network::response proceedPlugin(ArrayStream& data, bool successful = true);
        void resolve_join_conflict();
        base_objects::network::response work_packet(ArrayStream& packet) override;
        base_objects::network::response too_large_packet() override;
        base_objects::network::response exception(const std::exception& ex) override;
        base_objects::network::response unexpected_exception() override;

        std::list<std::pair<std::string, PluginRegistrationPtr>> plugins_query;
        uint8_t verify_token[4];
        int plugin_message_id = 0;
        bool is_authed = false;
        uint8_t excepted_packet = 0;
        bool has_conflict = false;

    public:
        handle_login(base_objects::network::tcp_session* sock);

        handle_login();

        base_objects::network::tcp_client* define_ourself(base_objects::network::tcp_session* sock) override;
    };
}

#endif /* SRC_PROTOCOLHELPER_CLIENT_HANDLER_767_LOGIN */
