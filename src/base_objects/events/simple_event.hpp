#ifndef SRC_BASE_OBJECTS_EVENTS_NO_CANCEL_SYNC_EVENT
#define SRC_BASE_OBJECTS_EVENTS_NO_CANCEL_SYNC_EVENT
#include <functional>
#include <random>
#include <src/base_objects/events/priority.hpp>
#include <unordered_map>

namespace copper_server::base_objects::events {

    struct simple_event_register_id {
        uint64_t id;
    };

    template <typename... Args>
    struct simple_event {
        using function = std::function<void(Args...)>;

        simple_event_register_id operator+=(function func) {
            return join(func);
        }

        void operator()(Args... args) {
            notify(std::forward<Args>(args)...);
        }

        simple_event_register_id join(function func, priority priority = priority::avg) {
            switch (priority) {
            case priority::heigh:
                return addOne(heigh_priority, func);
            case priority::upper_avg:
                return addOne(upper_avg_priority, func);
            case priority::avg:
                return addOne(avg_priority, func);
            case priority::lower_avg:
                return addOne(lower_avg_priority, func);
            case priority::low:
                return addOne(low_priority, func);
            }
            throw std::runtime_error("Invalid priority");
        }

        bool leave(simple_event_register_id func, priority priority = priority::avg) {
            switch (priority) {
            case priority::heigh:
                return removeOne(heigh_priority, func);
            case priority::upper_avg:
                return removeOne(upper_avg_priority, func);
            case priority::avg:
                return removeOne(avg_priority, func);
            case priority::lower_avg:
                return removeOne(lower_avg_priority, func);
            case priority::low:
                return removeOne(low_priority, func);
            }
            return false;
        }

        void notify(Args... args) {
            sync_call(heigh_priority, std::forward<Args>(args)...);
            sync_call(upper_avg_priority, std::forward<Args>(args)...);
            sync_call(avg_priority, std::forward<Args>(args)...);
            sync_call(lower_avg_priority, std::forward<Args>(args)...);
            sync_call(low_priority, std::forward<Args>(args)...);
        }

        void clear() {
            heigh_priority.clear();
            upper_avg_priority.clear();
            avg_priority.clear();
            lower_avg_priority.clear();
            low_priority.clear();
        }

        sync_event()
            : gen(rd()) {}

    private:
        std::random_device rd;
        std::mt19937 gen;
        std::unordered_map<uint64_t, function> heigh_priority;
        std::unordered_map<uint64_t, function> upper_avg_priority;
        std::unordered_map<uint64_t, function> avg_priority;
        std::unordered_map<uint64_t, function> lower_avg_priority;
        std::unordered_map<uint64_t, function> low_priority;

        static bool removeOne(std::unordered_map<uint64_t, function>& map, simple_event_register_id func) {
            auto it = map.find(func.id);
            if (it != map.end()) {
                map.erase(it);
                return true;
            }
            return false;
        }

        simple_event_register_id addOne(std::unordered_map<uint64_t, function>& map, function func) {
            std::uniform_int_distribution<uint64_t> dis;
            simple_event_register_id id;
            do {
                id.id = dis(gen);
            } while (map.find(id.id) != map.end());
            map[id.id] = func;
            return id;
        }

        void sync_call(std::unordered_map<uint64_t, function>& map, Args... args) {
            for (auto& [_, func] : map)
                func(std::forward<Args>(args)...);
        }
    };
}

#endif /* SRC_BASE_OBJECTS_EVENTS_NO_CANCEL_SYNC_EVENT */
