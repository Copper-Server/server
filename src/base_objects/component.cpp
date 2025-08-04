#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>
#include <src/util/reflect.hpp>

namespace copper_server::base_objects {
    auto potion_effect::data_t::copy() const -> data_t {
        return data_t{
            .amplifier = amplifier,
            .duration = duration,
            .is_ambient = is_ambient,
            .show_particles = show_particles,
            .show_icon = show_icon,
            .hidden_effect = hidden_effect ? std::make_optional<box<data_t>>(std::make_unique<data_t>((*hidden_effect)->copy())) : std::nullopt
        };
    }

    potion_effect potion_effect::copy() const {
        return potion_effect{
            .type_id = type_id,
            .data = std::make_unique<data_t>(data->copy())
        };
    }

    auto component::can_place_on::copy() const -> can_place_on {
        std::vector<box<component>> full_components_match_cp;
        full_components_match_cp.reserve(full_components_match.size());
        for (auto& it : full_components_match)
            full_components_match_cp.push_back(std::make_unique<component>(*it));

        return can_place_on{
            .blocks = blocks,
            .properties = properties,
            .nbt = nbt,
            .full_components_match = std::move(full_components_match_cp),
            .partial_components_match = partial_components_match
        };
    }

    auto component::can_break::copy() const -> can_break {
        std::vector<box<component>> full_components_match_cp;
        full_components_match_cp.reserve(full_components_match.size());
        for (auto& it : full_components_match)
            full_components_match_cp.push_back(std::make_unique<component>(*it));

        return can_break{
            .blocks = blocks,
            .properties = properties,
            .nbt = nbt,
            .full_components_match = std::move(full_components_match_cp),
            .partial_components_match = partial_components_match
        };
    }

    component::use_remainder::use_remainder(const slot& remainder) : remainder(std::make_unique<slot>(remainder)) {}

    component::use_remainder::use_remainder(slot&& remainder) : remainder(std::make_unique<slot>(std::move(remainder))) {}

    component::use_remainder::use_remainder(const box<slot>& remainder) : remainder(std::make_unique<slot>(*remainder)) {}

    component::use_remainder::use_remainder(box<slot>&& remainder) : remainder(std::move(remainder)) {}

    component::use_remainder::use_remainder(const use_remainder& remainder) : remainder(std::make_unique<slot>(*remainder.remainder)) {}

    component::use_remainder::use_remainder(use_remainder&& remainder) : remainder(std::move(remainder.remainder)) {}

    auto component::use_remainder::operator=(const slot& r) -> use_remainder& {
        remainder = std::make_unique<slot>(r);
        return *this;
    }

    auto component::use_remainder::operator=(slot&& r) -> use_remainder& {
        remainder = std::make_unique<slot>(std::move(r));
        return *this;
    }

    auto component::use_remainder::operator=(const box<slot>& r) -> use_remainder& {
        remainder = std::make_unique<slot>(*r);
        return *this;
    }

    auto component::use_remainder::operator=(box<slot>&& r) -> use_remainder& {
        remainder = std::move(r);
        return *this;
    }

    auto component::use_remainder::operator=(const use_remainder& r) -> use_remainder& {
        remainder = std::make_unique<slot>(*r.remainder);
        return *this;
    }

    auto component::use_remainder::operator=(use_remainder&& r) -> use_remainder& {
        remainder = std::move(r.remainder);
        return *this;
    }

    auto component::charged_projectiles::copy() const -> charged_projectiles {
        std::vector<box<slot>> projectiles_cp;
        projectiles_cp.reserve(projectiles.size());
        for (auto& it : projectiles)
            projectiles_cp.push_back(std::make_unique<slot>(*it));
        return charged_projectiles{.projectiles = std::move(projectiles_cp)};
    }

    auto component::bundle_contents::copy() const -> bundle_contents {
        std::vector<box<slot>> content_cp;
        content_cp.reserve(content.size());
        for (auto& it : content)
            content_cp.push_back(std::make_unique<slot>(*it));
        return bundle_contents{.content = std::move(content_cp)};
    }

    std::optional<size_t> component::container::get_free_slot() {
        size_t index = 0;

        for (auto& to_select_it : items) {
            auto& to_select = *to_select_it;
            if (!to_select)
                return index;
        }
        return std::nullopt;
    }

    int32_t component::container::add(const slot_data& item) {
        if (!item.count)
            return 0;
        int32_t to_add = item.count;
        int32_t max_count = item.get_component<max_stack_size>().size;
        if (max_count == 0)
            return 0;

        for (auto& to_insert_it : items) {
            if (to_add <= 0)
                return 0;
            auto& to_insert = *to_insert_it;
            if (!to_insert)
                continue;
            if (to_insert->id == item.id) {
                if (to_insert->is_same_def(item)) {
                    int32_t add_res = int32_t(max_count) - to_insert->count;
                    if (add_res > 0) {
                        add_res = std::min(add_res, to_add);
                        to_insert->count += add_res;
                        to_add -= add_res;
                    }
                }
            }
        }

        auto find_slot = get_free_slot();
        if (!find_slot)
            return to_add;

        set(*find_slot, item);
        get(*find_slot)->count = to_add;
        return 0;
    }

    void component::container::set(size_t slot, slot_data&& item) {
        *items[slot] = std::move(item);
    }

    void component::container::set(size_t slot, const slot_data& item) {
        *items[slot] = item;
    }

    slot& component::container::get(size_t slot) {
        return *items[slot];
    }

    bool component::container::contains(size_t slot) {
        return get(slot) != std::nullopt;
    }

    std::optional<size_t> component::container::contains(const slot_data& item) const {
        size_t index = 0;
        for (auto& i : items) {
            if (*i)
                if ((*i)->is_same_def(item))
                    return index;
            ++index;
        };
        return std::nullopt;
    }

    list_array<size_t> component::container::contains(const std::string& id, size_t count) const {
        list_array<size_t> res;
        id_item real_id(id);
        size_t index = 0;
        for (auto& i : items) {
            if (*i) {
                auto& check = **i;
                if (check.id == real_id.value) {
                    res.push_back(index);
                    if (count > check.count)
                        count -= check.count;
                    else {
                        count = 0;
                        break;
                    }
                }
            }
            ++index;
        };
        return res;
    }

    bool component::container::remove(size_t slot) {
        auto& res = get(slot);
        if (res) {
            res = std::nullopt;
            return true;
        } else
            return false;
    }

    void component::container::clear(int32_t id, size_t count) {
        if (count == (size_t)-1) {
            for (auto& to_erase_it : items) {
                auto& to_erase = *to_erase_it;
                if (to_erase)
                    if (to_erase->id == id)
                        to_erase = std::nullopt;
            }
        } else {
            for (auto& to_erase_it : items) {
                auto& to_erase = *to_erase_it;
                if (to_erase) {
                    if (to_erase->id == id) {
                        if (to_erase->count <= count) {
                            count -= to_erase->count;
                            to_erase = std::nullopt;
                        } else {
                            to_erase->count -= (int32_t)count;
                            break;
                        }
                    }
                }
            }
        }
    }

    void component::container::clear() {
        for (auto& to_erase_it : items) {
            *to_erase_it = std::nullopt;
        }
    }

    size_t component::container::count() const {
        size_t count = 0;
        for (auto& i : items)
            if (*i)
                count += (*i)->count;
        return count;
    }

    size_t component::container::size() const {
        size_t index = 0;
        for (auto& i : items)
            if (*i)
                ++index;
        return index;
    }

    component& component::operator=(component&& mov) noexcept {
        type = std::move(mov.type);
        return *this;
    }

    component& component::operator=(const component& copy) {
        using Type_T = std::decay_t<decltype(type)>;
        type = std::visit(
            [](auto& it) -> Type_T {
                if constexpr (std::is_copy_constructible_v<std::decay_t<decltype(it)>>) {
                    return it;
                } else
                    return it.copy();
            },
            type
        );
        return *this;
    }

    size_t component::get_id() const {
        return std::visit(
            [](auto& it) {
                return std::decay_t<decltype(it)>::item_id::value;
            },
            type
        );
    }
}