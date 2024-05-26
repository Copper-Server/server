#ifndef SRC_UTIL_TASK_MANAGEMENT
#define SRC_UTIL_TASK_MANAGEMENT
#include "../library/fast_task.hpp"

struct Task {
    static Task start(const std::function<void()>& fn) {
        fast_task::task::start(std::make_shared<fast_task::task>(fn));
    }
};

template <class T>
class Future {
    fast_task::task task;
    fast_task::task_mutex task_mt;
    fast_task::task_condition_variable task_cv;
    T result;
    bool _is_ready = false;

public:
    static std::shared_ptr<Future> start(const std::function<T()>& fn) {
        std::shared_ptr<Future> future = std::make_shared<Future>();
        fast_task::task::start(std::make_shared<fast_task::task>(
            [fn, future]() {
                try {
                    future->result = fn();
                } catch (...) {
                }
                std::lock_guard guard(task_mt);
                future->_is_ready = true;
            }
        ));
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
        while (!_is_ready) {
            std::unique_lock<std::mutex> lock(lock);
            task_cv.wait(lock);
        }
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

    bool is_ready() const {
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
class Future<void> {
    fast_task::task task;
    fast_task::task_mutex task_mt;
    fast_task::task_condition_variable task_cv;
    bool _is_ready = false;

public:
    static std::shared_ptr<Future> start(const std::function<void()>& fn) {
        std::shared_ptr<Future> future = std::make_shared<Future>();
        fast_task::task::start(std::make_shared<fast_task::task>(
            [fn, future]() {
                try {
                    fn();
                } catch (...) {
                }
                future->_is_ready = true;
            }
        ));
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

template <class T>
auto futureForEach(T& container, const std::function<void(const typename T::value_type&)>& fn) {
    if (container.empty())
        return Future<void>::make_ready();
    std::vector<Future<void>> futures;
    futures.reserve(container.size());
    for (auto& item : container) {
        futures.push_back(Future<void>::start([item, fn]() { fn(item); }));
    }
    return Future<void>::start([fut = std::move(futures)] {
        for (auto& future : fut)
            future.get();
    });
}

template <class T>
auto futureMoveForEach(T& container, const std::function<void(typename T::value_type&&)>& fn) {
    if (container.empty())
        return Future<void>::make_ready();
    std::vector<Future<void>> futures;
    futures.reserve(container.size());
    for (auto&& item : container) {
        futures.push_back(Future<void>::start([it = std::move(item), fn]() {
            fn(it);
        }));
    }
    return Future<void>::start([fut = std::move(futures)] {
        for (auto& future : fut)
            future.get();
    });
}

template <class T>
using FuturePtr = std::shared_ptr<Future<T>>;

template <class T>
auto make_ready_future(const T& value) {
    return Future<std::remove_reference_t<std::remove_cv_t<T>>>::make_ready(value);
}

template <class T>
auto make_ready_future(T&& value) {
    return Future<std::remove_reference_t<std::remove_cv_t<T>>>::make_ready(std::move(value));
}


#endif /* SRC_UTIL_TASK_MANAGEMENT */
