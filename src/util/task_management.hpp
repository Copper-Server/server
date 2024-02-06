#ifndef SRC_UTIL_TASK_MANAGEMENT
#define SRC_UTIL_TASK_MANAGEMENT
#include "library/fast_task.hpp"

struct Task {
    static void start(const std::function<void()>& fn) {
        fast_task::task::start(std::make_shared<fast_task::task>(fn));
    }
};

#endif /* SRC_UTIL_TASK_MANAGEMENT */
