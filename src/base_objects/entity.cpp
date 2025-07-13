#include <library/fast_task.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/packets.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/entity.hpp>
#include <src/storage/world_data.hpp>
#include <src/util/calculations.hpp>

namespace copper_server {
    namespace base_objects {
        struct entities_storage {
            std::unordered_map<uint16_t, entity_data> _registry;
            std::unordered_map<std::string, uint16_t> _name_to_id;
            std::unordered_map<std::string, std::shared_ptr<entity_data::world_processor>> entity_processors;
            uint16_t id_adder = 0;
        };

        fast_task::protected_value<entities_storage> data_for_entities;

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
            });
        }

        storage::world_data* entity::current_world() const {
            return world_syncing_data ? world_syncing_data->world : nullptr;
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

            if (nbt.contains("ride_by_entity")) {
                enbt::fixed_array arr;
                arr.reserve(ride_by_entity.size());
                for (auto& value : ride_by_entity)
                    arr.push_back(value->copy_to_enbt());
                res["ride_by_entity"] = std::move(arr);
            }

            if (attached_to) {
                std::visit(
                    [&res](auto& it) {
                        if constexpr (std::is_same_v<std::decay_t<decltype(it)>, enbt::raw_uuid>)
                            res["attached_to"] = it;
                        else
                            res["attached_to"] = it->id;
                    },
                    *attached_to
                );
            }

            if (attached.size()) {
                enbt::fixed_array arr;
                arr.reserve(attached.size());
                for (auto& it : attached) {
                    std::visit(
                        [&arr](auto& it) {
                            if constexpr (std::is_same_v<std::decay_t<decltype(it)>, enbt::raw_uuid>)
                                arr.push_back(it);
                            else
                                arr.push_back(it->id);
                        },
                        it
                    );
                }

                res["attached"] = std::move(arr);
            }
            return res;
        }

        void resolve_entity(std::variant<entity_ref, enbt::raw_uuid>& it) {
            if (std::holds_alternative<enbt::raw_uuid>(it)) {
                auto entity = api::entity_id_map::get_entity(std::get<enbt::raw_uuid>(it));
                if (entity)
                    it = entity;
            }
        }

        void reduce_effects(std::unordered_map<uint32_t, list_array<entity::effect>>& hidden_effects, std::unordered_map<uint32_t, entity::effect>& active_effects) {
            list_array<uint32_t> expired_effects;

            for (auto& [id, effect] : active_effects) {
                if (!effect.duration) {
                    expired_effects.push_back(id);
                    continue;
                }
                if (effect.duration != UINT32_MAX)
                    effect.duration--;
            }

            for (auto& [id, effects] : hidden_effects) {
                for (auto& effect : effects) {
                    if (!effect.duration)
                        continue;
                    if (effect.duration != UINT32_MAX)
                        effect.duration--;
                }

                effects.remove_if([](const entity::effect& effect) {
                    return !effect.duration;
                });
                effects.sort([](const entity::effect& effect0, const entity::effect& effect1) {
                    return effect0.amplifier > effect1.amplifier;
                });
            }
            for (auto& id : expired_effects) {
                if (hidden_effects.contains(id))
                    active_effects.at(id) = hidden_effects.at(id).take_front();
                else
                    active_effects.erase(id);
            }
        }

        void entity::tick() {
            if (world_syncing_data) {
                if (attached_to)
                    resolve_entity(*attached_to);

                for (auto& it : attached)
                    resolve_entity(it);

                auto proc = const_data().processor;
                if (proc)
                    if (proc->on_tick)
                        proc->on_tick(*this);
                reduce_effects(hidden_effects, active_effects);
            }
        }

        bool entity::kill() {
            if (!const_data().pre_death_callback(*this, false))
                return false;

            died = true;
            if (world_syncing_data)
                world_syncing_data->world->entity_death(*this);
            return true;
        }

        void entity::force_kill() {
            const_data().pre_death_callback(*this, true);
            died = true;
            if (world_syncing_data)
                world_syncing_data->world->entity_death(*this);
        }

        void entity::erase() {
            died = true;
            if (world_syncing_data)
                world_syncing_data->world->entity_deinit(*this);
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

            if (nbt.contains("ride_by_entity")) {
                auto ride_by_entity = nbt["ride_by_entity"].as_array();
                res->ride_by_entity.reserve(ride_by_entity.size());
                for (auto& value : ride_by_entity) {
                    auto ride_entity = entity::load_from_enbt(value.as_compound());
                    res->world_syncing_data->world->register_entity(ride_entity);
                    ride_entity->set_ride_entity(res);
                }
            }

            if (nbt.contains("attached_to")) {
                auto& attached_to = nbt["attached_to"];
                if (attached_to.is_uuid())
                    res->attached_to = (enbt::raw_uuid)nbt["attached_to"];
            }

            if (nbt.contains("attached")) {
                auto attached_enbt = nbt["attached"].as_array();
                res->attached.reserve(attached_enbt.size());
                for (auto& value : attached_enbt)
                    res->attached.push_back((enbt::raw_uuid)value);
            }
            return res;
        }

        void entity::teleport(util::VECTOR pos) {
            if (world_syncing_data)
                world_syncing_data->world->entity_teleport(*this, pos);
            position = pos;
        }

        void entity::teleport(util::VECTOR pos, float yaw, float pitch) {
            if (world_syncing_data) {
                world_syncing_data->world->entity_teleport(*this, pos);
                world_syncing_data->world->entity_rotation_changes(*this, {yaw, pitch});
            }
            position = pos;
            rotation = {yaw, pitch};
        }

        void entity::teleport(util::VECTOR pos, float yaw, float pitch, bool on_ground) {
            if (world_syncing_data) {
                world_syncing_data->world->entity_teleport(*this, pos);
                world_syncing_data->world->entity_rotation_changes(*this, {yaw, pitch});
            }
            position = pos;
            rotation = {yaw, pitch};
            set_on_ground(on_ground);
        }

        void entity::set_ride_entity(entity_ref entity) {
            if (entity && world_syncing_data) {
                if (entity->world_syncing_data->world == world_syncing_data->world) {
                    world_syncing_data->world->entity_rides(*this, entity->world_syncing_data->assigned_world_id);
                    ride_entity = entity;
                    return;
                }
            }
            ride_entity = std::nullopt;
        }

        void entity::remove_ride_entity() {
            if (ride_entity && world_syncing_data) {
                if ((*ride_entity)->world_syncing_data->world == world_syncing_data->world) {
                    world_syncing_data->world->entity_leaves_ride(*this, (*ride_entity)->world_syncing_data->assigned_world_id);
                    ride_entity = nullptr;
                    return;
                }
            }
            ride_entity = std::nullopt;
        }

        void entity::add_effect(uint32_t id, uint32_t duration, uint8_t amplifier, bool ambient, bool show_particles, bool show_icon, bool use_blend) {
            entity::effect to_add_effect{
                .duration = duration,
                .id = id,
                .amplifier = amplifier,
                .ambient = ambient,
                .particles = show_particles,
                .show_icon = show_icon,
                .use_blend = use_blend,
            };
            if (auto it = active_effects.find(id); it != active_effects.end()) {
                auto& effect = it->second;
                if (effect.amplifier >= amplifier) {
                    if (effect.duration < duration)
                        hidden_effects[id].push_back(to_add_effect);
                    if (world_syncing_data)
                        world_syncing_data->world->entity_add_effect(*this, id, duration, amplifier, ambient, show_particles, show_icon, use_blend);
                    return;
                } else
                    hidden_effects[id].push_back(effect);
            }
            active_effects[id] = to_add_effect;
            if (world_syncing_data)
                world_syncing_data->world->entity_add_effect(*this, id, duration, amplifier, ambient, show_particles, show_icon, use_blend);
        }

        void entity::remove_effect(uint32_t id) {
            active_effects.erase(id);
            hidden_effects.erase(id);
            if (world_syncing_data)
                world_syncing_data->world->entity_remove_effect(*this, id);
        }

        void entity::remove_all_effects() {
            if (world_syncing_data)
                for (auto& [id, effect] : active_effects)
                    world_syncing_data->world->entity_remove_effect(*this, id);
            active_effects.clear();
            hidden_effects.clear();
        }

        bool entity::is_sleeping() const {
            return world_syncing_data ? world_syncing_data->is_sleeping : false;
        }

        bool entity::is_on_ground() const {
            return world_syncing_data ? world_syncing_data->on_ground : false;
        }

        bool entity::is_sneaking() const {
            return world_syncing_data ? world_syncing_data->is_sneaking : false;
        }

        bool entity::is_sprinting() const {
            return world_syncing_data ? world_syncing_data->is_sprinting : false;
        }

        void entity::set_sleeping(bool sleeping) {
            if (world_syncing_data)
                world_syncing_data->is_sleeping = sleeping;
        }

        void entity::set_on_ground(bool on_ground) {
            if (world_syncing_data)
                world_syncing_data->on_ground = on_ground;
        }

        void entity::set_sneaking(bool sneaking) {
            if (world_syncing_data)
                world_syncing_data->is_sneaking = sneaking;
        }

        void entity::set_sprinting(bool sprinting) {
            if (world_syncing_data)
                world_syncing_data->is_sprinting = sprinting;
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
                api::packets::play::setHealth(*assigned_player, health, get_food(), get_saturation());

            if (health <= 0.0f)
                force_kill();
        }

        void entity::add_health(float health) {
            set_health(get_health() + health);
        }

        void entity::damage(float health, int32_t type_id, std::optional<util::VECTOR> pos) {
            if (world_syncing_data)
                world_syncing_data->world->entity_damage(*this, health, type_id, pos);
            reduce_health(health);
        }

        void entity::damage(float health, int32_t type_id, entity_ref& source, std::optional<util::VECTOR> pos) {
            if (world_syncing_data)
                world_syncing_data->world->entity_damage(*this, health, type_id, source, pos);
            reduce_health(health);
        }

        void entity::damage(float health, int32_t type_id, entity_ref& source, entity_ref& source_direct, std::optional<util::VECTOR> pos) {
            if (world_syncing_data)
                world_syncing_data->world->entity_damage(*this, health, type_id, source, source_direct, pos);
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
                api::packets::play::setHealth(*assigned_player, get_health(), food, get_saturation());
        }

        void entity::add_food(uint8_t food) {
            set_food(get_health() + food);
        }

        void entity::reduce_food(uint8_t food) {
            set_food(get_health() - food);
        }

        float entity::get_saturation() const {
            return nbt.at("saturation");
        }

        void entity::set_saturation(float saturation) {
            nbt.at("saturation") = saturation;
            if (assigned_player)
                api::packets::play::setHealth(*assigned_player, get_health(), get_food(), saturation);
        }

        void entity::add_saturation(float saturation) {
            set_saturation(get_saturation() - saturation);
        }

        void entity::reduce_saturation(float saturation) {
            set_saturation(get_saturation() - saturation);
        }

        float entity::get_breath() const {
            auto it = nbt.find("breath");
            if (it == nbt.end())
                return 0;
            else
                return it->second;
        }

        void entity::set_breath(float breath) {
            nbt["breath"] = breath;
            if (const_data().metadata.contains("AIR"))
                if (assigned_player)
                    api::packets::play::setEntityMetadata(*assigned_player, protocol_id, {}); //todo
        }

        void entity::add_breath(float breath) {
            set_breath(get_breath() + breath);
        }

        void entity::reduce_breath(float breath) {
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
                    required_exp = 2.5 * required_exp * required_exp - 40.5 * required_exp + 360;
                    break;
                default:
                    required_exp = 4.5 * required_exp * required_exp - 162.5 * required_exp + 2220;
                break;
            }
            // clang-format on
            return required_exp;
        }

        void entity::set_level(int32_t level) {
            int32_t old_lvl = nbt["level"];
            nbt["level"] = level;
            float progress_old = 1.0 / calculate_required_experience(old_lvl) * get_experience();
            set_experience(progress_old * calculate_required_experience(level));
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

            float progress = 1.0 / required_exp * experience;
            int32_t total = calculate_experience_from_level(levels) + experience;

            if (assigned_player)
                api::packets::play::setExperience(*assigned_player, progress, required_exp, total);
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
                api::packets::play::setHeldSlot(*assigned_player, selected_item);
        }

        void entity::move(float side, float forward, bool jump, bool sneaking) {
            //TODO
        }

        void entity::look(float yaw, float pitch) {
            set_head_rotation({yaw, pitch});
        }

        void entity::look_at(float x, float y, float z) {
            set_head_rotation(util::direction(position, util::VECTOR{x, y, z}));
        }

        void entity::look_at(util::VECTOR pos) {
            set_head_rotation(util::direction(position, pos));
        }

        void entity::look_at(const entity_ref& entity) {
            if (entity) {
                if (entity->world_syncing_data && world_syncing_data) {
                    if (entity->world_syncing_data->world == world_syncing_data->world) {
                        look_at(entity->position);
                    }
                }
            }
        }

        util::VECTOR entity::get_motion() const {
            return motion;
        }

        void entity::set_motion(util::VECTOR mot) {
            motion = mot;
            if (world_syncing_data)
                if (world_syncing_data->world)
                    world_syncing_data->world->entity_motion_changes(*this, mot);
        }

        void entity::add_motion(util::VECTOR mot) {
            set_motion(get_motion() += mot);
        }

        util::ANGLE_DEG entity::get_rotation() const {
            return rotation;
        }

        void entity::set_rotation(util::ANGLE_DEG rot) {
            rotation = rot;
            if (world_syncing_data)
                if (world_syncing_data->world)
                    world_syncing_data->world->entity_rotation_changes(*this, rot);
        }

        void entity::add_rotation(util::ANGLE_DEG rot) {
            set_rotation(get_rotation() += rot);
        }

        util::ANGLE_DEG entity::get_head_rotation() const {
            return head_rotation;
        }

        void entity::set_head_rotation(util::ANGLE_DEG rot) {
            head_rotation = rot;
            if (world_syncing_data)
                if (world_syncing_data->world)
                    world_syncing_data->world->entity_look_changes(*this, rot);
        }

        void entity::add_head_rotation(util::ANGLE_DEG rot) {
            set_head_rotation(get_head_rotation() += rot);
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
