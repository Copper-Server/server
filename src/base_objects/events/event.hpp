#ifndef SRC_BASE_OBJECTS_EVENTS_EVENT
#define SRC_BASE_OBJECTS_EVENTS_EVENT
#include <functional>
#include <library/fast_task.hpp>
#include <library/list_array.hpp>
#include <random>
#include <src/base_objects/events/base_event.hpp>
#include <src/base_objects/events/priority.hpp>
#include <unordered_map>

namespace copper_server::base_objects::events {

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
            return join(priority::avg, false, func);
        }

        event_register_id join(priority priority, function func) {
            return join(priority, false, func);
        }

        event_register_id join(priority priority, bool async_mode, function func) {
            std::lock_guard<fast_task::task_mutex> lock(mutex);
            if (async_mode) {
                switch (priority) {
                case priority::heigh:
                    return addOne(async_heigh_priority, func);
                case priority::upper_avg:
                    return addOne(async_upper_avg_priority, func);
                case priority::avg:
                    return addOne(async_avg_priority, func);
                case priority::lower_avg:
                    return addOne(async_lower_avg_priority, func);
                case priority::low:
                    return addOne(async_low_priority, func);
                }
            } else {
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
            }
            throw std::runtime_error("Invalid priority");
        }

        bool leave(event_register_id func, priority priority = priority::avg, bool async_mode = false) override {
            std::lock_guard<fast_task::task_mutex> lock(mutex);
            if (async_mode) {
                switch (priority) {
                case priority::heigh:
                    return removeOne(async_heigh_priority, func);
                case priority::upper_avg:
                    return removeOne(async_upper_avg_priority, func);
                case priority::avg:
                    return removeOne(async_avg_priority, func);
                case priority::lower_avg:
                    return removeOne(async_lower_avg_priority, func);
                case priority::low:
                    return removeOne(async_low_priority, func);
                }
            } else {
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
            }
            return false;
        }

        bool await_notify(const T& args) {
            std::unique_lock<fast_task::task_mutex> lock(mutex);
            if (can_skip())
                return false;
            if (await_call(async_heigh_priority, lock, args))
                return true;
            if (await_call(async_upper_avg_priority, lock, args))
                return true;
            if (await_call(async_avg_priority, lock, args))
                return true;
            if (await_call(async_lower_avg_priority, lock, args))
                return true;
            if (await_call(async_low_priority, lock, args))
                return true;
            if (sync_call(heigh_priority, lock, args))
                return true;
            if (sync_call(upper_avg_priority, lock, args))
                return true;
            if (sync_call(avg_priority, lock, args))
                return true;
            if (sync_call(lower_avg_priority, lock, args))
                return true;
            if (sync_call(low_priority, lock, args))
                return true;
            return false;
        }

        bool notify(const T& args) {
            std::unique_lock<fast_task::task_mutex> lock(mutex);
            if (can_skip())
                return false;
            if (await_call(async_heigh_priority, lock, args))
                return true;
            if (await_call(async_upper_avg_priority, lock, args))
                return true;
            if (await_call(async_avg_priority, lock, args))
                return true;
            if (await_call(async_lower_avg_priority, lock, args))
                return true;
            if (await_call(async_low_priority, lock, args))
                return true;
            if (sync_call(heigh_priority, lock, args))
                return true;
            if (sync_call(upper_avg_priority, lock, args))
                return true;
            if (sync_call(avg_priority, lock, args))
                return true;
            if (sync_call(lower_avg_priority, lock, args))
                return true;
            if (sync_call(low_priority, lock, args))
                return true;
            return false;
        }

        bool sync_notify(const T& args) {
            std::unique_lock<fast_task::task_mutex> lock(mutex);
            if (can_skip())
                return false;
            if (sync_call(async_heigh_priority, lock, args))
                return true;
            if (sync_call(async_upper_avg_priority, lock, args))
                return true;
            if (sync_call(async_avg_priority, lock, args))
                return true;
            if (sync_call(async_lower_avg_priority, lock, args))
                return true;
            if (sync_call(async_low_priority, lock, args))
                return true;

            if (sync_call(heigh_priority, lock, args))
                return true;
            if (sync_call(upper_avg_priority, lock, args))
                return true;
            if (sync_call(avg_priority, lock, args))
                return true;
            if (sync_call(lower_avg_priority, lock, args))
                return true;
            if (sync_call(low_priority, lock, args))
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
            return heigh_priority.empty() && upper_avg_priority.empty() && avg_priority.empty() && lower_avg_priority.empty() && low_priority.empty() && async_heigh_priority.empty() && async_upper_avg_priority.empty() && async_avg_priority.empty() && async_lower_avg_priority.empty() && async_low_priority.empty();
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

        bool await_call(std::unordered_map<uint64_t, function>& map, std::unique_lock<fast_task::task_mutex>& lock, const T& args) const {
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
            lock.unlock();
            fast_task::task::await_multiple(tasks, true, true);
            lock.lock();
            return result;
        }

        bool sync_call(std::unordered_map<uint64_t, function>& map, std::unique_lock<fast_task::task_mutex>& lock, const T& args) const {
            list_array<function> fns;
            fns.reserve(map.size());
            for (auto& [id, func] : map)
                fns.push_back(func);
            lock.unlock();
            for (auto& [id, func] : map)
                if (func(args))
                    return true;
            lock.lock();
            return false;
        }
    };

    template <>
    class event<void> : public base_event {
    public:
        using function = std::function<bool()>;

        event_register_id operator+=(function func) {
            return join(func);
        }

        bool operator()() {
            return notify();
        }

        event_register_id join(function func) {
            return join(priority::avg, false, func);
        }

        event_register_id join(priority priority, function func) {
            return join(priority, false, func);
        }

        event_register_id join(priority priority, bool async_mode, function func) {
            std::lock_guard<fast_task::task_mutex> lock(mutex);
            if (async_mode) {
                switch (priority) {
                case priority::heigh:
                    return addOne(async_heigh_priority, func);
                case priority::upper_avg:
                    return addOne(async_upper_avg_priority, func);
                case priority::avg:
                    return addOne(async_avg_priority, func);
                case priority::lower_avg:
                    return addOne(async_lower_avg_priority, func);
                case priority::low:
                    return addOne(async_low_priority, func);
                }
            } else {
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
            }
            throw std::runtime_error("Invalid priority");
        }

        bool leave(event_register_id func, priority priority = priority::avg, bool async_mode = false) override {
            std::lock_guard<fast_task::task_mutex> lock(mutex);
            if (async_mode) {
                switch (priority) {
                case priority::heigh:
                    return removeOne(async_heigh_priority, func);
                case priority::upper_avg:
                    return removeOne(async_upper_avg_priority, func);
                case priority::avg:
                    return removeOne(async_avg_priority, func);
                case priority::lower_avg:
                    return removeOne(async_lower_avg_priority, func);
                case priority::low:
                    return removeOne(async_low_priority, func);
                }
            } else {
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
            }
            return false;
        }

        bool await_notify() {
            std::unique_lock<fast_task::task_mutex> lock(mutex);
            if (can_skip())
                return false;
            if (await_call(async_heigh_priority, lock))
                return true;
            if (await_call(async_upper_avg_priority, lock))
                return true;
            if (await_call(async_avg_priority, lock))
                return true;
            if (await_call(async_lower_avg_priority, lock))
                return true;
            if (await_call(async_low_priority, lock))
                return true;
            if (sync_call(heigh_priority, lock))
                return true;
            if (sync_call(upper_avg_priority, lock))
                return true;
            if (sync_call(avg_priority, lock))
                return true;
            if (sync_call(lower_avg_priority, lock))
                return true;
            if (sync_call(low_priority, lock))
                return true;
            return false;
        }

        bool notify() {
            std::unique_lock<fast_task::task_mutex> lock(mutex);
            if (can_skip())
                return false;
            if (await_call(async_heigh_priority, lock))
                return true;
            if (await_call(async_upper_avg_priority, lock))
                return true;
            if (await_call(async_avg_priority, lock))
                return true;
            if (await_call(async_lower_avg_priority, lock))
                return true;
            if (await_call(async_low_priority, lock))
                return true;
            if (sync_call(heigh_priority, lock))
                return true;
            if (sync_call(upper_avg_priority, lock))
                return true;
            if (sync_call(avg_priority, lock))
                return true;
            if (sync_call(lower_avg_priority, lock))
                return true;
            if (sync_call(low_priority, lock))
                return true;
            return false;
        }

        bool sync_notify() {
            std::unique_lock<fast_task::task_mutex> lock(mutex);
            if (can_skip())
                return false;
            if (sync_call(async_heigh_priority, lock))
                return true;
            if (sync_call(async_upper_avg_priority, lock))
                return true;
            if (sync_call(async_avg_priority, lock))
                return true;
            if (sync_call(async_lower_avg_priority, lock))
                return true;
            if (sync_call(async_low_priority, lock))
                return true;

            if (sync_call(heigh_priority, lock))
                return true;
            if (sync_call(upper_avg_priority, lock))
                return true;
            if (sync_call(avg_priority, lock))
                return true;
            if (sync_call(lower_avg_priority, lock))
                return true;
            if (sync_call(low_priority, lock))
                return true;
            return false;
        }

        std::shared_ptr<fast_task::task> async_notify() {
            std::lock_guard<fast_task::task_mutex> lock(mutex);
            if (can_skip())
                return nullptr;

            auto task = std::make_shared<fast_task::task>(
                [this]() { notify(); }
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
            return heigh_priority.empty() && upper_avg_priority.empty() && avg_priority.empty() && lower_avg_priority.empty() && low_priority.empty() && async_heigh_priority.empty() && async_upper_avg_priority.empty() && async_avg_priority.empty() && async_lower_avg_priority.empty() && async_low_priority.empty();
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

        bool await_call(std::unordered_map<uint64_t, function>& map, std::unique_lock<fast_task::task_mutex>& lock) const {
            std::list<std::shared_ptr<fast_task::task>> tasks;
            bool result = false;
            for (auto& [id, func] : map) {
                auto task = std::make_shared<fast_task::task>(
                    [func, &result]() {
                        if (func())
                            result = true;
                    }
                );
                tasks.push_back(task);
                fast_task::task::start(task);
            }
            lock.unlock();
            fast_task::task::await_multiple(tasks, true, true);
            lock.lock();
            return result;
        }

        bool sync_call(std::unordered_map<uint64_t, function>& map, std::unique_lock<fast_task::task_mutex>& lock) const {
            list_array<function> fns;
            fns.reserve(map.size());
            for (auto& [id, func] : map)
                fns.push_back(func);
            lock.unlock();
            for (auto& [id, func] : map)
                if (func())
                    return true;
            lock.lock();
            return false;
        }
    };
}

#endif /* SRC_BASE_OBJECTS_EVENTS_EVENT */
