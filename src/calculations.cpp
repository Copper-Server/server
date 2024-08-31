#include "calculations.hpp"
#include <algorithm>
namespace calc {
	const double pi = 3.14159265358979323846;
	VECTOR convert(ANGLE_DEG rot) {
		VECTOR res;
		double x_cos = cos(rot.x);
		res.x = cos(rot.y) * x_cos;
		res.y = sin(rot.y) * x_cos;
		res.z = sin(rot.x);
		return res;
	}
	ANGLE_DEG convert(VECTOR val) {
		double yaw = atan2(val.z, val.x);
		double pitch = atan2(sqrt(val.z * val.z + val.x * val.x), val.y);
		return { yaw, pitch };//xy
	}
	VECTOR dif(VECTOR p0, VECTOR p1) {
		return VECTOR{ p0.x - p1.x, p0.y - p1.y , p0.z - p1.z };
	}
	VECTOR normalize(VECTOR val) {
		return convert(convert(val));
    }

    VECTOR strength(VECTOR val, double mult) {
        val.x *= mult;
		val.y *= mult;
		val.z *= mult;
		return val;
    }

    VECTOR weak(VECTOR val, double div) {
        val.x /= div;
        val.y /= div;
        val.z /= div;
        return val;
    }


    double rad_to_deg180(double val) {
		return (val * pi) / 180;
	}
	double deg_to_rad180(double val) {
		return (val * 180) / pi;
	}

	double rad_to_deg360(double val) {
		return (val * pi*2) / 360;
	}
	double deg_to_rad360(double val) {
		return (val * 360) / pi * 2;
    }

    YAW_PITCH to_yaw_pitch(ANGLE_DEG val) {
        return {rad_to_deg180(val.x), rad_to_deg180(val.y)};
    }

    YAW_PITCH to_yaw_pitch(ANGLE_RAD val) {
        return {rad_to_deg180(val.x), rad_to_deg180(val.y)};
    }

    YAW_PITCH to_yaw_pitch(VECTOR val) {
        return to_yaw_pitch(convert(val));
    }

    YAW_PITCH_256 to_yaw_pitch_256(ANGLE_DEG val) {
        return {(uint8_t)rad_to_deg360(val.x), (uint8_t)rad_to_deg360(val.y)};
    }

    YAW_PITCH_256 to_yaw_pitch_256(ANGLE_RAD val) {
        return {(uint8_t)rad_to_deg360(val.x), (uint8_t)rad_to_deg360(val.y)};
    }

    YAW_PITCH_256 to_yaw_pitch_256(VECTOR val) {
        return to_yaw_pitch_256(convert(val));
    }

    namespace minecraft {
        VECTOR velocity(ANGLE_DEG rot, ANGLE_DEG speed) {
            return strength(convert(rot), speed.y);
        }

        VECTOR velocity(VECTOR pos, VECTOR target, double speed) {
            return strength(normalize(dif(target, pos)), speed);
        }

        namespace packets {
            XYZ<int16_t> velocity(VECTOR rot) {
                return {(int16_t)(rot.x * 8000), (int16_t)(rot.y * 8000), (int16_t)(rot.z * 8000)};
            }

            XYZ<int16_t> delta_move(XYZ<float> pos) {
                int64_t x = (int64_t)pos.x * (4096);
                int64_t y = (int64_t)pos.y * (4096);
                int64_t z = (int64_t)pos.z * (4096);
                return {
                    (int16_t)std::clamp<int64_t>(x, INT16_MIN, INT16_MAX),
                    (int16_t)std::clamp<int64_t>(y, INT16_MIN, INT16_MAX),
                    (int16_t)std::clamp<int64_t>(z, INT16_MIN, INT16_MAX)
                };
            }

            XY<int16_t> delta_move(XY<float> pos) {
                int64_t x = (int64_t)pos.x * (4096);
                int64_t y = (int64_t)pos.y * (4096);
                return {
                    (int16_t)std::clamp<int64_t>(x, INT16_MIN, INT16_MAX),
                    (int16_t)std::clamp<int64_t>(y, INT16_MIN, INT16_MAX)
                };
            }
        }

    }
}