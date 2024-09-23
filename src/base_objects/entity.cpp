#include "entity.hpp"
#include "../api/world.hpp"
#include "../calculations.hpp"
#include "../library/fast_task.hpp"
#include "../storage/world_data.hpp"

namespace crafted_craft {
    namespace base_objects {
        struct entities_storage {
            std::unordered_map<uint16_t, entity_data> _registry;
            uint16_t id_adder = 0;
        };

        fast_task::protected_value<entities_storage> data_for_entities;
        std::unordered_map<uint32_t, std::unordered_map<std::string, uint32_t>> entity_data::internal_entity_aliases_protocol;


        const entity_data& entity_data::get_entity(uint16_t id) {
            return data_for_entities.get([&](auto& data) -> const entity_data& {
                auto it = data._registry.find(id);
                if (it == data._registry.end())
                    throw std::runtime_error("Entity not found.");
                else
                    return it->second;
            });
        }

        uint16_t entity_data::register_entity(entity_data entity) {
            return data_for_entities.set([&](auto& data) {
                uint16_t id = data.id_adder++;
                if (data.id_adder == 0) {
                    --data.id_adder;
                    throw std::runtime_error("Too many entities.");
                }
                data._registry[id] = entity;
                return id;
            });
        }

        const entity_data& entity_data::view(const entity& entity) {
            return get_entity(entity.entity_id);
        }

        void entity_data::reset_entities() {
            data_for_entities.set([&](auto& data) {
                data.id_adder = 0;
                data._registry.clear();
            });
        }

        void entity_data::initialize_entities() {
            data_for_entities.set([&](auto& data) {
                for (auto& [id, entity] : data._registry) {
                    entity.internal_entity_aliases.clear();
                    for (auto& [protocol, assignations] : internal_entity_aliases_protocol) {
                        if (assignations.find(entity.id) != assignations.end()) {
                            entity.internal_entity_aliases[protocol] = assignations[entity.id];
                            continue;
                        } else {
                            bool found = false;
                            for (auto& alias : entity.entity_aliases) {
                                if (assignations.find(alias) != assignations.end()) {
                                    entity.internal_entity_aliases[protocol] = assignations[alias];
                                    found = true;
                                    break;
                                }
                            }
                            if (!found)
                                throw std::runtime_error("Entity alias for " + entity.id + " not found in protocol " + std::to_string(protocol));
                        }
                    }
                }
            });
        }

        entity_ref entity::copy() const {
            entity_ref res = new entity();
            res->died = died;
            res->entity_id = entity_id;
            res->nbt = nbt;
            res->position = position;
            return res;
        }

        enbt::value entity::copy_to_enbt() const {
            enbt::compound res;
            res["nbt"] = nbt;
            res["server_data"] = server_data;
            res["position"] = enbt::fixed_array({position.x, position.y, position.z});
            res["motion"] = enbt::fixed_array({motion.x, motion.y, motion.z});
            res["rotation"] = enbt::fixed_array({rotation.x, rotation.y, rotation.z});
            res["head_rotation"] = enbt::fixed_array({head_rotation.x, head_rotation.y, head_rotation.z});
            res["entity_id"] = entity_id;
            res["id"] = id;
            res["died"] = died;
            if (world)
                res["bound_world"] = world->world_name.get();
            return res;
        }

        void entity::tick() {
            const_data().tick_callback(*this);
        }

        bool entity::kill() {
            if (!const_data().pre_death_callback(*this, false))
                return false;

            died = true;
            return true;
        }

        void entity::force_kill() {
            const_data().pre_death_callback(*this, true);
            died = true;
        }

        bool entity::is_died() {
            return died;
        }

        const entity_data& entity::const_data() {
            return entity_data::get_entity(entity_id);
        }

        entity_ref entity::load_from_enbt(const enbt::compound_ref& nbt) {
            entity_ref res = new entity();
            res->died = nbt["died"];
            res->entity_id = nbt["entity_id"];

            res->id = nbt["id"];

            auto motion = enbt::fixed_array::make_ref(nbt["motion"]);
            res->motion = {motion[0], motion[1], motion[2]};

            auto position = enbt::fixed_array::make_ref(nbt["position"]);
            res->position = {position[0], position[1], position[2]};

            auto rotation = enbt::fixed_array::make_ref(nbt["rotation"]);
            res->rotation = {rotation[0], rotation[1], rotation[2]};

            auto head_rotation = enbt::fixed_array::make_ref(nbt["head_rotation"]);
            res->head_rotation = {head_rotation[0], head_rotation[1], head_rotation[2]};

            res->nbt = nbt["nbt"];
            res->server_data = nbt["server_data"];

            if (nbt.contains("bound_world")) {
                res->world = (storage::world_data*)0XFFFFFFFFFFF;
                api::world::get((std::string)nbt["bound_world"], [&](storage::world_data& it) {
                    it.register_entity(res);
                });
                if (res->world == (storage::world_data*)0XFFFFFFFFFFF) {
                    res->world = nullptr;
                    throw std::runtime_error("World " + (std::string)nbt["bound_world"] + " not found.");
                }
            };
            try {
                auto creation_callback = entity_data::get_entity(res->entity_id).create_from_enbt_callback;
                if (creation_callback)
                    creation_callback(res, nbt);
            } catch (...) {
                if (res->world)
                    res->world->unregister_entity(res);
                throw;
            }
            return res;
        }

        entity_ref entity::create(uint16_t id) {
            return entity_data::get_entity(id).create_callback();
        }

        entity_ref entity::create(uint16_t id, const enbt::compound_ref& nbt) {
            return entity_data::get_entity(id).create_callback_with_nbt(nbt);
        }
    }
}
