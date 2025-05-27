#ifndef SRC_API_NETWORK_TCP
#define SRC_API_NETWORK_TCP
#include <library/list_array.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <src/base_objects/events/sync_event.hpp>

#include <span>

namespace copper_server::base_objects {
    struct SharedClientData;
    using client_data_holder = atomic_holder<SharedClientData>;
}

namespace copper_server::api::network::tcp {
    class session {
    public:
        const uint64_t id;
        int32_t protocol_version = -1;
        int32_t compression_threshold = -1;
        bool is_not_legacy : 1 = false;

        session(uint64_t id) : id(id) {}

        virtual bool is_active() = 0;
        virtual void disconnect() = 0;
        virtual base_objects::client_data_holder& shared_data_ref() = 0;
        virtual base_objects::SharedClientData& shared_data() = 0;
        virtual bool start_symmetric_encryption(const list_array<uint8_t>& encryption_key, const list_array<uint8_t>& encryption_iv) = 0;
    };

    bool decrypt_data(list_array<uint8_t>& data);
    bool encrypt_data(list_array<uint8_t>& data);
    std::span<uint8_t> private_key_buffer();
    std::span<uint8_t> public_key_buffer();
}

#endif /* SRC_API_NETWORK_TCP */
