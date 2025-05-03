#include <src/api/configuration.hpp>
#include <src/log.hpp>
#include <src/plugin/special.hpp>
#include <src/protocolHelper/packets.hpp>
#include <src/protocolHelper/state_status.hpp>

namespace copper_server {
    std::string tcp_client_handle_status::build_response() {
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
            auto max_count = special_status->MaxPlayers();
            auto online = special_status->OnlinePlayers();
            if (max_count == 0)
                max_count = online + 1;

            res += "\"players\":{\"max\":" + std::to_string(max_count) + ",\"online\":" + std::to_string(online);

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
        if (base64_fav != "")
            res += ",\"favicon\": \"data:image/png;base64," + base64_fav + "\"";


        auto preventsChatReports = special_status->PreventsChatReports();

        if (preventsChatReports.has_value())
            res += ",\"preventChatReports\":" + std::string(*preventsChatReports ? "true" : "false");

        res += ",\"enforcesSecureChat\":" + std::string(api::configuration::get().mojang.enforce_secure_profile ? "true" : "false");

        auto custom_json = special_status->CustomJson();
        if (!custom_json.empty())
            res += ", " + custom_json;

        res += "}";
        return res;
    }

    base_objects::network::response tcp_client_handle_status::work_packet(ArrayStream& packet) {
        if (!api::configuration::get().status.enable)
            return base_objects::network::response::disconnect();
        if (packet.size_read() == 0)
            return base_objects::network::response::empty();
        else if (packet.size_read() == 1) {
            list_array<uint8_t> response;
            response.push_back(0);
            std::string tmp = build_response();
            log::debug("status", tmp);
            WriteVar<int32_t>(tmp.size(), response);
            response.push_back((uint8_t*)tmp.data(), tmp.size());
            return base_objects::network::response::answer({std::move(response)});
        } else
            return base_objects::network::response::disconnect({packet.to_vector()});
    }

    base_objects::network::response tcp_client_handle_status::too_large_packet() {
        return base_objects::network::response::disconnect();
    }

    base_objects::network::response tcp_client_handle_status::exception(const std::exception& ex) {
        return base_objects::network::response::disconnect();
    }

    base_objects::network::response tcp_client_handle_status::unexpected_exception() {
        return base_objects::network::response::disconnect();
    }

    tcp_client_handle_status::tcp_client_handle_status(base_objects::network::tcp::session* session)
        : tcp_client_handle(session) {
    }

    base_objects::network::tcp::client* tcp_client_handle_status::define_ourself(base_objects::network::tcp::session* sock) {
        return new tcp_client_handle_status(sock);
    }
}
