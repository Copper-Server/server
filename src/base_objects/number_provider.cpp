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

    number_provider::number_provider()
        : provider(new number_provider_constant(0)) {}

    number_provider::number_provider(number_provider&& mov) noexcept {
        std::visit(
            [this]<class T>(T*& it) {
                provider = it;
                it = nullptr;
            },
            mov.provider
        );
    }

    number_provider::number_provider(const number_provider& copy) {
        std::visit(
            [this]<class T>(const T* it) {
                provider = new T(*it);
            },
            copy.provider
        );
    }

    number_provider::number_provider(const number_provider_constant& value)
        : provider(new number_provider_constant(value)) {}

    number_provider::number_provider(const number_provider_uniform& value)
        : provider(new number_provider_uniform(value)) {}

    number_provider::number_provider(const number_provider_clamped_normal& value)
        : provider(new number_provider_clamped_normal(value)) {}

    number_provider::number_provider(const number_provider_trapezoid& value)
        : provider(new number_provider_trapezoid(value)) {}

    number_provider::number_provider(const number_provider_clamped& value)
        : provider(new number_provider_clamped(value)) {}

    number_provider::number_provider(const number_provider_weighted_list& value)
        : provider(new number_provider_weighted_list(value)) {}

    number_provider::number_provider(const number_provider_biased_to_bottom& value)
        : provider(new number_provider_biased_to_bottom(value)) {}

    number_provider::number_provider(const number_provider_binomial& value)
        : provider(new number_provider_binomial(value)) {}

    number_provider::number_provider(const number_provider_score& value)
        : provider(new number_provider_score(value)) {}

    number_provider::number_provider(const number_provider_storage& value)
        : provider(new number_provider_storage(value)) {}

    number_provider::number_provider(const number_provider_enchantment_level& value)
        : provider(new number_provider_enchantment_level(value)) {}

    number_provider::~number_provider() {
        std::visit([](auto&& arg) { if(arg) delete arg; }, provider);
    }

    number_provider& number_provider::operator=(number_provider&& mov) noexcept {
        if (this == &mov)
            return *this;
        std::visit([](auto&& arg) { if(arg) delete arg; }, provider);
        std::visit(
            [this]<class T>(T*& it) {
                provider = it;
                it = nullptr;
            },
            mov.provider
        );
        return *this;
    }

    number_provider& number_provider::operator=(const number_provider& copy) {
        if (this == &copy)
            return *this;
        std::visit([](auto&& arg) { if(arg) delete arg; }, provider);
        std::visit(
            [this]<class T>(const T* it) {
                provider = new T(*it);
            },
            copy.provider
        );
        return *this;
    }

    float number_provider::get_float() const {
        return std::visit(
            [](auto&& arg) -> float {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_constant*>)
                    return arg->get_float();
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_uniform*>) {
                    auto engine = std::default_random_engine();
                    return std::uniform_real_distribution<float>(arg->get_min_inclusive_float(), arg->get_max_exclusive_float())(engine);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_clamped_normal*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<float>(std::normal_distribution<float>(arg->mean, arg->deviation)(engine), (float)arg->min, (float)arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_trapezoid*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<float>(std::uniform_real_distribution<float>((float)arg->min, (float)arg->max)(engine), (float)arg->min, (float)arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_clamped*>)
                    return std::clamp<float>(arg->source.get_float(), arg->get_min_inclusive_float(), arg->get_max_inclusive_float());
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_weighted_list*>) {
                    double sum = 0;
                    for (const auto& [value, weight] : arg->values)
                        sum += weight;
                    auto engine = std::default_random_engine();
                    double random = std::uniform_real_distribution<double>(0, sum)(engine);
                    for (const auto& [value, weight] : arg->values) {
                        random -= weight;
                        if (random <= 0)
                            return value.get_float();
                    }
                    return arg->values.back().first.get_float();
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_biased_to_bottom*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<float>(std::uniform_real_distribution<float>(arg->get_min_inclusive_float(), arg->get_max_exclusive_float())(engine), arg->get_min_inclusive_float(), arg->get_max_exclusive_float());
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_binomial*>) {
                    auto engine = std::default_random_engine();
                    return (float)std::binomial_distribution<int32_t>(arg->n.get_int(), arg->p.get_int())(engine);
                }
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_score*>)
                //    return arg->source.get_float();
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_storage*>)
                //    return arg->source.get_float();
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_enchantment_level*>)
                //    return arg->source.get_float();
                else
                    return 0;
            },
            provider
        );
    }

    int32_t number_provider::get_int() const {
        return std::visit(
            [](auto&& arg) -> int32_t {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_constant*>)
                    return arg->get_int();
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_uniform*>) {
                    auto engine = std::default_random_engine();
                    return std::uniform_int_distribution<int32_t>(arg->get_min_inclusive_int(), arg->get_max_exclusive_int())(engine);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_clamped_normal*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<int32_t>((int32_t)std::normal_distribution<float>(arg->mean, arg->deviation)(engine), arg->min, arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_trapezoid*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<int32_t>(std::uniform_int_distribution<int32_t>(arg->min, arg->max)(engine), arg->min, arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_clamped*>)
                    return std::clamp<int32_t>(arg->source.get_int(), arg->get_min_inclusive_int(), arg->get_max_inclusive_int());
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_weighted_list*>) {
                    double sum = 0;
                    for (const auto& [value, weight] : arg->values)
                        sum += weight;
                    auto engine = std::default_random_engine();
                    double random = std::uniform_real_distribution<double>(0, sum)(engine);
                    for (const auto& [value, weight] : arg->values) {
                        random -= weight;
                        if (random <= 0)
                            return value.get_int();
                    }
                    return arg->values.back().first.get_int();
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_biased_to_bottom*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<int32_t>(std::uniform_int_distribution<int32_t>(arg->get_min_inclusive_int(), arg->get_max_exclusive_int())(engine), arg->get_min_inclusive_int(), arg->get_max_exclusive_int());
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_binomial*>) {
                    auto engine = std::default_random_engine();
                    return std::binomial_distribution<int32_t>(arg->n.get_int(), arg->p.get_int())(engine);
                }
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_score*>)
                //    return arg->source.get_int();
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_storage*>)
                //    return arg->source.get_int();
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_enchantment_level*>)
                //    return arg->source.get_int();
                else
                    return 0;
            },
            provider
        );
    }

    number_provider number_provider::parse_provider(const enbt::value& other_data) {
        if (other_data.get_type() == enbt::type::floating)
            return number_provider_constant((float)other_data);
        else if (other_data.is_numeric() || other_data.is_none()) {
            return number_provider_constant((int32_t)other_data);
        } else if (other_data.is_compound()) {
            if (other_data.contains("type")) {
                auto& type = other_data.at("type").as_string();
                if (type == "constant" || type == "minecraft:constant") {
                    if (other_data.get_type() == enbt::type::floating)
                        return number_provider_constant((float)other_data);
                    else
                        return number_provider_constant((int32_t)other_data);
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

                    return base_objects::number_provider_uniform(min, max);
                } else if (type == "binominal" || type == "minecraft:binominal") {
                    return number_provider_binomial{
                        .n = parse_provider(other_data.at("n")),
                        .p = parse_provider(other_data.at("p"))
                    };
                } else if (type == "clamped_normal" || type == "minecraft:clamped_normal") {
                    float mean = other_data.at("mean");
                    float deviation = other_data.at("deviation");
                    int32_t min_inclusive = other_data.at("min_inclusive");
                    int32_t max_inclusive = other_data.at("max_inclusive");
                    return base_objects::number_provider_clamped_normal(mean, deviation, min_inclusive, max_inclusive);
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

                    return base_objects::number_provider_clamped(min, max, parse_provider(other_data.at("source")));
                } else if (type == "trapezoid" || type == "minecraft:trapezoid") {
                    int32_t min = other_data.at("min");
                    int32_t max = other_data.at("max");
                    int32_t plateau = other_data.at("plateau");
                    return base_objects::number_provider_trapezoid(min, max, plateau);
                } else if (type == "weighted_list" || type == "minecraft:weighted_list") {
                    std::vector<std::pair<base_objects::number_provider, double>> values;
                    auto values_e = other_data.at("values").as_array();
                    values.reserve(values_e.size());
                    for (auto&& val : values_e) {
                        auto weight = val.contains("weight") ? (float)val["weight"] : 1.0;
                        values.push_back({parse_provider(val.at("data")), weight});
                    }
                    return base_objects::number_provider_weighted_list(values);
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
                    return base_objects::number_provider_biased_to_bottom(min, max);
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
                    return number_provider_score(std::move(res));
                } else if (type == "storage" || type == "minecraft:storage") {
                    base_objects::number_provider_storage res;
                    res.storage = (std::string)other_data.at("storage");
                    res.path = (std::string)other_data.at("path");
                    return number_provider_storage(std::move(res));
                } else if (type == "enchantment_level" || type == "minecraft:enchantment_level")
                    return base_objects::number_provider_enchantment_level((std::string)other_data.at("amount"));
                else
                    throw std::runtime_error("Invalid number provider type: " + type);
            } else {
                if (other_data.at("min").get_type() == enbt::type::floating)
                    return number_provider_uniform((float)other_data.at("min"), (float)other_data.at("max"));
                else
                    return number_provider_uniform((int32_t)other_data.at("min"), (int32_t)other_data.at("max"));
            }
        } else
            return number_provider_constant((int32_t)other_data);
    }

    enbt::value number_provider::get_enbt() const {
        return std::visit(
            []<class T>(const T& val) -> enbt::value {
                if constexpr (std::is_same_v<T, number_provider_constant*>) {
                    return std::visit([](auto it) -> enbt::value { return it; }, val->value);
                } else if constexpr (std::is_same_v<T, number_provider_uniform*>) {
                    return enbt::compound{
                        {"type", "minecraft:uniform"},
                        {"min", std::visit([](auto it) -> enbt::value { return it; }, val->min_inclusive)},
                        {"max", std::visit([](auto it) -> enbt::value { return it; }, val->max_exclusive)}
                    };
                } else if constexpr (std::is_same_v<T, number_provider_clamped_normal*>) {
                    return enbt::compound{
                        {"type", "minecraft:clamped_normal"},
                        {"mean", val->mean},
                        {"deviation", val->deviation},
                        {"min", val->min},
                        {"max", val->max}
                    };
                } else if constexpr (std::is_same_v<T, number_provider_trapezoid*>) {
                    return enbt::compound{
                        {"type", "minecraft:trapezoid"},
                        {"min", val->min},
                        {"max", val->max},
                        {"max", val->plateau}
                    };
                } else if constexpr (std::is_same_v<T, number_provider_clamped*>) {
                    return enbt::compound{
                        {"type", "minecraft:clamped"},
                        {"min", std::visit([](auto it) -> enbt::value { return it; }, val->min_inclusive)},
                        {"max", std::visit([](auto it) -> enbt::value { return it; }, val->max_inclusive)},
                        {"source", val->source.get_enbt()}
                    };
                } else if constexpr (std::is_same_v<T, number_provider_weighted_list*>) {
                    enbt::fixed_array arr;
                    arr.reserve(val->values.size());
                    for (auto& it : val->values) {
                        arr.push_back(enbt::compound{{"data", it.first.get_enbt()}, {"weight", it.second}});
                    }
                    return enbt::compound{
                        {"type", "minecraft:weighted_list"},
                        {"values", std::move(arr)}
                    };
                } else if constexpr (std::is_same_v<T, number_provider_biased_to_bottom*>) {
                    return enbt::compound{
                        {"type", "minecraft:biased_to_bottom"},
                        {"min", std::visit([](auto it) -> enbt::value { return it; }, val->min_inclusive)},
                        {"max", std::visit([](auto it) -> enbt::value { return it; }, val->max_exclusive)}
                    };
                } else if constexpr (std::is_same_v<T, number_provider_binomial*>) {
                    return enbt::compound{
                        {"type", "minecraft:binomial"},
                        {"n", val->n.get_enbt()},
                        {"p", val->p.get_enbt()}
                    };
                } else if constexpr (std::is_same_v<T, number_provider_score*>) {
                    if (val->target.type == "fixed") {
                        return enbt::compound{
                            {"type", "minecraft:score"},
                            {"scale", val->scale},
                            {"score", val->score},
                            {"target", enbt::compound{{"type", val->target.type}, {"name", val->target.value}}}
                        };
                    } else {
                        return enbt::compound{
                            {"type", "minecraft:score"},
                            {"scale", val->scale},
                            {"score", val->score},
                            {"target", enbt::compound{{"type", val->target.type}, {"target", val->target.value}}}
                        };
                    }
                } else if constexpr (std::is_same_v<T, number_provider_storage*>) {
                    return enbt::compound{
                        {"type", "minecraft:storage"},
                        {"storage", val->storage},
                        {"path", val->path}
                    };
                } else // number_provider_enchantment_level*
                    return enbt::compound{
                        {"type", "minecraft:enchantment_level"},
                        {"amount", val->amount}
                    };
            },
            provider
        );
    }
}
