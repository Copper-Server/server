#ifndef SRC_API_CONFIGURATION
#define SRC_API_CONFIGURATION
#include <src/base_objects/event.hpp>
#include <src/base_objects/server_configuaration.hpp>

namespace copper_server::api::configuration {
    base_objects::ServerConfiguration& get();

    void load(bool fill_default_values = true);

    void set_item(const std::string& config_item_path, const std::string& value); //accepts json
    std::string get_item(const std::string& config_item_path);                    //returns json

    extern base_objects::event<void> updated;
}

#endif /* SRC_API_CONFIGURATION */
