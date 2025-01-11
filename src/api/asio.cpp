#if defined(_MSC_VER)
    #include <SDKDDKVer.h>
#endif
#include <boost/asio.hpp>
#include <thread>

namespace copper_server::api::asio {
    boost::asio::io_service service;
    boost::asio::thread_pool* thread_pool;

    boost::asio::io_service& get_service() {
        return service;
    }

    boost::asio::thread_pool& get_threads() {
        if (thread_pool == nullptr)
            thread_pool = new boost::asio::thread_pool(1);
        return *thread_pool;
    }

    void init(size_t threads) {
        if (thread_pool == nullptr)
            thread_pool = new boost::asio::thread_pool(threads ? threads : std::thread::hardware_concurrency());
    }
}