#include "players_data.hpp"
#include <filesystem>

namespace crafted_craft {
    namespace storage {
        static void extract_slot(ENBT& data, base_objects::slot& slot) {
            ENBT* res = data.getOptional();
            if (res == nullptr) {
                slot.reset();
                return;
            }
            base_objects::slot_data slot_data;
            slot_data.id = (*res)["id"];
            slot_data.count = (*res)["count"];
            slot_data.nbt = (*res)["nbt"];
            slot = slot_data;
        }

        static ENBT compact_slot(base_objects::slot& slot) {
            if (slot) {
                enbt::compound compound;

                base_objects::slot_data& slot_data = *slot;
                compound["id"] = slot_data.id;
                compound["count"] = slot_data.count;
                compound["nbt"] = slot_data.nbt;
                return ENBT::optional(std::move(compound));
            }
            return ENBT::optional();
        }

        player_data::player_data(const std::filesystem::path& path)
            : path(path) {}

        void player_data::load() {
            std::ifstream file(path, std::ios::binary);
            if (!file.is_open()) {
                file.close();
                throw std::runtime_error("Failed to open file: " + path.string());
            }
            ENBT file_data = ENBTHelper::ReadToken(file);
            file.close();
            {
                auto abilities = enbt::compound::make_ref(file_data["abilities"]);
                {
                    auto flags = enbt::compound::make_ref(abilities["flags"]);
                    player.abilities.flags.invulnerable = flags["invulnerable"];
                    player.abilities.flags.flying = flags["flying"];
                    player.abilities.flags.allow_flying = flags["allow_flying"];
                    player.abilities.flags.creative_mode = flags["creative_mode"];
                    player.abilities.flags.flying_speed = flags["flying_speed"];
                    player.abilities.flags.walking_speed = flags["walking_speed"];
                }
                player.abilities.flying_speed = abilities["flying_speed"];
                player.abilities.field_of_view_modifier = abilities["field_of_view_modifier"];
            }
            {
                auto position = enbt::compound::make_ref(file_data["position"]);
                player.position.x = position["x"];
                player.position.y = position["y"];
                player.position.z = position["z"];
                player.position.yaw = position["yaw"];
                player.position.pitch = position["pitch"];
            }
            {
                auto inventory = enbt::compound::make_ref(file_data["inventory"]);
                if (inventory.contains("crafting")) {
                    auto crafting = enbt::compound::make_ref(inventory["crafting"]);
                    extract_slot(crafting["output"], player.inventory.crafting_output);
                    for (int i = 0; i < 4; i++) {
                        extract_slot(crafting["input_" + std::to_string(i)], player.inventory.crafting[i]);
                    }
                } else {
                    player.inventory.crafting_output.reset();
                    for (int i = 0; i < 4; i++) {
                        player.inventory.crafting[i].reset();
                    }
                }
                if (inventory.contains("armor")) {
                    auto armor = enbt::compound::make_ref(inventory["armor"]);
                    extract_slot(armor["head"], player.inventory.armor.head);
                    extract_slot(armor["chest"], player.inventory.armor.chest);
                    extract_slot(armor["legs"], player.inventory.armor.legs);
                    extract_slot(armor["feet"], player.inventory.armor.feet);
                } else {
                    player.inventory.armor.head.reset();
                    player.inventory.armor.chest.reset();
                    player.inventory.armor.legs.reset();
                    player.inventory.armor.feet.reset();
                }
                if (inventory.contains("main_inventory")) {
                    auto main_inventory = enbt::compound::make_ref(inventory["main_inventory"]);
                    for (int i = 0; i < 27; i++)
                        extract_slot(main_inventory[std::to_string(i)], player.inventory.main_inventory[i]);
                } else {
                    for (int i = 0; i < 27; i++) {
                        player.inventory.main_inventory[i].reset();
                    }
                }
                if (inventory.contains("hotbar")) {
                    auto hotbar = enbt::compound::make_ref(inventory["hotbar"]);
                    for (int i = 0; i < 9; i++)
                        extract_slot(hotbar[std::to_string(i)], player.inventory.hotbar[i]);
                } else {
                    for (int i = 0; i < 9; i++)
                        player.inventory.hotbar[i].reset();
                }
                if (inventory.contains("offhand"))
                    extract_slot(inventory["offhand"], player.inventory.offhand);
                else
                    player.inventory.offhand.reset();
            }
            {
                auto health = enbt::compound::make_ref(file_data["HUD"]);
                player.health = health["health"];
                player.food = health["food"];
                player.saturation = health["saturation"];
                player.experience = health["experience"];
            }
            {
                auto flags = enbt::compound::make_ref(file_data["flags"]);
                player.is_died = flags["is_died"];
                player.is_sleeping = flags["is_sleeping"];
                player.on_ground = flags["on_ground"];
                player.is_sprinting = flags["is_sprinting"];
                player.is_sneaking = flags["is_sneaking"];
                player.hardcore_hearts = flags["hardcore_hearts"];
                player.reduced_debug_info = flags["reduced_debug_info"];
                player.show_death_screen = flags["show_death_screen"];
            }
            {
                auto gamemode = enbt::compound::make_ref(file_data["gamemode"]);
                player.op_level = gamemode["op_level"];
                player.gamemode = gamemode["gamemode"];
                player.prev_gamemode = gamemode["prev_gamemode"];
            }
            if (file_data.contains("death_location")) {
                auto death_location = enbt::compound::make_ref(file_data["death_location"]);
                player.last_death_location = base_objects::player::DeathLocation{
                    death_location["x"],
                    death_location["y"],
                    death_location["z"],
                    death_location["world_id"]
                };
            } else
                player.last_death_location.reset();
            if (file_data.contains("ride_entity_id"))
                player.ride_entity_id = (ENBT::UUID)file_data["ride_entity_id"];
            else
                player.ride_entity_id.reset();

            if (file_data.contains("additional_data"))
                player.additional_data = ENBT::optional(file_data["additional_data"]);
            else
                player.additional_data = ENBT::optional();
            if (file_data.contains("local_data"))
                player.local_data = ENBT::optional(file_data["local_data"]);

            else
                player.local_data = ENBT::optional();
        }

        void player_data::save() {
            enbt::compound as_file_data;
            {
                enbt::compound abilities;
                {
                    enbt::compound flags;
                    flags["invulnerable"] = player.abilities.flags.invulnerable;
                    flags["flying"] = player.abilities.flags.flying;
                    flags["allow_flying"] = player.abilities.flags.allow_flying;
                    flags["creative_mode"] = player.abilities.flags.creative_mode;
                    flags["flying_speed"] = player.abilities.flags.flying_speed;
                    flags["walking_speed"] = player.abilities.flags.walking_speed;
                    abilities["flags"] = flags;
                }
                abilities["flying_speed"] = player.abilities.flying_speed;
                abilities["field_of_view_modifier"] = player.abilities.field_of_view_modifier;
                as_file_data["abilities"] = abilities;
            }
            {
                enbt::compound position;
                position["x"] = player.position.x;
                position["y"] = player.position.y;
                position["z"] = player.position.z;
                position["yaw"] = player.position.yaw;
                position["pitch"] = player.position.pitch;
                as_file_data["position"] = position;
            }
            {
                enbt::compound inventory;
                if (player.inventory.crafting_output.has_value() || player.inventory.crafting[0].has_value() || player.inventory.crafting[1].has_value() || player.inventory.crafting[2].has_value() || player.inventory.crafting[3].has_value()) {
                    enbt::compound crafting;
                    crafting["output"] = compact_slot(player.inventory.crafting_output);
                    for (int i = 0; i < 4; i++)
                        crafting["input_" + std::to_string(i)] = compact_slot(player.inventory.crafting[i]);

                    inventory["crafting"] = crafting;
                }
                {
                    enbt::compound armor;
                    armor["head"] = compact_slot(player.inventory.armor.head);
                    armor["chest"] = compact_slot(player.inventory.armor.chest);
                    armor["legs"] = compact_slot(player.inventory.armor.legs);
                    armor["feet"] = compact_slot(player.inventory.armor.feet);
                    inventory["armor"] = armor;
                }
                {
                    enbt::compound main_inventory;
                    for (int i = 0; i < 27; i++)
                        main_inventory[std::to_string(i)] = compact_slot(player.inventory.main_inventory[i]);
                    inventory["main_inventory"] = main_inventory;
                }
                {
                    enbt::compound hotbar;
                    for (int i = 0; i < 9; i++)
                        hotbar[std::to_string(i)] = compact_slot(player.inventory.hotbar[i]);
                    inventory["hotbar"] = hotbar;
                }
                inventory["offhand"] = compact_slot(player.inventory.offhand);
                as_file_data["inventory"] = inventory;
            }
            {
                enbt::compound health;
                health["health"] = player.health;
                health["food"] = player.food;
                health["saturation"] = player.saturation;
                health["experience"] = player.experience;
                as_file_data["HUD"] = health;
            }
            {
                enbt::compound flags;
                flags["is_died"] = player.is_died;
                flags["is_sleeping"] = player.is_sleeping;
                flags["on_ground"] = player.on_ground;
                flags["is_sprinting"] = player.is_sprinting;
                flags["is_sneaking"] = player.is_sneaking;
                flags["hardcore_hearts"] = player.hardcore_hearts;
                flags["reduced_debug_info"] = player.reduced_debug_info;
                flags["show_death_screen"] = player.show_death_screen;
                as_file_data["flags"] = flags;
            }
            {
                enbt::compound gamemode;
                gamemode["op_level"] = player.op_level;
                gamemode["gamemode"] = player.gamemode;
                gamemode["prev_gamemode"] = player.prev_gamemode;
                as_file_data["gamemode"] = gamemode;
            }
            if (player.last_death_location.has_value()) {
                enbt::compound death_location;
                death_location["x"] = player.last_death_location->x;
                death_location["y"] = player.last_death_location->y;
                death_location["z"] = player.last_death_location->z;
                death_location["world_id"] = player.last_death_location->world_id;
                as_file_data["death_location"] = death_location;
            }
            if (player.ride_entity_id.has_value())
                as_file_data["ride_entity_id"] = (ENBT::UUID)player.ride_entity_id.value();

            if (!player.additional_data.type_equal(ENBT::Type::none))
                as_file_data["additional_data"] = player.additional_data;

            if (!player.local_data.type_equal(ENBT::Type::none))
                as_file_data["local_data"] = player.local_data;
            std::ofstream file(path, std::ios::binary);
            if (!file.is_open()) {
                file.close();
                throw std::runtime_error("Failed to open file: " + path.string());
            }
            ENBTHelper::WriteToken(file, as_file_data);
            file.flush();
            file.close();
        }

        players_data::players_data(const std::filesystem::path& base_path)
            : base_path(base_path) {
            std::filesystem::create_directories(base_path);
        }

        player_data players_data::get_player_data(const std::string& player_uuid) {
            return player_data(base_path / (player_uuid + ".e_dat"));
        }

    } // namespace storage


} // namespace crafted_craft
