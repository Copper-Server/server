/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <algorithm>
#include <cassert>
#include <random>
#include <src/base_objects/number_provider.hpp>

namespace copper_server::base_objects {
    std::default_random_engine& get_thread_local_engine() {
        static thread_local std::default_random_engine engine(std::random_device{}());
        return engine;
    }

    std::shared_ptr<number_provider> number_provider::parse_provider(const enbt::value& other_data) {
        if (other_data.get_type() == enbt::type::floating)
            return std::make_shared<number_provider_constant>((float)other_data);
        else if (other_data.is_numeric() || other_data.is_none()) {
            return std::make_shared<number_provider_constant>((int32_t)other_data);
        } else if (other_data.is_compound()) {
            if (other_data.contains("type")) {
                auto& type = other_data.at("type").as_string();
                if (type == "constant" || type == "minecraft:constant") {
                    if (other_data.get_type() == enbt::type::floating)
                        return std::make_shared<number_provider_constant>((float)other_data);
                    else
                        return std::make_shared<number_provider_constant>((int32_t)other_data);
                } else if (type == "uniform" || type == "minecraft:uniform") {
                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;
                    if (other_data.contains("min")) {
                        auto& min_ = other_data["min"];
                        min = min_.type_equal(enbt::type::floating) ? (float)min_ : (int32_t)min_;
                    } else if (other_data.contains("min_inclusive")) {
                        auto& min_ = other_data["min_inclusive"];
                        min = min_.type_equal(enbt::type::floating) ? (float)min_ : (int32_t)min_;
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (other_data.contains("max")) {
                        auto max_ = other_data["max"];
                        max = max_.type_equal(enbt::type::floating) ? (float)max_ : (int32_t)max_;
                    } else if (other_data.contains("max_inclusive")) {
                        auto max_ = other_data["max_inclusive"];
                        max = max_.type_equal(enbt::type::floating) ? (float)max_ : (int32_t)max_;
                    } else
                        max = std::numeric_limits<int32_t>::max();

                    return std::make_shared<number_provider_uniform>(min, max);
                } else if (type == "binominal" || type == "minecraft:binominal") {
                    return std::make_shared<number_provider_binomial>(
                        parse_provider(other_data.at("n")),
                        parse_provider(other_data.at("p"))
                    );
                } else if (type == "clamped_normal" || type == "minecraft:clamped_normal") {
                    float mean = other_data.at("mean");
                    float deviation = other_data.at("deviation");
                    int32_t min_inclusive = other_data.at("min_inclusive");
                    int32_t max_inclusive = other_data.at("max_inclusive");
                    return std::make_shared<number_provider_clamped_normal>(mean, deviation, min_inclusive, max_inclusive);
                } else if (type == "clamped" || type == "minecraft:clamped") {
                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;

                    if (other_data.contains("min")) {
                        auto& min_ = other_data["min"];
                        min = min_.type_equal(enbt::type::floating) ? (float)min_ : (int32_t)min_;
                    } else if (other_data.contains("min_inclusive")) {
                        auto& min_ = other_data["min_inclusive"];
                        min = min_.type_equal(enbt::type::floating) ? (float)min_ : (int32_t)min_;
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (other_data.contains("max")) {
                        auto max_ = other_data["max"];
                        max = max_.type_equal(enbt::type::floating) ? (float)max_ : (int32_t)max_;
                    } else if (other_data.contains("max_inclusive")) {
                        auto max_ = other_data["max_inclusive"];
                        max = max_.type_equal(enbt::type::floating) ? (float)max_ : (int32_t)max_;
                    } else
                        max = std::numeric_limits<int32_t>::max();

                    return std::make_shared<number_provider_clamped>(min, max, parse_provider(other_data.at("source")));
                } else if (type == "trapezoid" || type == "minecraft:trapezoid") {
                    int32_t min = other_data.at("min");
                    int32_t max = other_data.at("max");
                    int32_t plateau = other_data.at("plateau");
                    return std::make_shared<number_provider_trapezoid>(min, max, plateau);
                } else if (type == "weighted_list" || type == "minecraft:weighted_list") {
                    std::vector<std::pair<std::shared_ptr<number_provider>, double>> values;
                    auto values_e = other_data.at("values").as_array();
                    values.reserve(values_e.size());
                    for (auto&& val : values_e) {
                        auto weight = val.contains("weight") ? (float)val["weight"] : 1.0;
                        values.push_back({parse_provider(val.at("data")), weight});
                    }
                    return std::make_shared<number_provider_weighted_list>(values);
                } else if (type == "biased_to_bottom" || type == "minecraft:biased_to_bottom") {

                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;
                    if (other_data.contains("min")) {
                        auto& min_ = other_data["min"];
                        min = min_.type_equal(enbt::type::floating) ? (float)min_ : (int32_t)min_;
                    } else if (other_data.contains("min_inclusive")) {
                        auto& min_ = other_data["min_inclusive"];
                        min = min_.type_equal(enbt::type::floating) ? (float)min_ : (int32_t)min_;
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (other_data.contains("max")) {
                        auto max_ = other_data["max"];
                        max = max_.type_equal(enbt::type::floating) ? (float)max_ : (int32_t)max_;
                    } else if (other_data.contains("max_inclusive")) {
                        auto max_ = other_data["max_inclusive"];
                        max = max_.type_equal(enbt::type::floating) ? (float)max_ : (int32_t)max_;
                    } else
                        max = std::numeric_limits<int32_t>::max();
                    return std::make_shared<number_provider_biased_to_bottom>(min, max);
                } else if (type == "score" || type == "minecraft:score") {
                    base_objects::number_provider_score res;
                    res.score = (std::string)other_data.at("score");
                    res.scale = other_data.contains("scale") ? std::optional<float>((float)other_data["scale"]) : std::nullopt;
                    auto target = other_data.at("target").as_compound();
                    std::string score_type = target.at("type");
                    if (score_type == "fixed")
                        res.target.value = (std::string)target.at("name");
                    else if (score_type == "context")
                        res.target.value = (std::string)target.at("target");
                    else
                        throw std::runtime_error("Invalid target type: " + score_type);
                    res.target.type = score_type;
                    return std::make_shared<number_provider_score>(std::move(res));
                } else if (type == "storage" || type == "minecraft:storage") {
                    return std::make_shared<number_provider_storage>((std::string)other_data.at("storage"), (std::string)other_data.at("path"));
                } else if (type == "enchantment_level" || type == "minecraft:enchantment_level")
                    return std::make_shared<number_provider_enchantment_level>((std::string)other_data.at("amount"));
                else
                    throw std::runtime_error("Invalid number provider type: " + type);
            } else {
                if (other_data.at("min").get_type() == enbt::type::floating)
                    return std::make_shared<number_provider_uniform>((float)other_data.at("min"), (float)other_data.at("max"));
                else
                    return std::make_shared<number_provider_uniform>((int32_t)other_data.at("min"), (int32_t)other_data.at("max"));
            }
        } else
            return std::make_shared<number_provider_constant>((int32_t)other_data);
    }

    float number_provider_constant::get_float() const noexcept {
        return std::visit(
            [](auto&& arg) -> float {
                return static_cast<float>(arg);
            },
            value
        );
    }

    int32_t number_provider_constant::get_int() const noexcept {
        return std::visit(
            [](auto&& arg) -> int32_t {
                return static_cast<int32_t>(arg);
            },
            value
        );
    }

    enbt::value number_provider_constant::get_enbt() const {
        return std::visit([](auto it) -> enbt::value { return it; }, value);
    }

    float number_provider_uniform::get_float() const noexcept {
        return std::uniform_real_distribution<float>(get_min_inclusive_float(), get_max_exclusive_float())(get_thread_local_engine());
    }

    int32_t number_provider_uniform::get_int() const noexcept {
        return std::uniform_int_distribution<int>(get_min_inclusive_int(), get_max_exclusive_int())(get_thread_local_engine());
    }

    enbt::value number_provider_uniform::get_enbt() const {
        return enbt::compound{
            {"type", "minecraft:uniform"},
            {"min", std::visit([](auto it) -> enbt::value { return it; }, min_inclusive)},
            {"max", std::visit([](auto it) -> enbt::value { return it; }, max_exclusive)}
        };
    }

    float number_provider_clamped_normal::get_float() const noexcept {
        return std::clamp<float>(std::normal_distribution<float>(mean, deviation)(get_thread_local_engine()), (float)min, (float)max);
    }

    int32_t number_provider_clamped_normal::get_int() const noexcept {
        return std::clamp<int32_t>((int32_t)std::normal_distribution<float>(mean, deviation)(get_thread_local_engine()), min, max);
    }

    enbt::value number_provider_clamped_normal::get_enbt() const {
        return enbt::compound{
            {"type", "minecraft:clamped_normal"},
            {"mean", mean},
            {"deviation", deviation},
            {"min", min},
            {"max", max}
        };
    }

    float number_provider_trapezoid::get_float() const noexcept {
        auto& engine = get_thread_local_engine();

        const float f_min = static_cast<float>(min);
        const float f_max = static_cast<float>(max);
        const float f_plateau = static_cast<float>(plateau);

        if (f_plateau >= (f_max - f_min))
            return std::uniform_real_distribution<float>(f_min, f_max)(engine);

        const float slope_width = (f_max - f_min - f_plateau) / 2.0f;
        const float plateau_and_slope = f_plateau + slope_width;

        std::uniform_real_distribution<float> dist1(0.0f, slope_width);
        std::uniform_real_distribution<float> dist2(0.0f, plateau_and_slope);

        return f_min + dist1(engine) + dist2(engine);
    }

    int32_t number_provider_trapezoid::get_int() const noexcept {
        auto& engine = get_thread_local_engine();

        if (plateau >= (max - min))
            return std::uniform_int_distribution<int32_t>(min, max)(engine);

        const int32_t slope_width = (max - min - plateau) / 2;
        const int32_t plateau_and_slope = plateau + slope_width;

        std::uniform_int_distribution<int32_t> dist1(0, slope_width);
        std::uniform_int_distribution<int32_t> dist2(0, plateau_and_slope);

        return min + dist1(engine) + dist2(engine);
    }

    enbt::value number_provider_trapezoid::get_enbt() const {
        return enbt::compound{
            {"type", "minecraft:trapezoid"},
            {"min", min},
            {"max", max},
            {"plateau", plateau}
        };
    }

    float number_provider_clamped::get_float() const noexcept {
        return std::clamp<float>(source->get_float(), get_min_inclusive_float(), get_max_inclusive_float());
    }

    int32_t number_provider_clamped::get_int() const noexcept {
        return std::clamp<int32_t>(source->get_int(), get_min_inclusive_int(), get_max_inclusive_int());
    }

    enbt::value number_provider_clamped::get_enbt() const {
        return enbt::compound{
            {"type", "minecraft:clamped"},
            {"min", std::visit([](auto it) -> enbt::value { return it; }, min_inclusive)},
            {"max", std::visit([](auto it) -> enbt::value { return it; }, max_inclusive)},
            {"source", source->get_enbt()}
        };
    }

    float number_provider_weighted_list::get_float() const noexcept {
        double sum = 0;
        for (const auto& [value, weight] : values)
            sum += weight;
        double random = std::uniform_real_distribution<double>(0, sum)(get_thread_local_engine());
        for (const auto& [value, weight] : values) {
            random -= weight;
            if (random <= 0)
                return value->get_float();
        }
        return values.back().first->get_float();
    }

    int32_t number_provider_weighted_list::get_int() const noexcept {
        double sum = 0;
        for (const auto& [value, weight] : values)
            sum += weight;
        double random = std::uniform_real_distribution<double>(0, sum)(get_thread_local_engine());
        for (const auto& [value, weight] : values) {
            random -= weight;
            if (random <= 0)
                return value->get_int();
        }
        return values.back().first->get_int();
    }

    enbt::value number_provider_weighted_list::get_enbt() const {
        enbt::fixed_array arr;
        arr.reserve(values.size());
        for (auto& it : values)
            arr.push_back(enbt::compound{{"data", it.first->get_enbt()}, {"weight", it.second}});

        return enbt::compound{
            {"type", "minecraft:weighted_list"},
            {"values", std::move(arr)}
        };
    }

    float number_provider_biased_to_bottom::get_float() const noexcept {
        std::uniform_real_distribution<float> dist(get_min_inclusive_float(), get_max_exclusive_float());
        return std::min(dist(get_thread_local_engine()), dist(get_thread_local_engine()));
    }

    int32_t number_provider_biased_to_bottom::get_int() const noexcept {
        std::uniform_int_distribution<int32_t> dist(get_min_inclusive_int(), get_max_exclusive_int());
        return std::min(dist(get_thread_local_engine()), dist(get_thread_local_engine()));
    }

    enbt::value number_provider_biased_to_bottom::get_enbt() const {
        return enbt::compound{
            {"type", "minecraft:biased_to_bottom"},
            {"min", std::visit([](auto it) -> enbt::value { return it; }, min_inclusive)},
            {"max", std::visit([](auto it) -> enbt::value { return it; }, max_exclusive)}
        };
    }

    float number_provider_binomial::get_float() const noexcept {
        return static_cast<float>(std::binomial_distribution<int32_t>(n->get_int(), static_cast<double>(p->get_float()))(get_thread_local_engine()));
    }

    int32_t number_provider_binomial::get_int() const noexcept {
        return std::binomial_distribution<int32_t>(n->get_int(), static_cast<double>(p->get_float()))(get_thread_local_engine());
    }

    enbt::value number_provider_binomial::get_enbt() const {
        return enbt::compound{
            {"type", "minecraft:binomial"},
            {"n", n->get_enbt()},
            {"p", p->get_enbt()}
        };
    }

    float number_provider_score::get_float() const noexcept {
        return 0.0f; //TODO
    }

    int32_t number_provider_score::get_int() const noexcept {
        return 0; //TODO
    }

    enbt::value number_provider_score::get_enbt() const {
        if (target.type == "fixed") {
            return enbt::compound{
                {"type", "minecraft:score"},
                {"scale", scale},
                {"score", score},
                {"target", enbt::compound{{"type", target.type}, {"name", target.value}}}
            };
        } else {
            return enbt::compound{
                {"type", "minecraft:score"},
                {"scale", scale},
                {"score", score},
                {"target", enbt::compound{{"type", target.type}, {"target", target.value}}}
            };
        }
    }

    float number_provider_storage::get_float() const noexcept {
        return 0.0f; //TODO
    }

    int32_t number_provider_storage::get_int() const noexcept {
        return 0; //TODO
    }

    enbt::value number_provider_storage::get_enbt() const {
        return enbt::compound{
            {"type", "minecraft:storage"},
            {"storage", storage},
            {"path", path}
        };
    }

    float number_provider_enchantment_level::get_float() const noexcept {
        return 0.0f; //TODO
    }

    int32_t number_provider_enchantment_level::get_int() const noexcept {
        return 0; //TODO
    }

    enbt::value number_provider_enchantment_level::get_enbt() const {
        return enbt::compound{
            {"type", "minecraft:enchantment_level"},
            {"amount", amount}
        };
    }
}
