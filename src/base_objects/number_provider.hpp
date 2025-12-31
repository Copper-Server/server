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

#include <variant>

#include <src/util/nbt.hpp>

namespace copper_server::base_objects {

    struct number_provider {
        virtual float get_float() const noexcept = 0;
        virtual int32_t get_int() const noexcept = 0;
        virtual util::nbt get_nbt() const = 0;

        static std::shared_ptr<number_provider> parse_provider(const util::nbt& value);
    };

    struct number_provider_constant final : public number_provider {
        std::variant<int32_t, float> value;

        number_provider_constant(int32_t value)
            : value(value) {}

        number_provider_constant(float value)
            : value(value) {}

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_uniform final : public number_provider {
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

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_clamped_normal final : public number_provider {
        float mean;
        float deviation;
        int32_t min;
        int32_t max;

        number_provider_clamped_normal(float mean_, float deviation_, int32_t min_, int32_t max_)
            : mean(mean_), deviation(deviation_), min(min_), max(max_) {}

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_trapezoid final : public number_provider {
        int32_t min;
        int32_t max;
        int32_t plateau;

        number_provider_trapezoid(int32_t min_, int32_t max_, int32_t plateau_)
            : min(min_), max(max_), plateau(plateau_) {}

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_clamped final : public number_provider {
        std::variant<int32_t, float> min_inclusive;
        std::variant<int32_t, float> max_inclusive;
        std::shared_ptr<number_provider> source;

        number_provider_clamped(std::variant<int32_t, float> min_inclusive, std::variant<int32_t, float> max_inclusive, const std::shared_ptr<number_provider>& source)
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

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_weighted_list final : public number_provider {
        std::vector<std::pair<std::shared_ptr<number_provider>, double>> values;

        number_provider_weighted_list(std::vector<std::pair<std::shared_ptr<number_provider>, double>>&& values)
            : values(std::move(values)) {}

        number_provider_weighted_list(const std::vector<std::pair<std::shared_ptr<number_provider>, double>>& values)
            : values(values) {}

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_biased_to_bottom final : public number_provider {
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

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_binomial final : public number_provider {
        std::shared_ptr<number_provider> n;
        std::shared_ptr<number_provider> p;

        number_provider_binomial(const std::shared_ptr<number_provider>& n_, const std::shared_ptr<number_provider>& p_)
            : n(n_), p(p_) {}

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_score final : public number_provider {
        struct {
            std::string type;
            std::string value; //`name` for "fixed" or `target` for "context"
        } target;

        std::string score;
        float scale = 1.0f;

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_storage final : public number_provider {
        std::string storage;
        std::string path;

        number_provider_storage(std::string storage, std::string path) : storage(std::move(storage)), path(std::move(path)) {}

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };

    struct number_provider_enchantment_level final : public number_provider {
        std::string amount;

        number_provider_enchantment_level(std::string amount_) : amount(std::move(amount_)) {}

        float get_float() const noexcept override;
        int32_t get_int() const noexcept override;
        util::nbt get_nbt() const override;
    };
}
#endif /* SRC_BASE_OBJECTS_NUMBER_PROVIDER */
