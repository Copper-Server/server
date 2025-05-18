#include <library/fast_task.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/entity.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/calculations.hpp>

namespace copper_server {
    namespace base_objects {
        struct entities_storage {
            std::unordered_map<uint16_t, entity_data> _registry;
            std::unordered_map<std::string, uint16_t> _name_to_id;
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

        const entity_data& entity_data::get_entity(const std::string& id) {
            return data_for_entities.get([&](auto& data) -> const entity_data& {
                auto it = data._registry.find(data._name_to_id.at(id));
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
                data._name_to_id[entity.id] = id;
                entity.entity_id = id;
                data._registry[id] = std::move(entity);
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

        enbt::compound entity::copy_to_enbt() const {
            enbt::compound res;
            res["nbt"] = nbt;
            res["server_data"] = server_data;
            res["position"] = enbt::fixed_array({position.x, position.y, position.z});
            res["motion"] = enbt::fixed_array({motion.x, motion.y, motion.z});
            res["rotation"] = enbt::fixed_array({rotation.x, rotation.y});
            res["head_rotation"] = enbt::fixed_array({head_rotation.x, head_rotation.y});
            res["entity_id"] = entity_id;
            res["id"] = id;
            res["died"] = died;
            if (world_syncing_data)
                res["bound_world"] = world_syncing_data->world->world_name;

            if (inventory.size()) {
                enbt::compound inventory_enbt;
                inventory_enbt.reserve(inventory.size());
                for (auto& [id, value] : inventory)
                    inventory_enbt[std::to_string(id)] = value.to_enbt();
                res["inventory"] = std::move(inventory_enbt);
            }


            if (custom_inventory.size()) {
                enbt::compound custom_inventory_enbt;
                custom_inventory_enbt.reserve(custom_inventory.size());
                for (auto& [id, inventory] : custom_inventory) {
                    enbt::compound inventory_enbt;
                    inventory_enbt.reserve(inventory.size());
                    for (auto& [id, value] : inventory)
                        inventory_enbt[std::to_string(id)] = value.to_enbt();
                    custom_inventory_enbt[id] = std::move(inventory_enbt);
                }

                res["custom_inventory"] = std::move(custom_inventory_enbt);
            }

            if (ride_entity_id)
                res["ride_entity_id"] = *ride_entity_id;

            if (hidden_effects.size()) {
                enbt::compound hidden_effects_enbt;
                hidden_effects_enbt.reserve(hidden_effects.size());
                for (auto& [id, effects] : hidden_effects) {
                    enbt::fixed_array effects_enbt;
                    effects_enbt.reserve(effects.size());
                    for (auto& effect : effects) {
                        effects_enbt.push_back(enbt::compound{
                            {"is_ambient", effect.ambient},
                            {"amplifier", effect.amplifier},
                            {"duration", effect.duration},
                            {"id", effect.id},
                            {"particles", effect.particles},
                        });
                    }
                    hidden_effects_enbt[std::to_string(id)] = std::move(effects_enbt);
                }
                res["hidden_effects"] = std::move(hidden_effects_enbt);
            }
            if (active_effects.size()) {
                enbt::compound active_effects_enbt;
                active_effects_enbt.reserve(active_effects.size());
                for (auto& [id, effect] : active_effects)
                    active_effects_enbt[std::to_string(id)] = enbt::compound{
                        {"is_ambient", effect.ambient},
                        {"amplifier", effect.amplifier},
                        {"duration", effect.duration},
                        {"id", effect.id},
                        {"particles", effect.particles},
                    };
                res["active_effects"] = std::move(active_effects_enbt);
            }

            if (attached_to)
                res["attached_to"] = *attached_to;

            if (attached.size()) {
                enbt::fixed_array arr;
                arr.reserve(attached.size());
                for (auto& it : attached)
                    arr.push_back(it);

                res["attached"] = std::move(arr);
            }
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

        entity_ref entity::load_from_enbt(const enbt::compound_const_ref& nbt) {
            entity_ref res = new entity();
            res->died = nbt["died"];
            res->entity_id = nbt["entity_id"];

            res->id = nbt["id"];

            auto motion = nbt["motion"].as_fixed_array();
            res->motion = {motion[0], motion[1], motion[2]};

            auto position = nbt["position"].as_fixed_array();
            res->position = {position[0], position[1], position[2]};

            auto rotation = nbt["rotation"].as_fixed_array();
            res->rotation = {rotation[0], rotation[1]};

            auto head_rotation = nbt["head_rotation"].as_fixed_array();
            res->head_rotation = {head_rotation[0], head_rotation[1]};

            res->nbt = nbt["nbt"];
            res->server_data = nbt["server_data"];

            if (nbt.contains("bound_world")) {
                api::world::get((std::string)nbt["bound_world"], [&](storage::world_data& it) {
                    it.register_entity(res);
                });
                if (!res->world_syncing_data)
                    throw std::runtime_error("World " + (std::string)nbt["bound_world"] + " not found.");
            };
            try {
                auto creation_callback = entity_data::get_entity(res->entity_id).create_from_enbt_callback;
                if (creation_callback)
                    creation_callback(res, nbt);
            } catch (...) {
                if (res->world_syncing_data)
                    res->world_syncing_data->world->unregister_entity(res);
                throw;
            }

            if (nbt.contains("inventory")) {
                auto inventory_enbt = nbt["inventory"].as_compound();
                res->inventory.reserve(inventory_enbt.size());
                for (auto& [id, value] : inventory_enbt) {
                    uint32_t id_ = 0;
                    auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                    if (parsing_res.ec == std::errc{})
                        res->inventory[id_] = slot_data::from_enbt(value.as_compound());
                }
            }
            if (nbt.contains("custom_inventory")) {
                auto custom_inventory_enbt = nbt["custom_inventory"].as_compound();
                res->custom_inventory.reserve(custom_inventory_enbt.size());
                for (auto& [id, value] : custom_inventory_enbt) {
                    decltype(res->inventory) custom_inventory;
                    res->inventory.reserve(value.size());
                    for (auto& [id, value] : value.as_compound()) {
                        uint32_t id_ = 0;
                        auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                        if (parsing_res.ec == std::errc{})
                            custom_inventory[id_] = slot_data::from_enbt(value.as_compound());
                    }
                    res->custom_inventory[id] = std::move(custom_inventory);
                }
            }

            if (nbt.contains("ride_entity_id"))
                res->ride_entity_id = (enbt::raw_uuid)nbt["ride_entity_id"];


            if (nbt.contains("hidden_effects")) {
                auto hidden_effects_enbt = nbt["hidden_effects"].as_compound();
                res->hidden_effects.reserve(hidden_effects_enbt.size());
                for (auto& [id, value] : hidden_effects_enbt) {
                    uint32_t id_ = 0;
                    auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                    if (parsing_res.ec == std::errc{}) {
                        list_array<entity::effect> effects;
                        res->inventory.reserve(value.size());
                        for (auto& value : value.as_array()) {
                            auto effect = value.as_compound();
                            effects.push_back(entity::effect{
                                .duration = effect.at("duration"),
                                .id = effect.at("id"),
                                .amplifier = effect.at("amplifier"),
                                .ambient = effect.at("is_ambient"),
                                .particles = effect.at("particles"),
                            });
                        }
                        res->hidden_effects[id_] = std::move(effects);
                    }
                }
            }
            if (nbt.contains("active_effects")) {
                auto active_effects_enbt = nbt["active_effects"].as_compound();
                res->active_effects.reserve(active_effects_enbt.size());
                for (auto& [id, value] : active_effects_enbt) {
                    uint32_t id_ = 0;
                    auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                    if (parsing_res.ec == std::errc{}) {
                        auto effect = value.as_compound();
                        res->active_effects[id_] = entity::effect{
                            .duration = effect.at("duration"),
                            .id = effect.at("id"),
                            .amplifier = effect.at("amplifier"),
                            .ambient = effect.at("is_ambient"),
                            .particles = effect.at("particles"),
                        };
                    }
                }
            }


            if (nbt.contains("attached_to"))
                res->attached_to = nbt["attached_to"];

            if (nbt.contains("attached")) {
                auto attached_enbt = nbt["attached"].as_array();
                res->attached.reserve(attached_enbt.size());
                for (auto& value : attached_enbt)
                    res->attached.push_back(value);
            }
            return res;
        }

        void entity::teleport(util::VECTOR pos, float yaw, float pitch, bool on_ground) {
            //TODO
        }

        void entity::teleport(int32_t x, int32_t y, int32_t z, float yaw, float pitch, bool on_ground) {
            //TODO
        }

        void entity::set_ride_entity(enbt::raw_uuid entity) {
            //TODO
        }

        void entity::remove_ride_entity() {
            //TODO
        }

        void entity::add_effect(uint32_t id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles) {
            //TODO
        }

        void entity::remove_effect(uint32_t id) {
            //TODO
        }

        void entity::remove_all_effects() {
            //TODO
        }

        bool entity::is_sleeping() const {
            return false;
            //TODO
        }

        bool entity::is_on_ground() const {
            return false;
            //TODO
        }

        bool entity::is_sneaking() const {
            return false;
            //TODO
        }

        bool entity::is_sprinting() const {
            return false;
            //TODO
        }

        void entity::set_sleeping(bool sleeping) {
            //TODO
        }

        void entity::set_on_ground(bool on_ground) {
            //TODO
        }

        void entity::set_sneaking(bool sneaking) {
            //TODO
        }

        void entity::set_sprinting(bool sprinting) {
            //TODO
        }

        float entity::get_health() const {
            return nbt.at("health");
        }

        void entity::set_health(float health) {
            //TODO
        }

        void entity::add_health(float health) {
            //TODO
        }

        void entity::damage(float health) {
            //TODO
        }

        void entity::reduce_health(float health) {
            //TODO
        }

        uint8_t entity::get_food() const {
            return nbt.at("food");
        }

        void entity::set_food(uint8_t food) {
            //TODO
        }

        void entity::add_food(uint8_t food) {
            //TODO
        }

        void entity::reduce_food(uint8_t food) {
            //TODO
        }

        float entity::get_saturation() const {
            return nbt.at("saturation");
        }

        void entity::set_saturation(float saturation) {
            //TODO
        }

        void entity::add_saturation(float saturation) {
            //TODO
        }

        void entity::reduce_saturation(float saturation) {
            //TODO
        }

        float entity::get_breath() const {
            return nbt.at("breath");
        }

        void entity::set_breath(float breath) {
            //TODO
        }

        void entity::add_breath(float breath) {
            //TODO
        }

        void entity::reduce_breath(float breath) {
            //TODO
        }

        float entity::get_experience() const {
            return nbt.at("experience");
        }

        void entity::set_experience(float experience) {
            //TODO
        }

        void entity::add_experience(float experience) {
            //TODO
        }

        void entity::reduce_experience(float experience) {
            //TODO
        }

        int32_t entity::get_fall_distance() const {
            return nbt.at("fall_distance");
        }

        void entity::set_fall_distance(int32_t fall_distance) {
            //TODO
        }

        void entity::add_fall_distance(int32_t fall_distance) {
            //TODO
        }

        void entity::reduce_fall_distance(int32_t fall_distance) {
            //TODO
        }

        uint8_t entity::get_selected_item() const {
            auto it = nbt.find("selected_item");
            if (it == nbt.end())
                return 0;
            else
                return it->second;
        }

        void entity::set_selected_item(uint8_t selected_item) {
            //TODO
        }

        void entity::set_position(util::VECTOR pos) {
            //TODO
        }

        void entity::move(float side, float forward, bool jump, bool sneaking) {
            //TODO
        }

        void entity::look(float yaw, float pitch) {
            //TODO
        }

        void entity::look_at(float x, float y, float z) {
            //TODO
        }

        void entity::look_at(util::VECTOR pos) {
            //TODO
        }

        util::VECTOR entity::get_motion() const {
            return {0, 0, 0}; //TODO
        }

        void entity::set_motion(util::VECTOR mot) {
            //TODO
        }

        void entity::add_motion(util::VECTOR mot) {
            //TODO
        }

        util::ANGLE_DEG entity::get_rotation() const {
            return {0, 0}; //TODO
        }

        void entity::set_rotation(util::ANGLE_DEG mot) {
            //TODO
        }

        void entity::add_rotation(util::ANGLE_DEG mot) {
            //TODO
        }

        util::ANGLE_DEG entity::get_head_rotation() const {
            return {0, 0}; //TODO
        }

        void entity::set_head_rotation(util::ANGLE_DEG rot) {
            //TODO
        }

        void entity::add_head_rotation(util::ANGLE_DEG rot) {
            //TODO
        }

        entity_ref entity::create(uint16_t id) {
            auto it = entity_data::get_entity(id);
            entity_ref res = new entity();
            res->entity_id = id;
            if (it.create_callback)
                it.create_callback(*res);
            return res;
        }

        entity_ref entity::create(uint16_t id, const enbt::compound_const_ref& nbt) {
            auto it = entity_data::get_entity(id);
            entity_ref res = new entity();
            res->entity_id = id;
            if (it.create_callback)
                it.create_callback_with_nbt(*res, nbt);
            else
                res->nbt = nbt;
            return res;
        }

        entity_ref entity::create(const std::string& id) {
            auto it = entity_data::get_entity(id);
            entity_ref res = new entity();
            res->entity_id = it.entity_id;
            if (it.create_callback)
                it.create_callback(*res);
            return res;
        }

        entity_ref entity::create(const std::string& id, const enbt::compound_const_ref& nbt) {
            auto it = entity_data::get_entity(id);
            entity_ref res = new entity();
            res->entity_id = it.entity_id;
            if (it.create_callback)
                it.create_callback_with_nbt(*res, nbt);
            else
                res->nbt = nbt;
            return res;
        }

        std::optional<int32_t> entity::get_object_field() {
            auto& obj_field_getter = entity_data::get_entity(entity_id).get_object_field;
            if (!obj_field_getter)
                return std::nullopt;
            return obj_field_getter(*this);
        }
    }
}
