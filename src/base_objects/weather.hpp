#ifndef SRC_BASE_OBJECTS_WEATHER
#define SRC_BASE_OBJECTS_WEATHER
#include <string>
#include <unordered_map>
namespace copper_server::base_objects {
    struct weather {
        enum _value {
            clear,
            rain,
            thunder
        };

        weather(_value value) : value(value) {}

        weather(const weather& value) : value(value.value) {}

        static weather from_string(const std::string& view) {
            static std::unordered_map<std::string, _value> map{
                {"clear", clear},
                {"rain", rain},
                {"thunder", thunder},
            };
            return map.at(view);
        }

        std::string to_string() {
            std::unordered_map<_value, std::string> map{
                {clear, "clear"},
                {rain, "rain"},
                {thunder, "thunder"},
            };
            return map.at(value);
        }

    private:
        _value value;
    };
}
#endif /* SRC_BASE_OBJECTS_WEATHER */
