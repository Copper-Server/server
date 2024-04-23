#include "ClientHandleHelper.hpp"
#include "library/fast_task.hpp"
#include "protocolHelper.hpp"

#include "build_in_plugins/minecraft.hpp"
#include "plugin/main.hpp"
#include "registers.hpp"

int main() {

    TCPclient::logConsole(crafted_craft::registers::registryDataPacket());

    crafted_craft::pluginManagement.registerPlugin("minecraft", std::make_shared<crafted_craft::build_in_plugins::Minecraft>());

    fast_task::task::create_executor();
    fast_task::task::task::enable_task_naming = true;
    boost::asio::io_service service;

    crafted_craft::special_handshake = new crafted_craft::SpecialPluginHandshake();
    crafted_craft::special_status = new crafted_craft::SpecialPluginStatus();

    first_client_holder = new crafted_craft::TCPClientHandleHandshaking();
    TCPserver server(&service, "localhost", 1234);

    server.start();
    return 0;
}