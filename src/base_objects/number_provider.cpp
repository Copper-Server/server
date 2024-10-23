
#include "number_provider.hpp"
#include <random>

namespace crafted_craft::base_objects {

    number_provider::number_provider()
        : provider(new number_provider_constant(0)) {}

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
        std::visit([](auto&& arg) { delete arg; }, provider);
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
                    return std::clamp<float>(std::normal_distribution<float>(arg->mean, arg->deviation)(engine), arg->min, arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_trapezoid*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<float>(std::uniform_real_distribution<float>(arg->min, arg->max)(engine), arg->min, arg->max);
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
                    return std::binomial_distribution<int32_t>(arg->n.get_int(), arg->p.get_int())(engine);
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
                    return std::clamp<int32_t>(std::normal_distribution<float>(arg->mean, arg->deviation)(engine), arg->min, arg->max);
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

    float number_provider::get_float(const enbt::compound_const_ref& other_data) const {
        return std::visit(
            [&other_data](auto&& arg) -> float {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_constant*>)
                    return arg->get_float();
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_uniform*>) {
                    auto engine = std::default_random_engine();
                    return std::uniform_real_distribution<float>(arg->get_min_inclusive_float(), arg->get_max_exclusive_float())(engine);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_clamped_normal*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<float>(std::normal_distribution<float>(arg->mean, arg->deviation)(engine), arg->min, arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_trapezoid*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<float>(std::uniform_real_distribution<float>(arg->min, arg->max)(engine), arg->min, arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_clamped*>)
                    return std::clamp<float>(arg->source.get_float(other_data), arg->get_min_inclusive_float(), arg->get_max_inclusive_float());
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_weighted_list*>) {
                    double sum = 0;
                    for (const auto& [value, weight] : arg->values)
                        sum += weight;
                    auto engine = std::default_random_engine();
                    double random = std::uniform_real_distribution<double>(0, sum)(engine);
                    for (const auto& [value, weight] : arg->values) {
                        random -= weight;
                        if (random <= 0)
                            return value.get_float(other_data);
                    }
                    return arg->values.back().first.get_float(other_data);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_biased_to_bottom*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<float>(std::uniform_real_distribution<float>(arg->get_min_inclusive_float(), arg->get_max_exclusive_float())(engine), arg->get_min_inclusive_float(), arg->get_max_exclusive_float());
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_binomial*>) {
                    auto engine = std::default_random_engine();
                    return std::binomial_distribution<int32_t>(arg->n.get_int(other_data), arg->p.get_int(other_data))(engine);
                }
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_score*>)
                //    return arg->source.get_float(other_data);
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_storage*>)
                //    return arg->source.get_float(other_data);
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_enchantment_level*>)
                //    return arg->source.get_float(other_data);
                else
                    return 0;
            },
            provider
        );
    }

    int32_t number_provider::get_int(const enbt::compound_const_ref& other_data) const {
        return std::visit(
            [&other_data](auto&& arg) -> int32_t {
                if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_constant*>)
                    return arg->get_int();
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_uniform*>) {
                    auto engine = std::default_random_engine();
                    return std::uniform_int_distribution<int32_t>(arg->get_min_inclusive_int(), arg->get_max_exclusive_int())(engine);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_clamped_normal*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<int32_t>(std::normal_distribution<float>(arg->mean, arg->deviation)(engine), arg->min, arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_trapezoid*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<int32_t>(std::uniform_int_distribution<int32_t>(arg->min, arg->max)(engine), arg->min, arg->max);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_clamped*>)
                    return std::clamp<int32_t>(arg->source.get_int(other_data), arg->get_min_inclusive_int(), arg->get_max_inclusive_int());
                else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_weighted_list*>) {
                    double sum = 0;
                    for (const auto& [value, weight] : arg->values)
                        sum += weight;
                    auto engine = std::default_random_engine();
                    double random = std::uniform_real_distribution<double>(0, sum)(engine);
                    for (const auto& [value, weight] : arg->values) {
                        random -= weight;
                        if (random <= 0)
                            return value.get_int(other_data);
                    }
                    return arg->values.back().first.get_int(other_data);
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_biased_to_bottom*>) {
                    auto engine = std::default_random_engine();
                    return std::clamp<int32_t>(std::uniform_int_distribution<int32_t>(arg->get_min_inclusive_int(), arg->get_max_exclusive_int())(engine), arg->get_min_inclusive_int(), arg->get_max_exclusive_int());
                } else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_binomial*>) {
                    auto engine = std::default_random_engine();
                    return std::binomial_distribution<int32_t>(arg->n.get_int(other_data), arg->p.get_int(other_data))(engine);
                }
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_score*>)
                //    return arg->source.get_int(other_data);
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_storage*>)
                //    return arg->source.get_int(other_data);
                //else if constexpr (std::is_same_v<std::decay_t<decltype(arg)>, number_provider_enchantment_level*>)
                //    return arg->source.get_int(other_data);
                else
                    return 0;
            },
            provider
        );
    }
}