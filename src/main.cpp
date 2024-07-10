#include "log.hpp"

#include "ClientHandleHelper.hpp"
#include "library/fast_task.hpp"
#include "protocolHelper.hpp"

#include "build_in_plugins/allowlist.hpp"
#include "build_in_plugins/ban.hpp"
#include "build_in_plugins/communication_core.hpp"
#include "build_in_plugins/console.hpp"
#include "build_in_plugins/minecraft.hpp"
#include "build_in_plugins/permissions.hpp"
#include "build_in_plugins/server.hpp"
#include "build_in_plugins/special/status.hpp"
#include "build_in_plugins/world.hpp"
#include "library/list_array.hpp"
#include "plugin/main.hpp"
#include "registers.hpp"
#include "utf8.h"
#include <boost/bimap.hpp>
#include <memory>

using namespace crafted_craft;

#include <conio.h>
#include <windows.h>
int main() {

    log::commands::init();

    fast_task::task::create_executor();
    fast_task::task::task::enable_task_naming = true;

    boost::asio::io_service service;
    Server server(&service, "localhost", 1234);

    log::disable_log_level(log::level::debug);

    pluginManagement.registerPlugin("server", std::make_shared<build_in_plugins::ServerPlugin>());
    pluginManagement.registerPlugin("world", std::make_shared<build_in_plugins::WorldManagementPlugin>());
    pluginManagement.registerPlugin("allowlist", std::make_shared<build_in_plugins::AllowListPlugin>());
    pluginManagement.registerPlugin("ban", std::make_shared<build_in_plugins::BanPlugin>());
    pluginManagement.registerPlugin("communication_core", std::make_shared<build_in_plugins::CommunicationCorePlugin>());
    pluginManagement.registerPlugin("minecraft", std::make_shared<build_in_plugins::MinecraftPlugin>());
    pluginManagement.registerPlugin("console", std::make_shared<build_in_plugins::ConsolePlugin>());
    pluginManagement.registerPlugin("permissions", std::make_shared<build_in_plugins::PermissionsPlugin>());

    special_handshake = new SpecialPluginHandshake();
    special_status = new build_in_plugins::special::Status(server.config, server.online_players);

    first_client_holder = new TCPClientHandleHandshaking();

    pluginManagement.callLoad();
    server.start();
    log::commands::deinit();
    fast_task::task::shutDown();
    return 0;
}
