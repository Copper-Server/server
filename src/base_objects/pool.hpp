#ifndef SRC_BASE_OBJECTS_POOL
#define SRC_BASE_OBJECTS_POOL
#include <library/list_array.hpp>
#include <memory>
#include <random>


namespace copper_server::base_objects {
    template<class T>
    class pool {
        int64_t total_pool;
        list_array<std::pair<int32_t, T>> data;

        static std::default_random_engine& get_thread_local_engine() {
            static thread_local std::default_random_engine engine(std::random_device{}());
            return engine;
        }

    public:
        using value_type = T;

        pool() : total_pool(0) {}

        pool(pool&& mov) : total_pool(mov.total_pool), data(std::move(mov.data)) {}
        pool(const pool& cop) : total_pool(cop.total_pool), data(cop.data) {}

        pool(std::initializer_list<std::pair<int32_t, T>> init) : total_pool(0) {
            for (auto& [weight, value] : init) {
                data.emplace_back(weight, std::move(value));
                total_pool += weight;
            }
        }

        pool& operator=(pool&& mov){
            total_pool = mov.total_pool;
            data = std::move(mov.data);
            return *this;
        }

        pool& operator=(const pool& cop) {
            if(this == &cop)
                return *this;
            total_pool = cop.total_pool;
            data = cop.data;
            return *this;
        }

        void add(int32_t weight, T&& move){
            data.emplace_back(weight, std::move(move));
            total_pool += weight;
        }

        void add(int32_t weight, const T& copy){
            data.emplace_back(weight, copy);
        }

        void clear() {
            data.clear();
            total_pool = 0;
        }

        T& make_select() {
            int64_t value = std::uniform_int_distribution<int64_t>(0, total_pool)(get_thread_local_engine());

            for (auto& [weight, value] : data) {
                random -= weight;
                if (random <= 0)
                    return value;
            }
            return data.back().second;
        }

        const T& make_select() const {
            int64_t value = std::uniform_int_distribution<int64_t>(0, total_pool)(get_thread_local_engine());

            for (const auto& [weight, value] : data) {
                random -= weight;
                if (random <= 0)
                    return value;
            }
            return data.back().second;
        }

        template <class FN>
        void iterate_all(FN&& fn) {
            for (auto& [weight, value] : data)
                fn(weight, value);
        }

        template <class FN>
        void iterate_all(FN&& fn) const {
            for (const auto& [weight, value] : data)
                fn(weight, value);
        }
    };
}


#endif /* SRC_BASE_OBJECTS_POOL */
