#include <src/api/tags.hpp>
#include <src/base_objects/slot_display.hpp>
#include <src/registers.hpp>

namespace copper_server::base_objects {
    namespace slot_displays {
        namespace minecraft {

            smithing_trim::smithing_trim(slot_display&& base, slot_display&& material, slot_display&& pattern)
                : base(new slot_display(std::move(base))), material(new slot_display(std::move(material))), pattern(new slot_display(std::move(pattern))) {}

            smithing_trim::~smithing_trim() {
                delete base;
                delete material;
                delete pattern;
            }

            with_remainder::with_remainder(slot_display&& ingredient, slot_display&& remainder)
                : ingredient(new slot_display(std::move(ingredient))), remainder(new slot_display(std::move(remainder))) {}

            with_remainder::~with_remainder() {
                delete ingredient;
                delete remainder;
            }

            composite::composite(std::vector<slot_display*>&& options)
                : options(std::move(options)) {}

            composite ::~composite() {
                for (auto& option : options)
                    delete option;
            }
        }
    }

    list_array<slot> slot_display::to_slots() const {
        return std::visit(
            [](auto& it) -> list_array<slot> {
                using T = std::decay_t<decltype(it)>;
                if constexpr (std::is_same_v<slot_displays::minecraft::empty, T>) {
                    return {};
                } else if constexpr (std::is_same_v<slot_displays::minecraft::any_fuel, T>) {
                    return api::tags::unfold_tag(api::tags::builtin_entry::item, "minecraft:fuel").convert<slot>([](const auto& item) {
                        return std::make_optional(slot_data::create_item(item));
                    });
                } else if constexpr (std::is_same_v<slot_displays::minecraft::item, T>) {
                    return {std::make_optional(slot_data::create_item(it.item))};
                } else if constexpr (std::is_same_v<slot_displays::minecraft::item_stack, T>) {
                    return {std::make_optional(it.item)};
                } else if constexpr (std::is_same_v<slot_displays::minecraft::tag, T>) {
                    return api::tags::unfold_tag(api::tags::builtin_entry::item, it.tag).convert<slot>([](const auto& item) {
                        return std::make_optional(slot_data::create_item(item));
                    });
                } else if constexpr (std::is_same_v<slot_displays::minecraft::smithing_trim, T>) {
                    return {
                        it.base->to_slots(),
                        it.material->to_slots(),
                        it.pattern->to_slots()
                    };
                } else if constexpr (std::is_same_v<slot_displays::minecraft::with_remainder, T>) {
                    return it.ingredient->to_slots();
                } else if constexpr (std::is_same_v<slot_displays::minecraft::composite, T>) {
                    list_array<slot> res;
                    for (auto& it : it.options)
                        res.push_back(it->to_slots());
                    return res;
                } else if constexpr (std::is_same_v<slot_displays::custom, T>) {
                    if (it.to_slots)
                        return it.to_slots(it.value);
                    else
                        return {};
                }
            },
            value
        );
    }
}
