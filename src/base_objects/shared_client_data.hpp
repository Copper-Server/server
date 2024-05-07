#ifndef SRC_BASE_OBJECTS_SHARED_CLIENT_DATA
#define SRC_BASE_OBJECTS_SHARED_CLIENT_DATA

#include <chrono>
#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "../library/enbt.hpp"
#include "../library/fast_task.hpp"
#include "../library/list_array.hpp"
#include "../mojang_api/session_server.hpp"
#include "chat.hpp"
#include "player.hpp"
#include "response.hpp"

namespace crafted_craft {
    namespace base_objects {
        struct SharedClientData {
            std::string name;
            std::string ip;
            std::string str_uuid;
            ENBT::UUID uuid;
            std::shared_ptr<mojang::api::session_server::player_data> data;
            std::string client_brand;


            std::string locale; //max 16 chars
            std::unordered_set<std::string> compatible_plugins;
            uint8_t view_distance = 0;
            enum class ChatMode : uint8_t {
                ENABLED = 0,
                COMMANDS_ONLY = 1,
                HIDDEN = 2
            } chat_mode = ChatMode::ENABLED;

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
            } main_hand = MainHand::RIGHT;

            bool enable_filtering : 1 = false;
            bool allow_server_listings : 1 = true;
            bool enable_chat_colors : 1 = true;
            crafted_craft::base_objects::player player_data;

            struct ResourcePackData {
                bool required : 1 = false;
            };

            struct packets_state_t {
                std::unordered_set<ENBT::UUID> active_resource_packs;
                std::unordered_map<ENBT::UUID, ResourcePackData> pending_resource_packs;
                std::list<int32_t> pending_teleport_ids;
                int32_t entity_id_generator;
                int32_t current_block_sequence_id = 0;
                int32_t container_state_id = 0;


                int32_t protocol_version = -1;
                enum class protocol_state {
                    initialization, //handshake, status, login
                    configuration,
                    play
                } state = protocol_state::initialization;
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

            std::function<void(SharedClientData*)> on_destruct;

            void sendPacket(const Response& packet) {
                fast_task::write_lock lock(pending_packets_lock);
                pending_packets.push_back(packet);
            }

            std::list<Response> getPendingPackets() {
                fast_task::read_lock lock(pending_packets_lock);
                return std::move(pending_packets);
            }

            ~SharedClientData() {
                if (on_destruct)
                    on_destruct(this);
            }

        private:
            fast_task::task_rw_mutex pending_packets_lock;
            std::list<Response> pending_packets;
        };

        class client_data_holder {
            SharedClientData* data;
            std::atomic_size_t* ref_count;

            client_data_holder(SharedClientData* data, std::atomic_size_t* ref_count)
                : data(data), ref_count(ref_count) {}

            void decrease_counter() {
                if (ref_count) {
                    if (--(*ref_count) == 0) {
                        delete ref_count;
                        delete data;
                    }
                }
            }

        public:
            client_data_holder()
                : data(), ref_count() {}

            client_data_holder(SharedClientData* data)
                : data(data), ref_count(new std::atomic_size_t(1)) {}

            client_data_holder(const client_data_holder& other)
                : data(other.data), ref_count(other.ref_count) {
                if (ref_count) {
                    (*ref_count)++;
                }
            }

            client_data_holder(client_data_holder&& other)
                : data(other.data), ref_count(other.ref_count) {
                other.data = nullptr;
                other.ref_count = nullptr;
            }

            client_data_holder& operator=(SharedClientData* data) {
                if (this->data == data)
                    return *this;
                decrease_counter();
                this->data = data;
                ref_count = new std::atomic_size_t(1);
                return *this;
            }

            client_data_holder& operator=(const client_data_holder& other) {
                if (data == other.data)
                    return *this;
                decrease_counter();
                data = other.data;
                ref_count = other.ref_count;
                if (ref_count)
                    (*ref_count)++;
                return *this;
            }

            client_data_holder& operator=(client_data_holder&& other) {
                if (data == other.data)
                    return *this;
                decrease_counter();
                data = other.data;
                ref_count = other.ref_count;
                other.data = nullptr;
                other.ref_count = nullptr;
                return *this;
            }

            ~client_data_holder() {
                decrease_counter();
            }

            SharedClientData* operator->() {
                if (!data)
                    throw std::runtime_error("Client not found or caught internal error.");
                return data;
            }

            SharedClientData& operator*() {
                if (!data)
                    throw std::runtime_error("Client not found or caught internal error.");
                return *data;
            }

            SharedClientData* operator->() const {
                if (!data)
                    throw std::runtime_error("Client not found or caught internal error.");
                return data;
            }

            SharedClientData& operator*() const {
                if (!data)
                    throw std::runtime_error("Client not found or caught internal error.");
                return *data;
            }

            bool operator==(const client_data_holder& other) const {
                return data == other.data;
            }

            bool operator!=(const client_data_holder& other) const {
                return data != other.data;
            }

            operator bool() const {
                return data != nullptr;
            }

            bool operator!() const {
                return data == nullptr;
            }
        };
    }

}

using SharedClientData = crafted_craft::base_objects::SharedClientData;
#endif /* SRC_BASE_OBJECTS_SHARED_CLIENT_DATA */
