#include "entity.hpp"
#include <Windows.h>
#include "calculations.hpp"
namespace mcCore {

	ENBT* readENBT(std::ifstream& rf) {
		return new ENBT(ENBTHelper::ReadToken(rf));
	}
	ENBT* readENBT_safe(std::ifstream& rf) {
		__try {
			return readENBT(rf);
		}
		__except (GetExceptionCode() == STATUS_STACK_OVERFLOW) {
			return nullptr;
		}
	}

	bool Entity::Save(std::filesystem::path path) {
		ENBT save_nbt(ENBT::Type_ID::Type::darray, 0);
		save_nbt.resize(5);
		save_nbt[0] = id;
		save_nbt[1] = nbt;
		save_nbt[2] = position.x;
		save_nbt[3] = position.y;
		save_nbt[4] = position.z;
		calc::ANGLE rot = convert(rotation);
		save_nbt[5] = rot.x;
		save_nbt[6] = rot.y;
		save_nbt[7] = montion.x;
		save_nbt[8] = montion.y;
		save_nbt[9] = montion.z;
		save_nbt[10] = enitity_id;
		save_nbt[11] = keep_reaction;
		save_nbt[12] = ENBT(std::vector<uint8_t>(std::begin(keep_reaction_data), std::end(keep_reaction_data)));
		save_nbt[13] = world->world_id;
		path += "/" + std::to_string(((uint16_t*)id.data)[0]);
		path += "/" + std::to_string(((uint16_t*)id.data)[1]);
		path += "/" + std::to_string(((uint16_t*)id.data)[2]);
		path += "/" + std::to_string(((uint16_t*)id.data)[3]);
		path += "/" + std::to_string(((uint16_t*)id.data)[4]);
		path += "/" + std::to_string(((uint16_t*)id.data)[5]);
		path += "/" + std::to_string(((uint16_t*)id.data)[6]) + ".enbt";
		uint16_t eid = ((uint16_t*)id.data)[7];
		std::ifstream rf(path);
		ENBT* tmp = readENBT_safe(rf);
		rf.close();
		if (!tmp) return false;
		tmp->getByCompoundKey(eid) = save_nbt;
		std::ofstream wf(path);
		ENBTHelper::WriteToken(wf, *tmp);
		wf.close();
		return true;
	}
	bool Entity::Load(std::filesystem::path path) {

		path += "/" + std::to_string(((uint16_t*)id.data)[0]);
		path += "/" + std::to_string(((uint16_t*)id.data)[1]);
		path += "/" + std::to_string(((uint16_t*)id.data)[2]);
		path += "/" + std::to_string(((uint16_t*)id.data)[3]);
		path += "/" + std::to_string(((uint16_t*)id.data)[4]);
		path += "/" + std::to_string(((uint16_t*)id.data)[5]);
		path += "/" + std::to_string(((uint16_t*)id.data)[6]) + ".enbt";
		uint16_t eid = ((uint16_t*)id.data)[7];
		
		std::ifstream rf(path);
		ENBT* tmp = readENBT_safe(rf);
		rf.close();
		if (!tmp)
			return false;
		ENBT temp = tmp->getByCompoundKey(eid);
		delete tmp;
		id = temp[0];
		nbt = temp[1];
		position.x = temp[2];
		position.y = temp[3];
		position.z = temp[4];
		rotation = convert(calc::ANGLE{ (double)temp[5] ,(double)temp[6] });
		montion.x = temp[7];
		montion.y = temp[8];
		montion.z = temp[9];
		enitity_id = temp[10];
		keep_reaction = temp[11];
		int8_t i = 0;
		for (auto& it : temp[12].operator std::vector<uint8_t>()) {
			if (i >= 40)
				break;
			keep_reaction_data[i++] = it;
		}
		world = WorldClusters::getWorldByID((uint32_t)temp[13]);

		return true;
	}
	bool Entity::Remove(WorldClusters& world, std::filesystem::path path) {
		path += "/" + std::to_string(((uint16_t*)id.data)[0]);
		path += "/" + std::to_string(((uint16_t*)id.data)[1]);
		path += "/" + std::to_string(((uint16_t*)id.data)[2]);
		path += "/" + std::to_string(((uint16_t*)id.data)[3]);
		path += "/" + std::to_string(((uint16_t*)id.data)[4]);
		path += "/" + std::to_string(((uint16_t*)id.data)[5]);
		path += "/" + std::to_string(((uint16_t*)id.data)[6]) + ".enbt";
		uint16_t eid = ((uint16_t*)id.data)[7];
		std::ifstream rf(path);
		ENBT* tmp = readENBT_safe(rf);
		rf.close();
		if (!tmp)return false;
		if (!tmp->removeCompoundKey(eid))
			return false;
		std::ofstream wf(path);
		ENBTHelper::WriteToken(wf, *tmp);
		wf.close();
	}



	void Entity_data::WorkReaction(class Entity& target_entity, class Entity& extern_entity, bool in_view, bool hurted, bool hear) {

	}

	// block
	void Entity_data::WorkReaction(class Entity& target_entity, block_pos_t x, uint16_t y, block_pos_t z, const Block& block, bool in_view, bool hurted, bool hear) {

	}

 


	void Entity_data::KeepReaction(Entity& entity) {

	}
}
