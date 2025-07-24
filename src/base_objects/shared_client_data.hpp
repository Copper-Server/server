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
            } data;

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
                std::list<int32_t> pending_teleport_ids;
                int32_t teleport_id_sequence = 0;
                int32_t windows_id = 0;
                int32_t current_block_sequence_id = 0;
                int32_t container_state_id = 0;
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

        void sendPacket(base_objects::network::response&& packet) {
            {
                fast_task::write_lock lock(pending_packets_lock);
                pending_packets.push_back(std::move(packet));
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
        friend struct virtual_client;
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
