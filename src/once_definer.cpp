#include "protocolHelper.hpp"

namespace crafted_craft {
    std::unordered_set<boost::asio::ip::address> TCPClientHandle::banned_clients;
    size_t TCPClientHandle::max_packet_size = 4096;
}
