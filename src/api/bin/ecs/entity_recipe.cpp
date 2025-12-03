/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <src/api/bin/ecs/manager.hpp>

namespace copper_server::api::ecs {
    entity_recipe::entity_recipe() = default;

    entity_recipe::~entity_recipe() {
        cleanup();
    }

    entity_recipe::entity_recipe(const entity_recipe& other) : component_ids(other.component_ids), hash(other.hash), is_frozen_(other.is_frozen_) {
        for (auto& [id, ptr] : other.default_values) {
            clone_value(id, ptr);
        }
    }

    entity_recipe::entity_recipe(entity_recipe&& other) noexcept
        : component_ids(std::move(other.component_ids)),
          default_values(std::move(other.default_values)),
          hash(other.hash),
          is_frozen_(other.is_frozen_) {
    }

    // Copy Assignment
    entity_recipe& entity_recipe::operator=(const entity_recipe& other) {
        if (this != &other) {
            cleanup();
            component_ids = other.component_ids;
            hash = other.hash;
            is_frozen_ = other.is_frozen_;
            for (auto& [id, ptr] : other.default_values) {
                clone_value(id, ptr);
            }
        }
        return *this;
    }

    // Move Assignment
    entity_recipe& entity_recipe::operator=(entity_recipe&& other) noexcept {
        if (this != &other) {
            cleanup();
            component_ids = std::move(other.component_ids);
            default_values = std::move(other.default_values);
            hash = other.hash;
            is_frozen_ = other.is_frozen_;
        }
        return *this;
    }

    entity_recipe& entity_recipe::with(const entity_recipe& recipe) {
        if (!is_frozen_) {
            // Merge IDs
            component_ids.reserve(component_ids.size() + recipe.component_ids.size());
            component_ids.insert(component_ids.end(), recipe.component_ids.begin(), recipe.component_ids.end());

            // Merge Defaults (Deep Copy)
            for (auto& [id, ptr] : recipe.default_values) {
                // If we already have a default for this ID, replace it
                if (default_values.find(id) != default_values.end()) {
                    const auto& info = detail::component_info_registry.at(id);
                    info.destroy(default_values[id]);
                    ::operator delete(default_values[id]);
                    default_values.erase(id);
                }
                clone_value(id, ptr);
            }
        }
        return *this;
    }

    entity_recipe& entity_recipe::with(component_id id) {
        if (!is_frozen_)
            component_ids.push_back(id);
        return *this;
    }

    entity_recipe& entity_recipe::freeze() {
        if (is_frozen_)
            return *this;
        is_frozen_ = true;
        std::sort(component_ids.begin(), component_ids.end());
        component_ids.erase(
            std::unique(component_ids.begin(), component_ids.end()),
            component_ids.end()
        );
        hash = archetype_hash{}(component_ids);
        return *this;
    }

    bool entity_recipe::is_frozen() const {
        return is_frozen_;
    }

    const std::vector<whole_component_id>& entity_recipe::get_ids() const {
        return component_ids;
    }

    size_t entity_recipe::get_hash() const {
        assert(is_frozen_ && "Cannot get hash from an unfrozen recipe!");
        return hash;
    }

    void entity_recipe::cleanup() {
        for (auto& [id, ptr] : default_values) {
            const auto& info = detail::component_info_registry.at(id);
            info.destroy(ptr);
            ::operator delete(ptr);
        }
        default_values.clear();
    }

    void entity_recipe::clone_value(component_id id, void* src) {
        const auto& info = detail::component_info_registry.at(id);
        void* dest = ::operator new(info.size, std::align_val_t(info.alignment));
        info.construct(dest);
        info.copy_assign(dest, src);
        default_values[id] = dest;
    }
}