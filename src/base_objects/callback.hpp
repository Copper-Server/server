#ifndef SRC_BASE_OBJECTS_CALLBACK
#define SRC_BASE_OBJECTS_CALLBACK
#include <functional>
#include <library/list_array.hpp>
#include <unordered_map>

namespace copper_server {
    namespace base_objects {

        struct callback_register_id {
            size_t id;
        };

        template <class ReturnType, class... Args>
        struct callback {
            using function = std::function<ReturnType(Args...)>;

            event_register_id operator+=(function func) {
                return join(func);
            }

            void operator-=(event_register_id id) {
                leave(id);
            }

            list_array<ReturnType> operator()(const T& args) {
                list_array<ReturnType> result;
                result.reserve(callbacks.size());
                for (auto& [id, func] : callbacks)
                    result.push_back(func(args));
                return result;
            }

        private:
            event_register_id join(function func) {
                size_t id = id_gen++;
                callbacks[id] = func;
                return {id};
            }

            void leave(event_register_id id) {
                callbacks.erase(id.id);
            }

            size_t id_gen = 0;
            std::unordered_map<uint64_t, function> callbacks;
        };
    }
}


#endif /* SRC_BASE_OBJECTS_CALLBACK */
