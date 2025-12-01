/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
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

        template <class component, typename... Args>
        component& emplace(Args&&... args) {
            auto id = detail::get_component_id<component>();
            if (auto it = components.find(id); it != components.end())
                delete static_cast<component*>(it->second);

            auto res = std::make_unique<component>(std::forward<Args>(args)...);
            component* ptr = res.get();
            components[id] = res.release();
            return *ptr;
        }

        template <class component>
        void set(component&& move) {
            auto id = detail::get_component_id<component>();

            if (auto it = components.find(id); it != components.end())
                *static_cast<component*>(it->second) = std::move(move);
            else {
                auto res = std::make_unique<component>(std::move(move));
                components[id] = res.release();
            }
        }

        template <class component>
        void set() {
            emplace<component>();
        }

        template <class component>
        void remove() {
            if (auto it = components.find(id); it != components.end()) {
                delete static_cast<component*>(it->second);
                components.erase(it);
            }
        }

        void remove_by_id(component_id id) {
            if (auto it = components.find(id); it != components.end()) {
                const auto& info = detail::component_info_registry.at(id);
                info.destroy(it->second);
                ::operator delete(it->second);
                components.erase(it);
            }
        }

        void* get_raw_or_create(component_id id) {
            if (auto it = components.find(id); it != components.end())
                return it->second;

            const auto& info = detail::component_info_registry.at(id);
            void* ptr = ::operator new(info.size, std::align_val_t(info.alignment));
            info.construct(ptr);

            components[id] = ptr;
            return ptr;
        }

        template <class component>
        [[nodiscard]] bool has() const {
            return components.contains(detail::get_component_id<component>());
        }

        entity create_and_wait(std::optional<world*> world_opt = std::nullopt) && {
            auto req = std::make_unique<detail::components_holder>();
            req->components_reference.reserve(components.size());
            for (auto& [id, ptr] : components)
                req->components_reference.emplace_back(id, ptr);

            return detail::create_entity(world_opt, std::move(req))->take();
        }

        entity create_and_wait(const entity_recipe& base_recipe, std::optional<world*> world_opt = std::nullopt) && {
            auto req = std::make_unique<detail::components_holder>();
            req->components_reference.reserve(components.size());
            for (auto& [id, ptr] : components)
                req->components_reference.emplace_back(id, ptr);

            return detail::create_entity(world_opt, base_recipe, std::move(req))->take();
        }

        entity_construction() = default;

        entity_construction(entity_construction&& other) noexcept
            : components(std::move(other.components)) {}

        entity_construction& operator=(entity_construction&& other) noexcept {
            if (this != &other) {
                clear();
                components = std::move(other.components);
            }
            return *this;
        }

        entity_construction(const entity_construction&) = delete;
        entity_construction& operator=(const entity_construction&) = delete;

        ~entity_construction() {
            clear();
        }

        void clear() {
            for (auto& [id, ptr] : components) {
                auto& info = detail::component_info_registry.at(id);
                info.destroy(ptr);
                ::operator delete(ptr);
            }
            components.clear();
        }

    private:
        std::unordered_map<component_id, void*> components;
    };
}

#endif /* SRC_API_ECS_ENTITY_CONSTRUCTION */
