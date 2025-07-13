#include <src/base_objects/block.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/data_packs/known_pack.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/packets.hpp>
#include <src/base_objects/particle_data.hpp>
#include <src/base_objects/position.hpp>
#include <src/base_objects/recipe.hpp>
#include <src/registers.hpp>

namespace copper_server::build_in_plugins::network::tcp::protocol {
    namespace play {
        base_objects::network::response updateAttributes__(int32_t entity_id, const list_array<base_objects::packets::attributes>& properties) {
            list_array<uint8_t> packet;
            packet.push_back(0x7C);
            WriteVar<int32_t>(entity_id, packet);
            WriteVar<int32_t>(properties.size(), packet);
            for (auto& [key, value, modifiers] : properties) {
                WriteVar<int32_t>(registers::attributes.at(key).id, packet);
                WriteValue<double>(value, packet);
                WriteVar<int32_t>(modifiers.size(), packet);
                for (auto& [uuid, amount, operation] : modifiers) {
                    WriteUUID(uuid, packet);
                    WriteValue<double>(amount, packet);
                    packet.push_back(operation);
                }
            }
            return base_objects::network::response::answer({std::move(packet)});
        }
    }
}