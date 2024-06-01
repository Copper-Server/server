#include "entity.hpp"
#include "../calculations.hpp"
#include <Windows.h>

namespace crafted_craft {
    namespace base_objects {
        void entity_data::work_reaction(class entity& target_entity, class entity& extern_entity, bool in_view, bool hurted, bool hear) {
        }

        // block
        void entity_data::work_reaction(class entity& target_entity, int64_t x, uint64_t y, int64_t z, const base_objects::block& block, bool in_view, bool hurted, bool hear) {
        }

        void entity_data::work_reaction(entity& target_entity) {
        }

        void entity_data::keep_reaction(entity& entity) {
        }

        entity_ref entity::copy() const {
            entity_ref res = new entity();
            res->data = data;
            res->died = died;
            res->entity_id = entity_id;
            res->head_rotation = head_rotation;
            res->id = id;
            res->keep_reaction = keep_reaction;
            std::copy(keep_reaction_data, keep_reaction_data + 16, res->keep_reaction_data);
            res->motion = motion;
            res->nbt = nbt;
            res->position = position;
            res->rotation = rotation;
            return res;
        }

        ENBT entity::copy_to_enbt() const {
            enbt::compound res;
            res["data"] = data;
            res["died"] = died;
            res["entity_id"] = entity_id;
            res["head_rotation"] = enbt::fixed_array({head_rotation.x, head_rotation.y, head_rotation.z});
            res["id"] = id;
            res["keep_reaction"] = keep_reaction;
            res["keep_reaction_data"] = enbt::simple_array_ui8(keep_reaction_data);
            res["motion"] = enbt::fixed_array({motion.x, motion.y, motion.z});
            res["nbt"] = nbt;
            res["position"] = enbt::fixed_array({position.x, position.y, position.z});
            res["rotation"] = enbt::fixed_array({rotation.x, rotation.y, rotation.z});
            return res;
        }

        void entity::tick() {
        }

        void entity::kill() {
            died = true;
        }

        entity_ref entity::load_from_enbt(const ENBT& nbt) {
            const auto compound = enbt::compound::make_ref(nbt);
            entity_ref res = new entity();
            res->data = compound["data"];
            res->died = compound["died"];
            res->entity_id = compound["entity_id"];
            auto head_rotation = enbt::fixed_array::make_ref(compound["head_rotation"]);
            res->head_rotation = {head_rotation[0], head_rotation[1], head_rotation[2]};
            res->id = compound["id"];
            res->keep_reaction = compound["keep_reaction"];
            auto keep_reaction_data = enbt::simple_array_ui8::make_ref(compound["keep_reaction_data"]);
            std::copy(keep_reaction_data.begin(), keep_reaction_data.end(), res->keep_reaction_data);
            auto motion = enbt::fixed_array::make_ref(compound["motion"]);
            res->motion = {motion[0], motion[1], motion[2]};
            res->nbt = compound["nbt"];
            auto position = enbt::fixed_array::make_ref(compound["position"]);
            res->position = {position[0], position[1], position[2]};
            auto rotation = enbt::fixed_array::make_ref(compound["rotation"]);
            res->rotation = {rotation[0], rotation[1], rotation[2]};
            return res;
        }
    }
}
