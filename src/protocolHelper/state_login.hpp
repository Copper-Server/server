#ifndef SRC_PROTOCOLHELPER_STATE_LOGIN
#define SRC_PROTOCOLHELPER_STATE_LOGIN
#include "../mojang_api/hash.hpp"
#include "../mojang_api/session_server.hpp"
#include "packets.hpp"
#include "state_configuration.hpp"
#include <boost/json.hpp>
#include <list>
#include <random>
#include <unordered_map>

namespace crafted_craft {
    class TCPClientHandleLogin : public TCPClientHandle {
    protected:
        virtual Chat* AllowPlayersName(std::string nick) {
            return nullptr;
            Chat chat = Chat("Server closed");
            chat.SetColor("red");
            return new Chat(std::move(chat));
        }

        Response encryptionRequest() {
            excepted_packet = 1;
            auto generate_ui8 = []() -> uint8_t {
                static std::random_device rd;
                static std::mt19937_64 gen;
                static std::uniform_int_distribution<uint16_t> dis;
                uint16_t ui16 = dis(gen);
                return (uint8_t)((ui16 & 0xFF ^ (ui16 >> 8)) & 0xFF);
            };

            list_array<uint8_t> response;
            auto public_key = session->public_key();


            response.push_back(1);
            WriteString(response, "", 20); //server id
            WriteVar<int32_t>(public_key.size(), response);
            response.push_back((uint8_t*)public_key.data(), public_key.size());
            WriteVar<int32_t>(4, response);
            response.push_back(verify_token[0] = generate_ui8());
            response.push_back(verify_token[1] = generate_ui8());
            response.push_back(verify_token[2] = generate_ui8());
            response.push_back(verify_token[3] = generate_ui8());
            log::debug("login", "encryption request");
            return Response::Answer({std::move(response)});
        }

        Response loginSuccess() {
            if (session->serverData().server_config.offline_mode)
                session->sharedData().data = session->serverData().getSessionServer().hasJoined(session->sharedData().name, "", false);
            if (!session->sharedData().data)
                return packets::login::kick("Internal error");

            excepted_packet = 3; //Login Acknowledged
            list_array<uint8_t> response;
            response.push_back(2);
            WriteUUID(session->sharedData().data->uuid, response);
            WriteString(response, session->sharedData().name, 16);
            auto& properties = session->sharedData().data->properties;
            WriteVar<int32_t>(properties.size(), response);
            for (auto& it : properties) {
                WriteString(response, it.name, 32767);
                WriteString(response, it.value, 32767);
                WriteValue(it.signature.has_value(), response);
                if (it.signature.has_value())
                    WriteString(response, *it.signature, 32767);
            }
            response.commit();
            log::debug("login", "request login completion");
            return Response::Answer({std::move(response)});
        }

        Response setCompression(int32_t threshold) {
            Response response = Response::Empty();
            {
                list_array<uint8_t> packet;
                packet.push_back(3);
                WriteVar<int32_t>(threshold, packet);
                response += Response::EnableCompressAnswer(packet, threshold);
            }
            if (!plugins_query.empty())
                response += loginSuccess();
            log::debug("login", "set compression");
            return response;
        }

        Response proceedPlugin(ArrayStream& data, bool successful = true) {
            log::debug("login", "handle plugin request");
            excepted_packet = -1;
            while (plugins_query.size()) {
                auto&& it = plugins_query.front();
                auto response = it.second->OnLoginHandle(it.first, data.to_vector(), successful, session->sharedData());
                if (std::holds_alternative<PluginRegistration::PluginResponse>(response)) {
                    auto& plugin = std::get<PluginRegistration::PluginResponse>(response);
                    plugin_message_id++;
                    list_array<uint8_t> packet;
                    packet.reserve_push_back(1 + 4 + 1 + plugin.data.size());
                    packet.push_back(0x04);
                    WriteVar<int32_t>(plugin_message_id, packet);
                    WriteIdentifier(packet, plugin.plugin_chanel);
                    packet.push_back(plugin.data);
                    return Response::Answer({packet});
                } else if (std::holds_alternative<Response>(response))
                    return std::move(std::get<Response>(response));
                else
                    plugins_query.pop_front();
            }
            return loginSuccess();
        }

        Response WorkPacket(ArrayStream& data) override {
            uint8_t packet_id = data.read();

            if (packet_id != excepted_packet && excepted_packet != -1)
                return Response::Disconnect();
            switch (packet_id) {
            case 0: { //login start

                log::debug("login", "login start");
                std::string nickname = ReadString(data, 16);
                session->sharedData().name = nickname;
                Chat* kick_reason_chat = AllowPlayersName(nickname);
                if (kick_reason_chat) {
                    Chat str = *kick_reason_chat;
                    delete kick_reason_chat;
                    log::debug("login", "kick...");
                    return packets::login::kick(str);
                }
                if (!session->serverData().server_config.offline_mode) {
                    return encryptionRequest(); //still not encrypted
                } else if (session->serverData().server_config.protocol.compression_threshold != -1) {
                    return setCompression(session->serverData().server_config.protocol.compression_threshold);
                } else if (!plugins_query.empty()) {
                    ArrayStream empty((const uint8_t*)nullptr, 0);
                    return proceedPlugin(empty);
                } else {
                    return loginSuccess();
                }
                break;
            }
            case 1: { //encryption response
                log::debug("login", "encryption response");
                int32_t shared_secret_size = ReadVar<int32_t>(data);
                list_array<uint8_t> shared_secret = data.range_read(shared_secret_size).to_vector();
                int32_t verify_token_size = ReadVar<int32_t>(data);
                list_array<uint8_t> verify_token = data.range_read(verify_token_size).to_vector();

                if (!session->serverData().decrypt_data(verify_token)) {
                    log::error("login", "invalid verify token");
                    return packets::login::kick("Invalid verify token");
                }

                if (memcmp(verify_token.data(), this->verify_token, 4)) {
                    log::error("login", "invalid verify token");
                    return packets::login::kick("Invalid verify token");
                }


                if (!session->serverData().decrypt_data(shared_secret)) {
                    log::error("login", "encryption error");
                    return packets::login::kick("Encryption error");
                }

                mojang::api::hash serverId;
                serverId.update(shared_secret);
                serverId.update(session->public_key().data(), session->public_key().size());

                log::debug("login", "mojang request");
                session->sharedData().data = session->serverData().getSessionServer().hasJoined(
                    session->sharedData().name,
                    serverId.hexdigest(),
                    true
                );
                session->start_symmetric_encryption(shared_secret, shared_secret);
                return loginSuccess();
            }
            case 2: { //login plugin response
                log::debug("login", "login plugin response");
                int32_t message_id = ReadVar<int32_t>(data);
                bool successful = data.read();
                if (message_id != plugin_message_id)
                    log::debug("login", "invalid plugin message id");
                return proceedPlugin(data, successful);
            }
            case 3: //Login Acknowledged
                if (excepted_packet == -1) {
                    log::debug("login", "[protocol error] unexpected packet while plugin handle");
                    return Response::Disconnect();
                }

                log::debug("login", "login ack");
                next_handler = new TCPClientHandleConfiguration(session);
                return Response::Empty();
            default:
                log::debug("login", "invalid packet");
            }
            return Response::Disconnect();
        }

        Response TooLargePacket() override {
            return packets::login::kick("Packet too large");
        }

        Response Exception(const std::exception& ex) override {
            return packets::login::kick("Caught exception: " + std::string(ex.what()));
        }

        Response UnexpectedException() override {
            return packets::login::kick("Caught unexpected exception");
        }

        std::list<std::pair<std::string, PluginRegistrationPtr>> plugins_query;
        uint8_t verify_token[4];
        int plugin_message_id = 0;
        bool is_authed = false;
        uint8_t excepted_packet = 0;

    public:
        static std::unordered_map<std::string, PluginRegistrationPtr> plugins;

        TCPClientHandleLogin(TCPsession* sock)
            : TCPClientHandle(sock) {
            for (auto&& it : plugins)
                plugins_query.push_back(it);
        }

        TCPClientHandleLogin()
            : TCPClientHandle(nullptr) {}

        TCPclient* DefineOurself(TCPsession* sock) override {
            return new TCPClientHandleLogin(sock);
        }
    };
}


#endif /* SRC_PROTOCOLHELPER_STATE_LOGIN */
