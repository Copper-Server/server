#ifndef SRC_BASE_OBJECTS_SYNC_EVENT
#define SRC_BASE_OBJECTS_SYNC_EVENT
#include <functional>
#include <random>
#include <unordered_map>

namespace crafted_craft {
    namespace base_objects {

        struct sync_event_register_id {
            uint64_t id;
        };

        template <typename... Args>
        struct sync_event {

            using function = std::function<bool(Args...)>;
            enum class Priority {
                heigh,
                upper_avg,
                avg,
                lower_avg,
                low
            };

            sync_event_register_id operator+=(function func) {
                return join(func);
            }

            bool operator()(Args... args) {
                return notify(std::forward(args)...);
            }

            sync_event_register_id join(function func, bool async_mode = false, Priority priority = Priority::avg) {
                switch (priority) {
                case Priority::heigh:
                    return addOne(heigh_priority, func);
                case Priority::upper_avg:
                    return addOne(upper_avg_priority, func);
                case Priority::avg:
                    return addOne(avg_priority, func);
                case Priority::lower_avg:
                    return addOne(lower_avg_priority, func);
                case Priority::low:
                    return addOne(low_priority, func);
                }
                throw std::runtime_error("Invalid priority");
            }

            bool leave(sync_event_register_id func, Priority priority = Priority::avg) {
                switch (priority) {
                case Priority::heigh:
                    return removeOne(heigh_priority, func);
                case Priority::upper_avg:
                    return removeOne(upper_avg_priority, func);
                case Priority::avg:
                    return removeOne(avg_priority, func);
                case Priority::lower_avg:
                    return removeOne(lower_avg_priority, func);
                case Priority::low:
                    return removeOne(low_priority, func);
                }
                return false;
            }

            bool notify(Args... args) {
                if (sync_call(heigh_priority, std::forward(args)...))
                    return true;
                if (sync_call(upper_avg_priority, std::forward(args)...))
                    return true;
                if (sync_call(avg_priority, std::forward(args)...))
                    return true;
                if (sync_call(lower_avg_priority, std::forward(args)...))
                    return true;
                if (sync_call(low_priority, std::forward(args)...))
                    return true;
                return false;
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

            static bool removeOne(std::unordered_map<uint64_t, function>& map, sync_event_register_id func) {
                auto it = map.find(func.id);
                if (it != map.end()) {
                    map.erase(it);
                    return true;
                }
                return false;
            }

            sync_event_register_id addOne(std::unordered_map<uint64_t, function>& map, function func) {
                std::uniform_int_distribution<uint64_t> dis;
                sync_event_register_id id;
                do {
                    id.id = dis(gen);
                } while (map.find(id.id) != map.end());
                map[id.id] = func;
                return id;
            }

            bool sync_call(std::unordered_map<uint64_t, function>& map, Args... args) {
                for (auto& [_, func] : map)
                    if (func(std::forward(args)...))
                        return true;
                return false;
            }
        };
    }
}
#endif /* SRC_BASE_OBJECTS_SYNC_EVENT */
