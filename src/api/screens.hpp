/*
 * Copyright 2025-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_API_SCREENS
#define SRC_API_SCREENS
#include <cstdint>
#include <functional>
#include <src/api/entity.hpp>
#include <src/base_objects/events/sync_event.hpp>
#include <src/base_objects/shared_client_data.hpp>
#include <src/base_objects/slot.hpp>

namespace copper_server::api::screens {
    using base_screen = base_objects::SharedClientData::packets_state_t::play_data_t::screen;
    using click_data = base_objects::SharedClientData::packets_state_t::play_data_t::screen::click_data;

    namespace detail {
        //the generic supports only sizes supported by protocol:
        //
        //9x1,9x2,9x3,9x4,9x5,9x6 and 3x3
        // any other size is undefined and would lead to linking error
        template <size_t x, size_t y>
        struct generic : public base_screen {
            static constexpr inline size_t columns = x;
            static constexpr inline size_t rows = y;
            static constexpr inline size_t player_inventory_offset = columns * rows;

            int32_t max_size() const override {
                return player_inventory_offset + 36; // + player_inventory
            }

            void event_place_recipe(int32_t recipe_id, bool make_all) override;
            void event_button_click(int32_t button_id) override;
            void event_click(click_data& data) override;
            void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

            virtual bool the_container_is_frozen() const {
                return false;
            }

            generic(base_objects::SharedClientData& client);
            virtual ~generic();

            virtual void clicked(const click_data&) = 0;
            virtual void closed() = 0;
        };

        //helper for plugins to implement the menu feature using switch in menu_click
        //
        //the container should set items into the inventory map and receive the clicked slot in the menu_click or other specific overload
        template <size_t x, size_t y>
        struct _generic_custom_menu : public generic<x, y> {
            std::unordered_map<int32_t, base_objects::slot_data&> inventory;

            _generic_custom_menu(base_objects::SharedClientData& client) : generic<x, y>(client) {}
            virtual ~_generic_custom_menu() = default;
            
            bool the_container_is_frozen() const override final {
                return true;
            }

            bool has_item(int32_t slot) const override {
                return inventory.contains(slot);
            }

            base_objects::slot_data& get_slot(int32_t slot) override {
                return inventory.at(slot);
            }

            void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override {
                for (auto& [slot, data] : inventory)
                    fn(data, slot);
            }

            void clicked(const click_data& click) override final {
                switch (click.mode) {
                case 0:
                case 1:
                    if (click.button == 0)
                        menu_left_click(click.slot);
                    else if (click.button == 1)
                        menu_right_click(click.slot);
                    break;
                case 3:
                    menu_drop_item(click.slot);
                    break;
                case 6:
                    menu_double_click(click.slot);
                    break;
                default:
                    break;
                }
            }

            virtual void menu_right_click(int32_t slot_affected) {
                menu_click(slot_affected);
            }

            virtual void menu_double_click(int32_t slot_affected) {
                menu_click(slot_affected);
            }

            virtual void menu_left_click(int32_t slot_affected) {
                menu_click(slot_affected);
            }

            virtual void menu_drop_item(int32_t slot_affected) {
                menu_click(slot_affected);
            }

            virtual void menu_click(int32_t slot_affected) = 0;
        };
    }

    class horse : public base_screen {
        api::ecs::entity entity;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        //the slots is dynamic and depends on context, ie, entity type: horse, undead horse variants, camel, llama, donkey theirs container size
        bool valid_slot(int32_t) const override;
        bool has_item(int32_t) const override;
        int32_t max_size() const override;
        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();

        horse(base_objects::SharedClientData& client, api::ecs::entity entity);
        virtual ~horse();
    };

    class anvil : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t first_item_slot = 0;
        static constexpr inline size_t second_item_slot = 1;
        static constexpr inline size_t result_slot = 2;
        static constexpr inline size_t player_inventory_offset = 3;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;
        virtual void clicked(const click_data&);
        virtual void closed();
        void set_repair_cost(int16_t);

        anvil(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~anvil();
    };

    class beacon : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t payment_slot = 0;
        static constexpr inline size_t player_inventory_offset = 1;

        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;
        //the client uses separate packet for buttons
        virtual void clicked(const click_data&);
        virtual void closed();
        void set_power_level(int16_t);
        void set_first_potion(int16_t);
        void set_second_potion(int16_t);

        beacon(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~beacon();
    };

    class furnace : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t first_item_slot = 0;
        static constexpr inline size_t second_item_slot = 1;
        static constexpr inline size_t result_slot = 2;
        static constexpr inline size_t player_inventory_offset = 3;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();
        void set_fuel_left(int16_t);
        void set_max_fuel(int16_t);
        void set_progress(int16_t);
        void set_max_progress(int16_t);

        furnace(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~furnace();
    };

    class blast_furnace : public furnace {
    public:
        blast_furnace(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~blast_furnace();
    };

    class brewing_stand : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t result_0_slot = 0;
        static constexpr inline size_t result_1_slot = 1;
        static constexpr inline size_t result_2_slot = 2;
        static constexpr inline size_t potion_ingredient_slot = 3;
        static constexpr inline size_t blaze_powder_slot = 4;
        static constexpr inline size_t player_inventory_offset = 5;

        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();
        void set_brew_time(int16_t);
        void set_fuel_left(int16_t);

        brewing_stand(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~brewing_stand();
    };

    class cartography_table : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t map_slot = 0;
        static constexpr inline size_t paper_slot = 1;
        static constexpr inline size_t output_slot = 2;
        static constexpr inline size_t player_inventory_offset = 3;

        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();

        cartography_table(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~cartography_table();
    };

    class crafter_3x3 : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_slot_state_changed(int32_t slot_id, bool state) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t result_slot = 45;
        static constexpr inline size_t input_slot_start = 0;
        static constexpr inline size_t input_slots_size = 9;
        static constexpr inline size_t player_inventory_offset = 9;

        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;
        virtual void clicked(const click_data&);
        virtual void closed();

        crafter_3x3(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~crafter_3x3();
    };

    class crafting : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t result_slot = 0;
        static constexpr inline size_t input_slot_start = 1;
        static constexpr inline size_t input_slots_size = 9;
        static constexpr inline size_t player_inventory_offset = 10;

        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;
        virtual void clicked(const click_data&);
        virtual void closed();

        crafting(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~crafting();
    };

    class enchantment : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t enchant_slot = 0;
        static constexpr inline size_t lapis_slot = 1;
        static constexpr inline size_t player_inventory_offset = 2;

        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void enchant_first();  //button 0
        virtual void enchant_second(); //button 1
        virtual void enchant_thrid();  //button 2
        virtual void clicked(const click_data&);
        virtual void closed();

        void set_level_requirement_top(int32_t level);
        void set_level_requirement_middle(int32_t level);
        void set_level_requirement_bottom(int32_t level);
        void set_enchantment_seed(int32_t seed);
        void set_enchantment_id_top(int32_t id);
        void set_enchantment_id_middle(int32_t id);
        void set_enchantment_id_bottom(int32_t id);
        void set_enchantment_lvl_top(int32_t level);
        void set_enchantment_lvl_middle(int32_t level);
        void set_enchantment_lvl_bottom(int32_t level);


        enchantment(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~enchantment();
    };

    using generic_9x1 = detail::generic<9, 1>;
    using generic_9x2 = detail::generic<9, 2>;
    using generic_9x3 = detail::generic<9, 3>;
    using generic_9x4 = detail::generic<9, 4>;
    using generic_9x5 = detail::generic<9, 5>;
    using generic_9x6 = detail::generic<9, 6>;
    using generic_3x3 = detail::generic<3, 3>;

    class chest : public generic_9x3 {
        util::XYZ<int32_t> pos;

    public:
        bool has_item(int32_t) const override;
        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();

        chest(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~chest();
    };

    class large_chest : public generic_9x6 {
        util::XYZ<int32_t> pos0;
        util::XYZ<int32_t> pos1;

    public:
        bool has_item(int32_t) const override;
        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;
        virtual void clicked(const click_data&);
        virtual void closed();

        large_chest(base_objects::SharedClientData& client, util::XYZ<int32_t> pos0, util::XYZ<int32_t> pos1);
        virtual ~large_chest();
    };

    class minecart_chest : public generic_9x3 {
        api::ecs::entity entity;

    public:
        bool has_item(int32_t) const override;
        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;
        virtual void clicked(const click_data&);
        virtual void closed();

        minecart_chest(base_objects::SharedClientData& client, api::ecs::entity entity);
        virtual ~minecart_chest();
    };

    class ender_chest : public generic_9x3 {
    public:
        bool has_item(int32_t) const override;
        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;
        virtual void clicked(const click_data&);
        virtual void closed();

        ender_chest(base_objects::SharedClientData& client);
        virtual ~ender_chest();
    };

    class barrel : public generic_9x3 {
        util::XYZ<int32_t> pos;

    public:
        bool has_item(int32_t) const override;
        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;
        virtual void clicked(const click_data&);
        virtual void closed();

        barrel(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~barrel();
    };

    class grindstone : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t first_item_slot = 0;
        static constexpr inline size_t second_item_slot = 1;
        static constexpr inline size_t result_slot = 2;
        static constexpr inline size_t player_inventory_offset = 3;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();

        grindstone(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~grindstone();
    };

    class hopper : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t player_inventory_offset = 5;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();

        hopper(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~hopper();
    };

    class lectern : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t book_slot = 0;
        static constexpr inline size_t player_inventory_offset = 1;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return 1; // player inventory is disabled
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void previous_page_button_clicked(); //button_click == 1
        virtual void next_page_button_clicked();     //button_click == 2
        virtual void take_book_request();            // button_click == 3
        virtual void select_page(int32_t);           //button_click + 100

        virtual void clicked(const click_data&);
        virtual void closed();
        void set_page_number(int16_t); //yes this is limited by protocol, see container_set_data packet

        lectern(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~lectern();
    };

    class loom : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t banner_slot = 0;
        static constexpr inline size_t dye_slot = 1;
        static constexpr inline size_t pattern_slot = 2;
        static constexpr inline size_t result_slot = 3;
        static constexpr inline size_t player_inventory_offset = 4;
        virtual void select_recipe(int32_t);
        virtual void clicked(const click_data&);
        virtual void closed();
        void set_selected_pattern(int16_t);

        loom(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~loom();
    };

    class merchant : public base_screen {
        api::ecs::entity entity;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t trade_0_slot = 0;
        static constexpr inline size_t trade_1_slot = 1;
        static constexpr inline size_t player_inventory_offset = 3;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();

        merchant(base_objects::SharedClientData& client, api::ecs::entity entity);
        virtual ~merchant();
    };

    class shulker_box : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t player_inventory_offset = 27;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();

        shulker_box(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~shulker_box();
    };

    class smithing : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t template_slot = 0;
        static constexpr inline size_t base_item_slot = 1;
        static constexpr inline size_t additional_item = 2;
        static constexpr inline size_t result_slot = 3;
        static constexpr inline size_t player_inventory_offset = 4;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void clicked(const click_data&);
        virtual void closed();

        smithing(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~smithing();
    };

    class smoker : public furnace {
    public:
        smoker(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~smoker();
    };

    class stonecutter : public base_screen {
        util::XYZ<int32_t> pos;
        void event_place_recipe(int32_t recipe_id, bool make_all) override;
        void event_button_click(int32_t button_id) override;
        void event_click(click_data& data) override;
        void event_request_bundle_item_take(int32_t bundle_slot_id, int32_t in_bundle_slot_id) override;

    public:
        static constexpr inline size_t input_slot = 0;
        static constexpr inline size_t result_slot = 1;
        static constexpr inline size_t player_inventory_offset = 2;
        bool has_item(int32_t) const override;

        virtual int32_t max_size() const {
            return player_inventory_offset + 36; // + player_inventory
        }

        base_objects::slot_data& get_slot(int32_t) override;
        void iterate(std::move_only_function<void(base_objects::slot_data&, int32_t)>&& fn) override;

        virtual void select_recipe(int32_t);
        virtual void clicked(const click_data&);
        virtual void closed();
        void set_selected_recipe(int16_t);

        stonecutter(base_objects::SharedClientData& client, util::XYZ<int32_t> pos);
        virtual ~stonecutter();
    };

    using generic_custom_menu_9x1 = detail::_generic_custom_menu<9, 1>;
    using generic_custom_menu_9x2 = detail::_generic_custom_menu<9, 2>;
    using generic_custom_menu_9x3 = detail::_generic_custom_menu<9, 3>;
    using generic_custom_menu_9x4 = detail::_generic_custom_menu<9, 4>;
    using generic_custom_menu_9x5 = detail::_generic_custom_menu<9, 5>;
    using generic_custom_menu_9x6 = detail::_generic_custom_menu<9, 6>;

    template <class Screen, class... Args>
    void open_screen(base_objects::SharedClientData& client, Args... arguments) {
    }

    void close_screen(base_objects::SharedClientData& client);
}
#endif /* SRC_API_SCREENS */
