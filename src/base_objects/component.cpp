#include <src/base_objects/component.hpp>
#include <src/base_objects/slot.hpp>

namespace copper_server::base_objects {
    potion_effect::data_t::data_t() : amplifier(0), duration(0), is_ambient(false), show_particles(false), show_icon(false) {}

    potion_effect::data_t::data_t(data_t&& mov) : amplifier(mov.amplifier), duration(mov.duration), is_ambient(mov.is_ambient), show_particles(mov.show_particles), show_icon(mov.show_icon), hidden_effect(std::move(mov.hidden_effect)) {}

    potion_effect::data_t::data_t(const data_t& copy) : amplifier(copy.amplifier), duration(copy.duration), is_ambient(copy.is_ambient), show_particles(copy.show_particles), show_icon(copy.show_icon), hidden_effect(copy.hidden_effect) {}

    potion_effect::data_t::data_t(var_int32 amplifier, var_int32 duration, bool is_ambient, bool show_particles, bool show_icon, std::optional<box<data_t>>&& hidden_effect)
        : amplifier(amplifier), duration(duration), is_ambient(is_ambient), show_particles(show_particles), show_icon(show_icon), hidden_effect(std::move(hidden_effect)) {}

    potion_effect::data_t::data_t(var_int32 amplifier, var_int32 duration, bool is_ambient, bool show_particles, bool show_icon, const std::optional<box<data_t>>& hidden_effect)
        : amplifier(amplifier), duration(duration), is_ambient(is_ambient), show_particles(show_particles), show_icon(show_icon), hidden_effect(hidden_effect) {}

    bool potion_effect::data_t::operator==(const data_t& other) const {
        return amplifier == other.amplifier && duration == other.duration && is_ambient == other.is_ambient && show_particles == other.show_particles && show_icon == other.show_icon && hidden_effect == other.hidden_effect;
    }

    bool potion_effect::data_t::operator!=(const data_t& other) const {
        return !operator==(other);
    }

    potion_effect::data_t& potion_effect::data_t::operator=(data_t&& mov) {
        amplifier = mov.amplifier;
        duration = mov.duration;
        is_ambient = mov.is_ambient;
        show_particles = mov.show_particles;
        show_icon = mov.show_icon;
        hidden_effect = std::move(mov.hidden_effect);
        return *this;
    }

    potion_effect::data_t& potion_effect::data_t::operator=(const data_t& copy) {
        amplifier = copy.amplifier;
        duration = copy.duration;
        is_ambient = copy.is_ambient;
        show_particles = copy.show_particles;
        show_icon = copy.show_icon;
        hidden_effect = copy.hidden_effect;
        return *this;
    }

    bool component::can_place_on::operator==(const can_place_on& other) const = default;

    bool component::can_break::operator==(const can_break& other) const = default;

    component::use_remainder::use_remainder() {}

    component::use_remainder::use_remainder(const slot& remainder) : remainder(std::make_shared<slot>(remainder)) {}

    component::use_remainder::use_remainder(slot&& remainder) : remainder(std::make_shared<slot>(std::move(remainder))) {}

    component::use_remainder::use_remainder(const box<slot>& remainder) : remainder(std::make_shared<slot>(*remainder)) {}

    component::use_remainder::use_remainder(box<slot>&& remainder) : remainder(std::move(remainder)) {}

    component::use_remainder::use_remainder(const use_remainder& remainder) : remainder(std::make_shared<slot>(*remainder.remainder)) {}

    component::use_remainder::use_remainder(use_remainder&& remainder) : remainder(std::move(remainder.remainder)) {}

    component::use_remainder::~use_remainder() {}

    auto component::use_remainder::operator=(const slot& r) -> use_remainder& {
        remainder = std::make_shared<slot>(r);
        return *this;
    }

    auto component::use_remainder::operator=(slot&& r) -> use_remainder& {
        remainder = std::make_shared<slot>(std::move(r));
        return *this;
    }

    auto component::use_remainder::operator=(const box<slot>& r) -> use_remainder& {
        remainder = std::make_shared<slot>(*r);
        return *this;
    }

    auto component::use_remainder::operator=(box<slot>&& r) -> use_remainder& {
        remainder = std::move(r);
        return *this;
    }

    auto component::use_remainder::operator=(const use_remainder& r) -> use_remainder& {
        remainder = std::make_shared<slot>(*r.remainder);
        return *this;
    }

    auto component::use_remainder::operator=(use_remainder&& r) -> use_remainder& {
        remainder = std::move(r.remainder);
        return *this;
    }

    bool component::use_remainder::operator==(const use_remainder& other) const {
        return *remainder == *other.remainder;
    }

    bool component::charged_projectiles::operator==(const charged_projectiles& other) const {
        return projectiles.equal(other.projectiles, [](auto& i0, auto& i1) { return i0 == i1; });
    }

    bool component::bundle_contents::operator==(const bundle_contents& other) const {
        return content.equal(other.content, [](auto& i0, auto& i1) { return i0 == i1; });
    }

    component::container::container() {}

    component::container::container(container&& mov) : items(std::move(mov.items)) {}

    component::container::container(const container& copy) : items(copy.items) {}

    component::container& component::container::operator=(container&& mov) {
        items = std::move(mov.items);
        return *this;
    }

    component::container& component::container::operator=(const container& copy) {
        items = copy.items;
        return *this;
    }
    std::optional<size_t> component::container::get_free_slot() {
        size_t index = 0;

        for (auto& to_select : items) {
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
            if (to_insert.id == item.id) {
                if (to_insert.is_same_def(item)) {
                    int32_t add_res = int32_t(max_count) - to_insert.count;
                    if (add_res > 0) {
                        add_res = std::min(add_res, to_add);
                        to_insert.count += add_res;
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
        return items[slot];
    }

    bool component::container::contains(size_t slot) {
        return get(slot) != std::nullopt;
    }

    std::optional<size_t> component::container::contains(const slot_data& item) const {
        size_t index = 0;
        for (auto& i : items) {
            if (i)
                if (i->is_same_def(item))
                    return index;
            ++index;
        };
        return std::nullopt;
    }

    list_array<size_t> component::container::contains(const std::string& id, size_t count) const {
        list_array<size_t> res;
        id_item real_id(id);
        size_t index = 0;
        for (auto& check : items) {
            if (check) {
                if (check->id == real_id.value) {
                    res.push_back(index);
                    if (count > (size_t)check->count)
                        count -= (size_t)check->count;
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
            for (auto& to_erase : items) {
                if (to_erase)
                    if (to_erase->id == id)
                        to_erase = std::nullopt;
            }
        } else {
            for (auto& to_erase : items) {
                if (to_erase) {
                    if (to_erase->id == id) {
                        if (size_t(to_erase->count) <= count) {
                            count -= size_t(to_erase->count);
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
            to_erase_it = std::nullopt;
        }
    }

    size_t component::container::count() const {
        size_t count = 0;
        for (auto& i : items)
            count += i->count;
        return count;
    }

    size_t component::container::size() const {
        size_t index = 0;
        for (auto& i : items)
            if (i)
                ++index;
        return index;
    }

    bool component::container::operator==(const container& other) const {
        return items.equal(other.items, [](auto& i0, auto& i1) { return i0 == i1; });
    }

    component::component() {}

    component::component(component&& mov) : type(std::move(mov.type)) {}

    component::component(const component& copy) : type(copy.type) {}

    component& component::operator=(component&& mov) noexcept {
        type = std::move(mov.type);
        return *this;
    }

    component& component::operator=(const component& copy) {
        using Type_T = std::decay_t<decltype(type)>;
        type = copy.type;
        return *this;
    }

    int32_t component::get_id() const {
        return std::visit(
            [](auto& it) {
                return std::decay_t<decltype(it)>::item_id::value;
            },
            type
        );
    }

    bool component::operator==(const component& other) const = default;
    bool component::operator!=(const component& other) const = default;
}