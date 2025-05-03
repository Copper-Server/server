#ifndef SRC_BASE_OBJECTS_NETWORK_TCP_ACCEPT_PACKET_REGISTRY
#define SRC_BASE_OBJECTS_NETWORK_TCP_ACCEPT_PACKET_REGISTRY
#include <src/base_objects/network/tcp/session.hpp>
#include <src/util/readers.hpp>

#include <unordered_map>

namespace copper_server::base_objects::network::tcp {
    using read_packet_handler = void (*)(base_objects::network::tcp::session*, ArrayStream&);

    struct protocol_packet_registry_t {
        std::unordered_map<int32_t, read_packet_handler> registry;
        std::unordered_map<int32_t, std::string> registry_id_name;
        std::unordered_map<std::string, int32_t> registry_name_id;


        void handle(int32_t packet_state, base_objects::network::tcp::session* session, ArrayStream& packet) const;
        std::string get_packet_name(int32_t packet_state) const;
        int32_t get_packet_state(std::string packet_name) const;

        bool has_packet_state(int32_t packet_state) const;
        bool has_packet_name(std::string packet_name) const;
    };

    struct accept_packet_registry {
        std::unordered_map<int32_t, protocol_packet_registry_t> registry;

        void unregister_protocol(int32_t protocol_id);
        void register_packet(int32_t protocol_id, int32_t packet_state, std::string packet_name, read_packet_handler handler);
        void register_seq(int32_t protocol_id, std::initializer_list<std::pair<std::string, read_packet_handler>> handlers);
        void handle(int32_t protocol_id, int32_t packet_state, base_objects::network::tcp::session* session, ArrayStream& packet) const;

        std::string get_packet_name(int32_t protocol_id, int32_t packet_state) const;
        int32_t get_packet_state(int32_t protocol_id, std::string packet_name) const;
        bool has_packet_name(int32_t protocol_id, std::string packet_name) const;
        bool has_packet_state(int32_t protocol_id, int32_t packet_state) const;

        const protocol_packet_registry_t& get(int32_t protocol_id) const;
    };

    struct packet_registry_t {
        struct {
            accept_packet_registry login;
            accept_packet_registry configuration;
            accept_packet_registry play;
        } serverbound;
    };

    extern packet_registry_t packet_registry;
}

#endif /* SRC_BASE_OBJECTS_NETWORK_TCP_ACCEPT_PACKET_REGISTRY */
