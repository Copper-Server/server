#ifndef SRC_UTIL_TASK_MANAGEMENT
#define SRC_UTIL_TASK_MANAGEMENT
#include <library/fast_task.hpp>
#include <library/list_array.hpp>

struct Task {
    static void start(const std::function<void()>& fn) {
        fast_task::task::start(std::make_shared<fast_task::task>(fn));
    }
};

template <class T>
struct Future {
    fast_task::task_mutex task_mt;
    fast_task::task_condition_variable task_cv;
    T result;
    bool _is_ready = false;


    static std::shared_ptr<Future> start(const std::function<T()>& fn) {
        std::shared_ptr<Future> future = std::make_shared<Future>();
        fast_task::task::start(std::make_shared<fast_task::task>([fn, future]() {
            try {
                future->result = fn();
            } catch (...) {
            }
            std::lock_guard guard(future->task_mt);
            future->_is_ready = true;
            future->task_cv.notify_all();
        }));
        return future;
    }

    static std::shared_ptr<Future> make_ready(const T& value) {
        std::shared_ptr<Future> future = std::make_shared<Future>();
        future->result = value;
        future->_is_ready = true;
        return future;
    }

    static std::shared_ptr<Future> make_ready(T&& value) {
        std::shared_ptr<Future> future = std::make_shared<Future>();
        future->result = std::move(value);
        future->_is_ready = true;
        return future;
    }

    T get() {
        fast_task::mutex_unify um(task_mt);
        std::unique_lock lock(um);
        while (!_is_ready)
            task_cv.wait(lock);
        return result;
    }

    void when_ready(const std::function<void(T)>& fn) {
        fast_task::mutex_unify um(task_mt);
        std::unique_lock lock(um);
        if (_is_ready) {
            fn(result);
        } else {
            task_cv.callback(
                lock,
                std::make_shared<fast_task::task>(
                    [this, fn]() {
                        fn(get());
                    }
                )
            );
        }
    }

    bool is_ready()  {
        std::unique_lock lock(task_mt);
        return _is_ready;
    }

    void wait() {
        fast_task::mutex_unify um(task_mt);
        std::unique_lock lock(um);
        while (!_is_ready) {
            task_cv.wait(lock);
        }
    }

    bool wait_for(std::chrono::milliseconds ms) {
        return wait_until(std::chrono::high_resolution_clock::now() + ms);
    }

    bool wait_until(std::chrono::time_point<std::chrono::high_resolution_clock> time) {
        fast_task::mutex_unify um(task_mt);
        std::unique_lock lock(um);
        while (!_is_ready)
            if (!task_cv.wait_until(lock, time))
                return false;
        return true;
    }
};

template <>
struct Future<void> {
    fast_task::task_mutex task_mt;
    fast_task::task_condition_variable task_cv;
    bool _is_ready = false;

    static std::shared_ptr<Future> start(const std::function<void()>& fn) {
        std::shared_ptr<Future> future = std::make_shared<Future>();
        fast_task::task::start(std::make_shared<fast_task::task>([fn, future]() {
            try {
                fn();
            } catch (...) {
            }
            std::lock_guard guard(future->task_mt);
            future->_is_ready = true;
            future->task_cv.notify_all();
        }));
        return future;
    }

    static std::shared_ptr<Future> make_ready() {
        std::shared_ptr<Future> future = std::make_shared<Future>();
        future->_is_ready = true;
        return future;
    }

    void get() {
        fast_task::mutex_unify um(task_mt);
        std::unique_lock lock(um);
        while (!_is_ready) {
            task_cv.wait(lock);
        }
    }

    void when_ready(const std::function<void()>& fn) {
        fast_task::mutex_unify um(task_mt);
        std::unique_lock lock(um);
        if (_is_ready) {
            fn();
        } else {
            task_cv.callback(
                lock,
                std::make_shared<fast_task::task>(fn)
            );
        }
    }

    bool is_ready() const {
        return _is_ready;
    }

    void wait() {
        fast_task::mutex_unify um(task_mt);
        std::unique_lock lock(um);
        while (!_is_ready)
            task_cv.wait(lock);
    }

    bool wait_for(std::chrono::milliseconds ms) {
        return wait_until(std::chrono::high_resolution_clock::now() + ms);
    }

    bool wait_until(std::chrono::time_point<std::chrono::high_resolution_clock> time) {
        fast_task::mutex_unify um(task_mt);
        std::unique_lock lock(um);
        while (!_is_ready)
            if (!task_cv.wait_until(lock, time))
                return false;
        return true;
    }
};

template <class T>
using FuturePtr = std::shared_ptr<Future<T>>;

namespace future {
    template <class T>
    FuturePtr<void> forEach(T& container, const std::function<void(const typename T::value_type&)>& fn) {
        if (container.empty())
            return Future<void>::make_ready();
        std::vector<FuturePtr<void>> futures;
        futures.reserve(container.size());
        for (auto& item : container)
            futures.push_back(Future<void>::start([item, fn]() { fn(item); }));

        return Future<void>::start([fut = std::move(futures)] {
            for (auto& future : fut)
                future->wait();
        });
    }

    template <class T>
    FuturePtr<void> forEachMove(T& container, const std::function<void(typename T::value_type&&)>& fn) {
        if (container.empty())
            return Future<void>::make_ready();
        std::vector<FuturePtr<void>> futures;
        futures.reserve(container.size());
        for (auto&& item : container)
            futures.push_back(Future<void>::start([it = std::move(item), fn]() mutable {
                fn(std::move(it));
            }));

        return Future<void>::start([fut = std::move(futures)] {
            for (auto& future : fut)
                future->wait();
        });
    }

    template <class Ret, class Accept>
    FuturePtr<Ret> chain(const FuturePtr<Accept>& future, const std::function<Ret()>& fn) {
        FuturePtr<Ret> new_future = std::make_shared<Future<Ret>>();
        future->when_ready([future, fn, new_future]() mutable {
            try {
                if constexpr (std::is_same_v<Accept, void>)
                    if constexpr (std::is_same_v<Ret, void>)
                        fn();
                    else
                        new_future->result = fn();
                else {
                    if constexpr (std::is_same_v<Ret, void>)
                        fn(future->get());
                    else
                        new_future->result = fn(future->get());
                }
            } catch (...) {
            };
            std::lock_guard guard(new_future->task_mt);
            new_future->_is_ready = true;
            new_future->task_cv.notify_all();
        });
        return new_future;
    }

    template <class Ret, class Accept>
    FuturePtr<Ret> chain(FuturePtr<Accept>&& future, const std::function<Ret()>& fn) {
        FuturePtr<Ret> new_future = std::make_shared<Future<Ret>>();
        future->when_ready([future = std::move(future), fn, new_future]() mutable {
            try {
                if constexpr (std::is_same_v<Accept, void>)
                    if constexpr (std::is_same_v<Ret, void>)
                        fn();
                    else
                        new_future->result = fn();
                else {
                    if constexpr (std::is_same_v<Ret, void>)
                        fn(future->get());
                    else
                        new_future->result = fn(future->get());
                }
            } catch (...) {
            };
            std::lock_guard guard(new_future->task_mt);
            new_future->_is_ready = true;
            new_future->task_cv.notify_all();
        });
        return new_future;
    }

    static FuturePtr<void> combineAll(const list_array<FuturePtr<void>>& futures) {
        if (futures.empty())
            return Future<void>::make_ready();
        std::vector<FuturePtr<void>> fut = {futures.begin(), futures.end()};
        return Future<void>::start([fut = std::move(fut)] {
            for (auto& future : fut)
                future->wait();
        });
    }

    static FuturePtr<void> combineAll(list_array<FuturePtr<void>>&& futures) {
        if (futures.empty())
            return Future<void>::make_ready();
        return Future<void>::start([fut = std::move(futures)] {
            for (auto& future : fut)
                future->wait();
        });
    }

    template <class... Futures>
    void each(Futures&&... futures) {
        std::vector<FuturePtr<void>> fut = {futures...};
        for (auto& future : fut)
            future->wait();
    }

    template <class... Futures>
    void eachIn(Futures&&... futures) {
        std::vector<FuturePtr<void>> fut = {futures...};
        for (auto& future : fut)
            future->wait();
    }
}

template <class T>
auto make_ready_future(T&& value) {
    return Future<std::remove_reference_t<std::remove_cv_t<T>>>::make_ready(std::forward<T>(value));
}


#endif /* SRC_UTIL_TASK_MANAGEMENT */
