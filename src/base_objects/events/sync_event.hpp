/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_EVENTS_SYNC_EVENT
#define SRC_BASE_OBJECTS_EVENTS_SYNC_EVENT
#include <functional>
#include <random>
#include <src/base_objects/events/base_event.hpp>
#include <src/base_objects/events/priority.hpp>
#include <src/util/templates.hpp>
#include <unordered_map>

namespace copper_server::base_objects::events {
    template <class... Args>
    struct sync_event : public base_event {
        using function = std::function<bool(Args...)>;

        event_register_id operator+=(function func) {
            return join(func);
        }

        template <class Fn>
        event_register_id operator+=(Fn&& func) {
            return join(util::convertible_function<bool, Args...>::make_proxy_from_callable(std::forward<Fn>(func)));
        }

        bool operator()(Args... args) {
            return notify(std::forward<Args>(args)...);
        }

        event_register_id join(function func, priority priority = priority::avg) {
            switch (priority) {
            case priority::high:
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

        template <class Fn>
        event_register_id join(Fn&& func, priority priority = priority::avg) {
            return join(util::convertible_function<bool, Args...>::make_proxy_from_callable(std::forward<Fn>(func)), priority);
        }

        bool leave(event_register_id func, priority priority = priority::avg) {
            switch (priority) {
            case priority::high:
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

        bool leave(event_register_id func, priority priority, bool) override {
            return leave(func, priority);
        }

        bool notify(Args... args) {
            if (sync_call(heigh_priority, std::forward<Args>(args)...))
                return true;
            if (sync_call(upper_avg_priority, std::forward<Args>(args)...))
                return true;
            if (sync_call(avg_priority, std::forward<Args>(args)...))
                return true;
            if (sync_call(lower_avg_priority, std::forward<Args>(args)...))
                return true;
            if (sync_call(low_priority, std::forward<Args>(args)...))
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

        bool empty() const {
            return heigh_priority.empty()
                   && upper_avg_priority.empty()
                   && avg_priority.empty()
                   && lower_avg_priority.empty()
                   && low_priority.empty();
        }

    private:
        static inline std::random_device rd;
        std::mt19937 gen;
        std::unordered_map<uint64_t, function> heigh_priority;
        std::unordered_map<uint64_t, function> upper_avg_priority;
        std::unordered_map<uint64_t, function> avg_priority;
        std::unordered_map<uint64_t, function> lower_avg_priority;
        std::unordered_map<uint64_t, function> low_priority;

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

        bool sync_call(std::unordered_map<uint64_t, function>& map, Args... args) {
            for (auto& [_, func] : map)
                if (func(std::forward<Args>(args)...))
                    return true;
            return false;
        }
    };

    template <typename... Args>
    struct sync_event_no_cancel : public base_event {
        using function = std::function<void(Args...)>;

        event_register_id operator+=(function func) {
            return join(func);
        }

        template <class Fn>
        event_register_id operator+=(Fn&& func) {
            return join(util::convertible_function<void, Args...>::make_proxy_from_callable(std::forward<Fn>(func)));
        }

        void operator()(Args... args) {
            notify(std::forward<Args>(args)...);
        }

        event_register_id join(function func, priority _ = priority::avg) {
            std::uniform_int_distribution<uint64_t> dis;
            event_register_id id;
            do {
                id.id = dis(gen);
            } while (regs.find(id.id) != regs.end());
            regs[id.id] = func;
            return id;
        }

        template <class Fn>
        event_register_id join(Fn&& func, priority _ = priority::avg) {
            return join(util::convertible_function<void, Args...>::make_proxy_from_callable(std::forward<Fn>(func)));
        }

        bool leave(event_register_id func) {
            return (bool)regs.erase(func.id);
        }

        bool leave(event_register_id func, priority, bool) override {
            return leave(func);
        }

        void notify(Args... args) {
            for (auto& [_, func] : regs)
                func(std::forward<Args>(args)...);
        }

        void clear() {
            regs.clear();
        }

        sync_event_no_cancel()
            : gen(rd()) {}

        bool empty() const {
            return regs.empty();
        }

    private:
        static inline std::random_device rd;
        std::mt19937 gen;
        std::unordered_map<uint64_t, function> regs;
    };

    template <typename... Args>
    struct sync_event_single : public base_event {
        using function = std::function<void(Args...)>;

        event_register_id operator+=(function func) {
            return join(func);
        }

        template <class Fn>
        event_register_id operator+=(Fn&& func) {
            return join(util::convertible_function<void, Args...>::make_proxy_from_callable(std::forward<Fn>(func)));
        }

        void operator()(Args... args) {
            notify(std::forward<Args>(args)...);
        }

        event_register_id join(function func, priority _ = priority::avg) {
            if (fun)
                throw std::runtime_error("The event already registered.");
            std::uniform_int_distribution<uint64_t> dis;
            event_register_id id{.id = dis(gen)};
            fun = func;
            curr_id = id.id;
            return id;
        }

        template <class Fn>
        event_register_id join(Fn&& func, priority _ = priority::avg) {
            return join(util::convertible_function<void, Args...>::make_proxy_from_callable(std::forward<Fn>(func)));
        }

        bool leave(event_register_id func) {
            if (fun)
                if (curr_id == func.id) {
                    fun = nullptr;
                    return true;
                }

            return false;
        }

        bool leave(event_register_id func, priority, bool) override {
            return leave(func);
        }

        void notify(Args... args) {
            fun(std::forward<Args>(args)...);
        }

        void clear() {
            fun = nullptr;
        }

        sync_event_single()
            : gen(rd()) {}

        bool empty() const {
            return !fun;
        }

    private:
        static inline std::random_device rd;
        function fun;
        std::mt19937 gen;
        uint64_t curr_id = 0;
    };
}
#endif /* SRC_BASE_OBJECTS_EVENTS_SYNC_EVENT */
