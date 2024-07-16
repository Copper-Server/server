#include "base_objects/block.hpp"
#include "base_objects/hitbox.hpp"
#include "protocolHelper.hpp"

namespace crafted_craft {
    std::unordered_map<short, HitBox> HitBox::hitBoxes;
    short HitBox::hitboxes_adder = 0;


    std::unordered_map<std::string, list_array<base_objects::block>> base_objects::block::tags;
    std::unordered_map<uint16_t, base_objects::full_block_data> base_objects::block::full_block_data_;
    uint16_t base_objects::block::block_adder = 0;
    std::unordered_set<boost::asio::ip::address> TCPClientHandle::banned_clients;
    size_t TCPClientHandle::max_packet_size = 4096;

    TCPclient* first_client_holder;
    std::atomic_uint64_t TCPsession::id_gen;
    bool TCPsession::do_log_connection_errors = true;
}
