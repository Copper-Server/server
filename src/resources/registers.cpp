#include <boost/iostreams/filter/zstd.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <library/enbt/io_tools.hpp>
#include <resources/include.hpp>
#include <src/api/configuration.hpp>
#include <src/api/recipe.hpp>
#include <src/api/tags.hpp>
#include <src/base_objects/data_packs/known_pack.hpp>
#include <src/base_objects/entity.hpp>
#include <src/log.hpp>
#include <src/registers.hpp>
#include <src/util/conversions.hpp>
#include <src/util/json_helpers.hpp>

namespace copper_server::resources {
    std::unordered_map<uint32_t, boost::json::object> internal_compatibility_versions;
    list_array<base_objects::data_packs::known_pack> loaded_packs_;
    int32_t latest_protocol_version = -1;

    list_array<base_objects::data_packs::known_pack> loaded_packs() {
        return loaded_packs_;
    }

    template <class T, class Iterator>
    void id_assigner(std::unordered_map<std::string, T>& map, list_array<Iterator>& cache) {
        size_t i = 0;
        auto it = map.begin();
        auto end = map.end();
        cache.reserve(map.size());
        for (; it != end; ++it) {
            it->second.id = i++;
            cache.push_back(it);
        }
    }

    namespace processor {
        std::string process_replace(std::string_view matched, std::string_view replace) {
            auto insert = replace.find('$');
            if (insert != std::string::npos) {
                auto replace_left = replace.substr(0, insert);
                auto replace_right = replace.substr(insert + 1);
                return std::string(replace_left) + std::string(matched) + std::string(replace_right);
            } else
                return std::string(replace);
        }

        std::optional<std::string> process_match(std::string_view str, std::string_view match, std::string_view replace) {
            if (match.ends_with('*')) {
                if (str.starts_with(match.substr(0, match.size() - 1)))
                    return process_replace(str.substr(match.size() - 1), replace);
                else
                    return std::nullopt;
            } else if (match.starts_with('*')) {
                if (str.ends_with(match.substr(1)))
                    return process_replace(str.substr(0, str.size() - match.size()), replace);
                else
                    return std::nullopt;
            } else {
                auto split = match.find('*');
                if (split == std::string::npos) {
                    if (str == match)
                        return std::string(replace);
                    else
                        return std::nullopt;
                } else {
                    std::string_view left = match.substr(0, split);
                    std::string_view right = match.substr(split + 1);

                    if (str.starts_with(left) && str.ends_with(right))
                        return process_replace(str.substr(left.size(), str.size() - left.size() - right.size()), replace);
                    else
                        return std::nullopt;
                }
            }
        }
    }

    template <class T, size_t length>
    void check_override(std::unordered_map<std::string, T>& map, const std::string& id, const char (&type_name)[length]) {
        auto it = map.find(id);
        if (it != map.end())
            if (!it->second.allow_override)
                throw std::runtime_error("This " + std::string(type_name) + " is already defined and does not allow override. [" + id + "]");
    }

    template <class T, size_t length>
    void check_conflicts(std::unordered_map<std::string, T>& map, const std::string& id, const char (&type_name)[length]) {
        auto it = map.find(id);
        if (it != map.end())
            throw std::runtime_error("This " + std::string(type_name) + " is already defined and cannot be overriden. [" + id + "]");
    }

    template <class T>
    void check_override(std::unordered_map<std::string, T>& map, const std::string& id, const std::string& type_name) {
        auto it = map.find(id);
        if (it != map.end())
            if (!it->second.allow_override)
                throw std::runtime_error("This " + type_name + " is already defined and does not allow override. [" + id + "]");
    }

    template <class T>
    void check_conflicts(std::unordered_map<std::string, T>& map, const std::string& id, const std::string& type_name) {
        auto it = map.find(id);
        if (it != map.end())
            throw std::runtime_error("This " + type_name + " is already defined and cannot be overriden. [" + id + "]");
    }

    using namespace util;
    using namespace registers;

    void registers_reset() {
        biomes.clear();
        biomes.clear();
        chatTypes.clear();
        armorTrimPatterns.clear();
        armorTrimMaterials.clear();
        wolfSoundVariants.clear();
        wolfVariants.clear();
        catVariants.clear();
        chickenVariants.clear();
        cowVariants.clear();
        pigVariants.clear();
        frogVariants.clear();
        dimensionTypes.clear();
        damageTypes.clear();
        bannerPatterns.clear();
        paintingVariants.clear();
        instruments.clear();
        biomes_cache.clear();
        biomes_cache.clear();
        chatTypes_cache.clear();
        armorTrimPatterns_cache.clear();
        armorTrimMaterials_cache.clear();
        wolfSoundVariants_cache.clear();
        wolfVariants_cache.clear();
        catVariants_cache.clear();
        chickenVariants_cache.clear();
        cowVariants_cache.clear();
        pigVariants_cache.clear();
        frogVariants_cache.clear();
        dimensionTypes_cache.clear();
        damageTypes_cache.clear();
        bannerPatterns_cache.clear();
        paintingVariants_cache.clear();
        instruments_cache.clear();
        api::tags::loading_stage_begin();
    }

    void load_registers_complete() {
        id_assigner(biomes, biomes_cache);
        id_assigner(chatTypes, chatTypes_cache);
        id_assigner(armorTrimPatterns, armorTrimPatterns_cache);
        id_assigner(armorTrimMaterials, armorTrimMaterials_cache);
        id_assigner(wolfSoundVariants, wolfSoundVariants_cache);
        id_assigner(wolfVariants, wolfVariants_cache);
        id_assigner(catVariants, catVariants_cache);
        id_assigner(chickenVariants, chickenVariants_cache);
        id_assigner(cowVariants, cowVariants_cache);
        id_assigner(pigVariants, pigVariants_cache);
        id_assigner(frogVariants, frogVariants_cache);
        id_assigner(dimensionTypes, dimensionTypes_cache);
        id_assigner(damageTypes, damageTypes_cache);
        id_assigner(bannerPatterns, bannerPatterns_cache);
        id_assigner(paintingVariants, paintingVariants_cache);
        id_assigner(instruments, instruments_cache);
        api::tags::loading_stage_end();
    }

    void hardcoded_values_for_entity(base_objects::entity_data& data) {
        if (data.living_entity_data || data.name == "player") {
            data.acceleration = 0.08;
            data.drag_vertical = 0.02;
            data.drag_horizontal = 0.09;
            data.terminal_velocity = 3.92;
            data.drag_applied_after_acceleration = true;
            data.loading_ticket_level = 31;
        } else {
            if (data.name == "experience_bottle") {
                data.acceleration = 0.07;
                data.drag_vertical = 0.01;
                data.drag_horizontal = 0.01;
                data.terminal_velocity = 7.0;
                data.drag_applied_after_acceleration = false;
            } else if (data.name == "llama_spit") {
                data.acceleration = 0.06;
                data.drag_vertical = 0.01;
                data.drag_horizontal = 0.01;
                data.terminal_velocity = 6.0;
                data.drag_applied_after_acceleration = false;
            } else if (data.name == "splash_potion" || data.name == "lingering_potion" || data.name == "arrow" || data.name == "trident") {
                data.acceleration = 0.05;
                data.drag_vertical = 0.01;
                data.drag_horizontal = 0.01;
                data.terminal_velocity = 5.0;
                data.drag_applied_after_acceleration = false;
            } else if (data.name == "egg" || data.name == "snowball" || data.name == "ender_pearl") {
                data.acceleration = 0.03;
                data.drag_vertical = 0.01;
                data.drag_horizontal = 0.01;
                data.terminal_velocity = 3.0;
                data.drag_applied_after_acceleration = false;
            } else if (data.name == "tnt" || data.name == "falling_block" || data.name == "item") {
                data.acceleration = 0.04;
                data.drag_vertical = 0.02;
                data.drag_horizontal = 0.02;
                data.terminal_velocity = 1.96;
                data.drag_applied_after_acceleration = true;
            } else if (data.name == "fireball" || data.name == "small_fireball" || data.name == "dragon_fireball" || data.name == "wither_skull") {
                data.acceleration = 0.10;
                data.drag_vertical = 0.05;
                data.drag_horizontal = 0.05;
                data.terminal_velocity = 1.9;
                data.drag_applied_after_acceleration = true;
            } else if (data.name == "experience_orb") {
                data.acceleration = 0.03;
                data.drag_vertical = 0.02;
                data.drag_horizontal = 0.02;
                data.terminal_velocity = 1.47;
                data.drag_applied_after_acceleration = true;
            } else if (data.name == "fishing_bobber") {
                data.acceleration = 0.03;
                data.drag_vertical = 0.08;
                data.drag_horizontal = 0.08;
                data.terminal_velocity = 0.345;
                data.drag_applied_after_acceleration = true;
            } else if (data.name.contains("minecart")) {
                data.acceleration = 0.04;
                data.drag_vertical = 0.05;
                data.drag_horizontal = 0.05;
                data.terminal_velocity = 0.76;
                data.drag_applied_after_acceleration = true;
            } else if (data.name.contains("boat") || data.name == "bamboo_chest_raft" || data.name == "bamboo_raft") {
                data.acceleration = 0.04;
                data.drag_vertical = 0.00;
                data.drag_horizontal = 0.00;
                data.terminal_velocity = 3400000.0f;
                data.drag_applied_after_acceleration = true;
            } else if (data.name.contains("boat")) {
                data.acceleration = 0.04;
                data.drag_vertical = 0.00;
                data.drag_horizontal = 0.00;
                data.terminal_velocity = 3400000.0f;
                data.drag_applied_after_acceleration = true;
            } else if (data.name.contains("wind_charge")) {
                data.acceleration = 0.04;
                data.drag_vertical = 0.00;
                data.drag_horizontal = 0.00;
                data.terminal_velocity = 3400000.0f;
                data.drag_applied_after_acceleration = true;
            } else {
                data.acceleration = 0.00;
                data.drag_vertical = 0.00;
                data.drag_horizontal = 0.00;
                data.terminal_velocity = 0.0;
                data.drag_applied_after_acceleration = true;
                log::debug("resource_load", "Entity " + data.name + " has no hardcoded values");
            }
        }
    }

    void initialize_entities() {
        auto parsed = boost::json::parse(resources::registry::entities);
        for (auto& [id, obj_] : parsed.as_object()) {
            auto& obj = obj_.as_object();
            base_objects::entity_data data;
            data.id = "minecraft:" + std::string(id);
            if (obj.contains("max_health")) {
                base_objects::entity_data::living_entity_data_t living_data;
                living_data.base_health = obj.at("max_health").to_number<float>();
                living_data.can_freeze = obj.at("can_freeze").as_bool();
                living_data.can_hit = obj.at("can_hit").as_bool();
                living_data.is_collidable = obj.at("is_collidable").as_bool();
                living_data.is_attackable = obj.at("attackable").as_bool();
                if (obj.contains("inventory_size"))
                    living_data.inventory_size = obj.at("inventory_size").to_number<int32_t>();
                if (obj.contains("max_air"))
                    living_data.max_air = obj.at("max_air").to_number<uint16_t>();
                if (obj.contains("can_avoid_traps"))
                    living_data.can_avoid_traps = obj.at("can_avoid_traps").as_bool();
                if (obj.contains("can_be_hit_by_projectile"))
                    living_data.can_be_hit_by_projectile = obj.at("can_be_hit_by_projectile").as_bool();
                {
                    auto tasks = obj.at("brain_tasks").as_array();
                    //TODO
                }
                {
                    auto sensors = obj.at("brain_sensors").as_array();
                    living_data.brain_sensors.reserve(sensors.size());
                    for (auto& sensor : sensors)
                        living_data.brain_sensors.push_back((std::string)sensor.as_string());
                }
                {
                    auto memories = obj.at("brain_memories").as_array();
                    living_data.brain_memories.reserve(memories.size());
                    for (auto& memory : memories)
                        living_data.brain_memories.push_back((std::string)memory.as_string());
                }
                data.living_entity_data = std::move(living_data);
            }

            data.is_summonable = obj.at("summonable").as_bool();
            data.is_fire_immune = obj.at("fire_immune").as_bool();
            data.is_saveable = obj.at("saveable").as_bool();
            data.is_spawnable_far_from_player = obj.at("spawnable_far_from_player").as_bool();
            data.max_track_distance = obj.at("max_track_distance").to_number<int32_t>();
            data.track_tick_interval = obj.at("track_tick_interval").to_number<int32_t>();
            data.spawn_group = obj.at("spawn_group").as_string();
            {
                auto dimension = obj.at("dimension").as_array();
                data.base_bounds.xz = dimension.at(0).to_number<double>();
                data.base_bounds.y = dimension.at(1).to_number<double>();
            }
            if (obj.contains("loot_table"))
                data.data["loot_table"] = util::conversions::json::from_json(obj.at("loot_table"));

            {
                using type = base_objects::entity_data::metadata_sync::type_t;
                //global_pos,
                static std::unordered_map<std::string, type> types = {
                    {"Byte", type::byte},
                    {"Integer", type::varint},
                    {"Long", type::varlong},
                    {"Boolean", type::boolean},
                    {"Float", type::_float},
                    {"OptionalInt", type::opt_varint},
                    {"NBT", type::nbt},
                    {"NbtCompound", type::nbt},
                    {"String", type::_string},

                    {"Text", type::text},
                    {"Optional<Text>", type::opt_text},

                    {"ItemStack", type::slot},

                    {"EulerAngle", type::euler_angle},
                    {"Vector3f", type::vector3},
                    {"Quaternionf", type::quaternion},

                    {"BlockPos", type::postion},
                    {"Optional<BlockPos>", type::opt_position},

                    {"Direction", type::direction},
                    {"EntityPose", type::pose},
                    {"BlockState", type::block_state},
                    {"Optional<BlockState>", type::opt_block_state},

                    {"VillagerData", type::villager_data},

                    {"ParticleEffect", type::particle},
                    {"List<ParticleEffect>", type::particles},

                    {"SnifferEntity$State", type::sniffer_state},
                    {"ArmadilloEntity$State", type::armadillo_state},
                    {"RegistryEntry<CatVariant>", type::cat_variant},
                    {"RegistryEntry<ChickenVariant>", type::chicken_variant},
                    {"RegistryEntry<CowVariant>", type::cow_variant},
                    {"RegistryEntry<FrogVariant>", type::frog_variant},
                    {"RegistryEntry<PaintingVariant>", type::painting_variant},
                    {"RegistryEntry<PigVariant>", type::pig_variant},
                    {"RegistryEntry<WolfVariant>", type::wolf_variant},
                    {"RegistryEntry<WolfSoundVariant>", type::wolf_sound_variant},

                    {"Optional<LazyEntityReference<LivingEntity>>", type::entity_refrence},
                };
                auto metadata = obj.at("metadata").as_array();
                data.metadata.reserve(metadata.size());
                for (auto& meta : obj.at("metadata").as_array()) {
                    base_objects::entity_data::metadata_sync metadata_sync;
                    metadata_sync.index = meta.at("network_id").to_number<uint8_t>();
                    std::string name = (std::string)meta.at("field_name").as_string();
                    metadata_sync.type = types.at((std::string)meta.at("type_name").as_string());
                    data.metadata[name] = std::move(metadata_sync);
                }
            }

            data.name = (std::string)id;
            data.translation_resource_key = obj.at("translation_key").as_string();
            data.eye_height = obj.at("eye_height").to_number<float>();

            hardcoded_values_for_entity(data);
            base_objects::entity_data::register_entity(std::move(data));
        }
    }

    void load_file_biomes(js_object&& bio_js, const std::string& id, bool send_via_network_body = true) {
        check_override(biomes, id, "biome");
        Biome bio;
        bio.send_via_network_body = send_via_network_body;
        bio.downfall = bio_js["downfall"];
        bio.temperature = bio_js["temperature"];
        bio.has_precipitation = bio_js["has_precipitation"];
        bio.creature_spawn_probability = bio_js["creature_spawn_probability"].or_apply(0.0);
        if (bio_js.contains("temperature_modifier"))
            bio.temperature_modifier = (std::string)bio_js["temperature_modifier"];

        {
            Biome::Effects effects;
            js_object effects_js = js_object::get_object(bio_js["effects"]);
            effects.sky_color = effects_js["sky_color"];
            effects.water_fog_color = effects_js["water_fog_color"];
            effects.fog_color = effects_js["fog_color"];
            effects.water_color = effects_js["water_color"];
            if (effects_js.contains("foliage_color"))
                effects.foliage_color = effects_js["foliage_color"];
            if (effects_js.contains("grass_color"))
                effects.grass_color = effects_js["grass_color"];
            if (effects_js.contains("grass_color_modifier"))
                effects.grass_color_modifier = (std::string)effects_js["grass_color_modifier"];
            if (effects_js.contains("particle")) {
                js_object particle_js = js_object::get_object(effects_js["particle"]);
                Biome::Particle particle;
                particle.probability = particle_js["probability"];
                auto options = js_object::get_object(particle_js["options"]);
                particle.options.type = (std::string)options["type"];
                particle.options.options = conversions::json::from_json(options.get());
                effects.particle = std::move(particle);
            }
            if (effects_js.contains("ambient_sound")) {
                auto eff = effects_js["ambient_sound"];
                if (eff.is_string())
                    effects.ambient_sound = (std::string)eff;
                else {
                    js_object ambient_sound_js = js_object::get_object(effects_js["ambient_sound"]);
                    Biome::AmbientSound ambient_sound;
                    ambient_sound.sound = (std::string)ambient_sound_js["sound"];
                    ambient_sound.range = ambient_sound_js["range"];
                    effects.ambient_sound = std::move(ambient_sound);
                }
            }
            if (effects_js.contains("mood_sound")) {
                js_object mood_sound_js = js_object::get_object(effects_js["mood_sound"]);
                Biome::MoodSound mood_sound;
                mood_sound.sound = (std::string)mood_sound_js["sound"];
                mood_sound.offset = mood_sound_js["offset"].or_apply(2.0);
                mood_sound.block_search_extent = mood_sound_js["block_search_extent"].or_apply(8);
                mood_sound.tick_delay = mood_sound_js["tick_delay"].or_apply(6000);
                effects.mood_sound = std::move(mood_sound);
            }
            if (effects_js.contains("additions_sound")) {
                js_object additions_sound_js = js_object::get_object(effects_js["additions_sound"]);
                Biome::AdditionsSound additions_sound;
                additions_sound.sound = (std::string)additions_sound_js["sound"];
                additions_sound.tick_chance = additions_sound_js["tick_chance"];
                effects.additions_sound = std::move(additions_sound);
            }
            if (effects_js.contains("music")) {
                std::vector<Biome::Music> music_arr;
                auto music_arr_js = js_array::get_array(effects_js["music"]);
                music_arr.reserve(music_arr_js.size());
                for (auto&& it : music_arr_js) {
                    js_object music_js = js_object::get_object(it);
                    Biome::Music music;
                    if (music_js.contains("data")) {
                        auto data = js_object::get_object(music_js["data"]);
                        music.sound = (std::string)data["sound"];
                        music.min_delay = data["min_delay"].or_apply(12000);
                        music.max_delay = data["max_delay"].or_apply(24000);
                        music.replace_current_music = data["replace_current_music"].or_apply(true);
                    } else {
                        music.sound = (std::string)music_js["sound"];
                        music.min_delay = music_js["min_delay"].or_apply(12000);
                        music.max_delay = music_js["max_delay"].or_apply(24000);
                        music.replace_current_music = music_js["replace_current_music"].or_apply(true);
                    }
                    music.music_weight = music_js["weight"].or_apply(1.0);
                    music_arr.emplace_back(std::move(music));
                }
                effects.music = std::move(music_arr);
            }
            bio.effects = std::move(effects);
        }
        {
            js_object cavers_js = js_object::get_object(bio_js["cavers"]);
            for (auto&& [name, values] : cavers_js) {
                if (values.is_string()) {
                    bio.carvers[name] = {values};
                } else {
                    auto& res = bio.carvers[name];
                    js_array arr = js_array::get_array(values);
                    res.reserve(arr.size());
                    for (auto&& it : arr)
                        res.push_back(it);
                }
            }
        }
        {
            auto features_js = js_array::get_array(bio_js["features"]);
            bio.features.reserve(features_js.size());
            for (auto&& items : features_js) {
                auto feature_js = js_array::get_array(items);
                std::vector<std::string> feature;
                feature.reserve(feature_js.size());
                for (auto&& it : feature_js)
                    feature.push_back(it);
                bio.features.push_back(feature);
            }
        }
        {
            js_object spawners_js = js_object::get_object(bio_js["spawners"]);
            for (auto&& [name, values] : spawners_js) {
                auto& category = bio.spawners[name];
                auto category_values = js_array::get_array(values);
                category.reserve(category_values.size());
                for (auto&& it : category_values) {
                    auto category_value = js_object::get_object(it);
                    Biome::SpawnersValue value;
                    value.type = category_value["type"];
                    value.weight = category_value["weight"];
                    value.weight = category_value["minCount"];
                    value.weight = category_value["maxCount"];
                    category.push_back(value);
                }
            }
        }
        {
            js_object spawn_costs_js = js_object::get_object(bio_js["spawn_costs"]);
            for (auto&& [eid, value] : spawn_costs_js) {
                auto& category = bio.spawn_costs[eid];
                auto category_value = js_object::get_object(value);
                category.energy_budget = category_value["energy_budget"];
                category.charge = category_value["charge"];
            }
        }

        biomes[id] = std::move(bio);
    }

    void load_file_biomes(const std::filesystem::path& file_path, const std::string& id) {
        check_override(biomes, id, "biome");

        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_biomes(js_object::get_object(res.value()), id);
    }

    ChatType::Decoration to_decoration(js_value&& json) {
        ChatType::Decoration decoration;
        js_object chat_js = js_object::get_object(json);
        if (chat_js.contains("style"))
            decoration.style = Chat::fromEnbt(conversions::json::from_json(chat_js["style"].get()));
        decoration.translation_key = (std::string)chat_js["translation_key"];
        {
            auto params = chat_js["parameters"];
            if (params.is_array()) {
                auto params_ = js_array::get_array(params);
                std::vector<std::string> parameters;
                parameters.reserve(params_.size());
                for (auto&& chat_ : params_)
                    parameters.push_back((std::string)chat_);
                decoration.parameters = std::move(parameters);
            } else
                decoration.parameters = (std::string)params;
        }
        return decoration;
    }

    base_objects::number_provider read_number_provider(js_value&& value) {
        if (value.is_number())
            return value.is_integral() ? base_objects::number_provider_constant((int32_t)value) : base_objects::number_provider_constant((float)value);
        else {
            auto obj = js_object::get_object(value);
            if (obj.contains("type")) {
                std::string type = obj["type"];

                if (type == "constant") {
                    auto value = obj.at("value");
                    return value.is_integral() ? base_objects::number_provider_constant((int32_t)value) : base_objects::number_provider_constant((float)value);
                } else if (type == "uniform") {
                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;

                    if (obj.contains("min")) {
                        auto min_ = obj["min"];
                        min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                    } else if (obj.contains("min_inclusive")) {
                        auto min_ = obj["min_inclusive"];
                        min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (obj.contains("max")) {
                        auto max_ = obj["max"];
                        max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                    } else if (obj.contains("max_inclusive")) {
                        auto max_ = obj["max_inclusive"];
                        max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                    } else
                        max = std::numeric_limits<int32_t>::max();

                    return base_objects::number_provider_uniform(min, max);
                } else if (type == "binomial")
                    return base_objects::number_provider_binomial(read_number_provider(obj.at("n")), read_number_provider(obj.at("p")));
                else if (type == "clamped_normal") {
                    float mean = obj.at("mean");
                    float deviation = obj.at("deviation");
                    int32_t min_inclusive = obj.at("min_inclusive");
                    int32_t max_inclusive = obj.at("max_inclusive");
                    return base_objects::number_provider_clamped_normal(mean, deviation, min_inclusive, max_inclusive);
                } else if (type == "uniform") {
                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;

                    if (obj.contains("min")) {
                        auto min_ = obj["min"];
                        min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                    } else if (obj.contains("min_inclusive")) {
                        auto min_ = obj["min_inclusive"];
                        min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (obj.contains("max")) {
                        auto max_ = obj["max"];
                        max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                    } else if (obj.contains("max_inclusive")) {
                        auto max_ = obj["max_inclusive"];
                        max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                    } else
                        max = std::numeric_limits<int32_t>::max();

                    return base_objects::number_provider_uniform(min, max);
                } else if (type == "clamped") {
                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;

                    if (obj.contains("min")) {
                        auto min_ = obj["min"];
                        min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                    } else if (obj.contains("min_inclusive")) {
                        auto min_ = obj["min_inclusive"];
                        min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (obj.contains("max")) {
                        auto max_ = obj["max"];
                        max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                    } else if (obj.contains("max_inclusive")) {
                        auto max_ = obj["max_inclusive"];
                        max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                    } else
                        max = std::numeric_limits<int32_t>::max();
                    return base_objects::number_provider_clamped(min, max, read_number_provider(obj.at("source")));
                } else if (type == "trapezoid") {
                    int32_t min = obj.at("min");
                    int32_t max = obj.at("max");
                    int32_t plateau = obj.at("plateau");
                    return base_objects::number_provider_trapezoid(min, max, plateau);
                } else if (type == "weighted_list") {
                    std::vector<std::pair<base_objects::number_provider, double>> values;
                    auto values_js = js_array::get_array(obj.at("values"));
                    values.reserve(values_js.size());
                    for (auto&& value : values_js) {
                        auto value_js = js_object::get_object(value);
                        auto weight = value_js.contains("weight") ? value_js["weight"] : 1.0;
                        values.push_back({read_number_provider(value_js.at("data")), weight});
                    }
                    return base_objects::number_provider_weighted_list(values);
                } else if (type == "biased_to_bottom") {
                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;

                    if (obj.contains("min")) {
                        auto min_ = obj["min"];
                        min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                    } else if (obj.contains("min_inclusive")) {
                        auto min_ = obj["min_inclusive"];
                        min = min_.is_integral() ? (int32_t)min_ : (float)min_;
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (obj.contains("max")) {
                        auto max_ = obj["max"];
                        max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                    } else if (obj.contains("max_inclusive")) {
                        auto max_ = obj["max_inclusive"];
                        max = max_.is_integral() ? (int32_t)max_ : (float)max_;
                    } else
                        max = std::numeric_limits<int32_t>::max();

                    return base_objects::number_provider_biased_to_bottom(min, max);
                } else if (type == "score") {
                    base_objects::number_provider_score res;
                    res.score = obj.at("score");
                    res.scale = obj.contains("scale") ? std::optional<float>((float)obj["scale"]) : std::nullopt;
                    auto target = js_object::get_object(obj.at("target"));
                    std::string type = target.at("type");
                    if (type == "fixed")
                        res.target.value = target.at("name");
                    else if (type == "context")
                        res.target.value = target.at("target");
                    else
                        target.parsing_error("Invalid target type: " + type);
                    return res;
                } else if (type == "storage") {
                    base_objects::number_provider_storage res;
                    res.storage = obj.at("storage");
                    res.path = obj.at("path");
                    return res;
                } else if (type == "enchantment_level")
                    return base_objects::number_provider_enchantment_level((std::string)obj.at("amount"));
                else
                    obj.parsing_error("Invalid number provider type: " + type);
            } else {
                int32_t min = obj.contains("min") ? obj["min"] : std::numeric_limits<int32_t>::min();
                int32_t max = obj.contains("max") ? obj["max"] : std::numeric_limits<int32_t>::max();
                return base_objects::number_provider_uniform(min, max);
            }
        }
    }

    void load_file_chatType(js_object&& type_js, const std::string& id, bool send_via_network_body = true) {
        check_override(chatTypes, id, "chat type");
        ChatType type;
        type.send_via_network_body = send_via_network_body;
        if (type_js.contains("chat"))
            type.chat = to_decoration(type_js["chat"]);
        if (type_js.contains("narration"))
            type.narration = to_decoration(type_js["narration"]);

        chatTypes[id] = std::move(type);
    }

    void load_file_chatType(const std::filesystem::path& file_path, const std::string& id) {
        check_override(chatTypes, id, "chat type");

        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        js_object type_js = js_object::get_object(res.value());
        ChatType type;
        if (type_js.contains("chat"))
            type.chat = to_decoration(type_js["chat"]);
        if (type_js.contains("narration"))
            type.narration = to_decoration(type_js["narration"]);

        chatTypes[id] = std::move(type);
    }

    void load_file_advancements(js_object&& advancement_js, const std::string& id, bool send_via_network_body = true) {
        check_conflicts(advancements, id, "advancements");
        Advancement advancement;
        advancement.send_via_network_body = send_via_network_body;
        if (advancement_js.contains("display")) {
            auto display_js = js_object::get_object(advancement_js["display"]);
            Advancement::Display display;
            {
                auto icon_js = js_object::get_object(display_js["icon"]);
                display.icon.item = (std::string)icon_js["item"];
                if (icon_js.contains("nbt"))
                    display.icon.nbt = icon_js["nbt"];
            }
            if (display_js.contains("frame"))
                display.frame = display_js["frame"];
            else
                display.frame = "task";

            display.description = display_js["description"].to_text();
            display.title = display_js["title"].to_text();

            if (display_js.contains("show_toast"))
                display.show_toast = display_js["show_toast"];
            if (display_js.contains("announce_to_chat"))
                display.announce_to_chat = display_js["announce_to_chat"];
            if (display_js.contains("hidden"))
                display.hidden = display_js["hidden"];
        }
        if (advancement_js.contains("parent"))
            advancement.parent = advancement_js["parent"];
        advancement.criteria = util::conversions::json::from_json(advancement_js["criteria"].get());
        if (advancement_js.contains("requirements")) {
            auto list_of_requirements_js = js_array::get_array(advancement_js["requirements"]);
            advancement.requirements.reserve(list_of_requirements_js.size());
            for (auto&& value : list_of_requirements_js) {
                auto requirements_js = js_array::get_array(value);
                std::vector<std::string> requirements;
                requirements.reserve(requirements_js.size());
                for (auto&& req : requirements_js)
                    requirements.push_back(req);
                advancement.requirements.push_back(requirements);
            }
        }
        if (advancement_js.contains("rewards")) {
            auto rewards_js = js_object::get_object(advancement_js["rewards"]);
            if (rewards_js.contains("recipes")) {
                auto recipes_js = js_array::get_array(rewards_js["recipes"]);
                advancement.rewards.recipes.reserve(rewards_js.size());
                for (auto&& req : recipes_js)
                    advancement.rewards.recipes.push_back(req);
            }
            if (rewards_js.contains("loot")) {
                auto loot_js = js_array::get_array(rewards_js["loot"]);
                advancement.rewards.loot.reserve(rewards_js.size());
                for (auto&& req : loot_js)
                    advancement.rewards.loot.push_back(req);
            }
            if (rewards_js.contains("experience"))
                advancement.rewards.experience = rewards_js["experience"];
            if (rewards_js.contains("function"))
                advancement.rewards.function = rewards_js["function"];
        }

        if (advancement_js.contains("sends_telemetry_event"))
            advancement.sends_telemetry_event = advancement_js["sends_telemetry_event"];
        advancements[id] = std::move(advancement);
    }

    void load_file_advancements(const std::filesystem::path& file_path, const std::string& id) {
        check_conflicts(advancements, id, "advancement");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_advancements(js_object::get_object(res.value()), id);
    }

    void load_file_jukebox_song(js_object&& song_js, const std::string& id, bool send_via_network_body = true) {
        check_conflicts(jukebox_songs, id, "jukebox song");
        JukeboxSong song;
        song.send_via_network_body = send_via_network_body;
        song.comparator_output = song_js["comparator_output"];
        song.length_in_seconds = song_js["length_in_seconds"];
        song.description = Chat::fromEnbt(util::conversions::json::from_json(song_js["description"].get()));
        auto sound_event_js = song_js["sound_event"];
        if (sound_event_js.is_string())
            song.sound_event = (std::string)sound_event_js;
        else {
            auto sound_event_obj = js_object::get_object(sound_event_js);
            JukeboxSong::custom ex;
            ex.sound_id = sound_event_obj["sound_id"];
            if (sound_event_obj.contains("fixed_range"))
                ex.fixed_range = sound_event_obj["fixed_range"];
            song.sound_event = std::move(ex);
        }
        jukebox_songs[id] = std::move(song);
    }

    void load_file_jukebox_song(const std::filesystem::path& file_path, const std::string& id) {
        check_conflicts(jukebox_songs, id, "jukebox songs");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_jukebox_song(js_object::get_object(res.value()), id);
    }

    void load_file_loot_table(js_object&& loot_table_js, const std::string& id, bool send_via_network_body = true) {
        check_conflicts(loot_table, id, "loot table");

        loot_table_item item;
        item.send_via_network_body = send_via_network_body;
        if (loot_table_js.contains("type"))
            item.type = (std::string)loot_table_js["type"];
        else
            item.type = "generic";

        if (loot_table_js.contains("functions")) {
            auto functions = js_array::get_array(loot_table_js["functions"]);
            item.functions.reserve(functions.size());
            for (auto&& function : functions) {
                enbt::compound comp;
                comp = util::conversions::json::from_json(function.get());
                item.functions.push_back(comp);
            }
        }

        if (loot_table_js.contains("pools")) {
            auto pools = js_array::get_array(loot_table_js["pools"]);
            item.pools.reserve(pools.size());
            for (auto&& pool_item : pools) {
                auto pool = js_object::get_object(pool_item);
                loot_table_item::pool pool_;
                if (pool.contains("conditions")) {
                    auto res = util::conversions::json::from_json(pool["conditions"].get());
                    auto ref = res.as_array();
                    pool_.conditions.reserve(ref.size());
                    for (auto& it : ref)
                        pool_.conditions.push_back(it.as_compound());
                }
                if (pool.contains("bonus_rolls"))
                    pool_.bonus_rolls = read_number_provider(pool["bonus_rolls"]);

                if (pool.contains("functions")) {
                    auto functions = js_array::get_array(pool["functions"]);
                    pool_.functions.reserve(functions.size());
                    for (auto&& function : functions) {
                        enbt::compound comp;
                        comp = util::conversions::json::from_json(function.get());
                        pool_.functions.push_back(comp);
                    }
                }
                pool_.rolls = read_number_provider(pool.at("rolls"));
                auto entries = js_array::get_array(pool.at("entries"));
                pool_.entries.reserve(entries.size());
                for (auto&& entry : entries) {
                    enbt::compound comp;
                    comp = util::conversions::json::from_json(entry.get());
                    pool_.entries.push_back(comp);
                }
                item.pools.push_back(std::move(pool_));
            }
        }


        if (loot_table_js.contains("random_sequence"))
            item.random_sequence = loot_table_js["random_sequence"];
        loot_table[id] = std::move(item);
    }

    void load_file_loot_table(const std::filesystem::path& file_path, const std::string& id) {
        check_override(armorTrimPatterns, id, "armor trim pattern");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_loot_table(js_object::get_object(res.value()), id);
    }

    void load_file_armorTrimPattern(js_object&& pattern_js, const std::string& id, bool send_via_network_body = true) {
        check_override(armorTrimPatterns, id, "armor trim pattern");
        ArmorTrimPattern pattern;
        pattern.send_via_network_body = send_via_network_body;
        pattern.asset_id = (std::string)pattern_js["asset_id"];
        pattern.decal = pattern_js["decal"];
        {
            auto desc = pattern_js["description"];
            if (desc.is_string())
                pattern.description = (std::string)desc;
            else
                pattern.description = Chat::fromEnbt(conversions::json::from_json(desc.get()));
        }
        armorTrimPatterns[id] = std::move(pattern);
    }

    void load_file_armorTrimPattern(const std::filesystem::path& file_path, const std::string& id) {
        check_override(armorTrimPatterns, id, "armor trim pattern");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_armorTrimPattern(js_object::get_object(res.value()), id);
    }

    void load_file_armorTrimMaterial(js_object&& material_js, const std::string& id, bool send_via_network_body = true) {
        check_override(armorTrimMaterials, id, "armor trim material");
        ArmorTrimMaterial material;
        material.send_via_network_body = send_via_network_body;
        material.asset_name = (std::string)material_js["asset_name"];
        {
            auto desc = material_js["description"];
            if (desc.is_string())
                material.description = (std::string)desc;
            else
                material.description = Chat::fromEnbt(conversions::json::from_json(desc.get()));
        }
        armorTrimMaterials[id] = std::move(material);
    }

    void load_file_armorTrimMaterial(const std::filesystem::path& file_path, const std::string& id) {
        check_override(armorTrimMaterials, id, "armor trim material");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_armorTrimMaterial(js_object::get_object(res.value()), id);
    }

    void load_file_wolfVariant(js_object&& variant_js, const std::string& id, bool send_via_network_body = true) {
        check_override(wolfVariants, id, "wolf variant");
        WolfVariant variant;
        variant.send_via_network_body = send_via_network_body;
        variant.assets = conversions::json::from_json(variant_js.at("assets").get());
        variant.spawn_conditions = conversions::json::from_json(variant_js.at("spawn_conditions").get());
        wolfVariants[id] = std::move(variant);
    }

    void load_file_wolfVariant(const std::filesystem::path& file_path, const std::string& id) {
        check_override(wolfVariants, id, "wolf variant");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_wolfVariant(js_object::get_object(res.value()), id);
    }

    void load_file_entityVariant(std::unordered_map<std::string, EntityVariant>& map, const std::string& key, js_object&& variant_js, const std::string& id, bool send_via_network_body = true) {
        check_override(map, id, key);
        EntityVariant variant;
        variant.send_via_network_body = send_via_network_body;
        variant.asset_id = (std::string)variant_js.at("asset_id");
        variant.spawn_conditions = conversions::json::from_json(variant_js.at("spawn_conditions").get());
        if (variant_js.contains("model"))
            variant.model = (std::string)variant_js.at("model");
        map[id] = std::move(variant);
    }

    void load_file_entityVariant(std::unordered_map<std::string, EntityVariant>& map, const std::string& key, const std::filesystem::path& file_path, const std::string& id) {
        check_override(map, id, key);
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_entityVariant(map, key, js_object::get_object(res.value()), id);
    }

#define LOAD_FILE_ENTITY_VARIANT(name)                                                                                 \
    void load_file_##name##Variant(js_object&& variant_js, const std::string& id, bool send_via_network_body = true) { \
        load_file_entityVariant(name##Variants, #name " variant", std::move(variant_js), id, send_via_network_body);   \
    }                                                                                                                  \
    void load_file_##name##Variant(const std::filesystem::path& file_path, const std::string& id) {                    \
        load_file_entityVariant(name##Variants, #name " variant", file_path, id);                                      \
    }
    LOAD_FILE_ENTITY_VARIANT(cat)
    LOAD_FILE_ENTITY_VARIANT(chicken)
    LOAD_FILE_ENTITY_VARIANT(cow)
    LOAD_FILE_ENTITY_VARIANT(pig)
    LOAD_FILE_ENTITY_VARIANT(frog)
#undef LOAD_FILE_ENTITY_VARIANT

    void load_file_wolfSoundVariant(js_object&& variant_js, const std::string& id, bool send_via_network_body = true) {
        check_override(wolfSoundVariants, id, "wolf sound variant");
        WolfSoundVariant variant;
        variant.send_via_network_body = send_via_network_body;
        variant.ambient_sound = (std::string)variant_js.at("ambient_sound");
        variant.death_sound = (std::string)variant_js.at("death_sound");
        variant.growl_sound = (std::string)variant_js.at("growl_sound");
        variant.hurt_sound = (std::string)variant_js.at("hurt_sound");
        variant.pant_sound = (std::string)variant_js.at("pant_sound");
        variant.whine_sound = (std::string)variant_js.at("whine_sound");
        wolfSoundVariants[id] = std::move(variant);
    }

    void load_file_wolfSoundVariant(const std::filesystem::path& file_path, const std::string& id) {
        check_override(wolfSoundVariants, id, "wolf sound variant");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_wolfSoundVariant(js_object::get_object(res.value()), id);
    }

    void load_file_dimensionType(js_object&& type_js, const std::string& id, bool send_via_network_body = true) {
        check_override(dimensionTypes, id, "dimension type");
        DimensionType type;
        type.send_via_network_body = send_via_network_body;
        if (type_js.contains("monster_spawn_light_level")) {
            auto monster_spawn_light_level = type_js["monster_spawn_light_level"];
            if (monster_spawn_light_level.is_number())
                type.monster_spawn_light_level = monster_spawn_light_level;
            else {

                js_object monster_spawn_light_level_js = js_object::get_object(monster_spawn_light_level);
                IntegerDistribution monster_spawn_light_level_;
                monster_spawn_light_level_.value = conversions::json::from_json(monster_spawn_light_level_js.get());
                monster_spawn_light_level_.type = (std::string)monster_spawn_light_level_js.at("type");
                type.monster_spawn_light_level = std::move(monster_spawn_light_level_);
            }
        }
        if (type_js.contains("fixed_time"))
            type.fixed_time = type_js["fixed_time"];

        type.infiniburn = (std::string)type_js["infiniburn"];
        type.effects = (std::string)type_js["effects"];
        type.coordinate_scale = type_js["coordinate_scale"];
        type.ambient_light = type_js["ambient_light"];
        type.min_y = type_js["min_y"];
        type.height = type_js["height"];
        type.logical_height = type_js["logical_height"];
        type.monster_spawn_block_light_limit = type_js["monster_spawn_block_light_limit"];
        type.has_skylight = type_js["has_skylight"];
        type.has_ceiling = type_js["has_ceiling"];
        type.ultrawarm = type_js["ultrawarm"];
        type.natural = type_js["natural"];
        type.piglin_safe = type_js["piglin_safe"];
        type.has_raids = type_js["has_raids"];
        type.respawn_anchor_works = type_js["respawn_anchor_works"];
        type.bed_works = type_js["bed_works"];
        dimensionTypes[id] = std::move(type);
    }

    void load_file_dimensionType(const std::filesystem::path& file_path, const std::string& id) {
        check_override(dimensionTypes, id, "dimension type");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_dimensionType(js_object::get_object(res.value()), id);
    }

    void load_file_enchantment(js_object&& type_js, const std::string& id, bool send_via_network_body = true) {
        check_conflicts(enchantments, id, "enchantments");
        enchantment type;
        type.send_via_network_body = send_via_network_body;
        type.description = Chat::fromEnbt(util::conversions::json::from_json(type_js.at("description").get()));
        type.max_level = type_js.at("max_level");
        type.weight = type_js.at("weight");
        type.anvil_cost = type_js.at("anvil_cost");
        auto slots = js_array::get_array(type_js.at("slots"));
        type.slots.reserve(slots.size());
        for (auto&& slot : slots)
            type.slots.push_back(slot);
        if (type_js.contains("exclusive_set")) {
            if (type_js.at("exclusive_set").is_string())
                type.exclusive_set = type_js.at("exclusive_set");
            else {
                auto exclusive_set_js = js_array::get_array(type_js.at("exclusive_set"));
                std::vector<std::string> exclusive_set;
                exclusive_set.reserve(exclusive_set_js.size());
                for (auto&& set : exclusive_set_js)
                    exclusive_set.push_back(set);
                type.exclusive_set = std::move(exclusive_set);
            }
        }
        if (type_js.at("supported_items").is_string())
            type.supported_items = type_js.at("supported_items");
        else {
            auto supported_items_js = js_array::get_array(type_js.at("supported_items"));
            std::vector<std::string> supported_items;
            supported_items.reserve(supported_items_js.size());
            for (auto&& set : supported_items_js)
                supported_items.push_back(set);
            type.supported_items = std::move(supported_items);
        }
        if (type_js.contains("primary_items")) {
            if (type_js.at("primary_items").is_string())
                type.primary_items = type_js.at("primary_items");
            else {
                auto primary_items_js = js_array::get_array(type_js.at("primary_items"));
                std::vector<std::string> primary_items;
                primary_items.reserve(primary_items_js.size());
                for (auto&& set : primary_items_js)
                    primary_items.push_back(set);
                type.primary_items = std::move(primary_items);
            }
        }
        auto min_cost = js_object::get_object(type_js.at("min_cost"));
        type.min_cost.base = min_cost.at("base");
        type.min_cost.per_level_above_first = min_cost.at("per_level_above_first");

        auto max_cost = js_object::get_object(type_js.at("max_cost"));
        type.max_cost.base = max_cost.at("base");
        type.max_cost.per_level_above_first = max_cost.at("per_level_above_first");

        if (type_js.contains("effects")) {
            auto effects = js_object::get_object(type_js.at("effects"));
            type.effects.reserve(effects.size());
            for (auto&& [component_id, effect] : effects)
                type.effects[component_id] = util::conversions::json::from_json(effect.get());
        }
        enchantments[id] = std::move(type);
    }

    void load_file_enchantment(const std::filesystem::path& file_path, const std::string& id) {
        check_conflicts(enchantments, id, "enchantments");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_enchantment(js_object::get_object(res.value()), id);
    }

    void load_file_enchantment_provider(boost::json::object& type_js, const std::string& id, bool send_via_network_body = true) {
        check_conflicts(enchantment_providers, id, "enchantment providers");
        enchantment_providers[id] = util::conversions::json::from_json(type_js);
    }

    void load_file_instrument(js_object&& type_js, const std::string& id, bool send_via_network_body = true) {
        check_conflicts(instruments, id, "instruments");
        Instrument type;
        type.description = Chat::fromEnbt(util::conversions::json::from_json(type_js.at("description").get()));
        type.use_duration = type_js.at("use_duration");
        type.range = type_js.at("range");
        type.send_via_network_body = send_via_network_body;
        if (type_js.at("sound_event").is_string()) {
            type.sound_event = (std::string)type_js.at("sound_event");
        } else {
            auto sound_event = js_object::get_object(type_js.at("sound_event"));
            type.sound_event = base_objects::slot_component::inner::sound_extended{.sound_name = sound_event.at("sound_name"), .fixed_range = sound_event.contains("fixed_range") ? std::optional<float>(sound_event.at("fixed_range")) : std::nullopt};
        }
        instruments[id] = std::move(type);
    }

    void load_file_instrument(const std::filesystem::path& file_path, const std::string& id) {
        check_conflicts(enchantments, id, "enchantments");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_instrument(js_object::get_object(res.value()), id);
    }

    void load_file_enchantment_provider(const std::filesystem::path& file_path, const std::string& id) {
        check_conflicts(enchantment_providers, id, "enchantment providers");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_enchantment_provider(res.value(), id);
    }

    void load_file_damageType(js_object&& type_js, const std::string& id, bool send_via_network_body = true) {
        check_override(damageTypes, id, "damage type");
        DamageType type;
        type.message_id = (std::string)type_js["message_id"];
        type.send_via_network_body = send_via_network_body;
        std::string scaling = type_js["scaling"];
        if (scaling == "never")
            type.scaling = DamageType::ScalingType::never;
        else if (scaling == "when_caused_by_living_non_player")
            type.scaling = DamageType::ScalingType::when_caused_by_living_non_player;
        else if (scaling == "always")
            type.scaling = DamageType::ScalingType::always;
        else
            type_js["scaling"].parsing_error("Unknown scaling type: " + scaling);

        if (type_js.contains("effects")) {
            std::string effects = type_js["effects"];
            if (effects == "hurt")
                type.effects = DamageType::EffectsType::hurt;
            else if (effects == "thorns")
                type.effects = DamageType::EffectsType::thorns;
            else if (effects == "drowning")
                type.effects = DamageType::EffectsType::drowning;
            else if (effects == "burning")
                type.effects = DamageType::EffectsType::burning;
            else if (effects == "poking")
                type.effects = DamageType::EffectsType::poking;
            else if (effects == "freezing")
                type.effects = DamageType::EffectsType::freezing;
            else
                type_js["effects"].parsing_error("Unknown effects type: " + effects);
        }

        if (type_js.contains("death_message_type")) {
            std::string death_message_type = type_js["death_message_type"];
            if (death_message_type == "default")
                type.death_message_type = DamageType::DeathMessageType::_default;
            else if (death_message_type == "fall_variants")
                type.death_message_type = DamageType::DeathMessageType::fall_variants;
            else if (death_message_type == "intentional_game_design")
                type.death_message_type = DamageType::DeathMessageType::intentional_game_design;
            else
                type_js["death_message_type"].parsing_error("Unknown death message type: " + death_message_type);
        }

        type.exhaustion = type_js["exhaustion"];
        damageTypes[id] = std::move(type);
    }

    void load_file_damageType(const std::filesystem::path& file_path, const std::string& id) {
        check_override(damageTypes, id, "damage type");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_damageType(js_object::get_object(res.value()), id);
    }

    void load_file_bannerPattern(js_object&& pattern_js, const std::string& id, bool send_via_network_body = true) {
        check_override(bannerPatterns, id, "banner pattern");
        BannerPattern pattern;
        pattern.asset_id = (std::string)pattern_js["asset_id"];
        pattern.translation_key = (std::string)pattern_js["translation_key"];
        pattern.send_via_network_body = send_via_network_body;
        bannerPatterns[id] = std::move(pattern);
    }

    void load_file_bannerPattern(const std::filesystem::path& file_path, const std::string& id) {
        check_override(bannerPatterns, id, "banner pattern");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_bannerPattern(js_object::get_object(res.value()), id);
    }

    void load_file_paintingVariant(js_object&& variant_js, const std::string& id, bool send_via_network_body = true) {
        check_override(paintingVariants, id, "painting variant");
        PaintingVariant variant;
        variant.asset_id = (std::string)variant_js["asset_id"];
        variant.height = variant_js["height"];
        variant.width = variant_js["width"];
        variant.send_via_network_body = send_via_network_body;
        paintingVariants[id] = std::move(variant);
    }

    void load_file_paintingVariant(const std::filesystem::path& file_path, const std::string& id) {
        check_override(paintingVariants, id, "painting variant");
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_paintingVariant(js_object::get_object(res.value()), id);
    }

    void load_file_recipe(js_object&& variant_js, const std::string& id, bool send_via_network_body = true) {
        if (!api::recipe::registered())
            throw std::runtime_error("Recipe api not registered!");

        enbt::compound res;
        res = util::conversions::json::from_json(variant_js.get());
        api::recipe::set_recipe(id, std::move(res));
    }

    void load_file_recipe(const std::filesystem::path& file_path, const std::string& id) {
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_recipe(js_object::get_object(res.value()), id);
    }

    void apply_tags(js_value val, const std::string& type, const std::string& namespace_, const std::string& path_, bool replace) {
        list_array<std::string> result;
        for (auto&& tag : js_array::get_array(val)) {
            std::string the_tag;
            if (tag.is_string())
                the_tag = (std::string)tag;
            else {
                auto tag_ = js_object::get_object(tag);
                the_tag = (std::string)tag_.at("id");
            }
            result.push_back(the_tag);
        }
        api::tags::add_tag(type, namespace_ + ":" + path_, result, !replace);
    }

    void load_file_tags(js_object&& tags_, const std::string& type, const std::string& namespace_, const std::string& path_) {
        if (tags_.contains("replace")) {
            bool replace = tags_["replace"];
            apply_tags(tags_["values"], type, namespace_, path_, replace);
        } else if (tags_.contains("values"))
            apply_tags(tags_["values"], type, namespace_, path_, false);
        else if (tags_.contains("root"))
            apply_tags(tags_["root"], type, namespace_, path_, false);
        else
            tags_.parsing_error("Invalid tag file format");
    }

    void load_file_tags(const std::filesystem::path& file_path, const std::string& type, const std::string& namespace_, const std::string& path_) {
        auto res = try_read_json_file(file_path);
        if (!res)
            throw std::runtime_error("Failed to read file: " + file_path.string());
        load_file_tags(js_object::get_object(res.value()), type, namespace_, path_);
    }

    void load_register_file(std::string_view memory, const std::string& namespace_, const std::string& path_, const std::string& type) {
        if (path_.empty())
            throw std::runtime_error("Path is empty");
        std::string id = (namespace_.empty() ? "minecraft" : namespace_) + ":" + path_;
        if (type == "advancement") {
            load_file_advancements(memory, id);
        } else if (type == "banner_pattern")
            load_file_bannerPattern(memory, id);
        else if (type == "chat_type")
            load_file_chatType(memory, id);
        else if (type == "damage_type")
            load_file_damageType(memory, id);
        else if (type == "dimension_type")
            load_file_dimensionType(memory, id);
        else if (type == "enchantment")
            load_file_enchantment(memory, id);
        else if (type == "enchantment_provider")
            load_file_enchantment_provider(memory, id);
        else if (type == "jukebox_song")
            load_file_jukebox_song(memory, id);
        else if (type == "loot_table")
            load_file_loot_table(memory, id);
        else if (type == "painting_variant")
            load_file_paintingVariant(memory, id);
        else if (type == "recipe")
            load_file_recipe(memory, id);
        else if (type == "trim_pattern")
            load_file_armorTrimPattern(memory, id);
        else if (type == "trim_material")
            load_file_armorTrimMaterial(memory, id);
        else if (type == "wolf_variant")
            load_file_wolfVariant(memory, id);
        else if (type == "cat_variant")
            load_file_catVariant(memory, id);
        else if (type == "chicken_variant")
            load_file_chickenVariant(memory, id);
        else if (type == "cow_variant")
            load_file_cowVariant(memory, id);
        else if (type == "frog_variant")
            load_file_frogVariant(memory, id);
        else if (type == "pig_variant")
            load_file_pigVariant(memory, id);
        else if (type == "wolf_sound_variant")
            load_file_wolfSoundVariant(memory, id);
        else if (type == "worldgen/biome")
            load_file_biomes(memory, id);
        else if (type == "worldgen/configured_carver")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/configured_feature")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/density_function")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/flat_level_generator_preset")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/multi_noise_biome_source_parameter_list")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/noise")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/noise_settings")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/placed_feature")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/processor_list")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/structure")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/structure_set")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/template_pool")
            ; //load_file_biomes(memory, id);
        else if (type == "worldgen/world_preset")
            ; //load_file_biomes(memory, id);
        else if (type.starts_with("tag")) {
            std::string tag_type;
            if (type.starts_with("tags/"))
                tag_type = type.substr(5);
            else if (type.starts_with("tag/"))
                tag_type = type.substr(4);
            else
                throw std::runtime_error("Unknown type: " + type);
            load_file_tags(memory, tag_type, namespace_, path_);
        } else
            throw std::runtime_error("Unknown type: " + type);
    }

    void load_register_file(const std::filesystem::path& file_path, const std::string& namespace_, const std::string& path_, const std::string& type) {
        if (path_.empty())
            throw std::runtime_error("Path is empty");
        std::string id = (namespace_.empty() ? "minecraft" : namespace_) + ":" + path_;
        if (type == "advancement") {
            load_file_advancements(file_path, id);
        } else if (type == "banner_pattern")
            load_file_bannerPattern(file_path, id);
        else if (type == "chat_type")
            load_file_chatType(file_path, id);
        else if (type == "damage_type")
            load_file_damageType(file_path, id);
        else if (type == "dimension_type")
            load_file_dimensionType(file_path, id);
        else if (type == "enchantment")
            load_file_enchantment(file_path, id);
        else if (type == "enchantment_provider")
            load_file_enchantment_provider(file_path, id);
        else if (type == "jukebox_song")
            load_file_jukebox_song(file_path, id);
        else if (type == "loot_table")
            load_file_loot_table(file_path, id);
        else if (type == "painting_variant")
            load_file_paintingVariant(file_path, id);
        else if (type == "recipe")
            load_file_recipe(file_path, id);
        else if (type == "trim_pattern")
            load_file_armorTrimPattern(file_path, id);
        else if (type == "trim_material")
            load_file_armorTrimMaterial(file_path, id);
        else if (type == "wolf_variant")
            load_file_wolfVariant(file_path, id);
        else if (type == "cat_variant")
            load_file_catVariant(file_path, id);
        else if (type == "chicken_variant")
            load_file_chickenVariant(file_path, id);
        else if (type == "cow_variant")
            load_file_cowVariant(file_path, id);
        else if (type == "frog_variant")
            load_file_frogVariant(file_path, id);
        else if (type == "pig_variant")
            load_file_pigVariant(file_path, id);
        else if (type == "wolf_sound_variant")
            load_file_wolfSoundVariant(file_path, id);
        else if (type == "worldgen/biome")
            load_file_biomes(file_path, id);
        else if (type == "worldgen/configured_carver")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/configured_feature")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/density_function")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/flat_level_generator_preset")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/multi_noise_biome_source_parameter_list")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/noise")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/noise_settings")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/placed_feature")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/processor_list")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/structure")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/structure_set")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/template_pool")
            ; //load_file_biomes(file_path, id);
        else if (type == "worldgen/world_preset")
            ; //load_file_biomes(file_path, id);
        else if (type.starts_with("tag")) {
            std::string tag_type;
            if (type.starts_with("tags/"))
                tag_type = type.substr(5);
            else if (type.starts_with("tag/"))
                tag_type = type.substr(4);
            else
                throw std::runtime_error("Unknown type: " + type);
            load_file_tags(file_path, tag_type, namespace_, path_);
        } else
            throw std::runtime_error("Unknown type: " + type);
    }

    static inline uint64_t as_uint(const boost::json::value& val) {
        if (val.is_int64())
            return val.to_number<int32_t>();
        else if (val.is_uint64())
            return val.as_uint64();
        else
            throw std::runtime_error("Invalid value type");
    }

    void load_blocks() {
        {
            auto block_properties = boost::json::parse(resources::registry::block_properties).as_array();
            base_objects::static_block_data::all_properties.reserve(block_properties.size());
            for (auto& item : block_properties) {
                auto hash = item.at("hash_key").to_number<int32_t>();
                auto name = item.at("enum_name").as_string();
                auto ser_name = item.at("serialized_name").as_string();
                auto type = item.at("type").as_string();

                if (type == "boolean") {
                    base_objects::static_block_data::all_properties[hash] = {"true", "false"};
                    base_objects::static_block_data::assigned_property_name.insert({hash, (std::string)ser_name});
                } else if (type == "int") {
                    auto min = item.at("min").to_number<int32_t>();
                    auto max = item.at("max").to_number<int32_t>();
                    std::unordered_set<std::string> values;
                    for (int i = min; i <= max; i++)
                        values.insert(std::to_string(i));
                    base_objects::static_block_data::all_properties[hash] = std::move(values);
                    base_objects::static_block_data::assigned_property_name.insert({hash, (std::string)ser_name});
                } else if (type == "enum") {
                    auto values = item.at("values").as_array();
                    std::unordered_set<std::string> values_set;
                    for (auto&& value : values)
                        values_set.insert((std::string)value.as_string());
                    base_objects::static_block_data::all_properties[hash] = std::move(values_set);
                    base_objects::static_block_data::assigned_property_name.insert({hash, (std::string)ser_name});
                } else
                    throw std::runtime_error("Unknown type: " + std::string(type));
            }
        }

        std::string tmp;
        {
            boost::iostreams::filtering_istream filter;
            boost::iostreams::array_source source(resources::registry::blocks.data(), resources::registry::blocks.size());
            filter.push(boost::iostreams::zstd_decompressor());
            filter.push(source);
            std::ostringstream buffer;
            buffer << filter.rdbuf();
            tmp = std::move(buffer).str();
        }
        std::stringstream ss(std::move(tmp));
        enbt::io_helper::value_read_stream read_stream(ss);
        read_stream.peek_at("shapes", [](auto& shapes) {
            shapes.iterate(
                [](auto size) {
                    base_objects::static_block_data::all_shapes.reserve(size);
                },
                [&](auto& shape) {
                    auto item = shape.read();
                    auto& min = item.at("min");
                    auto& max = item.at("max");
                    base_objects::static_block_data::all_shapes.push_back(
                        base_objects::shape_data{
                            .min_x = min.at(0),
                            .min_y = min.at(1),
                            .min_z = min.at(2),
                            .max_x = max.at(0),
                            .max_y = max.at(1),
                            .max_z = max.at(2)
                        }
                    );
                }
            );
        });


        read_stream.peek_at("block_entity_types", [](auto& block_entity_types) {
            block_entity_types.iterate(
                [](auto size) {
                    base_objects::static_block_data::block_entity_types.reserve(size);
                },
                [&](auto& item) {
                    base_objects::static_block_data::block_entity_types.push_back((std::string)item.read());
                }
            );
        });

        read_stream.peek_at("blocks", [&](enbt::io_helper::value_read_stream& blocks) {
            base_objects::block::access_full_block_data(std::function([&](list_array<std::shared_ptr<base_objects::static_block_data>>& full_block_data_, std::unordered_map<std::string, std::shared_ptr<base_objects::static_block_data>>& named_full_block_data) {
                blocks.iterate([&](enbt::io_helper::value_read_stream& decl) {
                    std::shared_ptr<base_objects::static_block_data> default_state_data = std::make_shared<base_objects::static_block_data>();
                    list_array<base_objects::block_id_t> init_states;
                    std::shared_ptr<base_objects::static_block_data::map_of_states> associated_states = std::make_shared<base_objects::static_block_data::map_of_states>();
                    base_objects::block_id_t default_state = 0;
                    bool has_default_state = false;


                    decl.iterate([&](std::string_view name, enbt::io_helper::value_read_stream& item) {
                        if (name == "name") {
                            default_state_data->name = "minecraft:" + item.read().as_string();
                            if (named_full_block_data.contains(default_state_data->name))
                                throw std::runtime_error("Duplicate block name: " + default_state_data->name);
                        } else if (name == "id")
                            default_state_data->general_block_id = item.read();
                        else if (name == "translation_key")
                            default_state_data->translation_key = item.read().as_string();
                        else if (name == "slipperiness")
                            default_state_data->slipperiness = item.read();
                        else if (name == "velocity_multiplier")
                            default_state_data->velocity_multiplier = item.read();
                        else if (name == "jump_velocity_multiplier")
                            default_state_data->jump_velocity_multiplier = item.read();
                        else if (name == "hardness")
                            default_state_data->hardness = item.read();
                        else if (name == "blast_resistance")
                            default_state_data->blast_resistance = item.read();
                        else if (name == "item_id")
                            default_state_data->default_drop_item_id = item.read();
                        else if (name == "map_color_rgb")
                            default_state_data->map_color_rgb = item.read();
                        else if (name == "loot_table")
                            *(default_state_data->loot_table = std::make_shared<enbt::compound>()) = item.read();
                        else if (name == "properties") {
                            std::vector<int32_t> properties;
                            item.iterate(
                                [&properties](auto size) { properties.reserve(size); },
                                [&properties](enbt::io_helper::value_read_stream& item) { properties.push_back(item.read()); }
                            );
                            default_state_data->allowed_properties = std::move(properties);
                        } else if (name == "states") {
                            item.iterate(
                                [&](auto size) { init_states.reserve(size); },
                                [&](enbt::io_helper::value_read_stream& decl) {
                                std::shared_ptr<base_objects::static_block_data> block_data = std::make_shared<base_objects::static_block_data>();
                                base_objects::block_id_t block_data_id = 0;
                                block_data->opacity = 255;
#define ARGS__d base_objects::block_id_t &default_state, bool &has_default_state, list_array<std::shared_ptr<base_objects::static_block_data>>&full_block_data_, std::shared_ptr<base_objects::static_block_data>&block_data, base_objects::block_id_t &block_data_id, enbt::io_helper::value_read_stream &item
#define ARGS__pass default_state, has_default_state, full_block_data_, block_data, block_data_id, item

                                static std::unordered_map<std::string, void (*)(ARGS__d)> map{
                                    {"id", [](ARGS__d) {
                                         base_objects::block_id_t id = block_data_id = item.read();
                                         if (id >= full_block_data_.size()) {
                                             if (full_block_data_.reserved() == 0)
                                                 full_block_data_.reserve(id);
                                             full_block_data_.resize(id + 1);
                                         }
                                         auto& ref = full_block_data_.at(id);
                                         if (ref)
                                             throw std::runtime_error("Duplicate block id: " + std::to_string(id));
                                         ref = block_data;
                                         if (!has_default_state) {
                                             default_state = id;
                                             has_default_state = true;
                                         }
                                     }},
                                    {"air", [](ARGS__d) { block_data->is_air = item.read(); }},
                                    {"is_solid", [](ARGS__d) { block_data->is_solid = item.read(); }},
                                    {"opacity", [](ARGS__d) { block_data->opacity = item.read(); }},
                                    {"instrument", [](ARGS__d) { block_data->instrument = item.read().as_string(); }},
                                    {"is_liquid", [](ARGS__d) { block_data->is_liquid = item.read(); }},
                                    {"luminance", [](ARGS__d) { block_data->luminance = item.read(); }},
                                    {"burnable", [](ARGS__d) { block_data->is_burnable = item.read(); }},
                                    {"emits_redstone", [](ARGS__d) { block_data->is_emits_redstone = item.read(); }},
                                    {"is_full_cube", [](ARGS__d) { block_data->is_full_cube = item.read(); }},
                                    {"tool_required", [](ARGS__d) { block_data->is_tool_required = item.read(); }},
                                    {"piston_behavior", [](ARGS__d) { block_data->piston_behavior = item.read().as_string(); }},
                                    {"replaceable", [](ARGS__d) { block_data->is_replaceable = item.read(); }},
                                    {"hardness", [](ARGS__d) { block_data->hardness = item.read(); }},
                                    {"sided_transparency", [](ARGS__d) { block_data->is_sided_transparency = item.read(); }},
                                    {"default_state_id", [](ARGS__d) {
                                         default_state = item.read();
                                         block_data->is_default_state = true;
                                         has_default_state = true;
                                     }},
                                    {"properties", [](ARGS__d) {
                                         std::unordered_map<std::string, std::string> properties;
                                         item.iterate([&](auto size) { properties.reserve(size); }, [&](const std::string& name, enbt::io_helper::value_read_stream& item) {
                                             properties[name] = item.read().as_string();
                                         });
                                         block_data->current_properties = std::move(properties);
                                     }},
                                    {"collision_shapes", [](ARGS__d) {
                                         item.iterate(
                                             [&](auto size) { block_data->collision_shapes.reserve(size); },
                                             [&](enbt::io_helper::value_read_stream& item) { block_data->collision_shapes.push_back(&base_objects::static_block_data::all_shapes.at(item.read())); }
                                         );
                                     }},
                                    {"block_entity_type", [](ARGS__d) {
                                         block_data->is_block_entity = true;
                                         block_data->block_entity_id = item.read();
                                     }},
                                };

                                decl.iterate([&](const std::string& name, enbt::io_helper::value_read_stream& item) {
                                    map.at(name)(ARGS__pass);
                                });
                                init_states.push_back(block_data_id);
#undef ARGS__d
#undef ARGS__pass
                            });
                        }
                    });

                    for (auto& state : init_states) {
                        auto& block_data = full_block_data_.at(state);
                        block_data->assigned_states_to_properties = associated_states;
                        block_data->default_state = default_state;

                        block_data->name = default_state_data->name;
                        block_data->general_block_id = default_state_data->general_block_id;
                        block_data->translation_key = default_state_data->translation_key;
                        block_data->slipperiness = default_state_data->slipperiness;
                        block_data->velocity_multiplier = default_state_data->velocity_multiplier;
                        block_data->jump_velocity_multiplier = default_state_data->jump_velocity_multiplier;
                        block_data->hardness = default_state_data->hardness;
                        block_data->blast_resistance = default_state_data->blast_resistance;
                        block_data->default_drop_item_id = default_state_data->default_drop_item_id;
                        block_data->map_color_rgb = default_state_data->map_color_rgb;
                        block_data->loot_table = default_state_data->loot_table;
                        block_data->allowed_properties = default_state_data->allowed_properties;
                    }
                    named_full_block_data[(std::string)default_state_data->name] = full_block_data_.at(default_state);
                });

                for (auto it = full_block_data_.begin(); it != full_block_data_.end(); ++it) {
                    if (*it == nullptr)
                        throw std::runtime_error("Gap between block definitions");
                }
                full_block_data_.commit();
            }));
        });
    }

    void load_items() {
        auto parsed = boost::json::parse(resources::registry::items);
        std::unordered_set<std::string> filter;
        auto& to_filter = internal_compatibility_versions.at(api::configuration::get().protocol.allowed_versions_processed.min()).at("disabled_items").as_array();
        filter.reserve(to_filter.size());
        for (auto&& item : to_filter)
            filter.insert((std::string)item.as_string());

        for (auto&& [name, decl] : parsed.as_object()) {
            if (filter.contains(name))
                continue;
            base_objects::static_slot_data slot_data;
            slot_data.id = name;
            std::unordered_map<std::string, base_objects::slot_component::unified> components;
            for (auto& [name, value] : decl.as_object().at("components").as_object())
                components[name] = base_objects::slot_component::parse_component(name, conversions::json::from_json(value));
            slot_data.default_components = std::move(components);
            base_objects::slot_data::add_slot_data(std::move(slot_data));
        }
    }

    void prepare_versions() {
        std::unordered_map<uint32_t, std::string_view> internal_protocol_aliases = {
            {770, resources::registry::protocol::_770},
        };
        latest_protocol_version
            = api::configuration::get().protocol.allowed_versions_processed.for_each(
                                                                               [&](uint32_t version) {
                                                                                   auto res = util::conversions::json::from_json(boost::json::parse(internal_protocol_aliases.at(version)));
                                                                                   for (auto& it : res.as_compound()) {
                                                                                       auto entries = it.second.at("entries").as_compound();
                                                                                       enbt::fixed_array invert(entries.size());

                                                                                       for (auto& [key, value] : it.second.at("entries").as_compound())
                                                                                           invert.set(value.at("protocol_id"), key);

                                                                                       it.second["proto_invert"] = std::move(invert);
                                                                                   }
                                                                                   registers::individual_registers[version] = std::move(res);
                                                                               }
            ).max();
        registers::use_registry_latest = latest_protocol_version;
    }

    void __prepare_tags(boost::json::object& parsed, const std::string& type, const std::string& namespace_, const std::string& tag) {
        for (auto&& [name, decl] : parsed) {
            auto& obj = decl.as_object();
            if (name.ends_with(".json")) {
                if (obj.contains("values") || obj.contains("root")) {
                    auto& values = obj.contains("values") ? obj.at("values") : obj.at("root");
                    if (values.is_array()) {
                        bool replace = obj.contains("replace") ? obj.at("replace").as_bool() : false;
                        std::string computed_tag = tag;
                        if (computed_tag.size())
                            computed_tag += "/";
                        computed_tag += (std::string)name.substr(0, name.size() - 5);
                        list_array<std::string> res;
                        for (auto&& value : values.get_array())
                            res.push_back((std::string)value.as_string());
                        api::tags::add_tag(type, namespace_ + ":" + tag, res);
                        continue;
                    }
                }
            } else
                __prepare_tags(obj, type + "/" + (std::string)name, namespace_, tag);
        }
    }

    void prepare_tags(boost::json::object& parsed, const std::string& namespace_) {
        for (auto&& [type, decl] : parsed)
            __prepare_tags(decl.get_object(), type, namespace_, "");
    }

    void process_item_(boost::json::object& decl, const std::string& namespace_, void (*fn)(js_object&&, const std::string&, bool send_via_network_body), bool send_via_network_body) {
        for (auto& [id, value] : decl) {
            if (id.ends_with(".json")) {
                std::string _id = namespace_ + std::string(id.substr(0, id.size() - 5));
                fn(js_object::get_object(value), _id, send_via_network_body);
            } else
                process_item_(value.as_object(), namespace_ + std::string(id), fn, send_via_network_body);
        }
    }

    void process_item_(boost::json::object& decl, const std::string& namespace_, void (*fn)(boost::json::object&, const std::string&, bool send_via_network_body), bool send_via_network_body) {
        for (auto& [id, value] : decl) {
            if (id.ends_with(".json")) {
                std::string _id = namespace_ + std::string(id.substr(0, id.size() - 5));
                fn(value.as_object(), _id, send_via_network_body);
            } else
                process_item_(value.as_object(), namespace_ + std::string(id), fn, send_via_network_body);
        }
    }

    void process_item(js_value& decl, const std::string& namespace_, void (*fn)(js_object&&, const std::string&, bool send_via_network_body), bool send_via_network_body) {
        process_item_(decl.get().as_object(), namespace_ + ":", fn, send_via_network_body);
    }

    void process_item(js_value& decl, const std::string& namespace_, void (*fn)(boost::json::object&, const std::string&, bool send_via_network_body), bool send_via_network_body) {
        process_item_(decl.get().as_object(), namespace_ + ":", fn, send_via_network_body);
    }

    void process_pack(boost::json::object& parsed, const std::string& namespace_, const std::string& id, bool allowed_pack_nest, bool send_via_network_body) {
        auto data = js_object::get_object(parsed);
        if (data.contains("tags"))
            prepare_tags(data["tags"].get().as_object(), namespace_);
        for (auto&& [name, decl] : data) {
            if (name == "banner_pattern")
                process_item(decl, namespace_, load_file_bannerPattern, send_via_network_body);
            else if (name == "chat_type")
                process_item(decl, namespace_, load_file_chatType, send_via_network_body);
            else if (name == "damage_type")
                process_item(decl, namespace_, load_file_damageType, send_via_network_body);
            else if (name == "dimension_type")
                process_item(decl, namespace_, load_file_dimensionType, send_via_network_body);
            else if (name == "enchantment")
                process_item(decl, namespace_, load_file_enchantment, send_via_network_body);
            else if (name == "enchantment_provider")
                process_item(decl, namespace_, load_file_enchantment_provider, send_via_network_body);
            else if (name == "instrument")
                process_item(decl, namespace_, load_file_instrument, send_via_network_body);
            //else if (name == "jukebox_song")
            //    process_item(decl, namespace_, load_file_jukebox_song, send_via_network_body);
            //else if (name == "loot_table")
            //    process_item(decl, namespace_, load_file_loot_table, send_via_network_body);
            else if (name == "painting_variant")
                process_item(decl, namespace_, load_file_paintingVariant, send_via_network_body);
            //else if (name == "recipe")
            //    process_item(decl, namespace_, load_file_recipe, send_via_network_body);
            //else if (name == "structure")
            //    process_item(decl, namespace_, load_file_structure, send_via_network_body);
            //else if (name == "trial_spawner")
            //    process_item(decl, namespace_, load_file_trial_spawner, send_via_network_body);
            else if (name == "trim_material")
                process_item(decl, namespace_, load_file_armorTrimMaterial, send_via_network_body);
            else if (name == "trim_pattern")
                process_item(decl, namespace_, load_file_armorTrimPattern, send_via_network_body);
            else if (name == "wolf_variant")
                process_item(decl, namespace_, load_file_wolfVariant, send_via_network_body);
            else if (name == "cat_variant")
                process_item(decl, namespace_, load_file_catVariant, send_via_network_body);
            else if (name == "chicken_variant")
                process_item(decl, namespace_, load_file_chickenVariant, send_via_network_body);
            else if (name == "cow_variant")
                process_item(decl, namespace_, load_file_cowVariant, send_via_network_body);
            else if (name == "frog_variant")
                process_item(decl, namespace_, load_file_frogVariant, send_via_network_body);
            else if (name == "pig_variant")
                process_item(decl, namespace_, load_file_pigVariant, send_via_network_body);
            else if (name == "wolf_sound_variant")
                process_item(decl, namespace_, load_file_wolfSoundVariant, send_via_network_body);
            else if (name == "worldgen") {
                for (auto&& [name, decl] : js_object::get_object(decl)) {
                    if (name == "biome")
                        process_item(decl, namespace_, load_file_biomes, send_via_network_body);
                }
            } else if (allowed_pack_nest) {
                if (name == "datapacks") {
                    auto& enabled_features = api::configuration::get().game_play.enabled_features;
                    for (auto&& [name, decl] : js_object::get_object(decl)) {
                        auto id = std::string(name);
                        auto decl_obj = js_object::get_object(decl);
                        if (decl_obj.contains("pack.mcmeta")) {
                            auto pack_meta = js_object::get_object(decl_obj["pack.mcmeta"]);
                            if (pack_meta.contains("features")) {
                                auto features = js_object::get_object(pack_meta["features"]);
                                if (features.contains("enabled")) {
                                    auto enabled = js_array::get_array(features["enabled"]);
                                    bool not_enabled = false;
                                    for (auto&& feature : enabled) {
                                        if (!enabled_features.contains(feature)) {
                                            not_enabled = true;
                                            break;
                                        }
                                    }
                                    if (not_enabled && !enabled.empty())
                                        continue; //skip datapack
                                }
                            }

                            if (pack_meta.contains("pack")) {
                                auto pack = js_object::get_object(pack_meta["pack"]);
                                if (pack.contains("pack_format")) {
                                    auto pack_format = (int64_t)pack["pack_format"];
                                    if (pack_format != 61) {
                                        log::error("resource_load", "Unsupported pack format: " + std::to_string(pack_format) + " for child datapack: " + namespace_ + "->" + id);
                                        continue;
                                    }
                                }
                            }

                            if (pack_meta.contains("data")) {
                                auto data = js_object::get_object(pack_meta["data"]);
                                for (auto&& [name, decl] : data)
                                    process_pack(decl, name, id, false, send_via_network_body);
                            } else
                                loaded_packs_.push_back({.namespace_ = "minecraft", .id = id});
                        }
                    }
                }
            }
        }
        loaded_packs_.push_back({.namespace_ = namespace_, .id = id});
    }

    void process_pack(boost::json::object& parsed, const std::string& namespace_, const std::string& id) {
        process_pack(parsed, namespace_, id, false, true);
    }

    void ___recursive_merge_json(boost::json::object& out, const std::filesystem::directory_entry& file);

    void ___recursive_merge_json__file(boost::json::object& out, const std::filesystem::directory_entry& file) {
        auto it = try_read_json_file(file.path());
        if (!it)
            throw std::runtime_error("Failed to read file: " + file.path().string());
        out[file.path().filename().string()] = std::move(*it);
    }

    void ___recursive_merge_json__directory(boost::json::object& out, const std::filesystem::directory_entry& file) {
        for (const auto& f : std::filesystem::directory_iterator(file.path())) {
            auto& inner = (out[f.path().filename().string()] = boost::json::object()).get_object();
            ___recursive_merge_json(inner, f);
        }
    }

    void ___recursive_merge_json(boost::json::object& out, const std::filesystem::directory_entry& file) {
        if (file.is_directory())
            ___recursive_merge_json__directory(out, file);
        else
            ___recursive_merge_json__file(out, file);
    }

    void process_pack(const std::filesystem::path& folder_path_to_data_with_namespace, const std::string& namespace_, const std::string& id) {
        boost::json::value value = boost::json::object();
        auto& obj = value.get_object();
        for (auto& entry : std::filesystem::directory_iterator(folder_path_to_data_with_namespace)) {
            auto& inner = (obj[entry.path().filename().string()] = boost::json::object()).get_object();
            ___recursive_merge_json(inner, entry);
        }
        process_pack(obj, namespace_, id, false, true);
    }

    void process_pack(const std::filesystem::path& folder_path_to_data_packs) {
        auto& enabled_features = api::configuration::get().game_play.enabled_features;
        for (auto& entry : std::filesystem::directory_iterator(folder_path_to_data_packs)) {
            auto& path = entry.path();
            std::string id = path.stem().string();
            if (entry.is_directory()) {
                auto pack_meta_result = try_read_json_file(path / "pack.mcmeta");
                if (!pack_meta_result)
                    throw std::runtime_error("Failed to read datapack: " + path.string());
                auto pack_meta = js_object::get_object(*pack_meta_result);
                if (pack_meta.contains("features")) {
                    auto features = js_object::get_object(pack_meta["features"]);
                    if (features.contains("enabled")) {
                        auto enabled = js_array::get_array(features["enabled"]);
                        bool not_enabled = false;
                        for (auto&& feature : enabled) {
                            if (!enabled_features.contains(feature)) {
                                not_enabled = true;
                                break;
                            }
                        }
                        if (not_enabled && !enabled.empty())
                            continue; //skip datapack
                    }
                }

                if (pack_meta.contains("pack")) {
                    auto pack = js_object::get_object(pack_meta["pack"]);
                    if (pack.contains("pack_format")) {
                        auto pack_format = (int64_t)pack["pack_format"];
                        if (pack_format != 61) {
                            log::error("resource_load", "Unsupported pack format: " + std::to_string(pack_format) + " for datapack: " + path.string());
                            continue;
                        }
                    }
                }

                if (std::filesystem::exists(path / "data")) {
                    for (auto& entry : std::filesystem::directory_iterator(path / "data")) {
                        if (entry.is_directory()) {
                            auto& path = entry.path();
                            std::string namespace_ = path.stem().string();
                            process_pack(path, namespace_, id);
                        }
                    }
                }
            } else if (path.extension() == ".zip") {
                //TODO process zip file
            }
        }
    }

    void prepare_built_in_pack() {
        auto parsed = boost::json::parse(resources::data);
        process_pack(parsed.as_object(), "minecraft", "core", true, false);
    }

    void __initialization__versions_inital() { //skips items assignation
        auto& latest_version = registers::individual_registers.at(latest_protocol_version);
        {
            auto current_effect = latest_version.at("minecraft:mob_effect").at("entries").as_compound();
            for (auto& [name, decl] : current_effect)
                registers::effects[name] = effect{name, (uint32_t)decl.at("protocol_id")};
            {
                registers::effects_cache.resize(registers::effects.size());
                auto it = registers::effects.begin();
                auto end = registers::effects.end();
                while (it != end) {
                    registers::effects_cache.at(it->second.id) = it;
                    ++it;
                }
            }
        }
        {
            auto current_potion = latest_version.at("minecraft:potion").at("entries").as_compound();
            for (auto& [name, decl] : current_potion)
                registers::potions[name] = potion{name, (uint32_t)decl.at("protocol_id")};
            {
                registers::potions_cache.resize(registers::potions.size());
                auto it = registers::potions.begin();
                auto end = registers::potions.end();
                while (it != end) {
                    registers::potions_cache.at(it->second.id) = it;
                    ++it;
                }
            }
        }
        {
            auto attribute = latest_version.at("minecraft:attribute").at("entries").as_compound();
            for (auto& [name, decl] : attribute)
                registers::attributes[name] = item_attribute{name, (uint32_t)decl.at("protocol_id")};
            {
                registers::attributes_cache.resize(registers::attributes.size());
                auto it = registers::attributes.begin();
                auto end = registers::attributes.end();
                while (it != end) {
                    registers::attributes_cache.at(it->second.id) = it;
                    ++it;
                }
            }
        }

        for (auto& [proto_id, decl] : registers::individual_registers) {
            for (auto& [name, decl] : decl.at("minecraft:mob_effect").at("entries").as_compound()) {
                auto it = registers::effects.find(name);
                if (it != registers::effects.end()) {
                    it->second.protocol[proto_id] = (uint32_t)decl.at("protocol_id");
                    effect::protocol_aliases[proto_id][it->second.id] = it->second.id;
                }
            }
            for (auto& [name, decl] : decl.at("minecraft:potion").at("entries").as_compound()) {
                auto it = registers::potions.find(name);
                if (it != registers::potions.end()) {
                    it->second.protocol[proto_id] = (uint32_t)decl.at("protocol_id");
                    potion::protocol_aliases[proto_id][it->second.id] = it->second.id;
                }
            }
            for (auto& [name, decl] : decl.at("minecraft:attribute").at("entries").as_compound()) {
                auto it = registers::attributes.find(name);
                if (it != registers::attributes.end()) {
                    it->second.protocol[proto_id] = (uint32_t)decl.at("protocol_id");
                    item_attribute::protocol_aliases[proto_id][it->second.id] = it->second.id;
                }
            }
            {
                auto current_registry_blocks = decl.at("minecraft:block").at("entries").as_compound();
                auto& proto_data = base_objects::static_block_data::internal_block_aliases_protocol[proto_id];
                for (auto& [name, decl] : current_registry_blocks)
                    proto_data[name] = (uint32_t)decl.at("protocol_id");
            }
            {
                auto& proto_data = base_objects::entity_data::internal_entity_aliases_protocol[proto_id];
                for (auto& [name, decl] : decl.at("minecraft:entity_type").at("entries").as_compound())
                    proto_data[name] = (uint32_t)decl.at("protocol_id");
            }
        }
        //set aliases

        for (auto& [proto_id, decl] : internal_compatibility_versions) {
            auto it = base_objects::entity_data::internal_entity_aliases_protocol.find(proto_id);
            if (it == base_objects::entity_data::internal_entity_aliases_protocol.end())
                continue;
            auto& proto_data = it->second;
            for (auto& [name, id] : decl.at("protocol_entity_aliases").as_object())
                proto_data[name] = proto_data.at(std::string(id.as_string()));
        }

        {
            std::unordered_map<std::string, std::string> filter;
            {
                auto tmp = boost::json::parse(resources::registry::compatibility::block_aliases).as_object();
                filter.reserve(tmp.size());
                for (auto& [match, replace] : tmp)
                    filter[match] = std::string(replace.as_string());
            }

            base_objects::block::access_full_block_data(
                [&](auto& arr, auto& ignore) {
                    for (auto& block : arr) {
                        for (auto& [match, replace] : filter) {
                            auto res = processor::process_match(block->name, match, replace);
                            if (res)
                                block->block_aliases.push_back(std::move(*res));
                        }
                    }
                }
            );
        }
        base_objects::static_block_data::initialize_blocks();
        base_objects::entity_data::initialize_entities();
    }

    void __initialization__versions_post() {
        for (auto& [proto_id, decl] : registers::individual_registers)
            for (auto& [name, decl] : decl.at("minecraft:item").at("entries").as_compound())
                base_objects::static_slot_data::internal_item_aliases_protocol[proto_id][name] = (uint32_t)decl.at("protocol_id");
        //set aliases

        {
            std::unordered_map<std::string, std::string> filter;
            {
                auto tmp = boost::json::parse(resources::registry::compatibility::item_aliases).as_object();
                filter.reserve(tmp.size());
                for (auto& [match, replace] : tmp)
                    filter[match] = std::string(replace.as_string());
            }

            base_objects::slot_data::enumerate_slot_data(
                [&](auto& item) {
                    auto& name = item.id;
                    auto& item_aliases = item.item_aliases;

                    for (auto& [match, replace] : filter) {
                        auto res = processor::process_match(name, match, replace);
                        if (res)
                            item_aliases.push_back(std::move(*res));
                    }
                }
            );
        }

        for (auto&& item : internal_compatibility_versions.at(api::configuration::get().protocol.allowed_versions_processed.min()).at("disabled_armor_trim_patterns").as_array())
            registers::armorTrimPatterns.erase((std::string)item.as_string());

        base_objects::static_slot_data::initialize_items();
    }

    void initialize() {
        internal_compatibility_versions = {
            {770, boost::json::parse(resources::registry::compatibility::versions::_770).as_object()},
        };
        initialize_entities();
        load_blocks();
        auto parsed_items = boost::json::parse(resources::registry::items);
        std::unordered_set<std::string> items_filter;
        {
            auto& to_filter = internal_compatibility_versions.at(api::configuration::get().protocol.allowed_versions_processed.min()).at("disabled_items").as_array();
            items_filter.reserve(to_filter.size());
            for (auto&& item : to_filter)
                items_filter.insert((std::string)item.as_string());

            for (auto&& [name, decl] : parsed_items.as_object()) {
                if (items_filter.contains(name))
                    continue;
                base_objects::static_slot_data slot_data;
                slot_data.id = name;
                base_objects::slot_data::add_slot_data(std::move(slot_data));
            }
        }
        prepare_built_in_pack();
        prepare_versions();
        __initialization__versions_inital();
        {
            //complete initialization
            for (auto&& [name, decl] : parsed_items.as_object()) {
                if (items_filter.contains(name))
                    continue;
                std::unordered_map<std::string, base_objects::slot_component::unified> components;
                for (auto& [name, value] : decl.as_object().at("components").as_object())
                    components[name] = base_objects::slot_component::parse_component(name, conversions::json::from_json(value));
                base_objects::slot_data::get_slot_data(name).default_components = std::move(components);
            }
        }
        __initialization__versions_post();
        load_registers_complete();
        internal_compatibility_versions.clear();
    }
}
