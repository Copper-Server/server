#include "ClientHandleHelper.hpp"
#include "library/fast_task.hpp"
#include "protocolHelper.hpp"

int main() {
    fast_task::task::create_executor();
    fast_task::task::task::enable_task_naming = true;
    boost::asio::io_service service;

    first_client_holder = new TCPClientHandleHandshaking();
    TCPserver server(&service, boost::asio::ip::tcp::v4(), 1234);
    server.start();
    return 0;
}