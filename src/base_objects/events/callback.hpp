#ifndef SRC_BASE_OBJECTS_EVENTS_CALLBACK
#define SRC_BASE_OBJECTS_EVENTS_CALLBACK
#include <functional>
#include <library/fast_task.hpp>
#include <library/list_array.hpp>
#include <src/base_objects/events/base_event.hpp>
#include <unordered_map>

namespace copper_server::base_objects {
    template <class ReturnType, class... Args>
    struct callback : public base_event {
        using function = std::function<ReturnType(Args...)>;

        event_register_id operator+=(function func) {
            return join(func);
        }

        void operator-=(event_register_id id) {
            leave(id);
        }

        event_register_id join(function func) {
            std::lock_guard<fast_task::task_mutex> lock(mutex);
            size_t id = id_gen++;
            callbacks[id] = func;
            return {id};
        }

        void leave(event_register_id id) {
            std::lock_guard<fast_task::task_mutex> lock(mutex);
            callbacks.erase(id.id);
        }

        bool leave(event_register_id func, priority _ignored0 = priority::avg, bool _ignored1 = false) override {
            return leave(func);
        }

        list_array<ReturnType> operator()(const T& args) {
            std::unique_lock<fast_task::task_mutex> lock(mutex);
            list_array<ReturnType> result;
            result.reserve(callbacks.size());
            list_array<ReturnType> result;
            list_array<function> fns;
            result.reserve(callbacks.size());
            fns.reserve(callbacks.size());
            for (auto& [id, func] : map)
                fns.push_back(func);
            lock.unlock();
            for (auto& [id, func] : callbacks)
                result.push_back(func(args));
            return result;
        }

    private:
        fast_task::task_mutex mutex;
        std::unordered_map<uint64_t, function> callbacks;
        size_t id_gen = 0;
    };
}


#endif /* SRC_BASE_OBJECTS_EVENTS_CALLBACK */
