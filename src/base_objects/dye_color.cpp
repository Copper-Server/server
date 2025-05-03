#include <src/base_objects/dye_color.hpp>
namespace copper_server::base_objects {
    std::string dye_color::to_string() const {
        switch (value) {
        case white:
            return "white";
        case orange:
            return "orange";
        case magenta:
            return "magenta";
        case light_blue:
            return "light_blue";
        case yellow:
            return "yellow";
        case lime:
            return "lime";
        case pink:
            return "pink";
        case gray:
            return "gray";
        case light_gray:
            return "light_gray";
        case cyan:
            return "cyan";
        case purple:
            return "purple";
        case blue:
            return "blue";
        case brown:
            return "brown";
        case green:
            return "green";
        case red:
            return "red";
        case black:
            return "black";
        default:
            return "undefined";
        }
    }

    dye_color dye_color::from_string(const std::string& color) {
        if (color == "white")
            return white;
        else if (color == "orange")
            return orange;
        else if (color == "magenta")
            return magenta;
        else if (color == "light_blue")
            return light_blue;
        else if (color == "yellow")
            return yellow;
        else if (color == "lime")
            return lime;
        else if (color == "pink")
            return pink;
        else if (color == "gray")
            return gray;
        else if (color == "light_gray")
            return light_gray;
        else if (color == "cyan")
            return cyan;
        else if (color == "purple")
            return purple;
        else if (color == "blue")
            return blue;
        else if (color == "brown")
            return brown;
        else if (color == "green")
            return green;
        else if (color == "red")
            return red;
        else if (color == "black")
            return black;
        else
            return white;
    }
}