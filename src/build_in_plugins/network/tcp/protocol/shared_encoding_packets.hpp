#ifndef SRC_BUILD_IN_PLUGINS_PROTOCOL_SHARED_ENCODING_PACKETS
#define SRC_BUILD_IN_PLUGINS_PROTOCOL_SHARED_ENCODING_PACKETS
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/packets.hpp>
namespace copper_server::build_in_plugins::network::tcp::protocol {
    namespace play{
        base_objects::network::response updateAttributes__(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties, uint32_t protocol_version);
    }
}
#endif /* SRC_BUILD_IN_PLUGINS_PROTOCOL_SHARED_ENCODING_PACKETS */
