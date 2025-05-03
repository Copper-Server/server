#ifndef SRC_BUILD_IN_PLUGINS_PROTOCOL_SHARED_ENCODING_PACKETS
#define SRC_BUILD_IN_PLUGINS_PROTOCOL_SHARED_ENCODING_PACKETS
#include <src/base_objects/block.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/chunk.hpp>
#include <src/base_objects/data_packs/known_pack.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/particle_data.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/recipe.hpp>
namespace copper_server::build_in_plugins::protocol {
    namespace play{
        base_objects::network::response updateAttributes__(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties, uint32_t protocol_version);
    }
}
#endif /* SRC_BUILD_IN_PLUGINS_PROTOCOL_SHARED_ENCODING_PACKETS */
