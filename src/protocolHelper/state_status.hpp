#ifndef SRC_PROTOCOLHELPER_STATE_STATUS
#define SRC_PROTOCOLHELPER_STATE_STATUS
#include "../plugin/special.hpp"
#include "packets.hpp"

namespace crafted_craft {
    class TCPClientHandleStatus : public TCPClientHandle {
    protected:
        //response status
        virtual std::string buildResponse() {
            if (!special_status) {
                return "{"
                       "\"version\": {"
                       "\"name\": \"Undefined handler\","
                       "\"protocol\": 0"
                       "},"
                       "\"players\": {"
                       "\"max\": 0,"
                       "\"online\": 0"
                       "},"
                       "\"description\": {\"text\":\"Please set status handler!\",\"color\":\"red\"}"
                       "}";
            }
            int32_t protocol_version = special_status->ConnectionAvailable(session->protocol_version)
                                           ? session->protocol_version
                                           : 0;
            std::string version_name = special_status->StatusResponseVersionName();

            std::string res = "{"
                              "\"version\": {"
                              "\"name\": \""
                              + version_name + "\","
                                               "\"protocol\": "
                              + std::to_string(protocol_version) + "},";

            if (special_status->ShowConnectionStatus()) {
                res += "\"players\":{\"max\":" + std::to_string(special_status->MaxPlayers()) + ",\"online\":" + std::to_string(special_status->OnlinePlayers());

                auto players = special_status->OnlinePlayersSample();
                if (!players.empty()) {
                    res += ",\"sample\":[";
                    for (auto& it : players) {
                        res += "{\"name\":\"" + it.first + "\",\"id\":\"" + UUID2String(it.second) + "\"},";
                    }
                    res.pop_back();
                    res += "]";
                }

                res += "},";
            }


            res += "\"description\":" + special_status->Description().ToStr();

            std::string base64_fav = special_status->ServerIcon();
            if (base64_fav == "")
                res += "\n}";
            else
                res += ",\"favicon\": \"data:image/png;base64," + base64_fav + "\"\n}";
            return res;
        }

        Response WorkPacket(ArrayStream& packet) override {
            if (!special_status->config.status.enable)
                return Response::Disconnect();
            if (packet.size_read() == 0)
                return Response::Empty();
            else if (packet.size_read() == 1) {
                list_array<uint8_t> response;
                response.push_back(0);
                std::string tmp = buildResponse();
                WriteVar<int32_t>(tmp.size(), response);
                response.push_back((uint8_t*)tmp.data(), tmp.size());
                return Response::Answer({std::move(response)});
            } else
                return Response::Disconnect({packet.to_vector()});
        }

        virtual Response TooLargePacket() {
            return Response::Disconnect();
        }

        Response Exception(const std::exception& ex) override {
            return Response::Disconnect();
        }

        Response UnexpectedException() override {
            return Response::Disconnect();
        }

    public:
        TCPClientHandleStatus(TCPsession* session)
            : TCPClientHandle(session) {
        }

        TCPclient* DefineOurself(TCPsession* sock) override {
            return new TCPClientHandleStatus(sock);
        }
    };
}

#endif /* SRC_PROTOCOLHELPER_STATE_STATUS */
