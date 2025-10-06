/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <src/api/client.hpp>
#include <src/api/command.hpp>
#include <src/api/configuration.hpp>
#include <src/api/entity_id_map.hpp>
#include <src/api/players.hpp>
#include <src/api/recipe.hpp>
#include <src/api/registers.hpp>
#include <src/api/world.hpp>
#include <src/base_objects/commands.hpp>
#include <src/base_objects/entity.hpp>
#include <src/base_objects/player.hpp>
#include <src/plugin/main.hpp>

namespace copper_server::build_in_plugins {
    struct inventory : public PluginAutoRegister<"base/play_engine/inventory", inventory> {
        inventory() {}

        ~inventory() noexcept {}

        void OnInitialization(const PluginRegistrationPtr& _) override {
            register_packet_processor([](api::packets::server_bound::play::set_beacon&& packet, base_objects::SharedClientData& client) {
                client.packets_state.get_play_data([&packet](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                    if (play_data.current_screen)
                        play_data.current_screen->event_set_beacon(packet.primary_effect.transform([](auto it) { return it.value; }), packet.secondary_effect.transform([](auto it) { return it.value; }));
                });
            });
            register_packet_processor([](api::packets::server_bound::play::rename_item&& packet, base_objects::SharedClientData& client) {
                client.packets_state.get_play_data([&packet](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                    if (play_data.current_screen)
                        play_data.current_screen->event_anvil_set_name(packet.new_name.value);
                });
            });
            register_packet_processor([](api::packets::server_bound::play::set_carried_item&& packet, base_objects::SharedClientData& client) {
                if (packet.slot >= 0 && packet.slot <= 7)
                    if (client.player_data.assigned_entity)
                        client.player_data.assigned_entity->set_selected_item(packet.slot);
            });
            register_packet_processor([](api::packets::server_bound::play::set_creative_mode_slot&& packet, base_objects::SharedClientData& client) {
                if (client.player_data.assigned_entity && client.player_data.gamemode == 1) {
                    base_objects::slot slot;
                    std::move(packet.item).read(slot);
                    if (slot)
                        client.player_data.assigned_entity->inventory[packet.slot] = std::move(*slot);
                    else
                        client.player_data.assigned_entity->inventory.erase(packet.slot);
                }
            });
            register_packet_processor([](api::packets::server_bound::play::pick_item_from_block&& packet, base_objects::SharedClientData& client) {
                if (client.player_data.assigned_entity) {
                    auto& e = *client.player_data.assigned_entity;
                    if (e.current_world()) {
                        if (!packet.include_data) {
                            auto block = e.current_world()->get_block(packet.location.x, packet.location.y, packet.location.z);
                            auto item = block.getStaticData().item_id;
                            e.inventory.at(36 + e.get_selected_item()) = base_objects::slot_data::create_item(item);
                        } else {
                            //TODO
                        }
                    }
                }
            });
            register_packet_processor([](api::packets::server_bound::play::pick_item_from_entity&& packet, base_objects::SharedClientData& client) {
                if (client.player_data.assigned_entity) {
                    auto& e = *client.player_data.assigned_entity;
                    auto oe = packet.id.get_entity();
                    if (oe) {
                        auto item = oe->const_data().spawn_egg;

                        if (item) {
                            /*auto& slot_data =*/e.inventory.at(36 + e.get_selected_item()) = base_objects::slot_data::create_item(*item);
                            //if (packet.include_data) {
                            //    //TODO
                            //}
                        }
                    }
                }
            });
            register_packet_processor([](api::packets::server_bound::play::bundle_item_selected&& packet, base_objects::SharedClientData& client) {
                client.packets_state.get_play_data([&packet](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                    if (play_data.current_screen)
                        play_data.current_screen->event_request_bundle_item_take(packet.bundle_slot, packet.item_slot);
                    else if (play_data.main_screen)
                        play_data.main_screen->event_request_bundle_item_take(packet.bundle_slot, packet.item_slot);
                });
            });
            register_packet_processor([](api::packets::server_bound::play::container_button_click&& packet, base_objects::SharedClientData& client) {
                client.packets_state.get_play_data([&packet](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                    if (play_data.current_screen)
                        if (play_data.current_screen->get_windows_id() == packet.window_id.value)
                            play_data.current_screen->event_button_click(packet.button_id);
                });
            });
            register_packet_processor([](api::packets::server_bound::play::container_click&& packet, base_objects::SharedClientData& client) {
                client.packets_state.get_play_data([&packet](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                    using screen = base_objects::SharedClientData::packets_state_t::play_data_t::screen;
                    if (play_data.current_screen)
                        if (play_data.current_screen->get_windows_id() == packet.window_id.value) {
                            static constexpr auto convert_hashed_slot = [](const api::packets::server_bound::play::container_click::hashed_slot_data& data) {
                                screen::click_data::hashed_slot_data res;
                                res.add_components = data.add_components.convert_fn([](const api::packets::server_bound::play::container_click::hashed_slot_data::component& c) {
                                    return screen::click_data::hashed_slot_data::component{
                                        .type = c.type,
                                        .crc32c_hash = c.crc32c_hash
                                    };
                                });
                                res.count = data.count;
                                res.item_id = data.item_id;
                                res.remove_components = data.remove_components.convert_fn([](auto& it) { return (int32_t)it; });
                                return res;
                            };
                            screen::click_data cl_data;
                            cl_data.button = packet.button;
                            if (packet.carry_item)
                                cl_data.carry_item = convert_hashed_slot(*packet.carry_item);
                            cl_data.changed = packet.changed.convert_fn([](const copper_server::api::packets::server_bound::play::container_click::changed_slot& data) {
                                return screen::click_data::changed_slot{
                                    .slot = data.slot,
                                    .data = data.data ? std::make_optional(convert_hashed_slot(*data.data)) : std::nullopt
                                };
                            });
                            cl_data.mode = packet.mode;
                            cl_data.slot = packet.slot;
                            cl_data.state_id = packet.state_id;


                            play_data.current_screen->event_click(cl_data);
                        }
                });
            });

            register_packet_processor([](api::packets::server_bound::play::container_close&& packet, base_objects::SharedClientData& client) {
                client.packets_state.get_play_data([&packet](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                    if (play_data.current_screen)
                    /*if (play_data.current_screen->get_windows_id() == packet.window_id.value)*/ { //vanilla server ignores this
                        play_data.current_screen->event_close();
                        play_data.current_screen = nullptr;
                    }
                });
            });

            register_packet_processor([](api::packets::server_bound::play::container_slot_state_changed&& packet, base_objects::SharedClientData& client) {
                client.packets_state.get_play_data([&packet](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                    if (play_data.current_screen)
                        if (play_data.current_screen->get_windows_id() == packet.window_id.value)
                            play_data.current_screen->event_slot_state_changed(packet.slot_id, packet.state);
                });
            });
            register_packet_processor([](api::packets::server_bound::play::edit_book&& packet, base_objects::SharedClientData& client) {
                client.packets_state.get_play_data([&packet](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                    if (play_data.main_screen)
                        play_data.main_screen->event_book_edit_request(packet.slot, packet.entries.convert_fn([](auto& str) { return std::string_view(str.value); }), packet.title.transform([](auto& str) { return std::string_view(str.value); }));
                });
            });
        }

        //recipe result = 0
        //small craft:
        //  1  2
        //  3  4
        //
        //Armor:
        // head 5
        // chest 6
        // leggings 7
        // foot 8
        // inventory 9-35
        // hotbar 36-44
        // second hand 45
        struct main_screen : public base_objects::SharedClientData::packets_state_t::play_data_t::main_screen_i {
            base_objects::slot carry_item;
            enum class DragMode {
                NONE,
                LEFT,
                RIGHT,
                MIDDLE
            } drag_mode
                = DragMode::NONE;
            std::unordered_set<int32_t> drag_slots;

            main_screen(base_objects::SharedClientData& client) : main_screen_i(client) {}

            ~main_screen() {
                if (carry_item)
                    drop_item(carry_item);
            }

            base_objects::slot& get_carried_item() override {
                return carry_item;
            }

            virtual bool valid_slot(int32_t slot) const override {
                return slot <= 45 && slot >= 0;
            }

            virtual bool has_item(int32_t slot) const override {
                return client.player_data.assigned_entity->inventory.contains(slot);
            }

            virtual int32_t max_size() const override {
                return 46;
            }

            virtual base_objects::slot_data& get_slot(int32_t slot) override {
                return client.player_data.assigned_entity->inventory.at(slot);
            }

            void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override {
                for (auto& [slot, data] : client.player_data.assigned_entity->inventory)
                    fn(data, slot);
            }

            void add_item(base_objects::slot& item) {
                auto& inv = client.player_data.assigned_entity->inventory;
                if (item) {
                    for (size_t i = 9; i <= 44; i++) {
                        if (inv.find((int32_t)i) != inv.end()) {
                            if (inv[i].is_same_def(*item)) {
                                auto max_stack = inv[i].access_component<base_objects::component::max_stack_size>().size;
                                if (inv[i].count + item->count <= max_stack.value) {
                                    inv[i].count += item->count;
                                    item = std::nullopt;
                                    break;
                                } else {
                                    item->count -= (max_stack - inv[i].count);
                                    inv[i].count = max_stack;
                                }
                                set_slot(i, inv[i]);
                            }
                        }
                    }
                }
                if (item) {
                    for (size_t i = 9; i <= 44; i++) {
                        if (inv.find((int32_t)i) == inv.end()) {
                            inv[i] = std::move(*item);
                            set_slot(i, inv[i]);
                            item = std::nullopt;
                            break;
                        }
                    }
                }
            }

            void event_place_recipe(int32_t recipe_id, bool make_all) override {
                if (!client.player_data.known_recipes.contains(recipe_id))
                    return;
                auto& recipe = api::registers::recipe_table_cache.at(recipe_id)->second;
                if (make_all) {
                    base_objects::command_context context(client);
                    context.apply_executor_data();
                    auto placement = api::recipe::process_placement(recipe, 2, 2, context);
                    //TODO
                } else
                    client << api::packets::client_bound::play::place_ghost_recipe{
                        .window_id = get_windows_id(),
                        .display = api::packets::recipe_display::create(recipe)
                    };
            }

            void event_button_click(int32_t button_id) override {} //NOTING

            void event_click(click_data& data) override {
                auto& inv = client.player_data.assigned_entity->inventory;
                switch (data.mode) {
                case 0: {                // normal mouse click
                    if (data.slot < 0) { // Click outside inventory
                        if (carry_item) {
                            if (data.button == 0) { // Left-click: drop full stack
                                drop_item(*carry_item);
                                carry_item = std::nullopt;
                            } else if (data.button == 1) { // Right-click: drop one item
                                base_objects::slot single_item = *carry_item;
                                single_item->count = 1;
                                drop_item(*single_item);
                                carry_item->count--;
                                if (carry_item->count == 0) {
                                    carry_item = std::nullopt;
                                }
                            }
                        }
                        break;
                    }
                    // Click inside inventory
                    if (data.button == 0) { // Left click
                        if (carry_item) {
                            if (inv.count(data.slot)) { // Slot has an item
                                if (inv[data.slot].is_same_def(*carry_item)) {
                                    auto max_stack = carry_item->access_component<base_objects::component::max_stack_size>().size.value;
                                    int32_t can_add = max_stack - inv[data.slot].count;
                                    int32_t to_add = std::min(can_add, carry_item->count);
                                    inv[data.slot].count += to_add;
                                    carry_item->count -= to_add;
                                    if (carry_item->count == 0)
                                        carry_item = std::nullopt;
                                } else {
                                    std::swap(inv[data.slot], *carry_item);
                                }
                            } else { // Slot is empty
                                inv[data.slot] = *carry_item;
                                carry_item = std::nullopt;
                            }
                        } else { // Not carrying an item
                            if (inv.count(data.slot)) {
                                carry_item = inv[data.slot];
                                inv.erase(data.slot);
                            }
                        }
                    } else if (data.button == 1) { // Right click
                        if (carry_item) {
                            if (inv.count(data.slot)) {
                                if (inv[data.slot].is_same_def(*carry_item)) {
                                    auto max_stack = carry_item->access_component<base_objects::component::max_stack_size>().size.value;
                                    if (inv[data.slot].count < max_stack) {
                                        inv[data.slot].count++;
                                        carry_item->count--;
                                        if (carry_item->count == 0)
                                            carry_item = std::nullopt;
                                    }
                                } else {
                                    std::swap(inv[data.slot], *carry_item);
                                }
                            } else {
                                inv[data.slot] = *carry_item;
                                inv[data.slot].count = 1;
                                carry_item->count--;
                                if (carry_item->count == 0)
                                    carry_item = std::nullopt;
                            }
                        } else { // Not carrying an item
                            if (inv.count(data.slot)) {
                                int32_t half = (inv[data.slot].count + 1) / 2;
                                carry_item = inv[data.slot];
                                carry_item->count = half;
                                inv[data.slot].count -= half;
                                if (inv[data.slot].count == 0)
                                    inv.erase(data.slot);
                            }
                        }
                    }
                    set_slot(data.slot, inv.count(data.slot) ? base_objects::slot(inv[data.slot]) : std::nullopt);
                    break;
                }
                case 1: { // shift click (move between inventory/hotbar/armor)
                    if (!inv.count(data.slot))
                        break;
                    base_objects::slot item_to_move = inv[data.slot];

                    auto try_move = [&](int32_t start, int32_t end) {
                        for (int32_t i = start; i <= end; ++i) {
                            if (!inv.count(i)) {
                                inv[i] = *item_to_move;
                                inv.erase(data.slot);
                                set_slot(i, inv[i]);
                                set_slot(data.slot, std::nullopt);
                                return true;
                            }
                        }
                        return false;
                    };

                    if (data.slot >= 5 && data.slot <= 8) { // Armor -> Inventory
                        try_move(9, 44);
                    } else if (data.slot >= 9 && data.slot <= 35) { // Inventory -> Hotbar/Armor
                        // Armor check would go here based on item type
                        try_move(36, 44);
                    } else if (data.slot >= 36 && data.slot <= 44) { // Hotbar -> Inventory/Armor
                        // Armor check would go here
                        try_move(9, 35);
                    }
                    break;
                }
                case 2: { // swap with hotbar or offhand
                    if (data.button >= 0 && data.button <= 8) {
                        int32_t hotbar_slot = 36 + data.button;
                        if (inv.count(data.slot) || inv.count(hotbar_slot)) {
                            std::swap(inv[data.slot], inv[hotbar_slot]);
                            set_slot(data.slot, inv.count(data.slot) ? base_objects::slot(inv[data.slot]) : std::nullopt);
                            set_slot(hotbar_slot, inv.count(hotbar_slot) ? base_objects::slot(inv[hotbar_slot]) : std::nullopt);
                        }
                    } else if (data.button == 40) { // Offhand swap
                        if (inv.count(data.slot) || inv.count(45)) {
                            std::swap(inv[data.slot], inv[45]);
                            set_slot(data.slot, inv.count(data.slot) ? base_objects::slot(inv[data.slot]) : std::nullopt);
                            set_slot(45, inv.count(45) ? base_objects::slot(inv[45]) : std::nullopt);
                        }
                    }
                    break;
                }
                case 3: { // creative mode middle click
                    if (client.player_data.gamemode == 1 && inv.count(data.slot)) {
                        carry_item = inv[data.slot];
                        if (carry_item)
                            carry_item->count = carry_item->access_component<base_objects::component::max_stack_size>().size.value;
                    }
                    break;
                }
                case 4: {                   // drop
                    if (data.button == 0) { // Drop single item
                        if (inv.count(data.slot)) {
                            base_objects::slot single_item = inv[data.slot];
                            single_item->count = 1;
                            drop_item(*single_item);
                            inv[data.slot].count--;
                            if (inv[data.slot].count == 0) {
                                inv.erase(data.slot);
                                set_slot(data.slot, std::nullopt);
                            } else {
                                set_slot(data.slot, inv[data.slot]);
                            }
                        }
                    } else if (data.button == 1) { // Drop stack
                        if (inv.count(data.slot)) {
                            drop_item(inv[data.slot]);
                            inv.erase(data.slot);
                            set_slot(data.slot, std::nullopt);
                        }
                    }
                    break;
                }
                case 5: { // painting
                    switch (data.button) {
                    case 0: // start left drag
                        drag_mode = DragMode::LEFT;
                        drag_slots.clear();
                        break;
                    case 4: // start right drag
                        drag_mode = DragMode::RIGHT;
                        drag_slots.clear();
                        break;
                    case 8: // start middle drag
                        if (client.player_data.gamemode == 1) {
                            drag_mode = DragMode::MIDDLE;
                            drag_slots.clear();
                        }
                        break;
                    case 1: // add slot left drag
                    case 5: // add slot right drag
                    case 9: // add slot middle drag
                        if (drag_mode != DragMode::NONE && data.slot >= 0)
                            drag_slots.insert(data.slot);
                        break;
                    case 2: // end left drag
                        if (drag_mode == DragMode::LEFT && carry_item) {
                            int32_t total_items = carry_item->count;
                            int32_t slots_count = drag_slots.size();
                            if (slots_count > 0) {
                                int32_t per_slot = total_items / slots_count;
                                int32_t remainder = total_items % slots_count;
                                for (int32_t slot_id : drag_slots) {
                                    if (!inv.count(slot_id)) {
                                        inv[slot_id] = *carry_item;
                                        inv[slot_id].count = 0;
                                    }
                                    if (inv[slot_id].is_same_def(*carry_item)) {
                                        inv[slot_id].count += per_slot;
                                        set_slot(slot_id, inv[slot_id]);
                                    }
                                }
                                carry_item->count = remainder;
                                if (carry_item->count == 0)
                                    carry_item = std::nullopt;
                            }
                        }
                        drag_mode = DragMode::NONE;
                        drag_slots.clear();
                        break;
                    case 6: // end right drag
                        if (drag_mode == DragMode::RIGHT && carry_item) {
                            for (int32_t slot_id : drag_slots) {
                                if (carry_item->count > 0) {
                                    if (!inv.count(slot_id)) {
                                        inv[slot_id] = *carry_item;
                                        inv[slot_id].count = 0;
                                    }
                                    if (inv[slot_id].is_same_def(*carry_item)) {
                                        inv[slot_id].count++;
                                        carry_item->count--;
                                        set_slot(slot_id, inv[slot_id]);
                                    }
                                }
                            }
                            if (carry_item->count == 0)
                                carry_item = std::nullopt;
                        }
                        drag_mode = DragMode::NONE;
                        drag_slots.clear();
                        break;
                    case 10: // end middle drag
                        if (drag_mode == DragMode::MIDDLE && carry_item && client.player_data.gamemode == 1) {
                            for (int32_t slot_id : drag_slots) {
                                if (!inv.count(slot_id)) {
                                    inv[slot_id] = *carry_item;
                                    inv[slot_id].count = inv[slot_id].access_component<base_objects::component::max_stack_size>().size.value;
                                    set_slot(slot_id, inv[slot_id]);
                                }
                            }
                        }
                        drag_mode = DragMode::NONE;
                        drag_slots.clear();
                        break;
                    }
                    break;
                }
                case 6: { // double click
                    if (carry_item) {
                        auto& item_to_collect = *carry_item;
                        auto max_stack = item_to_collect.access_component<base_objects::component::max_stack_size>().size.value;
                        for (size_t i = 9; i <= 44; ++i) {
                            if (item_to_collect.count >= max_stack)
                                break;
                            if (inv.count(i) && inv[i].is_same_def(item_to_collect)) {
                                int32_t can_add = max_stack - item_to_collect.count;
                                int32_t to_move = std::min(can_add, inv[i].count);
                                item_to_collect.count += to_move;
                                inv[i].count -= to_move;
                                if (inv[i].count == 0) {
                                    inv.erase(i);
                                    set_slot(i, std::nullopt);
                                } else {
                                    set_slot(i, inv[i]);
                                }
                            }
                        }
                    }
                    break;
                }
                }
            }

            void event_close() override {} //NOTING

            void event_slot_state_changed(int32_t slot_id, bool state) override {} //NOTING

            void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override {
                if (client.player_data.assigned_entity) {
                    auto& inv = client.player_data.assigned_entity->inventory;
                    if (auto bundle_slot = inv.find(bundle_slot_id); bundle_slot != inv.end()) {
                        if (bundle_slot->second.has_component<base_objects::component::bundle_contents>()) {
                            auto& bundle = bundle_slot->second.get_component<base_objects::component::bundle_contents>();
                            if (bundle.content.size() < in_bundle_slot_id) {
                                auto item = bundle.content.take((size_t)in_bundle_slot_id);
                                add_item(item);
                                if (item)
                                    drop_item(item);
                            }
                        }
                    }
                }
            }

            void event_book_edit_request(int32_t slot_id, const list_array<std::string_view>& text, const std::optional<std::string_view>& title) override {
                if (client.player_data.assigned_entity) {
                    auto& inv = client.player_data.assigned_entity->inventory;
                    if (auto book_slot = inv.find(slot_id); book_slot != inv.end()) {
                        if (book_slot->second.has_component<base_objects::component::writable_book_content>()) {
                            auto& book = book_slot->second.get_component<base_objects::component::writable_book_content>();
                            book.pages.clear();
                            for (auto& it : text)
                                book.pages.push_back(base_objects::component::writable_book_content::page{.raw = std::string(it)});

                            if (title) {
                                auto& written_book_content = book_slot->second.access_component<base_objects::component::written_book_content>();
                                written_book_content.author = client.name;
                                written_book_content.generation = 0;
                                written_book_content.raw_title = std::string(*title);
                                written_book_content.resolved = false;
                                written_book_content.pages = book.pages.take().convert_fn([](base_objects::component::writable_book_content::page&& page) {
                                    base_objects::component::written_book_content::page{
                                        .raw = std::move(page.raw),
                                        .filtered = std::move(page.filtered)
                                    };
                                });
                                book_slot->second.remove_component<base_objects::component::writable_book_content>();
                            }
                            set_slot(slot_id, book_slot->second);
                        }
                    }
                }
            }
        };

        virtual void OnPlay_pre_initialize(base_objects::SharedClientData& client) {
            client.packets_state.get_play_data([&client](base_objects::SharedClientData::packets_state_t::play_data_t& play_data) {
                std::unique_ptr<base_objects::SharedClientData::packets_state_t::play_data_t::main_screen_i> res;
                res.reset(new main_screen(client));
                play_data.init_main_screen(std::move(res));
            });
        }
    };
}