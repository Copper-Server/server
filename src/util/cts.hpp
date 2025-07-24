#ifndef SRC_UTIL_CTS
#define SRC_UTIL_CTS
#include <cinttypes>

namespace copper_server::util {
    template <std::size_t N>
    struct CTS {
        char data[N]{};

        consteval CTS(const char (&str)[N]) {
            std::copy_n(str, N, data);
        }
    };
}


#endif /* SRC_UTIL_CTS */
