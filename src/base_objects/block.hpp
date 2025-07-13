#pragma once
#include <cstdint>
#include <library/enbt/enbt.hpp>
#include <library/list_array.hpp>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#pragma pack(push)
#pragma pack(1)

namespace copper_server {
    namespace storage {
        class world_data;
    }

    namespace base_objects {
        namespace world {
            struct sub_chunk_data;
        }
        union block;
        typedef uint16_t block_id_t;

        struct shape_data {
            double min_x, min_y, min_z;
            double max_x, max_y, max_z;
        };

        class static_block_data {
            struct block_state_hash {
                size_t operator()(const std::unordered_map<std::string, std::string>& value) const noexcept {
                    size_t result = 0;
                    std::hash<std::string> string_hasher;
                    for (auto& [key, value] : value) {
                        result ^= string_hasher(key) & string_hasher(value);
                    }
                    return result;
                }
            };

        public:
            std::shared_ptr<enbt::compound> loot_table;
            std::vector<shape_data*> collision_shapes;
            std::string instrument;
            std::string piston_behavior;
            std::string name;
            std::string translation_key;
            float slipperiness = 0;
            float velocity_multiplier = 0;
            float jump_velocity_multiplier = 0;
            float hardness = 0;
            float blast_resistance = 0;
            int32_t map_color_rgb = 0;
            int32_t block_entity_id = 0; //used only when is_block_entity == true, like interact and storage
            int32_t default_drop_item_id = 0;
            int32_t experience = 0;
            block_id_t general_block_id = 0; //does not represent state
            block_id_t default_state = 0;
            block_id_t current_state = 0;
            uint8_t luminance = 0;
            uint8_t opacity = 0; //255 not opaque
            bool is_air : 1 = true;
            bool is_solid : 1 = false;
            bool is_liquid : 1 = false;
            bool is_burnable : 1 = false;
            bool is_emits_redstone : 1 = false;
            bool is_full_cube : 1 = false;
            bool is_tool_required : 1 = false;
            bool is_sided_transparency : 1 = false;
            bool is_replaceable : 1 = false;
            bool is_block_entity : 1 = false;
            bool is_default_state : 1 = false;

            bool can_explode(float explode_strength) const {
                return blast_resistance < explode_strength;
            }

            bool can_break(float break_strength) const { //use when in client side player break, server need calculate break tick long
                return hardness < break_strength;
            }

            //on tick first checks `is_block_entity` and if true, checks `as_entity_on_tick` if one of them false/undefined then checks `on_tick`, if undefined then do nothing
            std::function<void(storage::world_data&, world::sub_chunk_data&, block& data, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked)> on_tick;
            std::function<void(storage::world_data&, world::sub_chunk_data&, block& data, enbt::value& extended_data, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked)> as_entity_on_tick;

            //used to check properties usage
            std::vector<int32_t> allowed_properties;


            using map_of_states = boost::bimaps::bimap<
                boost::bimaps::unordered_set_of<block_id_t, std::hash<block_id_t>>,
                boost::bimaps::unordered_set_of<std::unordered_map<std::string, std::string>, block_state_hash>>;
            std::shared_ptr<map_of_states> assigned_states_to_properties;
            std::unordered_map<std::string, std::string> current_properties;

            list_array<std::string> block_aliases; //string block ids(checks from first to last, if none found in `initialize_blocks()` throws) implicitly uses id first


            enum class tick_opt : uint16_t {
                undefined,
                block_tickable,
                entity_tickable,
                no_tick
            } tickable;

            bool is_tickable() const {
                switch (tickable) {
                case tick_opt::block_tickable:
                case tick_opt::entity_tickable:
                    return true;
                case tick_opt::undefined:
                    if (on_tick)
                        return true;
                    else if (as_entity_on_tick)
                        return true;
                    else
                        return false;
                default:
                case tick_opt::no_tick:
                    return false;
                }
            }

            bool is_tickable() {
                switch (tickable) {
                case tick_opt::block_tickable:
                case tick_opt::entity_tickable:
                    return true;
                case tick_opt::undefined:
                    tickable = resolve_tickable();
                    return tickable != tick_opt::no_tick;
                default:
                case tick_opt::no_tick:
                    return false;
                }
            }

            tick_opt resolve_tickable() const {
                if (on_tick)
                    return tick_opt::block_tickable;
                if (as_entity_on_tick)
                    return tick_opt::entity_tickable;
                return tick_opt::no_tick;
            }

            tick_opt get_tickable() {
                if (tickable == tick_opt::undefined)
                    tickable = resolve_tickable();
                return tickable;
            }

            static_block_data() {}

            static_block_data(const static_block_data& copy)
                : loot_table(copy.loot_table),
                  collision_shapes(copy.collision_shapes),
                  instrument(copy.instrument),
                  piston_behavior(copy.piston_behavior),
                  name(copy.name),
                  translation_key(copy.translation_key),
                  slipperiness(copy.slipperiness),
                  velocity_multiplier(copy.velocity_multiplier),
                  jump_velocity_multiplier(copy.jump_velocity_multiplier),
                  hardness(copy.hardness),
                  blast_resistance(copy.blast_resistance),
                  map_color_rgb(copy.map_color_rgb),
                  block_entity_id(copy.block_entity_id),
                  default_drop_item_id(copy.default_drop_item_id),
                  experience(copy.experience),
                  default_state(copy.default_state),
                  luminance(copy.luminance),
                  opacity(copy.opacity),
                  is_air(copy.is_air),
                  is_solid(copy.is_solid),
                  is_liquid(copy.is_liquid),
                  is_burnable(copy.is_burnable),
                  is_emits_redstone(copy.is_emits_redstone),
                  is_full_cube(copy.is_full_cube),
                  is_tool_required(copy.is_tool_required),
                  is_sided_transparency(copy.is_sided_transparency),
                  is_replaceable(copy.is_replaceable),
                  is_block_entity(copy.is_block_entity),
                  is_default_state(copy.is_default_state),
                  on_tick(copy.on_tick),
                  as_entity_on_tick(copy.as_entity_on_tick),
                  allowed_properties(copy.allowed_properties),
                  assigned_states_to_properties(copy.assigned_states_to_properties),
                  current_properties(copy.current_properties),
                  block_aliases(copy.block_aliases) {}

            //USED ONLY DURING FULL SERVER RELOAD!  DO NOT ALLOW CALL FROM THE USER CODE
            static void reset_blocks(); //INTERNAL

            static list_array<shape_data> all_shapes;
            static list_array<std::string> block_entity_types;
            static std::unordered_map<int32_t, std::unordered_set<std::string>> all_properties;


            static boost::bimaps::bimap<
                boost::bimaps::unordered_set_of<int32_t, std::hash<int32_t>>,
                boost::bimaps::unordered_set_of<std::string, std::hash<std::string>>>
                assigned_property_name;

            inline static const std::unordered_set<std::string>& get_allowed_property_values(int32_t property_id) {
                static std::unordered_set<std::string> local;
                auto it = all_properties.find(property_id);
                if (it != all_properties.end())
                    return it->second;
                else
                    return local;
            }

            inline static const std::unordered_set<std::string>& get_allowed_property_values(const std::string& property_id) {
                static std::unordered_set<std::string> local;
                auto it = all_properties.find(assigned_property_name.right.at(property_id));
                if (it != all_properties.end())
                    return it->second;
                else
                    return local;
            }
        };

        union block {
            using tick_opt = static_block_data::tick_opt;

            static void initialize();

            static block_id_t addNewStatelessBlock(static_block_data&& new_block) {
                if (named_full_block_data.contains(new_block.name))
                    throw std::runtime_error("Block with " + new_block.name + " name already defined.");

                struct {
                    block_id_t id : 15;
                } bound_check;

                bound_check.id = full_block_data_.size();
                if (size_t(bound_check.id) != full_block_data_.size())
                    throw std::out_of_range("Blocks count out of range, block can't added");
                auto block_ = std::make_shared<static_block_data>(std::move(new_block));
                named_full_block_data[block_->name] = block_;
                full_block_data_.emplace_back(block_);
                return bound_check.id;
            }

            static void access_full_block_data(std::function<void(list_array<std::shared_ptr<static_block_data>>&, std::unordered_map<std::string, std::shared_ptr<static_block_data>>&)> access) {
                access(full_block_data_, named_full_block_data);
            }

            static block_id_t addNewStatelessBlock(const static_block_data& new_block) {
                return addNewStatelessBlock(static_block_data(new_block));
            }

            struct {
                base_objects::block_id_t id : 15;
                uint16_t block_state_data : 15;
                tick_opt tickable : 2;
            };

            uint32_t raw;

            block(block_id_t block_id = 0, uint16_t block_state_data = 0)
                : id(block_id), block_state_data(block_state_data), tickable(tick_opt::undefined) {}

            block(const block& copy) {
                operator=(copy);
            }

            block(block&& move_me) noexcept {
                operator=(std::move(move_me));
            }

            block& operator=(const block& block) {
                id = block.id;
                return *this;
            }

            block& operator=(block&& block) noexcept {
                id = block.id;
                return *this;
            }

            const static_block_data& getStaticData() const {
                return *full_block_data_.at(id);
            }

            static const static_block_data& getStaticData(block_id_t id) {
                return *full_block_data_.at(id);
            }

            bool operator==(const block& b) const {
                return id == b.id;
            }

            bool operator!=(const block& b) const {
                return id != b.id;
            }

            void tick(storage::world_data&, base_objects::world::sub_chunk_data& sub_chunk, int64_t chunk_x, uint64_t sub_chunk_y, int64_t chunk_z, uint8_t local_x, uint8_t local_y, uint8_t local_z, bool random_ticked);

            static tick_opt resolve_tickable(base_objects::block_id_t block_id);
            bool is_tickable();
            bool is_tickable() const;
            bool is_solid() const;
            const std::vector<shape_data*>& collision_shapes() const;
            const std::string& instrument() const;
            const std::string& piston_behavior() const;
            const std::string& name() const;
            const std::string& translation_key() const;
            block_id_t general_block_id() const;
            float slipperiness() const;
            float velocity_multiplier() const;
            float jump_velocity_multiplier() const;
            float hardness() const;
            float blast_resistance() const;
            int32_t map_color_rgb() const;
            int32_t block_entity_id() const;
            int32_t default_drop_item_id() const;
            int32_t experience() const;
            block_id_t default_state() const;
            uint8_t luminance() const;
            uint8_t opacity() const;
            bool is_air() const;
            bool is_liquid() const;
            bool is_burnable() const;
            bool is_emits_redstone() const;
            bool is_full_cube() const;
            bool is_tool_required() const;
            bool is_sided_transparency() const;
            bool is_replaceable() const;
            bool is_block_entity() const;


            static static_block_data& get_block(const std::string& name) {
                return *named_full_block_data.at(name);
            }

            static static_block_data& get_block(block_id_t block_state_id) {
                return *full_block_data_.at(block_state_id);
            }

            static static_block_data& get_general_block(block_id_t general_id) {
                return *general_block_data_.at(general_id);
            }

            static block make_block(const std::string& name){
                return block(get_block(name).default_state);
            }
            static block make_block(block_id_t id){
                return block(id);
            }

            static size_t block_states_size();

        private:
            static std::unordered_map<std::string, std::shared_ptr<static_block_data>> named_full_block_data;
            static list_array<std::shared_ptr<static_block_data>> full_block_data_;
            static list_array<std::shared_ptr<static_block_data>> general_block_data_;
        };

        // clang-format off
        struct block_entity {
            block block;
            enbt::value data;
            bool is_tickable() const { return block.is_tickable();}
            bool is_solid() const { return block.is_solid();}
            const std::vector<shape_data*>& collision_shapes() const { return block.collision_shapes();}
            const std::string& instrument() const { return block.instrument();}
            const std::string& piston_behavior() const { return block.piston_behavior();}
            const std::string& name() const { return block.name();}
            const std::string& translation_key() const { return block.translation_key();}
            block_id_t general_block_id() const { return block.general_block_id();}
            float slipperiness() const { return block.slipperiness();}
            float velocity_multiplier() const { return block.velocity_multiplier();}
            float jump_velocity_multiplier() const { return block.jump_velocity_multiplier();}
            float hardness() const { return block.hardness();}
            float blast_resistance() const { return block.blast_resistance();}
            int32_t map_color_rgb() const { return block.map_color_rgb();}
            int32_t block_entity_id() const { return block.block_entity_id();}
            int32_t default_drop_item_id() const { return block.default_drop_item_id();}
            int32_t experience() const { return block.experience();}
            block_id_t default_state() const { return block.default_state();}
            uint8_t luminance() const { return block.luminance();}
            uint8_t opacity() const { return block.opacity();}
            bool is_air() const { return block.is_air();}
            bool is_liquid() const { return block.is_liquid();}
            bool is_burnable() const { return block.is_burnable();}
            bool is_emits_redstone() const { return block.is_emits_redstone();}
            bool is_full_cube() const { return block.is_full_cube();}
            bool is_tool_required() const { return block.is_tool_required();}
            bool is_sided_transparency() const { return block.is_sided_transparency();}
            bool is_replaceable() const { return block.is_replaceable();}
            bool is_block_entity() const { return block.is_block_entity();}
        };

        struct block_entity_ref {
            block& block;
            enbt::value& data;

            block_entity_ref(block_entity& ref) : block(ref.block), data(ref.data) {}

            block_entity_ref(base_objects::block& block, enbt::value& data) : block(block), data(data) {}
            bool is_tickable() const { return block.is_tickable();}
            bool is_solid() const { return block.is_solid();}
            const std::vector<shape_data*>& collision_shapes() const { return block.collision_shapes();}
            const std::string& instrument() const { return block.instrument();}
            const std::string& piston_behavior() const { return block.piston_behavior();}
            const std::string& name() const { return block.name();}
            const std::string& translation_key() const { return block.translation_key();}
            block_id_t general_block_id() const { return block.general_block_id();}
            float slipperiness() const { return block.slipperiness();}
            float velocity_multiplier() const { return block.velocity_multiplier();}
            float jump_velocity_multiplier() const { return block.jump_velocity_multiplier();}
            float hardness() const { return block.hardness();}
            float blast_resistance() const { return block.blast_resistance();}
            int32_t map_color_rgb() const { return block.map_color_rgb();}
            int32_t block_entity_id() const { return block.block_entity_id();}
            int32_t default_drop_item_id() const { return block.default_drop_item_id();}
            int32_t experience() const { return block.experience();}
            block_id_t default_state() const { return block.default_state();}
            uint8_t luminance() const { return block.luminance();}
            uint8_t opacity() const { return block.opacity();}
            bool is_air() const { return block.is_air();}
            bool is_liquid() const { return block.is_liquid();}
            bool is_burnable() const { return block.is_burnable();}
            bool is_emits_redstone() const { return block.is_emits_redstone();}
            bool is_full_cube() const { return block.is_full_cube();}
            bool is_tool_required() const { return block.is_tool_required();}
            bool is_sided_transparency() const { return block.is_sided_transparency();}
            bool is_replaceable() const { return block.is_replaceable();}
            bool is_block_entity() const { return block.is_block_entity();}
        };

        struct const_block_entity_ref {
            const block& block;
            const enbt::value& data;

            const_block_entity_ref(const block_entity& ref) : block(ref.block), data(ref.data) {}

            const_block_entity_ref(const block_entity_ref& ref) : block(ref.block), data(ref.data) {}

            const_block_entity_ref(const base_objects::block& block, const enbt::value& data) : block(block), data(data) {}

            bool is_tickable() const { return block.is_tickable();}
            bool is_solid() const { return block.is_solid();}
            const std::vector<shape_data*>& collision_shapes() const { return block.collision_shapes();}
            const std::string& instrument() const { return block.instrument();}
            const std::string& piston_behavior() const { return block.piston_behavior();}
            const std::string& name() const { return block.name();}
            const std::string& translation_key() const { return block.translation_key();}
            block_id_t general_block_id() const { return block.general_block_id();}
            float slipperiness() const { return block.slipperiness();}
            float velocity_multiplier() const { return block.velocity_multiplier();}
            float jump_velocity_multiplier() const { return block.jump_velocity_multiplier();}
            float hardness() const { return block.hardness();}
            float blast_resistance() const { return block.blast_resistance();}
            int32_t map_color_rgb() const { return block.map_color_rgb();}
            int32_t block_entity_id() const { return block.block_entity_id();}
            int32_t default_drop_item_id() const { return block.default_drop_item_id();}
            int32_t experience() const { return block.experience();}
            block_id_t default_state() const { return block.default_state();}
            uint8_t luminance() const { return block.luminance();}
            uint8_t opacity() const { return block.opacity();}
            bool is_air() const { return block.is_air();}
            bool is_liquid() const { return block.is_liquid();}
            bool is_burnable() const { return block.is_burnable();}
            bool is_emits_redstone() const { return block.is_emits_redstone();}
            bool is_full_cube() const { return block.is_full_cube();}
            bool is_tool_required() const { return block.is_tool_required();}
            bool is_sided_transparency() const { return block.is_sided_transparency();}
            bool is_replaceable() const { return block.is_replaceable();}
            bool is_block_entity() const { return block.is_block_entity();}
        };

        // clang-format on

        using full_block_data = std::variant<block, block_entity>;
        using full_block_data_ref = std::variant<std::reference_wrapper<block>, block_entity_ref>;


        struct local_block_pos {
            uint8_t x : 4;
            uint8_t y : 4;
            uint8_t z : 4;
        };

        struct chunk_block_pos {
            uint8_t x : 4;
            uint32_t y : 21;
            uint8_t z : 4;
        };

        union compressed_block_state {
            struct {
                uint64_t blockStateId : 52;
                uint64_t blockLocalX : 4;
                uint64_t blockLocalZ : 4;
                uint64_t blockLocalY : 4;
            };

            uint64_t value;
        };

        struct block_hash {
            std::size_t operator()(const block& k) const {
                using std::hash;
                return hash<block_id_t>()(k.id);
            }
        };
    }
}

#pragma pack(pop)
