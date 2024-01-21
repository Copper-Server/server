#include "protocolHelper.hpp"
std::unordered_set<baip::address> TCPClientHandle::banned_players;
uint8_t TCPClientHandlePlay::max_chunk_change_count;
bool TCPClientHandleLogin::offline_mode = false;
int TCPClientHandleLogin::compres_threshold = -1;
bool TCPClientHandleLogin::compression_enabled = false;

