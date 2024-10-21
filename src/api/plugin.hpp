#ifndef SRC_API_PLUGIN
#define SRC_API_PLUGIN

namespace crafted_craft::api::plugin {
    void load(const std::string& name);
    void unload(const std::string& name);
    void reload(const std::string& name);
    void reload_all();
    void iterate(std::function<void(const std::string& name)> callback);
    void get(const std::string& name, std::function<void(const std::string& name)> callback);
    void resolve_id(const std::string& name, std::function<void(const std::string& name)> callback);
    void pre_load(const std::string& name, std::function<void(const std::string& name)> initialization = nullptr);
    void create(const std::string& name);
    void create(const std::string& name, std::function<void(const std::string& name)> callback);
}

#endif /* SRC_API_PLUGIN */
