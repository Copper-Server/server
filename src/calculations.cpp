#include "calculations.hpp"
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
}