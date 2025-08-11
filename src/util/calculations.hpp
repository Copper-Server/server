/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#ifndef SRC_UTIL_CALCULATIONS
#define SRC_UTIL_CALCULATIONS
#include <cmath>

namespace copper_server::util {
    extern const double pi;

    template <class T>
    struct XYZ {
        T x;
        T y;
        T z;

        bool operator==(const XYZ& comp) {
            return x == comp.x && y == comp.y && z == comp.z;
        }

        bool operator!=(const XYZ& comp) {
            return x != comp.x || y != comp.y || z != comp.z;
        }

        XYZ& operator+=(const XYZ& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        XYZ& operator-=(const XYZ& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        XYZ& operator*=(const XYZ& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }

        XYZ& operator/=(const XYZ& other) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            return *this;
        }
    };

    typedef XYZ<double> VECTOR;

    template <class T>
    struct XY {
        T x;
        T y;

        bool operator==(const XY& comp) const {
            return x == comp.x && y == comp.y;
        }

        bool operator!=(const XY& comp) const {
            return x != comp.x || y != comp.y;
        }

        XY& operator+=(const XY& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        XY& operator-=(const XY& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        XY& operator*=(const XY& other) {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        XY& operator/=(const XY& other) {
            x /= other.x;
            y /= other.y;
            return *this;
        }
    };

    typedef XY<double> ANGLE;

    struct ANGLE_DEG {
        double x;
        double y;

        bool operator==(const ANGLE_DEG& comp) {
            return x == comp.x && y == comp.y;
        }

        bool operator!=(const ANGLE_DEG& comp) {
            return x != comp.x || y != comp.y;
        }

        ANGLE_DEG& operator+=(const ANGLE_DEG& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        ANGLE_DEG& operator-=(const ANGLE_DEG& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        ANGLE_DEG& operator*=(const ANGLE_DEG& other) {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        ANGLE_DEG& operator/=(const ANGLE_DEG& other) {
            x /= other.x;
            y /= other.y;
            return *this;
        }
    };

    struct ANGLE_RAD {
        double x;
        double y;

        bool operator==(const ANGLE_RAD& comp) {
            return x == comp.x && y == comp.y;
        }

        bool operator!=(const ANGLE_RAD& comp) {
            return x != comp.x || y != comp.y;
        }

        ANGLE_RAD& operator+=(const ANGLE_RAD& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        ANGLE_RAD& operator-=(const ANGLE_RAD& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        ANGLE_RAD& operator*=(const ANGLE_RAD& other) {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        ANGLE_RAD& operator/=(const ANGLE_RAD& other) {
            x /= other.x;
            y /= other.y;
            return *this;
        }
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
    //convert to ANGLE as degrees 180*
    ANGLE_DEG convert(VECTOR val);
    VECTOR dif(VECTOR p0, VECTOR p1);
    VECTOR normalize(VECTOR val);
    VECTOR strength(VECTOR val, double mult);
    VECTOR weak(VECTOR val, double div);

    ANGLE_DEG direction(VECTOR p0, VECTOR p1);


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
    struct hash<copper_server::util::XYZ<T>> {
        size_t operator()(const copper_server::util::XYZ<T>& val) const {
            return hash<T>()(val.x) ^ hash<T>()(val.y) ^ hash<T>()(val.z);
        }
    };

    template <class T>
    struct hash<copper_server::util::XY<T>> {
        size_t operator()(const copper_server::util::XY<T>& val) const {
            return hash<T>()(val.x) ^ hash<T>()(val.y);
        }
    };

    template <>
    struct hash<copper_server::util::ANGLE_DEG> {
        size_t operator()(const copper_server::util::ANGLE_DEG& val) const {
            return hash<double>()(val.x) ^ hash<double>()(val.y);
        }
    };

    template <>
    struct hash<copper_server::util::ANGLE_RAD> {
        size_t operator()(const copper_server::util::ANGLE_RAD& val) const {
            return hash<double>()(val.x) ^ hash<double>()(val.y);
        }
    };
}


#endif /* SRC_UTIL_CALCULATIONS */
