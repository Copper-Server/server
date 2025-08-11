/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_TASK_MANAGEMENT
#define SRC_UTIL_TASK_MANAGEMENT
#include <library/fast_task.hpp>
#include <library/fast_task/src/future.hpp>
#include <library/list_array.hpp>

struct Task {
    static void start(const std::function<void()>& fn) {
        fast_task::scheduler::start(std::make_shared<fast_task::task>(fn));
    }
};

template <class T>
using Future = fast_task::future<T>;

template <class T>
using CancelableFuture = fast_task::cancelable_future<T>;

template <class T>
using FuturePtr = fast_task::future_ptr<T>;

template <class T>
using CancelableFuturePtr = fast_task::cancelable_future_ptr<T>;

namespace future {
    template <class T, class FN>
    FuturePtr<void> forEach(T& container, FN&& fn) {
        if (container.empty())
            return Future<void>::make_ready();
        std::vector<CancelableFuturePtr<void>> futures;
        futures.reserve(container.size());
        for (auto& item : container)
            futures.push_back(CancelableFuture<void>::start([item, fn]() { fn(item); }));

        return Future<void>::start([fut = std::move(futures)] {
            try {
                for (auto& future : fut)
                    future->wait();
            } catch (...) {
                for (auto& future : fut)
                    future->cancel();
                throw;
            }
        });
    }

    template <class T, class FN>
    FuturePtr<void> forEachMove(T&& container, FN&& fn) {
        if (container.empty())
            return Future<void>::make_ready();
        std::vector<CancelableFuturePtr<void>> futures;
        futures.reserve(container.size());
        for (auto&& item : container)
            futures.push_back(CancelableFuture<void>::start([it = std::move(item), fn]() mutable {
                fn(std::move(it));
            }));

        return Future<void>::start([fut = std::move(futures)] {
            try {
                for (auto& future : fut)
                    future->wait();
            } catch (...) {
                for (auto& future : fut)
                    future->cancel();
                throw;
            }
        });
    }

    template <class Result, class T, class FN>
    list_array<Result> process(const list_array<T>& container, FN&& fn) {
        if (container.empty())
            return {};

        std::vector<CancelableFuturePtr<Result>> futures;
        futures.reserve(container.size());
        for (auto& item : container)
            futures.push_back(CancelableFuture<Result>::start([item, fn = fn]() mutable { return fn(item); }));

        list_array<Result> res;
        res.reserve(container.size());
        try {
            for (auto& future : futures)
                res.push_back(future->take());
        } catch (...) {
            for (auto& future : futures)
                future->cancel();
            throw;
        }
        return res;
    }

    template <class Ret, class Accept>
    FuturePtr<Ret> chain(const FuturePtr<Accept>& future, const std::function<Ret(Accept)>& fn) {
        FuturePtr<Ret> new_future = std::make_shared<Future<Ret>>();
        future->when_ready([fn, new_future](Accept a) mutable {
            try {
                if constexpr (std::is_same_v<Ret, void>)
                    fn(std::move(a));
                else
                    new_future->result = fn(std::move(a));
            } catch (...) {
            };
            std::lock_guard guard(new_future->task_mt);
            new_future->_is_ready = true;
            new_future->task_cv.notify_all();
        });
        return new_future;
    }

    template <class Ret>
    FuturePtr<Ret> chain(const FuturePtr<void>& future, const std::function<Ret()>& fn) {
        FuturePtr<Ret> new_future = std::make_shared<Future<Ret>>();
        future->when_ready([fn, new_future]() mutable {
            try {
                if constexpr (std::is_same_v<Ret, void>)
                    fn();
                else
                    new_future->result = fn();
            } catch (...) {
            };
            std::lock_guard guard(new_future->task_mt);
            new_future->_is_ready = true;
            new_future->task_cv.notify_all();
        });
        return new_future;
    }

    template <class Ret>
    FuturePtr<list_array<Ret>> accumulate(const list_array<FuturePtr<Ret>>& futures) {
        if (futures.empty())
            return Future<list_array<Ret>>::make_ready({});
        return Future<list_array<Ret>>::start([fut = futures] {
            list_array<Ret> res;
            res.resize(fut.size());
            fut.for_each([&](size_t pos, auto& it) {
                res[pos] = it.take();
            });
            return res;
        });
    }

    template <class Ret>
    FuturePtr<list_array<Ret>> accumulate(list_array<FuturePtr<Ret>>&& futures) {
        if (futures.empty())
            return Future<list_array<Ret>>::make_ready({});
        return Future<list_array<Ret>>::start([fut = std::move(futures)] {
            list_array<Ret> res;
            res.resize(fut.size());
            fut.for_each([&](size_t pos, auto& it) {
                res[pos] = it.take();
            });
            return res;
        });
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
