#ifndef SRC_API_ASIO
#define SRC_API_ASIO
#include <boost/asio.hpp>

namespace copper_server::api::asio {
    boost::asio::io_service& get_service();
    boost::asio::thread_pool& get_threads();

    void init(size_t threads);
}


#endif
