#pragma once
#include <cmath>
namespace calc {
	extern const double pi;
	template<class T>
	struct XYZ {
		T x;
		T y;
		T z;
	};
	typedef XYZ<double> VECTOR;
	template<class T>
	struct XY {
		T x;
		T y;
	};
    typedef XY<double> ANGLE;

    struct ANGLE_DEG {
        double x;
        double y;
    };

    struct ANGLE_RAD {
        double x;
        double y;
    };

    typedef XY<double> YAW_PITCH;
    typedef XY<uint8_t> YAW_PITCH_256;

    template <class T>
    inline void moved(XYZ<T>& pos, ANGLE rot, T distance) {
        T x_cos = cos(rot.x);
        pos.x += cos(rot.y) * x_cos * distance;
        pos.y += sin(rot.y) * x_cos * distance;
        pos.z += sin(rot.x) * distance;
    }

    template <class T>
    inline XYZ<T> moved(ANGLE rot, T distance) {
        XYZ<T> res;
        T x_cos = cos(rot.x);
        res.x = cos(rot.y) * x_cos * distance;
        res.y = sin(rot.y) * x_cos * distance;
        res.z = sin(rot.x) * distance;
		return res;
    }

    VECTOR convert(ANGLE_DEG rot);
    //convert to ANGLE as degres 180*
    ANGLE_DEG convert(VECTOR val);
    VECTOR dif(VECTOR p0, VECTOR p1);
    VECTOR normalize(VECTOR val);
    VECTOR strength(VECTOR val, double mult);
    VECTOR weak(VECTOR val, double div);


    double rad_to_deg180(double val);
	double deg_to_rad180(double val);

	double rad_to_deg360(double val);
	double deg_to_rad360(double val);


    YAW_PITCH to_yaw_pitch(ANGLE_DEG val);
    YAW_PITCH to_yaw_pitch(ANGLE_RAD val);
    YAW_PITCH to_yaw_pitch(VECTOR val);
    YAW_PITCH_256 to_yaw_pitch_256(ANGLE_DEG val);
    YAW_PITCH_256 to_yaw_pitch_256(ANGLE_RAD val);
    YAW_PITCH_256 to_yaw_pitch_256(VECTOR val);

    namespace minecraft {
        VECTOR velocity(ANGLE_DEG rot, ANGLE_DEG speed);
        VECTOR velocity(VECTOR pos, VECTOR target, double speed);

        namespace packets {
            XYZ<int16_t> velocity(VECTOR rot);
            XYZ<int16_t> delta_move(XYZ<float> pos);
            XY<int16_t> delta_move(XY<float> pos);
        }
    }
}

namespace std {
    template <class T>
    struct hash<calc::XYZ<T>> {
        size_t operator()(const calc::XYZ<T>& val) const {
            return hash<T>()(val.x) ^ hash<T>()(val.y) ^ hash<T>()(val.z);
        }
    };

    template <class T>
    struct hash<calc::XY<T>> {
        size_t operator()(const calc::XY<T>& val) const {
            return hash<T>()(val.x) ^ hash<T>()(val.y);
        }
    };

    template <>
    struct hash<calc::ANGLE_DEG> {
        size_t operator()(const calc::ANGLE_DEG& val) const {
            return hash<double>()(val.x) ^ hash<double>()(val.y);
        }
    };

    template <>
    struct hash<calc::ANGLE_RAD> {
        size_t operator()(const calc::ANGLE_RAD& val) const {
            return hash<double>()(val.x) ^ hash<double>()(val.y);
        }
    };
}