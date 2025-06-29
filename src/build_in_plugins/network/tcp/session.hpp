#ifndef SRC_BUILD_IN_PLUGINS_NETWORK_TCP_SESSION
#define SRC_BUILD_IN_PLUGINS_NETWORK_TCP_SESSION
#include <library/fast_task/src/networking/networking.hpp>
#include <src/api/network/tcp.hpp>
#include <src/base_objects/encryption/aes.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/network/tcp/client.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <vector>

namespace copper_server::build_in_plugins::network::tcp {

    class session : public api::network::tcp::session {
        fast_task::task_mutex tc;
        fast_task::networking::TcpNetworkStream* stream;

    public:
        static bool do_log_connection_errors;
        encryption::aes encryption;

        session(fast_task::networking::TcpNetworkStream& s, base_objects::network::tcp::client* client_handler, float& set_timeout);
        ~session();

        bool is_active() override;
        void disconnect() override;
        base_objects::client_data_holder& shared_data_ref() override;
        base_objects::SharedClientData& shared_data() override;
        bool start_symmetric_encryption(const list_array<uint8_t>& encryption_key, const list_array<uint8_t>& encryption_iv) override;

        base_objects::network::tcp::client& handler();

        void received(std::span<char> read_data);

        void request_buffer(size_t new_size) override;

        void send_indirect(base_objects::network::response&&) override;

    private:
        void send(base_objects::network::response&& resp);
        base_objects::network::response proceed_data();

        std::vector<uint8_t> read_data;
        list_array<uint8_t> read_data_cached;
        base_objects::client_data_holder _sharedData;
        float& timeout;
        base_objects::network::tcp::client* chandler = nullptr;
        bool encryption_enabled : 1 = false;
    };
}

#endif /* SRC_BUILD_IN_PLUGINS_NETWORK_TCP_SESSION */
