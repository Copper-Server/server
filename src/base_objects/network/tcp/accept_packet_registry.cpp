#include <src/base_objects/network/tcp/accept_packet_registry.hpp>

namespace copper_server::base_objects::network::tcp {
    void protocol_packet_registry_t::handle(int32_t packet_state, base_objects::network::tcp::session* session, ArrayStream& packet) const {
        auto it = registry.find(packet_state);
        if (it == registry.end())
            throw std::runtime_error("Unrecognized packet state: " + std::to_string(packet_state));
        it->second(session, packet);
    }

    std::string protocol_packet_registry_t::get_packet_name(int32_t packet_state) const {
        auto it = registry_id_name.find(packet_state);
        if (it == registry_id_name.end())
            return "Undefined[" + std::to_string(packet_state) + "]";
        return it->second;
    }

    int32_t protocol_packet_registry_t::get_packet_state(std::string packet_name) const {
        auto it = registry_name_id.find(packet_name);
        if (it == registry_name_id.end())
            throw std::runtime_error("Unrecognized packet name: " + packet_name);
        return it->second;
    }

    bool protocol_packet_registry_t::has_packet_state(int32_t packet_state) const {
        return registry.find(packet_state) != registry.end();
    }

    bool protocol_packet_registry_t::has_packet_name(std::string packet_name) const {
        return registry_name_id.find(packet_name) != registry_name_id.end();
    }

    void accept_packet_registry::unregister_protocol(int32_t protocol_id){
        registry.erase(protocol_id);
    }
    void accept_packet_registry::register_packet(int32_t protocol_id, int32_t packet_state, std::string packet_name, read_packet_handler handler) {
        auto& it = registry[protocol_id];
        it.registry[packet_state] = handler;
        it.registry_id_name[packet_state] = packet_name;
        it.registry_name_id[packet_name] = packet_state;
    }

    void accept_packet_registry::register_seq(int32_t protocol_id, std::initializer_list<std::pair<std::string, read_packet_handler>> handlers) {
        int32_t packet_state = 0;
        auto& it = registry[protocol_id];
        for (auto& [name, handler] : handlers) {
            it.registry[packet_state] = handler;
            it.registry_id_name[packet_state] = name;
            it.registry_name_id[name] = packet_state;
            packet_state++;
        }
    }

    void accept_packet_registry::handle(int32_t protocol_id, int32_t packet_state, base_objects::network::tcp::session* session, ArrayStream& packet) const {
        auto it = registry.find(protocol_id);
        if (it == registry.end())
            throw std::runtime_error("Unrecognized protocol id: " + std::to_string(protocol_id));
        it->second.handle(packet_state, session, packet);
    }

    std::string accept_packet_registry::get_packet_name(int32_t protocol_id, int32_t packet_state) const {
        auto it = registry.find(protocol_id);
        if (it == registry.end())
            throw std::runtime_error("Unrecognized protocol id: " + std::to_string(protocol_id));
        return it->second.get_packet_name(packet_state);
    }

    int32_t accept_packet_registry::get_packet_state(int32_t protocol_id, std::string packet_name) const {
        auto it = registry.find(protocol_id);
        if (it == registry.end())
            throw std::runtime_error("Unrecognized protocol id: " + std::to_string(protocol_id));
        return it->second.get_packet_state(packet_name);
    }

    bool accept_packet_registry::has_packet_state(int32_t protocol_id, int32_t packet_state) const {
        auto it = registry.find(protocol_id);
        if (it == registry.end())
            return false;
        return it->second.has_packet_state(packet_state);
    }

    bool accept_packet_registry::has_packet_name(int32_t protocol_id, std::string packet_name) const {
        auto it = registry.find(protocol_id);
        if (it == registry.end())
            return false;
        return it->second.has_packet_name(packet_name);
    }

    const protocol_packet_registry_t& accept_packet_registry::get(int32_t protocol_id) const {
        auto it = registry.find(protocol_id);
        if (it == registry.end())
            throw std::runtime_error("Unrecognized protocol id: " + std::to_string(protocol_id));
        return it->second;
    }

    packet_registry_t packet_registry;
}