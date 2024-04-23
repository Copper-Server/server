#include "base_objects/block.hpp"
#include "base_objects/hitbox.hpp"
#include "library/enbt.hpp"
#include "protocolHelper.hpp"
#include "registers.hpp"

namespace crafted_craft {
    std::unordered_map<short, HitBox> HitBox::hitBoxes;
    short HitBox::hitboxes_adder = 0;


    std::unordered_map<std::string, list_array<Block>> Block::tags;
    std::unordered_map<uint16_t, FullBlockData> Block::full_block_data;
    uint16_t Block::block_adder = 0;
    std::unordered_set<baip::address> TCPClientHandle::banned_players;
    size_t TCPClientHandle::max_packet_size = 4096;
    //std::function<std::string(std::string)> crafted_craft::TCPClientHandlePlay::text_filter;

    std::unordered_map<std::string, PluginRegistrationPtr> TCPClientHandleLogin::plugins;
    std::unordered_map<std::string, PluginRegistrationPtr> TCPClientHandleConfiguration::plugins_configuration;

    //uint8_t crafted_craft::TCPClientHandlePlay::max_chunk_change_count = 48;

    list_array<registers::ArmorTrimMaterial> registers::armorTrimMaterials;
    list_array<registers::ArmorTrimPattern> registers::armorTrimPatterns;
    list_array<registers::Biome> registers::biomes;
    list_array<registers::ChatType> registers::chatTypes;
    list_array<registers::DamageType> registers::damageTypes;
    list_array<registers::DimensionType> registers::dimensionTypes;


    std::unordered_map<Block, uint16_t, BlockHash> registers::blockPalette;
    std::unordered_map<uint16_t, registers::EntityType*> registers::entityList;
}

TCPclient* first_client_holder;
std::atomic_uint64_t TCPsession::id_gen;
bool TCPsession::do_log_connection_errors = false;