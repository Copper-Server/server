#include "base_objects/block.hpp"
#include "base_objects/hitbox.hpp"
#include "library/enbt.hpp"
#include "protocolHelper.hpp"
#include "registers.hpp"

namespace crafted_craft {
    std::unordered_map<short, HitBox> HitBox::hitBoxes;
    short HitBox::hitboxes_adder = 0;


    std::unordered_map<std::string, list_array<base_objects::block>> base_objects::block::tags;
    std::unordered_map<uint16_t, base_objects::full_block_data> base_objects::block::full_block_data_;
    uint16_t base_objects::block::block_adder = 0;
    std::unordered_set<boost::asio::ip::address> TCPClientHandle::banned_players;
    size_t TCPClientHandle::max_packet_size = 4096;
    //std::function<std::string(std::string)> crafted_craft::TCPClientHandlePlay::text_filter;

    std::unordered_map<std::string, PluginRegistrationPtr> TCPClientHandlePlay::plugins_play;
    std::unordered_map<std::string, PluginRegistrationPtr> TCPClientHandleLogin::plugins;
    std::unordered_map<std::string, PluginRegistrationPtr> TCPClientHandleConfiguration::plugins_configuration;

    list_array<PluginRegistrationPtr> TCPClientHandlePlay::base_plugins;
    list_array<PluginRegistrationPtr> TCPClientHandleConfiguration::base_plugins;

    //uint8_t crafted_craft::TCPClientHandlePlay::max_chunk_change_count = 48;

    list_array<registers::ArmorTrimMaterial> registers::armorTrimMaterials;
    list_array<registers::ArmorTrimPattern> registers::armorTrimPatterns;
    list_array<registers::Biome> registers::biomes;
    list_array<registers::ChatType> registers::chatTypes;
    list_array<registers::DamageType> registers::damageTypes;
    list_array<registers::DimensionType> registers::dimensionTypes;


    std::unordered_map<base_objects::block, uint16_t, base_objects::block_hash> registers::blockPalette;
    std::unordered_map<uint16_t, registers::EntityType*> registers::entityList;

    TCPclient* first_client_holder;
    std::atomic_uint64_t TCPsession::id_gen;
    bool TCPsession::do_log_connection_errors = true;
}
