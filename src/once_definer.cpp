#include "hitbox.hpp"
#include "block.hpp"
#include "protocolHelper.hpp"
#include "enbt.hpp"
#include "ClientHandleHelper.hpp"
namespace mcCore {
	std::unordered_map<short, HitBox> HitBox::hitBoxes;
	short HitBox::hitboxes_adder = 0;


	std::unordered_map<std::string, std::vector<Block>> Block::tags;
	std::unordered_map<uint16_t, FullBlockData> Block::full_block_data;
	uint16_t Block::block_adder = 0;
}
std::unordered_set<baip::address> TCPClientHandle::banned_players;
uint8_t mcCore::TCPClientHandlePlay::max_chunk_change_count = 48;
std::function<std::string(std::string)> mcCore::TCPClientHandlePlay::text_filter;

bool TCPClientHandleLogin::offline_mode = false;
int TCPClientHandleLogin::compres_threshold = -1;
bool TCPClientHandleLogin::compression_enabled = false;

const char** ENBT::global_strings;
uint16_t ENBT::total_strings = 0;


TCPclient* first_client_holder;
std::atomic_uint64_t TCPsession::id_gen;
bool TCPsession::do_log_connection_errors = false;