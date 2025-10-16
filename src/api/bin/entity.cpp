/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <library/enbt/io_tools.hpp>
#include <library/fast_task.hpp>
#include <src/api/ecs/base_components.hpp>
#include <src/api/entity.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/packets.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/calculations.hpp>

namespace enbt::io_helper {
    using namespace copper_server;
    using namespace copper_server::base_objects;

    template <>
    struct serialization<std::unordered_map<uint32_t, slot_data>> {
        static void read(std::unordered_map<uint32_t, slot_data>& res, value_read_stream& read_stream) {
            read_stream.iterate(
                [&res](auto size) { res.reserve(size); },
                [&res](std::string_view id, auto& value) {
                    uint32_t id_ = 0;
                    auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                    if (parsing_res.ec == std::errc{})
                        res[id_] = slot_data::from_enbt(value);
                }
            );
        }

        static void write(const std::unordered_map<uint32_t, slot_data>& res, value_write_stream& read_stream) {
            auto compound = read_stream.write_compound(res.size());
            for (auto& [id, value] : res)
                compound.write(std::to_string(id), [&value](auto& stream) { value.to_enbt(stream); });
        }

        static void read(std::unordered_map<uint32_t, slot_data>& res, const enbt::value& from) {
            res.reserve(from.size());
            for (auto& [id, value] : from.as_compound()) {
                uint32_t id_ = 0;
                auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                if (parsing_res.ec == std::errc{})
                    res[id_] = slot_data::from_enbt(value.as_compound());
            }
        }

        static void write(const std::unordered_map<uint32_t, slot_data>& res, enbt::value& to) {
            enbt::compound compound;
            compound.reserve(res.size());
            for (auto& [id, value] : res)
                compound[std::to_string(id)] = value.to_enbt();
            to = std::move(compound);
        }
    };

    template <>
    struct serialization<api::ecs::entity> {
        static void read(api::ecs::entity res, value_read_stream& read_stream) {
            read_stream //TODO replace this with standard format
                .read_compound()
                .collect("died", [&](auto& stream) { if (stream.read()) res.add<api::ecs::com::dead_mark>(); })
                .collect_as("entity_id", res.modify<api::ecs::com::entity_type>()->type)
                .collect_as("id", res.modify<api::ecs::com::uuid>()->id)
                .collect("motion", [&](auto& stream) { enbt::io_helper::serialization_read<util::VECTOR>(*res.modify<api::ecs::com::motion>(), stream); })
                .collect("position", [&](auto& stream) { enbt::io_helper::serialization_read<util::VECTOR>(*res.modify<api::ecs::com::position>(), stream); })
                .collect("rotation", [&](auto& stream) { enbt::io_helper::serialization_read<util::ANGLE_DEG>(*res.modify<api::ecs::com::rotation>(), stream); })
                .collect("head_rotation", [&](auto& stream) { enbt::io_helper::serialization_read<util::ANGLE_DEG>(*res.modify<api::ecs::com::head_rotation>(), stream); })
                .collect("inventory", [&](auto& stream) { enbt::io_helper::serialization_read(res.modify<api::ecs::com::inventory>()->value, stream); })
                .collect("custom_inventory", [&](auto& stream) { enbt::io_helper::serialization_read(res.modify<api::ecs::com::custom_inventory>()->value, stream); })
                .collect("nbt", [&](auto& stream) { res.modify<api::ecs::com::nbt>()->data = stream.read(); })
                .collect("server_data", [&](auto& stream) { res.modify<api::ecs::com::server_nbt>()->data = stream.read(); })
                .collect("bound_world", [&](auto& stream) {
                    std::string world_id = stream.read();
                    api::world::get(world_id, [&res](storage::world_data& it) {
                        it.register_entity(res);
                    });

                    if (!res.get<api::ecs::com::world_syncing>().world)
                        throw std::runtime_error("World " + world_id + " not found.");
                })
                .collect("hidden_effects", [&](auto& stream) {
                    auto res_effects = res.modify<api::ecs::com::effects>();
                    stream.iterate(
                        [&res_effects](auto size) { res_effects->hidden_effects.reserve(size); },
                        [&res_effects](std::string_view id, value_read_stream& effects) {
                            uint32_t id_ = 0;
                            auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                            if (parsing_res.ec == std::errc{}) {
                                list_array<api::ecs::com::effects::effect> set_effects;
                                effects.iterate(
                                    [&res_effects, id_](auto size) { res_effects->hidden_effects[id_].reserve(size); },
                                    [&res_effects, id_](value_read_stream& effect_) {
                                        auto effect = effect_.read();
                                        res_effects->hidden_effects[id_].push_back(api::ecs::com::effects::effect{
                                            .duration = effect.at("duration"),
                                            .id = effect.at("id"),
                                            .amplifier = effect.at("amplifier"),
                                            .ambient = effect.at("is_ambient"),
                                            .particles = effect.at("particles"),
                                            .use_blend = effect.at("use_blend"),
                                        });
                                    }
                                );
                            }
                        }
                    );
                })
                .collect("active_effects", [&](auto& stream) {
                    auto res_effects = res.modify<api::ecs::com::effects>();
                    stream.iterate(
                        [&res_effects](auto size) { res_effects->active_effects.reserve(size); },
                        [&res_effects](value_read_stream& effect_) {
                            auto effect = effect_.read();
                            res_effects->active_effects.emplace(
                                effect.at("id"),
                                api::ecs::com::effects::effect{
                                    .duration = effect.at("duration"),
                                    .id = effect.at("id"),
                                    .amplifier = effect.at("amplifier"),
                                    .ambient = effect.at("is_ambient"),
                                    .particles = effect.at("particles"),
                                    .use_blend = effect.at("use_blend"),
                                }
                            );
                        }
                    );
                })
                .collect("ride_by_entity", [&](auto& stream) {
                    stream.iterate(
                        [&res](auto size) { res->ride_by_entity.reserve(size); },
                        [&res](value_read_stream& value) {
                            auto ride_entity = entity::load_from_file(value);
                            if (res->current_world())
                                res->current_world()->register_entity(ride_entity);
                            ride_entity->set_ride_entity(res);
                        }
                    );
                })
                .collect("attached_to", [&](auto& stream) {
                    auto val = stream.read();
                    if (val.is_uuid())
                        res->attached_to = (enbt::raw_uuid)val;
                })
                .collect("attached", [&](auto& stream) {
                    stream.iterate(
                        [&res](auto size) { res->attached.reserve(size); },
                        [&res](value_read_stream& value) {
                            res->attached.push_back((enbt::raw_uuid)value.read());
                        }
                    );
                })
                .make_collect();
        }

        static void write(api::ecs::entity value, value_write_stream& write_stream) {
            auto compound = write_stream.write_compound(14 + bool(value->attached_to) + bool(value->current_world()));
            compound.write("died", value->died)
                .write("entity_id", value->entity_id)
                .write("id", value->id)
                .write("nbt", value->nbt)
                .write("server_data", value->server_data)
                .write("motion", [&value](value_write_stream& stream) {
                    enbt::io_helper::serialization_write(value->motion, stream);
                })
                .write("position", [&value](value_write_stream& stream) {
                    enbt::io_helper::serialization_write(value->position, stream);
                })
                .write("rotation", [&value](value_write_stream& stream) {
                    enbt::io_helper::serialization_write(value->rotation, stream);
                })
                .write("head_rotation", [&value](value_write_stream& stream) {
                    enbt::io_helper::serialization_write<util::ANGLE_DEG>(value.get<api::ecs::com::head_rotation>(), stream);
                })
                .write("inventory", [&value](value_write_stream& stream) {
                    enbt::io_helper::serialization_write(value->inventory, stream);
                })
                .write("custom_inventory", [&value](value_write_stream& stream) {
                    enbt::io_helper::serialization_write(value->custom_inventory, stream);
                })
                .write("active_effects", [&value](value_write_stream& stream) {
                    stream.write_array(value->active_effects.size()).iterable(value->active_effects, [](auto& item, value_write_stream& item_stream) {
                        auto& [id, effect] = item;
                        item_stream
                            .write_compound(5)
                            .write("duration", effect.duration)
                            .write("id", effect.id)
                            .write("amplifier", effect.amplifier)
                            .write("is_ambient", effect.ambient)
                            .write("particles", effect.particles)
                            .write("use_blend", effect.use_blend);
                    });
                })
                .write("hidden_effects", [&value](value_write_stream& stream) {
                    auto comp = stream.write_compound(value->hidden_effects.size());
                    for (auto& [id, effects] : value->hidden_effects) {
                        comp.write(std::to_string(id), [&effects](value_write_stream& effects_stream) {
                            effects_stream.write_array(effects.size())
                                .iterable(effects, [](auto& effect, value_write_stream& item_stream) {
                                    item_stream
                                        .write_compound(5)
                                        .write("duration", effect.duration)
                                        .write("id", effect.id)
                                        .write("amplifier", effect.amplifier)
                                        .write("is_ambient", effect.ambient)
                                        .write("particles", effect.particles)
                                        .write("use_blend", effect.use_blend);
                                });
                        });
                    }
                })
                .write("attached", [&value](value_write_stream& stream) {
                    stream
                        .write_array(value->attached.size())
                        .iterable(value->attached, [](auto& attached, value_write_stream& attached_stream) {
                            std::visit(
                                [&attached_stream]<class T>(const T& item) {
                                    if constexpr (std::is_same_v<T, enbt::raw_uuid>)
                                        attached_stream.write(item);
                                    else {
                                        attached_stream.write(item->id);
                                    }
                                },
                                attached
                            );
                        });
                });
            if (value->attached_to) {
                compound.write("attached_to", [&value](value_write_stream& stream) {
                    std::visit(
                        [&stream]<class T>(const T& item) {
                            if constexpr (std::is_same_v<T, enbt::raw_uuid>)
                                stream.write(item);
                            else {
                                stream.write(item->id);
                            }
                        },
                        *value->attached_to
                    );
                });
            }
            if (value->current_world())
                compound.write("bound_world", value->current_world()->world_name);
        }

        static void read(api::ecs::entity res, const enbt::value& from) {
            for (auto& [name, value] : from.as_compound()) {
                if (name == "died")
                    res->died = value;
                else if (name == "entity_id")
                    res->entity_id = value;
                else if (name == "id")
                    res->id = value;
                else if (name == "motion")
                    enbt::io_helper::serialization_read(res->motion, value);
                else if (name == "position")
                    enbt::io_helper::serialization_read(res->position, value);
                else if (name == "rotation")
                    enbt::io_helper::serialization_read(res->rotation, value);
                else if (name == "head_rotation")
                    enbt::io_helper::serialization_read(res->head_rotation, value);
                else if (name == "inventory")
                    enbt::io_helper::serialization_read(res->inventory, value);
                else if (name == "custom_inventory")
                    enbt::io_helper::serialization_read(res->custom_inventory, value);
                else if (name == "nbt")
                    res->nbt = value;
                else if (name == "server_data")
                    res->server_data = value;
                else if (name == "bound_world") {
                    std::string world_id = value;
                    api::world::get(world_id, [&res](storage::world_data& it) {
                        it.register_entity(res);
                    });
                    if (!res->current_world())
                        throw std::runtime_error("World " + world_id + " not found.");
                } else if (name == "hidden_effects") {
                    res->hidden_effects.reserve(value.size());
                    for (auto& [id, effects] : value.as_compound()) {
                        uint32_t id_ = 0;
                        auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                        auto& cur_effects = res->hidden_effects[id_];
                        cur_effects.reserve(effects.size());
                        for (auto& effect : effects.as_array()) {
                            cur_effects.push_back(entity::effect{
                                .duration = effect.at("duration"),
                                .id = effect.at("id"),
                                .amplifier = effect.at("amplifier"),
                                .ambient = effect.at("is_ambient"),
                                .particles = effect.at("particles"),
                                .use_blend = effect.at("use_blend"),
                            });
                        }
                    }
                } else if (name == "active_effects") {
                    res->active_effects.reserve(value.size());
                    for (auto& [id, effect] : value.as_compound()) {
                        uint32_t id_ = 0;
                        auto parsing_res = std::from_chars(id.data(), id.data() + id.size(), id_);
                        res->active_effects.emplace(
                            id_,
                            entity::effect{
                                .duration = effect.at("duration"),
                                .id = effect.at("id"),
                                .amplifier = effect.at("amplifier"),
                                .ambient = effect.at("is_ambient"),
                                .particles = effect.at("particles"),
                            }
                        );
                    }
                } else if (name == "ride_by_entity") {
                    res->ride_by_entity.reserve(value.size());
                    for (auto& entity : value.as_array()) {
                        auto ride_entity = entity::load_from_enbt(entity.as_compound());
                        if (res->current_world())
                            res->current_world()->register_entity(ride_entity);
                        ride_entity->set_ride_entity(res);
                    }
                } else if (name == "attached_to") {
                    res->attached_to = (enbt::raw_uuid)value;
                } else if (name == "attached") {
                    res->attached.reserve(value.size());
                    for (auto& entity : value.as_array())
                        res->attached.push_back((enbt::raw_uuid)entity);
                }
            }
        }

        static void write(ecs::entity value, enbt::value& to) {
            enbt::compound compound{
                {"died", value->died},
                {"id", value->id},
                {"nbt", value->nbt},
                {"server_data", value->server_data},
            };
            enbt::io_helper::serialization_write(value->motion, compound["motion"]);
            enbt::io_helper::serialization_write(value->position, compound["position"]);
            enbt::io_helper::serialization_write(value->rotation, compound["rotation"]);
            enbt::io_helper::serialization_write(value->head_rotation, compound["head_rotation"]);
            enbt::io_helper::serialization_write(value->inventory, compound["inventory"]);
            enbt::io_helper::serialization_write(value->custom_inventory, compound["custom_inventory"]);
            {
                enbt::fixed_array arr;
                arr.reserve(value->active_effects.size());
                for (auto& [id, effect] : value->active_effects) {
                    arr.push_back(enbt::compound{
                        {"duration", effect.duration},
                        {"id", effect.id},
                        {"amplifier", effect.amplifier},
                        {"is_ambient", (bool)effect.ambient},
                        {"particles", (bool)effect.particles},
                        {"use_blend", (bool)effect.use_blend},
                    });
                }
                compound.emplace("active_effects", std::move(arr));
            };
            {
                enbt::compound hidden_effects;
                hidden_effects.reserve(value->hidden_effects.size());
                for (auto& [id, effects] : value->hidden_effects) {
                    enbt::fixed_array arr;
                    arr.reserve(effects.size());
                    for (auto& effect : effects) {
                        arr.push_back(enbt::compound{
                            {"duration", effect.duration},
                            {"id", effect.id},
                            {"amplifier", effect.amplifier},
                            {"is_ambient", (bool)effect.ambient},
                            {"particles", (bool)effect.particles},
                            {"use_blend", (bool)effect.use_blend},
                        });
                    }
                    hidden_effects.emplace(std::to_string(id), std::move(arr));
                }
                compound.emplace("hidden_effects", std::move(hidden_effects));
            };
            {
                enbt::fixed_array arr;
                arr.reserve(value->attached.size());
                for (auto& attached : value->attached) {
                    std::visit(
                        [&arr]<class T>(const T& item) {
                            if constexpr (std::is_same_v<T, enbt::raw_uuid>)
                                arr.push_back(item);
                            else
                                arr.push_back(item->id);
                        },
                        attached
                    );
                }
                compound.emplace("attached", std::move(arr));
            }
            {
            }
            if (value->attached_to) {
                compound.emplace(
                    "attached_to",
                    std::visit(
                        []<class T>(const T& item) {
                            if constexpr (std::is_same_v<T, enbt::raw_uuid>)
                                return item;
                            else
                                return item->id;
                        },
                        *value->attached_to
                    )
                );
            }
            if (value->current_world())
                compound.emplace("bound_world", value->current_world()->world_name);
            to = std::move(compound);
        }
    };

    template <class T>
    struct serialization<util::XYZ<T>> {
        static void read(util::XYZ<T>& res, value_read_stream& read_stream) {
            read_stream
                .read_compound()
                .collect_as("x", res.x)
                .collect_as("y", res.y)
                .collect_as("z", res.z)
                .force_all_collect();
        }

        static void write(const util::XYZ<T>& res, value_write_stream& write_stream) {
            write_stream
                .write_compound()
                .write("x", res.x)
                .write("y", res.y)
                .write("z", res.z);
        }

        static void read(util::XYZ<T>& res, const enbt::value& from) {
            res.x = from.at("x");
            res.y = from.at("y");
            res.z = from.at("z");
        }

        static void write(const util::XYZ<T>& res, enbt::value& to) {
            to = enbt::compound{
                {"x", res.x},
                {"y", res.y},
                {"z", res.z}
            };
        }
    };

    template <class T>
    struct serialization<util::XY<T>> {
        static void read(util::XY<T>& res, value_read_stream& read_stream) {
            read_stream
                .read_compound()
                .collect_as("x", res.x)
                .collect_as("y", res.y)
                .force_all_collect();
        }

        static void write(const util::XY<T>& res, value_write_stream& write_stream) {
            write_stream
                .write_compound()
                .write("x", res.x)
                .write("y", res.y);
        }

        static void read(util::XY<T>& res, const enbt::value& from) {
            res.x = from.at("x");
            res.y = from.at("y");
        }

        static void write(const util::XY<T>& res, enbt::value& to) {
            to = enbt::compound{
                {"x", res.x},
                {"y", res.y}
            };
        }
    };

    template <>
    struct serialization<util::ANGLE_DEG> {
        static void read(util::ANGLE_DEG& res, value_read_stream& read_stream) {
            read_stream
                .read_compound()
                .collect_as("pitch", res.pitch)
                .collect_as("yaw", res.yaw)
                .force_all_collect();
        }

        static void write(const util::ANGLE_DEG& res, value_write_stream& write_stream) {
            write_stream
                .write_compound()
                .write("pitch", res.pitch)
                .write("yaw", res.yaw);
        }

        static void read(util::ANGLE_DEG& res, const enbt::value& from) {
            res.pitch = from.at("pitch");
            res.yaw = from.at("yaw");
        }

        static void write(const util::ANGLE_DEG& res, enbt::value& to) {
            to = enbt::compound{
                {"pitch", res.pitch},
                {"yaw", res.yaw}
            };
        }
    };

    template <>
    struct serialization<util::ANGLE_RAD> {
        static void read(util::ANGLE_RAD& res, value_read_stream& read_stream) {
            read_stream
                .read_compound()
                .collect_as("pitch", res.pitch)
                .collect_as("yaw", res.yaw)
                .force_all_collect();
        }

        static void write(const util::ANGLE_RAD& res, value_write_stream& write_stream) {
            write_stream
                .write_compound()
                .write("pitch", res.pitch)
                .write("yaw", res.yaw);
        }

        static void read(util::ANGLE_RAD& res, const enbt::value& from) {
            res.pitch = from.at("pitch");
            res.yaw = from.at("yaw");
        }

        static void write(const util::ANGLE_RAD& res, enbt::value& to) {
            to = enbt::compound{
                {"pitch", res.pitch},
                {"yaw", res.yaw}
            };
        }
    };
}

namespace copper_server {
    namespace api {
        struct entities_storage {
            std::unordered_map<int32_t, entity_data> _registry;
            std::unordered_map<std::string, int32_t> _name_to_id;
            std::unordered_map<std::string, std::shared_ptr<entity_data::world_processor>> entity_processors;
            int32_t id_adder = 0;
        };

        fast_task::protected_value<entities_storage> data_for_entities;

        const entity_data& entity_data::get_entity(int32_t id) {
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

        list_array<int32_t> entity_data::get_entity_ids() {
            return data_for_entities.get([&](auto& data) {
                list_array<int32_t> res;
                res.reserve(data._registry.size());
                for (auto& [id, ent] : data._registry)
                    res.push_back(id);
                return res;
            });
        }

        int32_t entity_data::register_entity(entity_data entity) {
            return data_for_entities.set([&](auto& data) {
                int32_t id = data.id_adder++;
                if (data.id_adder <= 0) {
                    --data.id_adder;
                    throw std::runtime_error("Too many entities.");
                }
                data._name_to_id[entity.id] = id;
                entity.entity_id = id;
                data._registry[id] = std::move(entity);
                return id;
            });
        }

        const entity_data& entity_data::view(ecs::entity entity) {
            return entity.get<ecs::com::entity_type>().const_data();
        }

        entity_data& entity_data::initialization_get(int32_t id) {
            return data_for_entities.set([&](auto& data) -> entity_data& {
                auto it = data._registry.find(id);
                if (it == data._registry.end())
                    throw std::runtime_error("Entity not found.");
                else
                    return it->second;
            });
        }

        void entity_data::register_entity_world_processor(std::shared_ptr<world_processor> processor, const std::string& id) {
            data_for_entities.set([&](auto& data) {
                if (!data.entity_processors.emplace(id, processor).second)
                    throw std::runtime_error("Processor for this entity already registered.");
            });
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
                    if (auto it = data.entity_processors.find(entity.id); it != data.entity_processors.end())
                        entity.processor = it->second;
                }
                player_entity_id = data._name_to_id.at("minecraft:player");
            });
        }

        int32_t entity_data::player_entity_id;

        storage::world_data* entity::current_world() const {
            return handle.get<ecs::com::world_syncing>().world;
        }

        int32_t entity::get_protocol_id() const {
            return handle.get<api::ecs::com::protocol_id>().value;
        }
        
        util::VECTOR entity::get_position() const{
            return handle.get<api::ecs::com::position>();
        }

        util::VECTOR entity::get_motion() const {
            return handle.get<api::ecs::com::motion>();
        }

        util::ANGLE_DEG entity::get_rotation() const {
            return handle.get<api::ecs::com::rotation>();
        }

        util::ANGLE_DEG entity::get_head_rotation() const {
            return handle.get<api::ecs::com::head_rotation>();
        }

        ecs::entity entity::copy() const {
            return handle.copy();
        }

        void resolve_entity(std::variant<ecs::entity, enbt::raw_uuid>& it) {
            if (std::holds_alternative<enbt::raw_uuid>(it)) {
                auto entity = api::entity_id_map::get_entity(std::get<enbt::raw_uuid>(it));
                if (entity)
                    it = *entity;
            }
        }

        void reduce_effects(ecs::com::effects& eff) { //TODO replace with system
            list_array<uint32_t> expired_effects;

            for (auto& [id, effect] : eff.active_effects) {
                if (!effect.duration) {
                    expired_effects.push_back(id);
                    continue;
                }
                if (effect.duration != UINT32_MAX)
                    effect.duration--;
            }

            for (auto& [id, effects] : eff.hidden_effects) {
                for (auto& effect : effects) {
                    if (!effect.duration)
                        continue;
                    if (effect.duration != UINT32_MAX)
                        effect.duration--;
                }

                effects.remove_if([](const ecs::com::effects::effect& effect) {
                    return !effect.duration;
                });
                effects.sort([](ecs::com::effects::effect& effect0, ecs::com::effects::effect& effect1) {
                    return effect0.amplifier > effect1.amplifier;
                });
            }
            for (auto& id : expired_effects) {
                if (eff.hidden_effects.contains(id))
                    eff.active_effects.at(id) = eff.hidden_effects.at(id).take_front();
                else
                    eff.active_effects.erase(id);
            }
        }

        void entity::tick() {

            if (handle.has<ecs::com::attached_to>())
                resolve_entity(handle.modify<ecs::com::attached_to>()->other);

            if (handle.has<ecs::com::attached>())
                for (auto& it : handle.modify<ecs::com::attached>()->ride_by_entity)
                    resolve_entity(it);

            auto proc = const_data().processor;
            if (proc)
                if (proc->on_tick)
                    proc->on_tick(handle);
            reduce_effects(*handle.modify<ecs::com::effects>());
        }

        base_objects::entity_metadata::entity_pose entity::get_pose() const {
            return handle.get<ecs::com::pose>().value;
        }

        void entity::set_pose(base_objects::entity_metadata::entity_pose pose) {
            handle.modify<ecs::com::pose>()->value = pose;
        }

        double entity::eye_height() const {
            return const_data().eye_height_in_each_pose.at(get_pose().value.value); //add scale and other modifiers
        }

        bool entity::kill() {
            if (!const_data().pre_death_callback(handle, false))
                return false;

            handle.add<ecs::com::dead_mark>();
            return true;
        }

        void entity::force_kill() {
            const_data().pre_death_callback(handle, true);
            handle.add<ecs::com::dead_mark>();
        }

        void entity::erase() {
            if (current_world())
                current_world()->entity_deinit(handle);
        }

        bool entity::is_died() const {
            return handle.has<ecs::com::dead_mark>();
        }

        const entity_data& entity::const_data() const {
            return entity_data::view(handle);
        }

        bool entity::hitboxes_touching_x(double min, double max) {
            auto& position = handle.get<api::ecs::com::position>();
            auto& bounds = handle.get<api::ecs::com::bounding_box>();
            return (position.x - bounds.xz) >= min && (position.x + bounds.xz) <= max;
        }

        bool entity::hitboxes_touching_y(double min, double max) {
            auto& position = handle.get<api::ecs::com::position>();
            auto& bounds = handle.get<api::ecs::com::bounding_box>();
            return (position.y) >= min && (position.y + bounds.y) <= max;
        }

        bool entity::hitboxes_touching_z(double min, double max) {
            auto& position = handle.get<api::ecs::com::position>();
            auto& bounds = handle.get<api::ecs::com::bounding_box>();
            return (position.z - bounds.xz) >= min && (position.z + bounds.xz) <= max;
        }

        void entity::moved(util::VECTOR pos) {
            if (current_world())
                current_world()->entity_move(handle, pos);
            *handle.modify<api::ecs::com::position>() = pos;
        }

        void entity::moved(util::VECTOR pos, float yaw, float pitch) {
            if (current_world()) {
                current_world()->entity_move(handle, pos);
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            }
            *handle.modify<api::ecs::com::position>() = pos;
            *handle.modify<api::ecs::com::rotation>() = {yaw, pitch};
        }

        void entity::moved(util::VECTOR pos, float yaw, float pitch, bool on_ground) {
            if (current_world()) {
                current_world()->entity_move(handle, pos);
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            }
            *handle.modify<api::ecs::com::position>() = pos;
            *handle.modify<api::ecs::com::rotation>() = {yaw, pitch};
            set_on_ground(on_ground);
        }

        void entity::rotated(float yaw, float pitch) {
            if (current_world())
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            *handle.modify<api::ecs::com::rotation>() = {yaw, pitch};
        }

        void entity::rotated(float yaw, float pitch, bool on_ground) {
            if (current_world())
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            *handle.modify<api::ecs::com::rotation>() = {yaw, pitch};
            set_on_ground(on_ground);
        }

        void entity::teleport(util::VECTOR pos) {
            if (current_world())
                current_world()->entity_teleport(handle, pos);
            *handle.modify<api::ecs::com::position>() = pos;
            auto assigned_player = handle.get<ecs::com::assigned_player>().player;
            auto protocol_id = handle.get<ecs::com::protocol_id>().value;
            auto mot = handle.get<ecs::com::motion>();
            auto rot = handle.get<ecs::com::rotation>();
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::entity_position_sync{
                    .id = protocol_id,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .velocity_x = mot.x,
                    .velocity_y = mot.y,
                    .velocity_z = mot.z,
                    .yaw = (float)rot.yaw,
                    .pitch = (float)rot.pitch,
                    .on_ground = is_on_ground()
                };
        }

        void entity::teleport(util::VECTOR pos, float yaw, float pitch) {
            if (current_world()) {
                current_world()->entity_teleport(handle, pos);
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            }
            auto assigned_player = handle.get<ecs::com::assigned_player>().player;
            auto protocol_id = handle.get<ecs::com::protocol_id>().value;
            *handle.modify<api::ecs::com::position>() = pos;
            auto mot = handle.get<ecs::com::motion>();
            auto& rot = *handle.modify<api::ecs::com::rotation>() = {yaw, pitch};
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::entity_position_sync{
                    .id = protocol_id,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .velocity_x = mot.x,
                    .velocity_y = mot.y,
                    .velocity_z = mot.z,
                    .yaw = (float)rot.yaw,
                    .pitch = (float)rot.pitch,
                    .on_ground = is_on_ground()
                };
        }

        void entity::teleport(util::VECTOR pos, float yaw, float pitch, bool on_ground) {
            if (current_world()) {
                current_world()->entity_teleport(handle, pos);
                current_world()->entity_rotation_changes(handle, {yaw, pitch});
            }
            auto assigned_player = handle.get<ecs::com::assigned_player>().player;
            auto protocol_id = handle.get<ecs::com::protocol_id>().value;
            *handle.modify<api::ecs::com::position>() = pos;
            auto mot = handle.get<ecs::com::motion>();
            *handle.modify<api::ecs::com::rotation>() = {yaw, pitch};
            set_on_ground(on_ground);
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::entity_position_sync{
                    .id = protocol_id,
                    .x = pos.x,
                    .y = pos.y,
                    .z = pos.z,
                    .velocity_x = mot.x,
                    .velocity_y = mot.y,
                    .velocity_z = mot.z,
                    .yaw = (float)yaw,
                    .pitch = (float)pitch,
                    .on_ground = is_on_ground()
                };
        }

        void entity::set_ride_entity(ecs::entity entity) {
            if (current_world()) {
                if (entity->current_world() == current_world()) {
                    current_world()->entity_rides(handle, handle.get<ecs::com::world_syncing>().assigned_world_id);
                    ride_entity = entity;
                    return;
                }
            }
        }

        void entity::remove_ride_entity() {
            if (ride_entity)
                if (current_world()) {
                    if ((*ride_entity)->current_world() == current_world()) {
                        current_world()->entity_leaves_ride(handle, (*ride_entity)->world_syncing_data.assigned_world_id);
                        ride_entity = std::nullopt;
                        return;
                    }
                }
            ride_entity = std::nullopt;
        }

        void entity::add_effect(uint32_t id_, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
            entity::effect to_add_effect{
                .duration = duration,
                .id = id_,
                .amplifier = amplifier,
                .ambient = ambient,
                .particles = show_particles,
                .show_icon = show_icon,
                .use_blend = use_blend,
            };
            if (auto it = active_effects.find(id_); it != active_effects.end()) {
                auto& effect = it->second;
                if (effect.amplifier >= amplifier) {
                    if (effect.duration < duration)
                        hidden_effects[id_].push_back(to_add_effect);
                    if (current_world())
                        current_world()->entity_add_effect(handle, id_, duration, amplifier, ambient, show_particles, show_icon, use_blend);
                    return;
                } else
                    hidden_effects[id_].push_back(effect);
            }
            active_effects[id_] = to_add_effect;
            if (current_world())
                current_world()->entity_add_effect(handle, id_, duration, amplifier, ambient, show_particles, show_icon, use_blend);
        }

        void entity::remove_effect(uint32_t id_) {
            active_effects.erase(id_);
            hidden_effects.erase(id_);
            if (current_world())
                current_world()->entity_remove_effect(handle, id_);
        }

        void entity::remove_all_effects() {
            if (current_world())
                for (auto& [id_, effect] : active_effects)
                    current_world()->entity_remove_effect(handle, id_);
            active_effects.clear();
            hidden_effects.clear();
        }

        bool entity::is_sleeping() const {
            return handle.get<ecs::com::world_syncing>().is_sleeping;
        }

        bool entity::is_on_ground() const {
            return handle.get<ecs::com::world_syncing>().on_ground;
        }

        bool entity::is_sneaking() const {
            return handle.get<ecs::com::world_syncing>().is_sneaking;
        }

        bool entity::is_sprinting() const {
            return handle.get<ecs::com::world_syncing>().is_sprinting;
        }

        void entity::set_sleeping(bool sleeping) {
            if (current_world())
                handle.modify<ecs::com::world_syncing>()->is_sleeping = sleeping;
        }

        void entity::set_on_ground(bool on_ground) {
            if (current_world())
                handle.modify<ecs::com::world_syncing>()->on_ground = on_ground;
        }

        void entity::set_sneaking(bool sneaking) {
            if (current_world())
                handle.modify<ecs::com::world_syncing>()->is_sneaking = sneaking;
        }

        void entity::set_sprinting(bool sprinting) {
            if (current_world())
                handle.modify<ecs::com::world_syncing>()->is_sprinting = sprinting;
        }

        float entity::get_health() const {
            auto it = nbt.find("health");
            if (it == nbt.end())
                return 0;
            else
                return it->second;
        }

        void entity::set_health(float health) {
            nbt["health"] = health;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_health{
                    .health = health,
                    .food = get_food(),
                    .saturation = {get_saturation()}
                };

            if (health <= 0.0f)
                force_kill();
        }

        void entity::add_health(float health) {
            set_health(get_health() + health);
        }

        void entity::damage(float health, int32_t type_id, std::optional<util::VECTOR> pos) {
            handle.modify<ecs::com::world_syncing>()->inactivity_counter = 0;
            if (current_world())
                current_world()->entity_damage(handle, health, type_id, pos);
            reduce_health(health);
        }

        void entity::damage(float health, int32_t type_id, std::optional<ecs::entity> source, std::optional<util::VECTOR> pos) {
            handle.modify<ecs::com::world_syncing>()->inactivity_counter = 0;
            if (current_world())
                current_world()->entity_damage(handle, health, type_id, source, pos);
            reduce_health(health);
        }

        void entity::damage(float health, int32_t type_id, std::optional<ecs::entity> source, std::optional<ecs::entity> source_direct, std::optional<util::VECTOR> pos) {
            handle.modify<ecs::com::world_syncing>()->inactivity_counter = 0;
            if (current_world())
                current_world()->entity_damage(handle, health, type_id, source, source_direct, pos);
            reduce_health(health);
        }

        void entity::reduce_health(float health) {
            set_health(get_health() - health);
        }

        uint8_t entity::get_food() const {
            auto it = nbt.find("food");
            if (it == nbt.end())
                return 0;
            else
                return it->second;
        }

        void entity::set_food(uint8_t food) {
            nbt["food"] = food;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_health{
                    .health = get_health(),
                    .food = food,
                    .saturation = {get_saturation()}
                };
        }

        void entity::add_food(uint8_t food) {
            set_food(get_food() + food);
        }

        void entity::reduce_food(uint8_t food) {
            set_food(get_food() - food);
        }

        float entity::get_saturation() const {
            return nbt.at("saturation");
        }

        void entity::set_saturation(float saturation) {
            nbt.at("saturation") = saturation;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_health{
                    .health = get_health(),
                    .food = get_food(),
                    .saturation = {saturation}
                };
        }

        void entity::add_saturation(float saturation) {
            set_saturation(get_saturation() - saturation);
        }

        void entity::reduce_saturation(float saturation) {
            set_saturation(get_saturation() - saturation);
        }

        int32_t entity::get_breath() const {
            auto it = metadata.find("breath");
            if (it == metadata.end())
                return 0;
            else
                return std::get<entity_metadata::var_int>(it->second.value).value;
        }

        void entity::set_breath(int32_t breath) {
            auto& meta = metadata["AIR"];
            meta.value = entity_metadata::var_int{.value = breath};
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_entity_data{
                    .id = protocol_id,
                    .metadata = {api::packets::client_bound::play::set_entity_data::metadata_item_t{
                        .index = 0,
                        .value = meta
                    }}
                };
        }

        void entity::add_breath(int32_t breath) {
            set_breath(get_breath() + breath);
        }

        void entity::reduce_breath(int32_t breath) {
            set_breath(get_breath() - breath);
        }

        int32_t entity::get_level() const {
            auto it = nbt.find("level");
            if (it == nbt.end())
                return 0;
            else
                return it->second;
        }

        int32_t calculate_required_experience(int32_t level) {
            // clang-format off
            int32_t required_exp = level;
            switch(level){
                case  0:case  1:case  2:case  3:case  4:case  5:case 6: case  7:
                case  8:case  9:case 10:case 11:case 12:case 13:case 14:case 15:
                    required_exp = required_exp * 2 + 7;
                    break;
                case 16:case 17:case 18:case 19:case 20:case 21:case 22:case 23:
                case 24:case 25:case 26:case 27:case 28:case 29:case 30:
                    required_exp = required_exp * 5 - 38;
                    break;
                default:
                    required_exp = required_exp * 9 - 158;
                    break;
            }
            // clang-format on
            return required_exp;
        }

        int32_t calculate_experience_from_level(int32_t level) {
            // clang-format off
            int32_t required_exp = level;
            switch(level){
                case  0:case  1:case  2:case  3:case  4:case  5:case 6: case  7:
                case  8:case  9:case 10:case 11:case 12:case 13:case 14:case 15:
                    required_exp = required_exp * required_exp + 6 * required_exp;
                    break;
                case 16:case 17:case 18:case 19:case 20:case 21:case 22:case 23:
                case 24:case 25:case 26:case 27:case 28:case 29:case 30:case 31:
                    required_exp = int32_t(2.5 * required_exp * required_exp - 40.5 * required_exp + 360);
                    break;
                default:
                    required_exp = int32_t(4.5 * required_exp * required_exp - 162.5 * required_exp + 2220);
                break;
            }
            // clang-format on
            return required_exp;
        }

        void entity::set_level(int32_t level) {
            int32_t old_lvl = nbt["level"];
            nbt["level"] = level;
            double progress_old = 1.0 / calculate_required_experience(old_lvl) * get_experience();
            set_experience(int32_t(progress_old * calculate_required_experience(level)));
        }

        void entity::add_level(int32_t level) {
            set_level(get_level() + level);
        }

        void entity::reduce_level(int32_t level) {
            set_level(get_level() - level);
        }

        int32_t entity::get_experience() const {
            auto it = nbt.find("experience");
            if (it == nbt.end())
                return 0;
            else
                return it->second;
        }

        void entity::set_experience(int32_t experience) {
            int32_t add_levels = 0;

            auto required_exp = calculate_required_experience(get_level());
            for (; required_exp <= experience; required_exp = calculate_required_experience(get_level())) {
                experience -= required_exp;
                ++add_levels;
            }

            int32_t levels = get_level() + add_levels;
            while (experience < 0 && levels) {
                levels--;
                experience += calculate_required_experience(levels);
            }

            nbt["experience"] = experience;
            nbt["level"] = levels;

            float progress = float(1.0 / required_exp * experience);
            int32_t total = calculate_experience_from_level(levels) + experience;

            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_experience{
                    .bar = {progress},
                    .level = required_exp,
                    .total_experience = total
                };
        }

        void entity::add_experience(int32_t experience) {
            set_experience(get_experience() + experience);
        }

        void entity::reduce_experience(int32_t experience) {
            set_experience(get_experience() - experience);
        }

        int32_t entity::get_fall_distance() const {
            auto it = nbt.find("fall_distance");
            if (it == nbt.end())
                return 0;
            else
                return it->second;
        }

        void entity::set_fall_distance(int32_t fall_distance) {
            nbt["fall_distance"] = fall_distance;
        }

        uint8_t entity::get_selected_item() const {
            auto it = nbt.find("selected_item");
            if (it == nbt.end())
                return 0;
            else
                return it->second;
        }

        void entity::set_selected_item(uint8_t selected_item) {
            nbt["selected_item"] = selected_item;
            if (assigned_player)
                *assigned_player << api::packets::client_bound::play::set_held_slot{
                    .slot = selected_item
                };
        }

        void entity::move([[maybe_unused]] float side, [[maybe_unused]] float forward, [[maybe_unused]] bool jump, [[maybe_unused]] bool sneaking) {
            //TODO
        }

        void entity::look(float yaw, float pitch) {
            set_head_rotation({yaw, pitch});
        }

        void entity::look_at(float x, float y, float z) {
            set_head_rotation(util::direction(handle.get<api::ecs::com::position>(), util::VECTOR{x, y, z}));
        }

        void entity::look_at(util::VECTOR pos) {
            set_head_rotation(util::direction(handle.get<api::ecs::com::position>(), pos));
        }

        void entity::look_at(ecs::entity entity) {
            if (api::entity(entity).current_world() == current_world())
                look_at(entity.get<api::ecs::com::position>());
        }

        util::VECTOR entity::get_motion() const {
            return handle.get<api::ecs::com::motion>();
        }

        void entity::set_motion(util::VECTOR mot) {
            if (current_world())
                current_world()->entity_motion_changes(handle, mot);
            *handle.modify<api::ecs::com::motion>() = mot;
        }

        void entity::add_motion(util::VECTOR mot) {
            set_motion(get_motion() += mot);
        }

        util::ANGLE_DEG entity::get_rotation() const {
            return handle.get<api::ecs::com::rotation>();
        }

        void entity::set_rotation(util::ANGLE_DEG rot) {
            if (current_world())
                current_world()->entity_rotation_changes(handle, rot);
            *handle.modify<api::ecs::com::rotation>() = rot;
        }

        void entity::add_rotation(util::ANGLE_DEG rot) {
            set_rotation(get_rotation() += rot);
        }

        util::ANGLE_DEG entity::get_head_rotation() const {
            return handle.get<api::ecs::com::head_rotation>();
        }

        void entity::set_head_rotation(util::ANGLE_DEG rot) {
            if (current_world())
                current_world()->entity_look_changes(handle, rot);
            *handle.modify<api::ecs::com::head_rotation>() = rot;
        }

        void entity::add_head_rotation(util::ANGLE_DEG rot) {
            set_head_rotation(get_head_rotation() += rot);
        }

        void entity::attack_from_this([[maybe_unused]] ecs::entity entity) {
        }

        void entity::breaking_block([[maybe_unused]] int64_t global_x, [[maybe_unused]] uint64_t global_y, [[maybe_unused]] int64_t global_z, [[maybe_unused]] uint32_t time) {
        }

        void entity::place_block([[maybe_unused]] int64_t global_x, [[maybe_unused]] uint64_t global_y, [[maybe_unused]] int64_t global_z, [[maybe_unused]] const base_objects::block&) {
        }

        void entity::place_block([[maybe_unused]] int64_t global_x, [[maybe_unused]] uint64_t global_y, [[maybe_unused]] int64_t global_z, [[maybe_unused]] base_objects::const_block_entity_ref) {
        }

        void entity::place_block([[maybe_unused]] int64_t global_x, [[maybe_unused]] uint64_t global_y, [[maybe_unused]] int64_t global_z, [[maybe_unused]] base_objects::block_entity&&) {
        }

        ecs::entity entity::create(int32_t id) {
            auto it = entity_data::get_entity(id);
            ecs::entity res = ecs::global_registry::create_entity_and_wait(it.recipe);
            if (it.create_callback)
                it.create_callback(res);
            return res;
        }

        ecs::entity entity::create(int32_t id, const enbt::compound_const_ref& nbt) {
            auto it = entity_data::get_entity(id);
            ecs::entity res = ecs::global_registry::create_entity_and_wait(it.recipe);
            res.modify<ecs::com::nbt>()->data = nbt;
            if (it.create_callback)
                it.create_callback(res);
            return res;
        }

        ecs::entity entity::create(const std::string& id) {
            auto it = entity_data::get_entity(id);
            ecs::entity res = ecs::global_registry::create_entity_and_wait(it.recipe);
            if (it.create_callback)
                it.create_callback(res);
            return res;
        }

        ecs::entity entity::create(const std::string& id, const enbt::compound_const_ref& nbt) {
            auto it = entity_data::get_entity(id);
            ecs::entity res = ecs::global_registry::create_entity_and_wait(it.recipe);
            res.modify<ecs::com::nbt>()->data = nbt;
            if (it.create_callback)
                it.create_callback(res);
            return res;
        }

        std::optional<int32_t> entity::get_object_field() const {
            auto& obj_field_getter = entity_data::view(handle).get_object_field;
            if (!obj_field_getter)
                return std::nullopt;
            return obj_field_getter(handle);
        }

        bool entity::is_player() const {
            return handle.get<ecs::com::entity_type>().type == entity_data::player_entity_id;
        }

        void entity::store_to_file(ecs::entity entity, enbt::io_helper::value_write_stream& w) {
            enbt::io_helper::serialization_write(entity, w);
        }

        ecs::entity entity::load_from_file(enbt::io_helper::value_read_stream& w) {
            ecs::entity res = ecs::global_registry::create_entity_and_wait(ecs::entity_recipe().freeze()); //TODO use something else for this
            enbt::io_helper::serialization_read(res, w);
            try {
                auto load_callback = entity_data::view(res).load_callback;
                if (load_callback)
                    load_callback(res);
            } catch (...) {
                if (api::entity(res).current_world())
                    api::entity(res).current_world()->unregister_entity(res);
                throw;
            }
            return res;
        }

        void entity::store_to_enbt(ecs::entity entity, enbt::compound& w) {
            enbt::value tmp;
            enbt::io_helper::serialization_write(entity, tmp);
            w = std::move(tmp);
        }

        ecs::entity entity::load_from_enbt(const enbt::compound_const_ref& nbt) {
            ecs::entity res = ecs::global_registry::create_entity_and_wait(ecs::entity_recipe().freeze()); //TODO use something else for this
            enbt::io_helper::serialization_read(res, (const enbt::value&)nbt);
            try {
                auto load_callback = entity_data::view(res).load_callback;
                if (load_callback)
                    load_callback(res);
            } catch (...) {
                if (api::entity(res).current_world())
                    api::entity(res).current_world()->unregister_entity(res);
                throw;
            }
            return res;
        }
    }
}
