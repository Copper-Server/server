#include "../base_objects/event.hpp"
#include "../base_objects/server_configuaration.hpp"

namespace crafted_craft::api::configuration {
    base_objects::ServerConfiguration config;
    bool loaded = false;


    extern base_objects::event<void> updated;

    void load(bool fill_default_values) {
        config.load(std::filesystem::current_path(), fill_default_values);
        updated();
    }


    base_objects::ServerConfiguration& get() {
        if (!loaded)
            load(true);
        return config;
    }

    void set_item(const std::string& config_item_path, const std::string& value) {
        config.set(std::filesystem::current_path(), config_item_path, value);
        updated();
    }

    std::string get_item(const std::string& config_item_path) {
        if (!loaded)
            load(true);
        return config.get(config_item_path);
    }
}