#ifndef SRC_BASE_OBJECTS_EVENT
#define SRC_BASE_OBJECTS_EVENT
#include "../library/fast_task.hpp"
#include <functional>
#include <random>
#include <unordered_map>

namespace crafted_craft {
    namespace base_objects {

        struct event_register_id {
            uint64_t id;
        };

        enum class event_priority {
            heigh,
            upper_avg,
            avg,
            lower_avg,
            low
        };

        class base_event {
        public:
            virtual bool leave(event_register_id func, event_priority priority = event_priority::avg, bool async_mode = false) = 0;
        };

        template <typename T>
        class event : public base_event {
        public:
            using function = std::function<bool(const T&)>;

            event_register_id operator+=(function func) {
                return join(func);
            }

            bool operator()(const T& args) {
                return notify(args);
            }

            event_register_id join(function func) {
                return join(event_priority::avg, false, func);
            }

            event_register_id join(event_priority priority, function func) {
                return join(priority, false, func);
            }

            event_register_id join(event_priority priority, bool async_mode, function func) {
                std::lock_guard<fast_task::task_mutex> lock(mutex);
                if (async_mode) {
                    switch (priority) {
                    case event_priority::heigh:
                        return addOne(async_heigh_priority, func);
                    case event_priority::upper_avg:
                        return addOne(async_upper_avg_priority, func);
                    case event_priority::avg:
                        return addOne(async_avg_priority, func);
                    case event_priority::lower_avg:
                        return addOne(async_lower_avg_priority, func);
                    case event_priority::low:
                        return addOne(async_low_priority, func);
                    }
                } else {
                    switch (priority) {
                    case event_priority::heigh:
                        return addOne(heigh_priority, func);
                    case event_priority::upper_avg:
                        return addOne(upper_avg_priority, func);
                    case event_priority::avg:
                        return addOne(avg_priority, func);
                    case event_priority::lower_avg:
                        return addOne(lower_avg_priority, func);
                    case event_priority::low:
                        return addOne(low_priority, func);
                    }
                }
                throw std::runtime_error("Invalid priority");
            }

            bool leave(event_register_id func, event_priority priority = event_priority::avg, bool async_mode = false) override {
                std::lock_guard<fast_task::task_mutex> lock(mutex);
                if (async_mode) {
                    switch (priority) {
                    case event_priority::heigh:
                        return removeOne(async_heigh_priority, func);
                    case event_priority::upper_avg:
                        return removeOne(async_upper_avg_priority, func);
                    case event_priority::avg:
                        return removeOne(async_avg_priority, func);
                    case event_priority::lower_avg:
                        return removeOne(async_lower_avg_priority, func);
                    case event_priority::low:
                        return removeOne(async_low_priority, func);
                    }
                } else {
                    switch (priority) {
                    case event_priority::heigh:
                        return removeOne(heigh_priority, func);
                    case event_priority::upper_avg:
                        return removeOne(upper_avg_priority, func);
                    case event_priority::avg:
                        return removeOne(avg_priority, func);
                    case event_priority::lower_avg:
                        return removeOne(lower_avg_priority, func);
                    case event_priority::low:
                        return removeOne(low_priority, func);
                    }
                }
                return false;
            }

            bool await_notify(const T& args) {
                std::lock_guard<fast_task::task_mutex> lock(mutex);
                if (can_skip())
                    return false;
                if (await_call(async_heigh_priority, args))
                    return true;
                if (await_call(async_upper_avg_priority, args))
                    return true;
                if (await_call(async_avg_priority, args))
                    return true;
                if (await_call(async_lower_avg_priority, args))
                    return true;
                if (await_call(async_low_priority, args))
                    return true;
                if (sync_call(heigh_priority, args))
                    return true;
                if (sync_call(upper_avg_priority, args))
                    return true;
                if (sync_call(avg_priority, args))
                    return true;
                if (sync_call(lower_avg_priority, args))
                    return true;
                if (sync_call(low_priority, args))
                    return true;
                return false;
            }

            bool notify(const T& args) {
                std::lock_guard<fast_task::task_mutex> lock(mutex);
                if (can_skip())
                    return false;
                if (await_call(async_heigh_priority, args))
                    return true;
                if (await_call(async_upper_avg_priority, args))
                    return true;
                if (await_call(async_avg_priority, args))
                    return true;
                if (await_call(async_lower_avg_priority, args))
                    return true;
                if (await_call(async_low_priority, args))
                    return true;
                if (sync_call(heigh_priority, args))
                    return true;
                if (sync_call(upper_avg_priority, args))
                    return true;
                if (sync_call(avg_priority, args))
                    return true;
                if (sync_call(lower_avg_priority, args))
                    return true;
                if (sync_call(low_priority, args))
                    return true;
                return false;
            }

            bool sync_notify(const T& args) {
                std::lock_guard<fast_task::task_mutex> lock(mutex);
                if (can_skip())
                    return false;
                if (sync_call(async_heigh_priority, args))
                    return true;
                if (sync_call(async_upper_avg_priority, args))
                    return true;
                if (sync_call(async_avg_priority, args))
                    return true;
                if (sync_call(async_lower_avg_priority, args))
                    return true;
                if (sync_call(async_low_priority, args))
                    return true;

                if (sync_call(heigh_priority, args))
                    return true;
                if (sync_call(upper_avg_priority, args))
                    return true;
                if (sync_call(avg_priority, args))
                    return true;
                if (sync_call(lower_avg_priority, args))
                    return true;
                if (sync_call(low_priority, args))
                    return true;
                return false;
            }

            std::shared_ptr<fast_task::task> async_notify(const T& args) {
                std::lock_guard<fast_task::task_mutex> lock(mutex);
                if (can_skip())
                    return nullptr;

                auto task = std::make_shared<fast_task::task>(
                    [this, args]() { notify(args); }
                );
                fast_task::task::start(task);
                return task;
            }

            void clear() {
                std::lock_guard<fast_task::task_mutex> lock(mutex);
                heigh_priority.clear();
                upper_avg_priority.clear();
                avg_priority.clear();
                lower_avg_priority.clear();
                low_priority.clear();

                async_heigh_priority.clear();
                async_upper_avg_priority.clear();
                async_avg_priority.clear();
                async_lower_avg_priority.clear();
                async_low_priority.clear();
            }

            event()
                : gen(rd()) {}

        private:
            bool can_skip() {
                return heigh_priority.empty() &&
                       upper_avg_priority.empty() &&
                       avg_priority.empty() &&
                       lower_avg_priority.empty() &&
                       low_priority.empty() &&
                       async_heigh_priority.empty() &&
                       async_upper_avg_priority.empty() &&
                       async_avg_priority.empty() &&
                       async_lower_avg_priority.empty() &&
                       async_low_priority.empty();
            }
            fast_task::task_mutex mutex;
            std::random_device rd;
            std::mt19937 gen;
            std::unordered_map<uint64_t, function> heigh_priority;
            std::unordered_map<uint64_t, function> upper_avg_priority;
            std::unordered_map<uint64_t, function> avg_priority;
            std::unordered_map<uint64_t, function> lower_avg_priority;
            std::unordered_map<uint64_t, function> low_priority;

            std::unordered_map<uint64_t, function> async_heigh_priority;
            std::unordered_map<uint64_t, function> async_upper_avg_priority;
            std::unordered_map<uint64_t, function> async_avg_priority;
            std::unordered_map<uint64_t, function> async_lower_avg_priority;
            std::unordered_map<uint64_t, function> async_low_priority;

            static bool removeOne(std::unordered_map<uint64_t, function>& map, event_register_id func) {
                auto it = map.find(func.id);
                if (it != map.end()) {
                    map.erase(it);
                    return true;
                }
                return false;
            }

            event_register_id addOne(std::unordered_map<uint64_t, function>& map, function func) {
                std::uniform_int_distribution<uint64_t> dis;
                event_register_id id;
                do {
                    id.id = dis(gen);
                } while (map.find(id.id) != map.end());
                map[id.id] = func;
                return id;
            }

            bool await_call(std::unordered_map<uint64_t, function>& map, const T& args) const {
                std::list<std::shared_ptr<fast_task::task>> tasks;
                bool result = false;
                for (auto& [id, func] : map) {
                    auto task = std::make_shared<fast_task::task>(
                        [func, &args, &result]() {
                            if (func(args))
                                result = true;
                        }
                    );
                    tasks.push_back(task);
                    fast_task::task::start(task);
                }
                fast_task::task::await_multiple(tasks, true, true);
                return result;
            }

            bool sync_call(std::unordered_map<uint64_t, function>& map, const T& args) const {
                for (auto& [id, func] : map)
                    if (func(args))
                        return true;

                return false;
            }
        };
    }
}

#endif /* SRC_BASE_OBJECTS_EVENT */
