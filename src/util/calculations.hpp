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
#include <cstdint>

namespace copper_server::util {
    extern const double pi;

    template <class T>
    struct XYZW {
        T x;
        T y;
        T z;
        T w;

        bool operator==(const XYZW& comp) {
            return x == comp.x && y == comp.y && z == comp.z && w == comp.w;
        }

        bool operator!=(const XYZW& comp) {
            return x != comp.x || y != comp.y || z != comp.z || w != comp.w;
        }

        XYZW& operator+=(const XYZW& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        XYZW& operator-=(const XYZW& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        XYZW& operator*=(const XYZW& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            w *= other.w;
            return *this;
        }

        XYZW& operator/=(const XYZW& other) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            w /= other.w;
            return *this;
        }
    };


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

    struct ANGLE_DEG {
        double pitch;
        double yaw;

        bool operator==(const ANGLE_DEG& comp) {
            return pitch == comp.pitch && yaw == comp.yaw;
        }

        bool operator!=(const ANGLE_DEG& comp) {
            return pitch != comp.pitch || yaw != comp.yaw;
        }

        ANGLE_DEG& operator+=(const ANGLE_DEG& other) {
            pitch += other.pitch;
            yaw += other.yaw;
            return *this;
        }

        ANGLE_DEG& operator-=(const ANGLE_DEG& other) {
            pitch -= other.pitch;
            yaw -= other.yaw;
            return *this;
        }

        ANGLE_DEG& operator*=(const ANGLE_DEG& other) {
            pitch *= other.pitch;
            yaw *= other.yaw;
            return *this;
        }

        ANGLE_DEG& operator/=(const ANGLE_DEG& other) {
            pitch /= other.pitch;
            yaw /= other.yaw;
            return *this;
        }
    };

    struct ANGLE_RAD {
        double pitch;
        double yaw;

        bool operator==(ANGLE_RAD comp) {
            return pitch == comp.pitch && yaw == comp.yaw;
        }

        bool operator!=(ANGLE_RAD comp) {
            return pitch != comp.pitch || yaw != comp.yaw;
        }

        ANGLE_RAD& operator+=(ANGLE_RAD other) {
            pitch += other.pitch;
            yaw += other.yaw;
            return *this;
        }

        ANGLE_RAD& operator-=(ANGLE_RAD other) {
            pitch -= other.pitch;
            yaw -= other.yaw;
            return *this;
        }

        ANGLE_RAD& operator*=(ANGLE_RAD other) {
            pitch *= other.pitch;
            yaw *= other.yaw;
            return *this;
        }

        ANGLE_RAD& operator/=(ANGLE_RAD other) {
            pitch /= other.pitch;
            yaw /= other.yaw;
            return *this;
        }
    };

    typedef XY<uint8_t> YAW_PITCH_256;

    constexpr double DEG_TO_RAD = 3.14159265358979323846 / 180.0;

    inline double sind(double degrees) {
        return std::sin(degrees * DEG_TO_RAD);
    }

    inline double cosd(double degrees) {
        return std::cos(degrees * DEG_TO_RAD);
    }

    inline double tand(double degrees) {
        return std::tan(degrees * DEG_TO_RAD);
    }

    template <class T>
    inline XYZ<T> moved(ANGLE_RAD rot, T distance) {
        T cos_pitch = cos(rot.pitch);
        XYZ<T> offset;
        offset.x = static_cast<T>(cos(rot.yaw) * cos_pitch * distance);
        offset.y = static_cast<T>(sin(rot.yaw) * cos_pitch * distance);
        offset.z = static_cast<T>(sin(rot.pitch) * distance);
        return offset;
    }

    template <class T>
    inline XYZ<T> moved(const XYZ<T>& startPosition, const ANGLE_RAD& rot, T distance) {
        XYZ<T> finalPosition = startPosition;
        finalPosition += moved(rot, distance);
        return finalPosition;
    }

    template <class T>
    inline XYZ<T> moved(ANGLE_DEG rot, T distance) {
        T cos_pitch = cosd(rot.pitch);
        XYZ<T> offset;
        offset.x = static_cast<T>(cosd(rot.yaw) * cos_pitch * distance);
        offset.y = static_cast<T>(sind(rot.yaw) * cos_pitch * distance);
        offset.z = static_cast<T>(sind(rot.pitch) * distance);
        return offset;
    }

    template <class T>
    inline XYZ<T> moved(const XYZ<T>& startPosition, const ANGLE_DEG& rot, T distance) {
        XYZ<T> finalPosition = startPosition;
        finalPosition += moved(rot, distance);
        return finalPosition;
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

    inline double distance_sq(const VECTOR& p0, const VECTOR& p1) {
        return (p0.x - p1.x) * (p0.x - p1.x) + (p0.y - p1.y) * (p0.y - p1.y) + (p0.z - p1.z) * (p0.z - p1.z);
    }

    inline double distance(const VECTOR& p0, const VECTOR& p1) { //recommended to use squared distance
        return std::sqrt(distance_sq(p0, p1));
    }

    YAW_PITCH_256 to_yaw_pitch_256(ANGLE_DEG val);
    YAW_PITCH_256 to_yaw_pitch_256(ANGLE_RAD val);
    YAW_PITCH_256 to_yaw_pitch_256(VECTOR val);

    namespace minecraft {
        VECTOR velocity(ANGLE_DEG rot, ANGLE_DEG speed);
        VECTOR velocity(VECTOR pos, VECTOR target, double speed);

        namespace packets {
            double velocity_clamp(double value);
            double velocity_round(double value);
            double velocity_deround(int64_t value);
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
            return hash<double>()(val.pitch) ^ hash<double>()(val.yaw);
        }
    };

    template <>
    struct hash<copper_server::util::ANGLE_RAD> {
        size_t operator()(const copper_server::util::ANGLE_RAD& val) const {
            return hash<double>()(val.pitch) ^ hash<double>()(val.yaw);
        }
    };
}


#endif /* SRC_UTIL_CALCULATIONS */
