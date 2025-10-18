#ifndef SRC_API_ECS_ENTITY_CONSTRUCTION
#define SRC_API_ECS_ENTITY_CONSTRUCTION
#include <src/api/ecs.hpp>

namespace copper_server::api::ecs {
    struct entity_construction {
        template <class component>
        [[nodiscard]] component& get() {
            auto id = detail::get_component_id<component>();

            if (auto it = components.find(id); it != components.end())
                return *static_cast<component*>(it->second);
            else {
                auto res = std::make_unique<component>();
                components[id] = res.get();
                return *res.release();
            }
        }

        template <class component>
        void set(component&& move) {
            auto id = detail::get_component_id<component>();

            if (auto it = components.find(id); it != components.end()){
                *static_cast<component*>(it->second) = std::move(move);
            }
            else {
                auto res = std::make_unique<component>(std::move(move));
                components[id] = res.release();
            }
        }

        template <class component>
        void set() {
            auto id = detail::get_component_id<component>();

            component* res_;
            if (auto it = components.find(id); it != components.end())
                res_ = static_cast<component*>(it->second);
            else {
                auto res = std::make_unique<component>();
                components[id] = res.get();
                res_ = res.release();
            }
            *res_ = component();
        }

        template <class component>
        void remove() {
            if (auto it = components.find(id); it != components.end()){
                delete it->second;
                components.erase(it);
            }
        }

        template <class component>
        [[nodiscard]] bool has() const {
            return components.contains(detail::get_component_id<component>());
        }

        entity create_and_wait(std::optional<int32_t> world_id = std::nullopt) && {
            auto req = std::make_unique<detail::components_holder>();
            req->components_reference.reserve(components.size());
            for (auto& [id, ptr] : components)
                req->components_reference.emplace_back(id, ptr);

            return detail::create_entity(world_id, std::move(req))->take();
        }

        entity create_and_wait(const entity_recipe& base_recipe, std::optional<int32_t> world_id = std::nullopt) && {
            auto req = std::make_unique<detail::components_holder>();
            req->components_reference.reserve(components.size());
            for (auto& [id, ptr] : components)
                req->components_reference.emplace_back(id, ptr);

            return detail::create_entity(world_id, base_recipe, std::move(req))->take();
        }

        entity_construction() = default;
        ~entity_construction() {
            for (auto& [id, ptr] : components) {
                auto& info = detail::component_info_registry.at(id);
                info.destroy(ptr);
            }
        }

    private:
        std::unordered_map<component_id, void*> components;
    };
}

#endif /* SRC_API_ECS_ENTITY_CONSTRUCTION */
