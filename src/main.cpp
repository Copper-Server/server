#include "log.hpp"

#include "ClientHandleHelper.hpp"
#include "library/fast_task.hpp"
#include "protocolHelper.hpp"

#include "build_in_plugins/allowlist.hpp"
#include "build_in_plugins/ban.hpp"
#include "build_in_plugins/communication_core.hpp"
#include "build_in_plugins/minecraft.hpp"
#include "build_in_plugins/server.hpp"
#include "plugin/main.hpp"
#include "registers.hpp"
#include <boost/bimap.hpp>
using namespace crafted_craft;

int main() {

    list_array<char>("Hello World 123 456 789 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5")
        .split_by(" ") //split by space and \0
        .convert<std::string>([](const list_array<char>& a) {
            if (a.empty())
                return std::string();
            return std::string(a.data(), a.size());
        })
        .forEach([](const std::string& a) {
            std::cout << a << std::endl;
        });


    fast_task::task::create_executor();
    fast_task::task::task::enable_task_naming = true;
    boost::asio::io_service service;
    TCPserver server(&service, "localhost", 1234);


    log::commands::init();
    std::string current_path = std::filesystem::current_path().string();
    std::string storage_path = current_path + "/storage";
    std::filesystem::create_directories(storage_path);

    pluginManagement.registerPlugin("allowlist", std::make_shared<build_in_plugins::AllowListPlugin>(storage_path));
    pluginManagement.registerPlugin("ban", std::make_shared<build_in_plugins::BanPlugin>(storage_path));
    pluginManagement.registerPlugin("communication_core", std::make_shared<build_in_plugins::CommunicationCorePlugin>(server.online_players));
    pluginManagement.registerPlugin("minecraft", std::make_shared<build_in_plugins::MinecraftPlugin>());

    //must be last, initializes commands
    pluginManagement.registerPlugin("server", std::make_shared<build_in_plugins::ServerPlugin>(storage_path, server.online_players));


    special_handshake = new SpecialPluginHandshake();
    special_status = new SpecialPluginStatus();

    first_client_holder = new TCPClientHandleHandshaking();

    server.start();
    return 0;
}
