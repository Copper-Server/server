#include "log.hpp"

#include "ClientHandleHelper.hpp"
#include "library/fast_task.hpp"
#include "protocolHelper.hpp"

#include "build_in_plugins/ban.hpp"
#include "build_in_plugins/minecraft.hpp"
#include "build_in_plugins/server.hpp"
#include "plugin/main.hpp"
#include "registers.hpp"
using namespace crafted_craft;

int main() {
    log::commands::init();
    std::string current_path = std::filesystem::current_path().string();
    std::string storage_path = current_path + "/storage";
    pluginManagement.registerPlugin("ban", std::make_shared<build_in_plugins::BanPlugin>(storage_path));
    pluginManagement.registerPlugin("server", std::make_shared<build_in_plugins::ServerPlugin>(storage_path));
    pluginManagement.registerPlugin("minecraft", std::make_shared<build_in_plugins::MinecraftPlugin>());

    fast_task::task::create_executor();
    fast_task::task::task::enable_task_naming = true;
    boost::asio::io_service service;

    special_handshake = new SpecialPluginHandshake();
    special_status = new SpecialPluginStatus();

    first_client_holder = new TCPClientHandleHandshaking();
    TCPserver server(&service, "localhost", 1234);

    server.start();
    return 0;
}
