#ifndef SRC_BASE_OBJECTS_SHARED_CLIENT_DATA
#define SRC_BASE_OBJECTS_SHARED_CLIENT_DATA
#include <chrono>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <library/enbt/enbt.hpp>
#include <library/fast_task.hpp>
#include <library/list_array.hpp>
#include <src/base_objects/atomic_holder.hpp>
#include <src/base_objects/chat.hpp>
#include <src/base_objects/network/response.hpp>
#include <src/base_objects/ptr_optional.hpp>
#include <src/mojang/api/session_server.hpp>
#include <src/plugin/registration.hpp>

namespace copper_server::base_objects {
    namespace network::tcp {
        class client;
    }
    class player;

    struct SharedClientData {
        std::string name;
        std::string ip;
        std::shared_ptr<mojang::api::session_server::player_data> data;
        std::string client_brand;


        std::string locale; //max 16 chars
        std::unordered_set<std::string> compatible_plugins;
        uint8_t view_distance = 0;
        uint8_t simulation_distance = 0;
        enum class ChatMode : uint8_t {
            ENABLED = 0,
            COMMANDS_ONLY = 1,
            HIDDEN = 2
        } chat_mode
            = ChatMode::ENABLED;

        union {
            struct {
                bool cape_enabled : 1;
                bool jacket_enabled : 1;
                bool left_sleeve_enabled : 1;
                bool right_sleeve_enabled : 1;
                bool left_pants_leg_enabled : 1;
                bool right_pants_leg_enabled : 1;
                bool hat_enabled : 1;
                bool _unused : 1;
            };

            uint8_t mask = UINT8_MAX - 1;
        } skin_parts;

        enum class MainHand : uint8_t {
            LEFT = 0,
            RIGHT = 1
        } main_hand
            = MainHand::RIGHT;

        bool enable_filtering : 1 = false;
        bool allow_server_listings : 1 = true;
        bool enable_chat_colors : 1 = true;
        bool is_virtual : 1 = false;
        enum class ParticleStatus : uint8_t {
            ALL = 0,
            DECREASED = 1,
            MINIMAL = 2
        } particle_status : 2
            = ParticleStatus::ALL;
        player& player_data;

        struct ResourcePackData {
            bool required : 1 = false;
        };

        struct packets_state_t {
            struct state_login {
                std::list<std::pair<std::string, PluginRegistrationPtr>> plugins_query;
                bool had_conflict = false;
                uint8_t verify_token[4] = {0};
                int32_t excepted_packet = -1;
                int32_t plugin_sequence_id = 0;
                int8_t login_check = 0; //0 - check encryption options, 1 - check compression options, 2 - proceed plugins, 3 - done
            };

            struct state_play {
                bit_list_array<> loaded_chunks{0};
                std::list<int32_t> pending_teleport_ids;
                int32_t teleport_id_sequence = 0;
                int32_t windows_id = 0;
                int32_t current_block_sequence_id = 0;
                int32_t container_state_id = 0;

                int32_t loading_center_x = 0;
                int32_t loading_center_z = 0;
                int32_t loading_diameter = 1;

                bool mark_chunk(int64_t pos_x, int64_t pos_z, bool loaded) {
                    if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                        return false;

                    int32_t radius = loading_diameter / 2;
                    int32_t offset_x = pos_x - (loading_center_x - radius);
                    int32_t offset_z = pos_z - (loading_center_z - radius);

                    if (offset_x < 0 || offset_x >= loading_diameter || offset_z < 0 || offset_z >= loading_diameter)
                        return false;

                    size_t index = offset_z * loading_diameter + offset_x;
                    loaded_chunks.set(index, loaded);
                    return true;
                }

                bool in_bounds(int64_t pos_x, int64_t pos_z) {
                    if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                        return false;

                    int32_t radius = loading_diameter / 2;
                    int32_t offset_x = pos_x - (loading_center_x - radius);
                    int32_t offset_z = pos_z - (loading_center_z - radius);

                    return offset_x < 0 || offset_x >= loading_diameter || offset_z < 0 || offset_z >= loading_diameter;
                }

                bool chunk_loaded(int64_t pos_x, int64_t pos_z) const {
                    if (pos_x > INT32_MAX || pos_x < INT32_MIN || pos_z > INT32_MAX || pos_z < INT32_MIN)
                        return false;

                    int32_t radius = loading_diameter / 2;
                    int32_t offset_x = pos_x - (loading_center_x - radius);
                    int32_t offset_z = pos_z - (loading_center_z - radius);

                    if (offset_x < 0 || offset_x >= loading_diameter || offset_z < 0 || offset_z >= loading_diameter)
                        return false;

                    size_t index = offset_z * loading_diameter + offset_x;
                    return loaded_chunks.at(index);
                }

                void update_loading(int32_t center_x, int32_t center_z, uint8_t render_distance) {
                    auto new_loading_diameter = 2 * render_distance + 7;
                    bit_list_array<> new_loading_data(new_loading_diameter * new_loading_diameter);

                    int32_t old_radius = loading_diameter / 2;
                    int32_t new_radius = new_loading_diameter / 2;

                    // For each position in the new loading area
                    for (int32_t dz = 0; dz < new_loading_diameter; ++dz) {
                        for (int32_t dx = 0; dx < new_loading_diameter; ++dx) {
                            // World coordinates for this chunk in the new area
                            int32_t chunk_x = center_x - new_radius + dx;
                            int32_t chunk_z = center_z - new_radius + dz;

                            // Map to old area offsets
                            int32_t old_offset_x = chunk_x - (loading_center_x - old_radius);
                            int32_t old_offset_z = chunk_z - (loading_center_z - old_radius);

                            // If the chunk was loaded in the old area, copy its bit
                            if (old_offset_x >= 0 && old_offset_x < loading_diameter && old_offset_z >= 0 && old_offset_z < loading_diameter) {
                                size_t old_index = old_offset_z * loading_diameter + old_offset_x;
                                size_t new_index = dz * new_loading_diameter + dx;
                                if (loaded_chunks[old_index]) {
                                    new_loading_data.set(new_index, true);
                                }
                            }
                        }
                    }

                    // Update center and diameter
                    loading_center_x = center_x;
                    loading_center_z = center_z;
                    loading_diameter = new_loading_diameter;
                    loaded_chunks = std::move(new_loading_data);
                }

                void flush_loading() {
                    loaded_chunks = bit_list_array<>(loading_diameter * loading_diameter);
                }
            };

            std::unordered_set<enbt::raw_uuid> active_resource_packs;
            std::unordered_map<enbt::raw_uuid, ResourcePackData> pending_resource_packs;
            std::chrono::time_point<std::chrono::system_clock> pong_timer;
            int32_t keep_alive_ping_ms = 0;
            int32_t excepted_pong = -1;


            int32_t protocol_version = -1;
            enum class protocol_state : uint8_t {
                initialization, //handshake, status, login
                configuration,
                play
            } state : 2
                = protocol_state::initialization;
            enum class configuration_load_state_t : uint8_t {
                to_init,
                await_known_packs,
                await_processing,
                done
            } load_state
                = configuration_load_state_t::to_init;
            ptr_optional<state_login> login_data;
            ptr_optional<state_play> play_data;
        } packets_state;

        std::chrono::milliseconds ping = std::chrono::milliseconds(0);

        bool mark_chunk(int64_t pos_x, int64_t pos_z, bool loaded) {
            if (packets_state.play_data)
                return packets_state.play_data->mark_chunk(pos_x, pos_z, loaded);
            return false;
        }

        bool chunk_in_bounds(int64_t pos_x, int64_t pos_z) {
            if (packets_state.play_data)
                return packets_state.play_data->in_bounds(pos_x, pos_z);
            return false;
        }

        bool chunk_loaded_at(int64_t pos_x, int64_t pos_z) const {
            if (packets_state.play_data)
                return packets_state.play_data->chunk_loaded(pos_x, pos_z);
            return false;
        }

        void registerPlugin(std::string& plugin) {
            compatible_plugins.insert(plugin);
        }

        void unregisterPlugin(std::string& plugin) {
            compatible_plugins.erase(plugin);
        }

        bool isCompatiblePlugin(std::string& plugin) {
            return compatible_plugins.contains(plugin);
        }

        void sendPacket(const base_objects::network::response& packet) {
            {
                fast_task::write_lock lock(pending_packets_lock);
                pending_packets.push_back(packet);
            }
            if (special_callback)
                special_callback(*this);
        }

        list_array<network::response> getPendingPackets() {
            fast_task::read_lock lock(pending_packets_lock);
            return pending_packets.take();
        }

        SharedClientData(void* assigned_data = nullptr, std::function<void(base_objects::SharedClientData& self)> special_callback = nullptr);
        ~SharedClientData();

        void* getAssignedData() const {
            return assigned_data;
        }

        bool canBeRemoved() const {
            return !special_callback;
        }

        bool isSpecial() const {
            return (bool)special_callback;
        }

        //internal
        void setKeepAliveCallback(std::function<void(int64_t)> callback) {
            keep_alive_callback = callback;
        }

        //internal
        void gotKeepAlive(int64_t timestamp) {
            if (keep_alive_callback)
                keep_alive_callback(timestamp);
        }

        //internal
        void setSwitchToHandlerCallback(std::function<void(network::tcp::client*)> callback) {
            switch_to_handler_callback = callback;
        }

        //internal
        void switchToHandler(network::tcp::client* handler) {
            if (switch_to_handler_callback)
                switch_to_handler_callback(handler);
        }

    private:
        friend class virtual_client;
        void* assigned_data;
        std::function<void(base_objects::SharedClientData& self)> special_callback;
        std::function<void(int64_t)> keep_alive_callback;
        std::function<void(network::tcp::client*)> switch_to_handler_callback;
        fast_task::task_rw_mutex pending_packets_lock;
        list_array<network::response> pending_packets;
    };

    using client_data_holder = atomic_holder<SharedClientData>;
}
#endif /* SRC_BASE_OBJECTS_SHARED_CLIENT_DATA */
