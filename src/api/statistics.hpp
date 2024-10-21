#ifndef SRC_API_STATISTICS
#define SRC_API_STATISTICS
#include "../base_objects/event.hpp"
#include "../base_objects/shared_client_data.hpp"
#include "../base_objects/shared_string.hpp"

namespace crafted_craft::api::statistics {
            struct statistic_event {
                base_objects::client_data_holder target;
                base_objects::shared_string trigger_source;
            };

            namespace minecraft {
                extern base_objects::event<statistic_event> custom;
                extern base_objects::event<statistic_event> mined;
                extern base_objects::event<statistic_event> broken;
                extern base_objects::event<statistic_event> crafted;
                extern base_objects::event<statistic_event> used;
                extern base_objects::event<statistic_event> picked_up;
                extern base_objects::event<statistic_event> dropped;
                extern base_objects::event<statistic_event> killed;
                extern base_objects::event<statistic_event> killed_by;
            }

            using namespace minecraft;
        }


#endif /* SRC_API_STATISTICS */
