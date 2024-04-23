#ifndef SRC_PROTOCOLHELPER_HANDSHAKING
#define SRC_PROTOCOLHELPER_HANDSHAKING
#include "../plugin/special.hpp"
#include "packets.hpp"
#include "state_login.hpp"
#include "state_status.hpp"

namespace crafted_craft {

    class TCPClientHandleHandshaking : public TCPClientHandle {
    protected:
        virtual bool AllowProtocolVersion(int proto_version) {
            if (765 == proto_version)
                return true;
            else
                return true;
        }

        virtual bool AllowServerAddressAndPort(std::string& str, uint16_t port) {
            return true;
        }

        //return empty chat string if everything normal
        virtual std::string AllowPlayersName(std::string& nick) {
            // return Chat("Server closed!").ToStr();
            // return "{\"text\":\"Server closed!\"}";
            return "";
        }

        template <class T>
        T readValue(ArrayStream& data) {
            uint8_t tmp[sizeof(T)];
            for (size_t i = 0; i < sizeof(T); i++)
                tmp[i] = data.read();
            return ENBT::ConvertEndian(std::endian::big, *(T*)tmp);
        }

        Response WorkPacket(ArrayStream& data) override {
            std::cout << session->id << "[handshaking] Handshaking..." << std::endl;
            uint8_t tmp = data.peek();
            if (tmp != '\0') {
                if (special_handshake) {
                    auto [handler, response] = special_handshake->InvalidPacket(tmp, data);
                    if (handler) {
                        next_handler = handler;
                        return Response::Answer({std::move(response)});
                    } else if (!response.empty())
                        return Response::Disconnect({std::move(response)});
                }
                return Response::Disconnect();
            }
            data.read(); //skip protocol version

            int32_t protocol_version = ReadVar<int32_t>(data);
            session->protocol_version = protocol_version;

            if (!AllowProtocolVersion(protocol_version))
                return Response::Disconnect();


            std::string server_address = ReadString(data, 255);
            if (!AllowServerAddressAndPort(server_address, readValue<uint16_t>(data)))
                return Response::Disconnect();

            switch (ReadVar<int32_t>(data)) {
            case 1: //status
                std::cout << session->id << "[handshaking] Switch to status " << std::endl;
                next_handler = new TCPClientHandleStatus(session);
                return Response::Empty();
            case 2: //login
                std::cout << session->id << "[handshaking] Switch to login " << std::endl;
                next_handler = new TCPClientHandleLogin(session);
                return Response::Empty();
            case 3: //transfer
                return Response::Disconnect();
            default:
                return Response::Disconnect();
            }
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
        TCPClientHandleHandshaking(TCPsession* sock)
            : TCPClientHandle(sock) {}

        TCPClientHandleHandshaking()
            : TCPClientHandle(nullptr) {}

        TCPclient* DefineOurself(TCPsession* sock) override {
            return new TCPClientHandleHandshaking(sock);
        }

        TCPclient* RedefineHandler() override {
            return next_handler;
        }

        Response WorkClient(list_array<uint8_t>& clientData, uint64_t timeout_ms) override {
            if (!session)
                return Response::Disconnect();
            return WorkPackets(clientData);
        }

        bool DoDisconnect(baip::address ip) override {
            return banned_players.contains(ip);
        }
    };
}

#endif /* SRC_PROTOCOLHELPER_HANDSHAKING */
