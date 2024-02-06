#pragma once
#include <list>
#include <unordered_map>

namespace mcCore {
	struct HitBox {
		struct Hitbox {
			struct Position {
				float x, y, z;
			};
			Position p0, p1;
		};
		std::list<Hitbox> boxes;
		HitBox(){}
		HitBox(std::list<Hitbox> init) : boxes(init){}

		bool in(float x, float y, float z) const {
			for (const auto& it : boxes)
				if (it.p0.x <= x && it.p0.y <= y && it.p0.z <= z)
					if (it.p1.x >= x && it.p1.y >= y && it.p1.z >= z)
						return true;
			return false;
		}
		bool out(float x, float y, float z) const {
			return !in(x, y, z);
		}


		static short AddHitBox(std::list<Hitbox> init) {
			hitBoxes[hitboxes_adder++] = init;
		}

		static const HitBox& getHitBox(short id) {
			return hitBoxes[id];
		}

	private:
		static short hitboxes_adder;
		static std::unordered_map<short, HitBox> hitBoxes;
	};
}