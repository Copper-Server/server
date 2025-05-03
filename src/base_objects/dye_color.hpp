#ifndef SRC_BASE_OBJECTS_DYE_COLOR
#define SRC_BASE_OBJECTS_DYE_COLOR
#include <string>

namespace copper_server::base_objects{
    struct dye_color {
        enum __internal : uint8_t {
            white = 0,
            orange = 1,
            magenta = 2,
            light_blue = 3,
            yellow = 4,
            lime = 5,
            pink = 6,
            gray = 7,
            light_gray = 8,
            cyan = 9,
            purple = 10,
            blue = 11,
            brown = 12,
            green = 13,
            red = 14,
            black = 15,
        } value;

        dye_color()
            : value(white) {}

        dye_color(__internal value)
            : value(value) {}

        dye_color(const dye_color& value)
            : value(value.value) {}

        explicit dye_color(uint8_t value)
            : value((__internal)value) {}

        std::string to_string() const;
        static dye_color from_string(const std::string&);

        operator __internal() const {
            return value;
        }

        auto operator<=>(const dye_color& other) const = default;
    };
}

#endif /* SRC_BASE_OBJECTS_DYE_COLOR */
