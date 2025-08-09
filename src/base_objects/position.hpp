#ifndef SRC_BASE_OBJECTS_POSITION
#define SRC_BASE_OBJECTS_POSITION

namespace copper_server::base_objects {
    struct position {
        int x : 26;
        int z : 26;
        int y : 12;

        inline void set(unsigned long long raw) {
            union u_t {
                position flag;
                unsigned long long r;
            } u{.r = raw};

            *this = u.flag;
        }

        inline unsigned long long get() const {
            union u_t {
                position flag;
                unsigned long long r;
            } u{.flag = *this};

            return u.r;
        }

        bool operator==(const position& other) const {
            return get() == other.get();
        }

        bool operator!=(const position& other) const {
            return get() != other.get();
        }

        auto operator<=>(const position& enbt) const = default;
    };
}

#endif /* SRC_BASE_OBJECTS_POSITION */
