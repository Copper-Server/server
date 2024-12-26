#ifndef SRC_BASE_OBJECTS_EVENTS_PRIORITY
#define SRC_BASE_OBJECTS_EVENTS_PRIORITY

namespace copper_server::base_objects::events {
    enum class priority {
        heigh,
        upper_avg,
        avg,
        lower_avg,
        low
    };
}

#endif /* SRC_BASE_OBJECTS_EVENTS_PRIORITY */
