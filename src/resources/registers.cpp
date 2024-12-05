#include <resources/include.hpp>
#include <src/api/configuration.hpp>
#include <src/api/recipe.hpp>
#include <src/base_objects/entity.hpp>
#include <src/log.hpp>
#include <src/registers.hpp>
#include <src/util/conversions.hpp>
#include <src/util/json_helpers.hpp>

namespace copper_server {
    namespace resources {
        std::unordered_map<uint32_t, boost::json::object> internal_compatibility_versions;

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

        using namespace util;
        using namespace registers;

        void registers_reset() {
            biomes.clear();
            biomes.clear();
            chatTypes.clear();
            armorTrimPatterns.clear();
            armorTrimMaterials.clear();
            wolfVariants.clear();
            dimensionTypes.clear();
            damageTypes.clear();
            bannerPatterns.clear();
            paintingVariants.clear();
            biomes_cache.clear();
            biomes_cache.clear();
            chatTypes_cache.clear();
            armorTrimPatterns_cache.clear();
            armorTrimMaterials_cache.clear();
            wolfVariants_cache.clear();
            dimensionTypes_cache.clear();
            damageTypes_cache.clear();
            bannerPatterns_cache.clear();
            paintingVariants_cache.clear();
            tags.clear();
        }

        void load_registers_complete() {
            id_assigner(biomes, biomes_cache);
            id_assigner(chatTypes, chatTypes_cache);
            id_assigner(armorTrimPatterns, armorTrimPatterns_cache);
            id_assigner(armorTrimMaterials, armorTrimMaterials_cache);
            id_assigner(wolfVariants, wolfVariants_cache);
            id_assigner(dimensionTypes, dimensionTypes_cache);
            id_assigner(damageTypes, damageTypes_cache);
            id_assigner(bannerPatterns, bannerPatterns_cache);
            id_assigner(paintingVariants, paintingVariants_cache);

            for (auto& it : tags)
                for (auto& it2 : it.second)
                    for (auto& it3 : it2.second)
                        it3.second.commit();
        }

        void initialize_entities() {
            auto parsed = boost::json::parse(resources::registry::entities);
            for (auto& [id, obj_] : parsed.as_object()) {
                auto& obj = obj_.as_object();
                base_objects::entity_data data;
                data.id = id;
                data.name = obj.at("name").as_string();
                data.translation_resource_key = obj.at("translation_resource_key").as_string();
                data.kind_name = obj.at("kind_name").as_string();
                if (obj.contains("base_bounds")) {
                    auto& val = obj.at("base_bounds");
                    base_objects::bounding bounds;
                    if (val.is_object()) {
                        bounds.xz = val.at("xz").to_number<double>();
                        bounds.y = val.at("y").to_number<double>();
                    } else {
                        bounds.xz = val.at(0).to_number<double>();
                        bounds.y = val.at(1).to_number<double>();
                    }
                    data.base_bounds = bounds;
                } else {
                    log::warn("Resource loader", "Entity " + (std::string)id + " has no base bounds declared. Using {0, 0}.");
                    data.base_bounds = {0, 0};
                }

                if (obj.contains("bounds_mode")) {
                    static const std::unordered_map<std::string, base_objects::entity_data::bounds_mode_t> mode_map{
                        {"solid", base_objects::entity_data::bounds_mode_t::solid},
                        {"solid_except_self_type", base_objects::entity_data::bounds_mode_t::solid_except_self_type},
                        {"solid_for_vehicles", base_objects::entity_data::bounds_mode_t::solid_for_vehicles},
                        {"weak", base_objects::entity_data::bounds_mode_t::weak},
                        {"none", base_objects::entity_data::bounds_mode_t::none},
                    };
                    data.bounds_mode = mode_map.at((std::string)obj.at("bounds_mode").as_string());
                }

                if (obj.contains("acceleration"))
                    data.acceleration = obj.at("acceleration").to_number<float>();
                else
                    data.acceleration = 5.6f;

                if (obj.contains("inventory"))
                    data.data["slot"] = util::conversions::json::from_json(obj.at("inventory"));

                if (obj.contains("custom_data"))
                    data.data["custom_data"].merge(util::conversions::json::from_json(obj.at("custom_data")));
                if (obj.contains("base_health")) {
                    auto& base_health = obj.at("base_health");
                    if (base_health.is_number())
                        data.base_health = obj.at("base_health").to_number<float>();
                    else if (base_health == "infinity")
                        data.base_health = NAN;
                } else
                    data.base_health = NAN;

                if (obj.contains("is_living_entity"))
                    data.living_entity = obj.at("is_living_entity").as_bool();
                else
                    data.living_entity = false;


                base_objects::entity_data::register_entity(std::move(data));
            }
        }

        void load_file_biomes(js_object&& bio_js, const std::string& id) {
            check_override(biomes, id, "biome");
            Biome bio;
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
                    mood_sound.block_search_extend = mood_sound_js["block_search_extend"].or_apply(8);
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
                    js_object music_js = js_object::get_object(effects_js["music"]);
                    Biome::Music music;
                    music.sound = (std::string)music_js["sound"];
                    music.min_delay = music_js["min_delay"].or_apply(12000);
                    music.max_delay = music_js["max_delay"].or_apply(24000);
                    music.replace_current_music = music_js["replace_current_music"].or_apply(true);
                    effects.music = std::move(music);
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

        void load_file_chatType(js_object&& type_js, const std::string& id) {
            check_override(chatTypes, id, "chat type");
            ChatType type;
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

        void load_file_advancements(js_object&& advancement_js, const std::string& id) {
            check_conflicts(advancements, id, "advancements");
            Advancement advancement;
            if (advancement_js.contains("display")) {
                auto display_js = js_object::get_object(advancement_js["display"]);
                Advancement::Display display;
                {
                    auto icon_js = js_object::get_object(display_js["icon"]);
                    display.icon.item = icon_js["item"];
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

        void load_file_jukebox_song(js_object&& song_js, const std::string& id) {
            check_conflicts(jukebox_songs, id, "jukebox song");
            JukeboxSong song;
            song.comparator_output = song_js["comparator_output"];
            song.length_in_seconds = song_js["length_in_seconds"];
            song.description = Chat::fromEnbt(util::conversions::json::from_json(song_js["description"].get()));
            auto sound_event_js = song_js["sound_event"];
            if (sound_event_js.is_string())
                song.sound_event = (std::string)sound_event_js;
            else {
                auto sound_event_obj = js_object::get_object(sound_event_js);
                base_objects::slot_component::inner::sound_extended ex;
                ex.sound_name = sound_event_obj["sound_id"];
                if (sound_event_obj.contains("range"))
                    ex.fixed_range = sound_event_obj["range"];
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

        void load_file_loot_table(js_object&& loot_table_js, const std::string& id) {
            check_conflicts(loot_table, id, "loot table");

            loot_table_item item;
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

        void load_file_armorTrimPattern(js_object&& pattern_js, const std::string& id) {
            check_override(armorTrimPatterns, id, "armor trim pattern");
            ArmorTrimPattern pattern;
            pattern.asset_id = (std::string)pattern_js["asset_id"];
            pattern.decal = pattern_js["decal"];
            pattern.template_item = (std::string)pattern_js["template_item"];
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

        void load_file_armorTrimMaterial(js_object&& material_js, const std::string& id) {
            check_override(armorTrimMaterials, id, "armor trim material");
            ArmorTrimMaterial material;
            material.asset_name = (std::string)material_js["asset_name"];
            material.ingredient = (std::string)material_js["ingredient"];
            material.item_model_index = material_js["item_model_index"];
            {
                auto desc = material_js["description"];
                if (desc.is_string())
                    material.description = (std::string)desc;
                else
                    material.description = Chat::fromEnbt(conversions::json::from_json(desc.get()));
            }
            if (material_js.contains("override_armor_materials")) {
                auto override_armor_materials = material_js["override_armor_materials"];
                for (auto&& [name, material_] : js_object::get_object(override_armor_materials))
                    material.override_armor_materials[(std::string)name] = (std::string)material_;
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

        void load_file_wolfVariant(js_object&& variant_js, const std::string& id) {
            check_override(wolfVariants, id, "wolf variant");
            WolfVariant variant;
            variant.wild_texture = (std::string)variant_js.at("wild_texture");
            variant.tame_texture = (std::string)variant_js.at("tame_texture");
            variant.angry_texture = (std::string)variant_js.at("angry_texture");
            auto _biomes = variant_js.at("biomes");
            if (_biomes.is_array()) {
                auto biomes = js_array::get_array(variant_js.at("biomes"));
                variant.biomes.reserve(biomes.size());
                for (auto&& biome : biomes)
                    variant.biomes.push_back(biome);
            } else
                variant.biomes.push_back((std::string)_biomes);
            wolfVariants[id] = std::move(variant);
        }

        void load_file_wolfVariant(const std::filesystem::path& file_path, const std::string& id) {
            check_override(wolfVariants, id, "wolf variant");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_wolfVariant(js_object::get_object(res.value()), id);
        }

        void load_file_dimensionType(js_object&& type_js, const std::string& id) {
            check_override(dimensionTypes, id, "dimension type");
            DimensionType type;
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

        void load_file_enchantment(js_object&& type_js, const std::string& id) {
            check_conflicts(enchantments, id, "enchantments");
            enchantment type;
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

        void load_file_enchantment_provider(boost::json::object& type_js, const std::string& id) {
            check_conflicts(enchantment_providers, id, "enchantment providers");
            enchantment_providers[id] = util::conversions::json::from_json(type_js);
        }

        void load_file_enchantment_provider(const std::filesystem::path& file_path, const std::string& id) {
            check_conflicts(enchantment_providers, id, "enchantment providers");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_enchantment_provider(res.value(), id);
        }

        void load_file_damageType(js_object&& type_js, const std::string& id) {
            check_override(damageTypes, id, "damage type");
            DamageType type;
            type.message_id = (std::string)type_js["message_id"];
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

        void load_file_bannerPattern(js_object&& pattern_js, const std::string& id) {
            check_override(bannerPatterns, id, "banner pattern");
            BannerPattern pattern;
            pattern.asset_id = (std::string)pattern_js["asset_id"];
            pattern.translation_key = (std::string)pattern_js["translation_key"];
            bannerPatterns[id] = std::move(pattern);
        }

        void load_file_bannerPattern(const std::filesystem::path& file_path, const std::string& id) {
            check_override(bannerPatterns, id, "banner pattern");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_bannerPattern(js_object::get_object(res.value()), id);
        }

        void load_file_paintingVariant(js_object&& variant_js, const std::string& id) {
            check_override(paintingVariants, id, "painting variant");
            PaintingVariant variant;
            variant.asset_id = (std::string)variant_js["asset_id"];
            variant.height = variant_js["height"];
            variant.width = variant_js["width"];
            paintingVariants[id] = std::move(variant);
        }

        void load_file_paintingVariant(const std::filesystem::path& file_path, const std::string& id) {
            check_override(paintingVariants, id, "painting variant");
            auto res = try_read_json_file(file_path);
            if (!res)
                throw std::runtime_error("Failed to read file: " + file_path.string());
            load_file_paintingVariant(js_object::get_object(res.value()), id);
        }

        void load_file_recipe(js_object&& variant_js, const std::string& id) {
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

                if (the_tag.starts_with("#")) {
                    result.push_back(unfold_tag(type, namespace_, the_tag.substr(1)));
                } else
                    result.push_back(the_tag);
            }
            tags[type][namespace_][path_].push_back(std::move(result));
        }

        void load_file_tags(js_object&& tags_, const std::string& type, const std::string& namespace_, const std::string& path_) {
            if (tags_.contains("replace")) {
                bool replace = tags_["replace"];
                if (replace)
                    tags[type][namespace_][path_].clear();
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
            std::string id = (namespace_.empty() ? default_namespace : namespace_) + ":" + path_;
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
            std::string id = (namespace_.empty() ? default_namespace : namespace_) + ":" + path_;
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
            auto parsed = boost::json::parse(resources::registry::blocks);

            base_objects::block::access_full_block_data(std::function(
                [&](
                    std::vector<std::shared_ptr<base_objects::static_block_data>>& full_block_data_,
                    std::unordered_map<std::string, std::shared_ptr<base_objects::static_block_data>>& named_full_block_data
                ) {
                    size_t usable = 30000;
                    full_block_data_.resize(usable);
                    for (auto&& [name, decl] : parsed.as_object()) {
                        std::unordered_map<std::string, std::unordered_set<std::string>> properties_def;
                        if (decl.as_object().contains("properties")) {
                            for (auto&& [prop_name, prop] : decl.at("properties").as_object()) {
                                std::unordered_set<std::string> prop_list;
                                for (auto&& prop_val : prop.as_array())
                                    prop_list.insert((std::string)prop_val.as_string());
                                properties_def[(std::string)prop_name] = std::move(prop_list);
                            }
                        }

                        decltype(base_objects::static_block_data::assigned_states) associated_states;
                        std::optional<base_objects::block_id_t> default_associated_state;
                        for (auto&& state : decl.at("states").as_array()) {
                            auto&& state_ = state.as_object();
                            base_objects::block_id_t id = as_uint(state_.at("id"));

                            std::unordered_map<std::string, std::string> state_properties;
                            if (state_.contains("properties")) {
                                for (auto&& [prop_name, prop] : state_.at("properties").as_object())
                                    state_properties[prop_name] = (std::string)prop.as_string();
                            }

                            if (state_.contains("default")) {
                                if (state_.at("default").as_bool())
                                    default_associated_state = id;
                            }
                            associated_states.insert({id, std::move(state_properties)});
                        }

                        auto data = std::make_shared<base_objects::static_block_data>();


                        for (auto& [id, _unused] : associated_states) {
                            if (full_block_data_[id] != nullptr) {
                                throw std::runtime_error("Duplicate block id: " + std::to_string(id));
                            }
                            full_block_data_[id] = data;
                        }
                        if (named_full_block_data.contains((std::string)name))
                            throw std::runtime_error("Duplicate block name: " + (std::string)name);


                        data->name = name;
                        data->default_state = default_associated_state.value_or(associated_states.left.begin()->first);
                        data->states = std::move(properties_def);
                        data->assigned_states = std::move(associated_states);
                        data->defintion = util::conversions::json::from_json(decl.at("definition"));

                        named_full_block_data[(std::string)name] = std::move(data);
                    }

                    for (auto it = full_block_data_.rbegin(); it != full_block_data_.rend(); ++it) {
                        if (*it != nullptr)
                            break;
                        --usable;
                    }
                    full_block_data_.resize(usable);
                    for (auto it = full_block_data_.begin(); it != full_block_data_.end(); ++it) {
                        if (*it == nullptr) {
                            throw std::runtime_error("Gap between block definitions");
                        }
                    }
                    full_block_data_.shrink_to_fit();
                }
            ));
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
                {765, resources::registry::protocol::_765},
                {766, resources::registry::protocol::_766},
                {767, resources::registry::protocol::_767},
                {768, resources::registry::protocol::_768},
            };
            api::configuration::get().protocol.allowed_versions_processed.for_each([&](uint32_t version) {
                auto res = util::conversions::json::from_json(boost::json::parse(internal_protocol_aliases.at(version)));
                registers::individual_registers[version] = std::move(res);
            });
        }

        using tags_obj = std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, list_array<std::string>>>>;

        void __prepare_tags(tags_obj& tmp_obj, boost::json::object& parsed, const std::string& type, const std::string& namespace_, const std::string& tag) {
            for (auto&& [name, decl] : parsed) {
                auto& obj = decl.as_object();
                if (name.ends_with(".json")) {
                    if (obj.contains("values") || obj.contains("root")) {
                        auto& values = obj.contains("values") ? obj.at("values") : obj.at("root");
                        if (values.is_array()) {
                            bool replace = obj.contains("replace") ? obj.at("replace").as_bool() : false;
                            auto& items = tmp_obj[type][namespace_][tag + ":" + (std::string)name.substr(0, name.size() - 5)];
                            if (replace)
                                items.clear();
                            for (auto&& value : values.get_array())
                                items.push_back((std::string)value.as_string());
                            continue;
                        }
                    }
                } else
                    __prepare_tags(tmp_obj, obj, type, namespace_, tag + ":" + (std::string)name);
            }
        }

        void prepare_tags(boost::json::object& parsed, const std::string& namespace_) {
            tags_obj tmp_obj = registers::tags;
            for (auto&& [type, decl] : parsed)
                __prepare_tags(tmp_obj, decl.get_object(), type, namespace_, "");

            registers::tags = tmp_obj;
            for (auto&& [type, decl] : tmp_obj) {
                for (auto&& [namespace_, decl] : decl) {
                    for (auto&& [tag, decl] : decl) {
                        list_array<std::string> resolved_items;
                        for (auto& item : decl) {
                            if (item.starts_with("#")) {
                                resolved_items.push_back(unfold_tag(type, item).where([](const std::string& tag) {
                                    return !tag.starts_with("#");
                                }));
                            }
                        }
                        decl = std::move(resolved_items.commit());
                    }
                }
            }

            registers::tags = std::move(tmp_obj);
        }

        void process_item_(boost::json::object& decl, const std::string& namespace_, void (*fn)(js_object&&, const std::string&)) {
            for (auto& [id, value] : decl) {
                if (id.ends_with(".json")) {
                    std::string _id = namespace_ + std::string(id.substr(0, id.size() - 5));
                    fn(js_object::get_object(value), _id);
                } else
                    process_item_(value.as_object(), namespace_ + std::string(id), fn);
            }
        }

        void process_item_(boost::json::object& decl, const std::string& namespace_, void (*fn)(boost::json::object&, const std::string&)) {
            for (auto& [id, value] : decl) {
                if (id.ends_with(".json")) {
                    std::string _id = namespace_ + std::string(id.substr(0, id.size() - 5));
                    fn(value.as_object(), _id);
                } else
                    process_item_(value.as_object(), namespace_ + std::string(id), fn);
            }
        }

        void process_item(js_value& decl, const std::string& namespace_, void (*fn)(js_object&&, const std::string&)) {
            process_item_(decl.get().as_object(), namespace_ + ":", fn);
        }

        void process_item(js_value& decl, const std::string& namespace_, void (*fn)(boost::json::object&, const std::string&)) {
            process_item_(decl.get().as_object(), namespace_ + ":", fn);
        }

        void process_pack(boost::json::object& parsed, const std::string& namespace_, bool allowed_pack_nest) {
            auto data = js_object::get_object(parsed);
            if (data.contains("tags"))
                prepare_tags(data["tags"].get().as_object(), namespace_);
            for (auto&& [name, decl] : data) {
                if (name == "banner_pattern")
                    process_item(decl, namespace_, load_file_bannerPattern);
                else if (name == "painting_variant")
                    process_item(decl, namespace_, load_file_paintingVariant);
                else if (name == "damage_type")
                    process_item(decl, namespace_, load_file_damageType);
                else if (name == "dimension_type")
                    process_item(decl, namespace_, load_file_dimensionType);
                else if (name == "wolf_variant")
                    process_item(decl, namespace_, load_file_wolfVariant);
                else if (name == "trim_material")
                    process_item(decl, namespace_, load_file_armorTrimMaterial);
                else if (name == "trim_pattern")
                    process_item(decl, namespace_, load_file_armorTrimPattern);
                else if (name == "enchantment")
                    process_item(decl, namespace_, load_file_enchantment);
                else if (name == "enchantment_provider")
                    process_item(decl, namespace_, load_file_enchantment_provider);
                else if (name == "chat_type")
                    process_item(decl, namespace_, load_file_chatType);
                else if (name == "worldgen") {
                    for (auto&& [name, decl] : js_object::get_object(decl)) {
                        if (name == "biome")
                            process_item(decl, namespace_, load_file_biomes);
                    }
                }
            }
        }

        void process_pack(boost::json::object& parsed, const std::string& namespace_) {
            process_pack(parsed, namespace_, false);
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

        void process_pack(const std::filesystem::path& folder_path_to_data_without_namespace, const std::string& namespace_) {
            boost::json::value value = boost::json::object();
            auto& obj = value.get_object();
            for (auto& entry : std::filesystem::directory_iterator(folder_path_to_data_without_namespace)) {
                auto& inner = (obj[entry.path().filename().string()] = boost::json::object()).get_object();
                ___recursive_merge_json(inner, entry);
            }
            process_pack(obj, namespace_);
        }

        void process_pack(const std::filesystem::path& folder_path_to_data) {
            for (auto& entry : std::filesystem::directory_iterator(folder_path_to_data)) {
                if (entry.is_directory()) {
                    auto& path = entry.path();
                    std::string namespace_ = path.stem().string();
                    process_pack(path, namespace_);
                }
            }
        }

        void prepare_built_in_pack() {
            auto parsed = boost::json::parse(resources::data);
            process_pack(parsed.as_object(), "minecraft");
        }

        void __initialization__versions_inital() { //skips items assignation
            auto& latest_version = registers::individual_registers.at(768);
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
                if (proto_id != 768) { //TODO update entities for 768
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
                {765, boost::json::parse(resources::registry::compatibility::versions::_765).as_object()},
                {766, boost::json::parse(resources::registry::compatibility::versions::_766).as_object()},
                {767, boost::json::parse(resources::registry::compatibility::versions::_767).as_object()},
                {768, boost::json::parse(resources::registry::compatibility::versions::_768).as_object()},
            };
            initialize_entities();
            load_blocks();
            prepare_built_in_pack();
            prepare_versions();
            __initialization__versions_inital();
            load_items();
            __initialization__versions_post();
            load_registers_complete();
            internal_compatibility_versions.clear();
        }
    }
}
