#include <src/mojang/api/session_server.hpp>

namespace copper_server::api::mojang {
    ::mojang::api::session_server& get_session_server() {
        static ::mojang::api::session_server sessions;
        return sessions;
    }
}