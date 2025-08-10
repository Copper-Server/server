/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_BASE_OBJECTS_NUMBER_PROVIDER
#define SRC_BASE_OBJECTS_NUMBER_PROVIDER

#include <library/enbt/enbt.hpp>
#include <variant>

namespace copper_server::base_objects {
    struct number_provider_constant;
    struct number_provider_uniform;
    struct number_provider_clamped_normal;
    struct number_provider_trapezoid;
    struct number_provider_clamped;
    struct number_provider_weighted_list;
    struct number_provider_biased_to_bottom;
    struct number_provider_binomial;
    struct number_provider_score;
    struct number_provider_storage;
    struct number_provider_enchantment_level;

    struct number_provider {
        std::variant<
            number_provider_constant*,
            number_provider_uniform*,
            number_provider_clamped_normal*,
            number_provider_trapezoid*,
            number_provider_clamped*,
            number_provider_weighted_list*,
            number_provider_biased_to_bottom*,
            number_provider_binomial*,
            number_provider_score*,
            number_provider_storage*,
            number_provider_enchantment_level*>
            provider;

    public:
        number_provider();
        number_provider(const number_provider_constant& value);
        number_provider(const number_provider_uniform& value);
        number_provider(const number_provider_clamped_normal& value);
        number_provider(const number_provider_trapezoid& value);
        number_provider(const number_provider_clamped& value);
        number_provider(const number_provider_weighted_list& value);
        number_provider(const number_provider_biased_to_bottom& value);
        number_provider(const number_provider_binomial& value);
        number_provider(const number_provider_score& value);
        number_provider(const number_provider_storage& value);
        number_provider(const number_provider_enchantment_level& value);
        ~number_provider();


        float get_float() const;
        int32_t get_int() const;

        //for context
        float get_float(const enbt::compound_const_ref& other_data) const;
        //for context
        int32_t get_int(const enbt::compound_const_ref& other_data) const;
    };

    struct number_provider_constant {
        std::variant<int32_t, float> value;

        number_provider_constant(int32_t value)
            : value(value) {}

        number_provider_constant(float value)
            : value(value) {}

        float get_float() const noexcept {
            return std::visit(
                [](auto&& arg) -> float {
                    return static_cast<float>(arg);
                },
                value
            );
        }

        int32_t get_int() const noexcept {
            return std::visit(
                [](auto&& arg) -> int32_t {
                    return static_cast<int32_t>(arg);
                },
                value
            );
        }
    };

    struct number_provider_uniform {
        std::variant<int32_t, float> min_inclusive;
        std::variant<int32_t, float> max_exclusive;

        number_provider_uniform(std::variant<int32_t, float> min_inclusive, std::variant<int32_t, float> max_exclusive)
            : min_inclusive(min_inclusive), max_exclusive(max_exclusive) {}

        float get_min_inclusive_float() const noexcept {
            return std::visit(
                [](auto&& arg) -> float {
                    return static_cast<float>(arg);
                },
                min_inclusive
            );
        }

        int32_t get_min_inclusive_int() const noexcept {
            return std::visit(
                [](auto&& arg) -> int32_t {
                    return static_cast<int32_t>(arg);
                },
                min_inclusive
            );
        }

        float get_max_exclusive_float() const noexcept {
            return std::visit(
                [](auto&& arg) -> float {
                    return static_cast<float>(arg);
                },
                max_exclusive
            );
        }

        int32_t get_max_exclusive_int() const noexcept {
            return std::visit(
                [](auto&& arg) -> int32_t {
                    return static_cast<int32_t>(arg);
                },
                max_exclusive
            );
        }
    };

    struct number_provider_clamped_normal {
        float mean;
        float deviation;
        int32_t min;
        int32_t max;
    };

    struct number_provider_trapezoid {
        int32_t min;
        int32_t max;
        int32_t plateau;
    };

    struct number_provider_clamped {
        std::variant<int32_t, float> min_inclusive;
        std::variant<int32_t, float> max_inclusive;
        number_provider source;

        number_provider_clamped(std::variant<int32_t, float> min_inclusive, std::variant<int32_t, float> max_inclusive, const number_provider& source)
            : min_inclusive(min_inclusive), max_inclusive(max_inclusive), source(source) {}

        float get_min_inclusive_float() const noexcept {
            return std::visit(
                [](auto&& arg) -> float {
                    return static_cast<float>(arg);
                },
                min_inclusive
            );
        }

        int32_t get_min_inclusive_int() const noexcept {
            return std::visit(
                [](auto&& arg) -> int32_t {
                    return static_cast<int32_t>(arg);
                },
                min_inclusive
            );
        }

        float get_max_inclusive_float() const noexcept {
            return std::visit(
                [](auto&& arg) -> float {
                    return static_cast<float>(arg);
                },
                max_inclusive
            );
        }

        int32_t get_max_inclusive_int() const noexcept {
            return std::visit(
                [](auto&& arg) -> int32_t {
                    return static_cast<int32_t>(arg);
                },
                max_inclusive
            );
        }
    };

    struct number_provider_weighted_list {
        std::vector<std::pair<number_provider, double>> values;
    };

    struct number_provider_biased_to_bottom {
        std::variant<int32_t, float> min_inclusive;
        std::variant<int32_t, float> max_exclusive;

        number_provider_biased_to_bottom(std::variant<int32_t, float> min_inclusive, std::variant<int32_t, float> max_exclusive)
            : min_inclusive(min_inclusive), max_exclusive(max_exclusive) {}

        float get_min_inclusive_float() const noexcept {
            return std::visit(
                [](auto&& arg) -> float {
                    return static_cast<float>(arg);
                },
                min_inclusive
            );
        }

        int32_t get_min_inclusive_int() const noexcept {
            return std::visit(
                [](auto&& arg) -> int32_t {
                    return static_cast<int32_t>(arg);
                },
                min_inclusive
            );
        }

        float get_max_exclusive_float() const noexcept {
            return std::visit(
                [](auto&& arg) -> float {
                    return static_cast<float>(arg);
                },
                max_exclusive
            );
        }

        int32_t get_max_exclusive_int() const noexcept {
            return std::visit(
                [](auto&& arg) -> int32_t {
                    return static_cast<int32_t>(arg);
                },
                max_exclusive
            );
        }
    };

    struct number_provider_binomial {
        number_provider n;
        number_provider p;
    };

    struct number_provider_score {
        struct {
            std::string type;
            std::string value; //`name` for "fixed" or `target` for "context"
        } target;

        std::string score;
        std::optional<float> scale;
    };

    struct number_provider_storage {
        std::string storage;
        std::string path;
    };

    struct number_provider_enchantment_level {
        std::string amount;
    };
}
#endif /* SRC_BASE_OBJECTS_NUMBER_PROVIDER */
