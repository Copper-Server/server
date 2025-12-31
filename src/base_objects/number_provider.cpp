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

    std::shared_ptr<number_provider> number_provider::parse_provider(const util::nbt& other_data) {
        if (other_data.is_floating())
            return std::make_shared<number_provider_constant>(other_data.as_float());
        else if (other_data.is_numeric()) {
            return std::make_shared<number_provider_constant>(other_data.as_int());
        } else if (other_data.is_compound()) {
            if (other_data.contains("type")) {
                auto& type = other_data.at("type").get_string();
                if (type == "constant" || type == "minecraft:constant") {
                    if (other_data.is_floating())
                        return std::make_shared<number_provider_constant>(other_data.as_float());
                    else
                        return std::make_shared<number_provider_constant>(other_data.as_int());
                } else if (type == "uniform" || type == "minecraft:uniform") {
                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;
                    if (other_data.contains("min")) {
                        auto& min_ = other_data.at("min");
                        min = min_.is_floating() ? min_.get_float() : min_.get_int();
                    } else if (other_data.contains("min_inclusive")) {
                        auto& min_ = other_data.at("min_inclusive");
                        min = min_.is_floating() ? min_.get_float() : min_.get_int();
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (other_data.contains("max")) {
                        auto max_ = other_data.at("max");
                        max = max_.is_floating() ? max_.get_float() : max_.get_int();
                    } else if (other_data.contains("max_inclusive")) {
                        auto max_ = other_data.at("max_inclusive");
                        max = max_.is_floating() ? max_.get_float() : max_.get_int();
                    } else
                        max = std::numeric_limits<int32_t>::max();

                    return std::make_shared<number_provider_uniform>(min, max);
                } else if (type == "binominal" || type == "minecraft:binominal") {
                    return std::make_shared<number_provider_binomial>(
                        parse_provider(other_data.at("n")),
                        parse_provider(other_data.at("p"))
                    );
                } else if (type == "clamped_normal" || type == "minecraft:clamped_normal") {
                    float mean = other_data.at("mean").get_float();
                    float deviation = other_data.at("deviation").get_float();
                    int32_t min_inclusive = other_data.at("min_inclusive").get_int();
                    int32_t max_inclusive = other_data.at("max_inclusive").get_int();
                    return std::make_shared<number_provider_clamped_normal>(mean, deviation, min_inclusive, max_inclusive);
                } else if (type == "clamped" || type == "minecraft:clamped") {
                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;

                    if (other_data.contains("min")) {
                        auto& min_ = other_data.at("min");
                        min = min_.is_floating() ? min_.get_float() : min_.get_int();
                    } else if (other_data.contains("min_inclusive")) {
                        auto& min_ = other_data.at("min_inclusive");
                        min = min_.is_floating() ? min_.get_float() : min_.get_int();
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (other_data.contains("max")) {
                        auto max_ = other_data.at("max");
                        max = max_.is_floating() ? (float)max_.get_float() : max_.get_int();
                    } else if (other_data.contains("max_inclusive")) {
                        auto max_ = other_data.at("max_inclusive");
                        max = max_.is_floating() ? max_.get_float() : max_.get_int();
                    } else
                        max = std::numeric_limits<int32_t>::max();

                    return std::make_shared<number_provider_clamped>(min, max, parse_provider(other_data.at("source")));
                } else if (type == "trapezoid" || type == "minecraft:trapezoid") {
                    int32_t min = other_data.at("min").get_int();
                    int32_t max = other_data.at("max").get_int();
                    int32_t plateau = other_data.at("plateau").get_int();
                    return std::make_shared<number_provider_trapezoid>(min, max, plateau);
                } else if (type == "weighted_list" || type == "minecraft:weighted_list") {
                    std::vector<std::pair<std::shared_ptr<number_provider>, double>> values;
                    auto values_e = other_data.at("values").get_list();
                    values.reserve(values_e.size());
                    for (auto&& val : values_e) {
                        auto weight = val.contains("weight") ? val.at("weight").get_float() : 1.0;
                        values.push_back({parse_provider(val.at("data")), weight});
                    }
                    return std::make_shared<number_provider_weighted_list>(values);
                } else if (type == "biased_to_bottom" || type == "minecraft:biased_to_bottom") {

                    std::variant<int32_t, float> min;
                    std::variant<int32_t, float> max;
                    if (other_data.contains("min")) {
                        auto& min_ = other_data.at("min");
                        min = min_.is_floating() ? min_.as_float() : min_.get_int();
                    } else if (other_data.contains("min_inclusive")) {
                        auto& min_ = other_data.at("min_inclusive");
                        min = min_.is_floating() ? min_.as_float() : min_.get_int();
                    } else
                        min = std::numeric_limits<int32_t>::min();

                    if (other_data.contains("max")) {
                        auto max_ = other_data.at("max");
                        max = max_.is_floating() ? max_.as_float() : max_.get_int();
                    } else if (other_data.contains("max_inclusive")) {
                        auto max_ = other_data.at("max_inclusive");
                        max = max_.is_floating() ? max_.as_float() : max_.get_int();
                    } else
                        max = std::numeric_limits<int32_t>::max();
                    return std::make_shared<number_provider_biased_to_bottom>(min, max);
                } else if (type == "score" || type == "minecraft:score") {
                    base_objects::number_provider_score res;
                    res.score = other_data.at("score").as_string();
                    res.scale = other_data.get_compound().contains("scale") ? other_data.at("scale").as_float() : 1.0f;
                    auto& target = other_data.at("target").get_compound();
                    std::string score_type = target.at("type").as_string();
                    if (score_type == "fixed")
                        res.target.value = target.at("name").as_string();
                    else if (score_type == "context")
                        res.target.value = target.at("target").as_string();
                    else
                        throw std::runtime_error("Invalid target type: " + score_type);
                    res.target.type = score_type;
                    return std::make_shared<number_provider_score>(std::move(res));
                } else if (type == "storage" || type == "minecraft:storage") {
                    return std::make_shared<number_provider_storage>(other_data.at("storage").as_string(), other_data.at("path").as_string());
                } else if (type == "enchantment_level" || type == "minecraft:enchantment_level")
                    return std::make_shared<number_provider_enchantment_level>(other_data.at("amount").as_string());
                else
                    throw std::runtime_error("Invalid number provider type: " + type);
            } else {
                if (other_data.at("min").get_type() == util::nbt_type::tag_float)
                    return std::make_shared<number_provider_uniform>(other_data.at("min").as_float(), other_data.at("max").as_float());
                else
                    return std::make_shared<number_provider_uniform>(other_data.at("min").as_int(), other_data.at("max").as_int());
            }
        } else
            return std::make_shared<number_provider_constant>(other_data.as_int());
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

    util::nbt number_provider_constant::get_nbt() const {
        return std::visit([](auto it) -> util::nbt { return it; }, value);
    }

    float number_provider_uniform::get_float() const noexcept {
        return std::uniform_real_distribution<float>(get_min_inclusive_float(), get_max_exclusive_float())(get_thread_local_engine());
    }

    int32_t number_provider_uniform::get_int() const noexcept {
        return std::uniform_int_distribution<int>(get_min_inclusive_int(), get_max_exclusive_int())(get_thread_local_engine());
    }

    util::nbt number_provider_uniform::get_nbt() const {
        return util::nbt_compound{
            {"type", "minecraft:uniform"},
            {"min", std::visit([](auto it) -> util::nbt { return it; }, min_inclusive)},
            {"max", std::visit([](auto it) -> util::nbt { return it; }, max_exclusive)}
        }.take_map();
    }

    float number_provider_clamped_normal::get_float() const noexcept {
        return std::clamp<float>(std::normal_distribution<float>(mean, deviation)(get_thread_local_engine()), (float)min, (float)max);
    }

    int32_t number_provider_clamped_normal::get_int() const noexcept {
        return std::clamp<int32_t>((int32_t)std::normal_distribution<float>(mean, deviation)(get_thread_local_engine()), min, max);
    }

    util::nbt number_provider_clamped_normal::get_nbt() const {
        return util::nbt_compound{
            {"type", "minecraft:clamped_normal"},
            {"mean", mean},
            {"deviation", deviation},
            {"min", min},
            {"max", max}
        }.take_map();
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

    util::nbt number_provider_trapezoid::get_nbt() const {
        return util::nbt_compound{
            {"type", "minecraft:trapezoid"},
            {"min", min},
            {"max", max},
            {"plateau", plateau}
        }.take_map();
    }

    float number_provider_clamped::get_float() const noexcept {
        return std::clamp<float>(source->get_float(), get_min_inclusive_float(), get_max_inclusive_float());
    }

    int32_t number_provider_clamped::get_int() const noexcept {
        return std::clamp<int32_t>(source->get_int(), get_min_inclusive_int(), get_max_inclusive_int());
    }

    util::nbt number_provider_clamped::get_nbt() const {
        return util::nbt_compound{
            {"type", "minecraft:clamped"},
            {"min", std::visit([](auto it) -> util::nbt { return it; }, min_inclusive)},
            {"max", std::visit([](auto it) -> util::nbt { return it; }, max_inclusive)},
            {"source", source->get_nbt()}
        }.take_map();
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

    util::nbt number_provider_weighted_list::get_nbt() const {
        list_array<util::nbt> arr;
        arr.reserve(values.size());
        for (auto& it : values)
            arr.push_back(util::nbt_compound{{"data", it.first->get_nbt()}, {"weight", it.second}}.take_map());

        return util::nbt_compound{
            {"type", "minecraft:weighted_list"},
            {"values", std::move(arr)}
        }.take_map();
    }

    float number_provider_biased_to_bottom::get_float() const noexcept {
        std::uniform_real_distribution<float> dist(get_min_inclusive_float(), get_max_exclusive_float());
        return std::min(dist(get_thread_local_engine()), dist(get_thread_local_engine()));
    }

    int32_t number_provider_biased_to_bottom::get_int() const noexcept {
        std::uniform_int_distribution<int32_t> dist(get_min_inclusive_int(), get_max_exclusive_int());
        return std::min(dist(get_thread_local_engine()), dist(get_thread_local_engine()));
    }

    util::nbt number_provider_biased_to_bottom::get_nbt() const {
        return util::nbt_compound{
            {"type", "minecraft:biased_to_bottom"},
            {"min", std::visit([](auto it) -> util::nbt { return it; }, min_inclusive)},
            {"max", std::visit([](auto it) -> util::nbt { return it; }, max_exclusive)}
        }.take_map();
    }

    float number_provider_binomial::get_float() const noexcept {
        return static_cast<float>(std::binomial_distribution<int32_t>(n->get_int(), static_cast<double>(p->get_float()))(get_thread_local_engine()));
    }

    int32_t number_provider_binomial::get_int() const noexcept {
        return std::binomial_distribution<int32_t>(n->get_int(), static_cast<double>(p->get_float()))(get_thread_local_engine());
    }

    util::nbt number_provider_binomial::get_nbt() const {
        return util::nbt_compound{
            {"type", "minecraft:binomial"},
            {"n", n->get_nbt()},
            {"p", p->get_nbt()}
        }.take_map();
    }

    float number_provider_score::get_float() const noexcept {
        return 0.0f; //TODO
    }

    int32_t number_provider_score::get_int() const noexcept {
        return 0; //TODO
    }

    util::nbt number_provider_score::get_nbt() const {
        if (target.type == "fixed") {
            return util::nbt_compound{
                {"type", "minecraft:score"},
                {"scale", scale},
                {"score", score},
                {"target", util::nbt_compound{{"type", target.type}, {"name", target.value}}.take_map()}
            }.take_map();
        } else {
            return util::nbt_compound{
                {"type", "minecraft:score"},
                {"scale", scale},
                {"score", score},
                {"target", util::nbt_compound{{"type", target.type}, {"target", target.value}}.take_map()}
            }.take_map();
        }
    }

    float number_provider_storage::get_float() const noexcept {
        return 0.0f; //TODO
    }

    int32_t number_provider_storage::get_int() const noexcept {
        return 0; //TODO
    }

    util::nbt number_provider_storage::get_nbt() const {
        return util::nbt_compound{
            {"type", "minecraft:storage"},
            {"storage", storage},
            {"path", path}
        }.take_map();
    }

    float number_provider_enchantment_level::get_float() const noexcept {
        return 0.0f; //TODO
    }

    int32_t number_provider_enchantment_level::get_int() const noexcept {
        return 0; //TODO
    }

    util::nbt number_provider_enchantment_level::get_nbt() const {
        return util::nbt_compound{
            {"type", "minecraft:enchantment_level"},
            {"amount", amount}
        }.take_map();
    }
}
