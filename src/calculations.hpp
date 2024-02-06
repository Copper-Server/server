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
	typedef XY<double> ANGLE_DEG;
	typedef XY<double> ANGLE_RAD;



	template<class T>
	inline void moved(XYZ<T>& pos, ANGLE rot, T distance) {
		T x_cos = cos(rot.x);
		pos.x += cos(rot.y) * x_cos * distance;
		pos.y += sin(rot.y) * x_cos * distance;
		pos.z += sin(rot.x) * distance;
	}
	template<class T>
	inline XYZ<T> moved(ANGLE rot, T distance) {
		XYZ<T>res;
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


    double rad_to_deg180(double val);
	double deg_to_rad180(double val);

	double rad_to_deg360(double val);
	double deg_to_rad360(double val);



}