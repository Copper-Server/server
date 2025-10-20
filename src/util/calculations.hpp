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
    struct xyzw {
        T x;
        T y;
        T z;
        T w;

        bool operator==(const xyzw& comp) {
            return x == comp.x && y == comp.y && z == comp.z && w == comp.w;
        }

        bool operator!=(const xyzw& comp) {
            return x != comp.x || y != comp.y || z != comp.z || w != comp.w;
        }

        xyzw& operator+=(const xyzw& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            w += other.w;
            return *this;
        }

        xyzw& operator-=(const xyzw& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            w -= other.w;
            return *this;
        }

        xyzw& operator*=(const xyzw& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            w *= other.w;
            return *this;
        }

        xyzw& operator/=(const xyzw& other) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            w /= other.w;
            return *this;
        }
    };

    template <class T>
    struct xyz {
        T x;
        T y;
        T z;

        bool operator==(const xyz& comp) {
            return x == comp.x && y == comp.y && z == comp.z;
        }

        bool operator!=(const xyz& comp) {
            return x != comp.x || y != comp.y || z != comp.z;
        }

        xyz& operator+=(const xyz& other) {
            x += other.x;
            y += other.y;
            z += other.z;
            return *this;
        }

        xyz& operator-=(const xyz& other) {
            x -= other.x;
            y -= other.y;
            z -= other.z;
            return *this;
        }

        xyz& operator*=(const xyz& other) {
            x *= other.x;
            y *= other.y;
            z *= other.z;
            return *this;
        }

        xyz& operator/=(const xyz& other) {
            x /= other.x;
            y /= other.y;
            z /= other.z;
            return *this;
        }
    };

    typedef xyz<double> vector;

    template <class T>
    struct xy {
        T x;
        T y;

        bool operator==(const xy& comp) const {
            return x == comp.x && y == comp.y;
        }

        bool operator!=(const xy& comp) const {
            return x != comp.x || y != comp.y;
        }

        xy& operator+=(const xy& other) {
            x += other.x;
            y += other.y;
            return *this;
        }

        xy& operator-=(const xy& other) {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        xy& operator*=(const xy& other) {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        xy& operator/=(const xy& other) {
            x /= other.x;
            y /= other.y;
            return *this;
        }
    };

    struct angle_deg {
        double pitch;
        double yaw;

        bool operator==(const angle_deg& comp) {
            return pitch == comp.pitch && yaw == comp.yaw;
        }

        bool operator!=(const angle_deg& comp) {
            return pitch != comp.pitch || yaw != comp.yaw;
        }

        angle_deg& operator+=(const angle_deg& other) {
            pitch += other.pitch;
            yaw += other.yaw;
            return *this;
        }

        angle_deg& operator-=(const angle_deg& other) {
            pitch -= other.pitch;
            yaw -= other.yaw;
            return *this;
        }

        angle_deg& operator*=(const angle_deg& other) {
            pitch *= other.pitch;
            yaw *= other.yaw;
            return *this;
        }

        angle_deg& operator/=(const angle_deg& other) {
            pitch /= other.pitch;
            yaw /= other.yaw;
            return *this;
        }
    };

    struct angle_rad {
        double pitch;
        double yaw;

        bool operator==(angle_rad comp) {
            return pitch == comp.pitch && yaw == comp.yaw;
        }

        bool operator!=(angle_rad comp) {
            return pitch != comp.pitch || yaw != comp.yaw;
        }

        angle_rad& operator+=(angle_rad other) {
            pitch += other.pitch;
            yaw += other.yaw;
            return *this;
        }

        angle_rad& operator-=(angle_rad other) {
            pitch -= other.pitch;
            yaw -= other.yaw;
            return *this;
        }

        angle_rad& operator*=(angle_rad other) {
            pitch *= other.pitch;
            yaw *= other.yaw;
            return *this;
        }

        angle_rad& operator/=(angle_rad other) {
            pitch /= other.pitch;
            yaw /= other.yaw;
            return *this;
        }
    };

    typedef xy<uint8_t> yaw_pitch_256;

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
    inline xyz<T> moved(angle_rad rot, T distance) {
        T cos_pitch = cos(rot.pitch);
        xyz<T> offset;
        offset.x = static_cast<T>(cos(rot.yaw) * cos_pitch * distance);
        offset.y = static_cast<T>(sin(rot.yaw) * cos_pitch * distance);
        offset.z = static_cast<T>(sin(rot.pitch) * distance);
        return offset;
    }

    template <class T>
    inline xyz<T> moved(const xyz<T>& startPosition, const angle_rad& rot, T distance) {
        xyz<T> finalPosition = startPosition;
        finalPosition += moved(rot, distance);
        return finalPosition;
    }

    template <class T>
    inline xyz<T> moved(angle_deg rot, T distance) {
        T cos_pitch = cosd(rot.pitch);
        xyz<T> offset;
        offset.x = static_cast<T>(cosd(rot.yaw) * cos_pitch * distance);
        offset.y = static_cast<T>(sind(rot.yaw) * cos_pitch * distance);
        offset.z = static_cast<T>(sind(rot.pitch) * distance);
        return offset;
    }

    template <class T>
    inline xyz<T> moved(const xyz<T>& startPosition, const angle_deg& rot, T distance) {
        xyz<T> finalPosition = startPosition;
        finalPosition += moved(rot, distance);
        return finalPosition;
    }

    vector convert(angle_deg rot);
    //convert to ANGLE as degrees 180*
    angle_deg convert(vector val);
    vector dif(vector p0, vector p1);
    vector normalize(vector val);
    vector strength(vector val, double mult);
    vector weak(vector val, double div);

    angle_deg direction(vector p0, vector p1);


    double rad_to_deg180(double val);
    double deg_to_rad180(double val);

    double rad_to_deg360(double val);
    double deg_to_rad360(double val);

    inline double distance_sq(const vector& p0, const vector& p1) {
        return (p0.x - p1.x) * (p0.x - p1.x) + (p0.y - p1.y) * (p0.y - p1.y) + (p0.z - p1.z) * (p0.z - p1.z);
    }

    inline double distance(const vector& p0, const vector& p1) { //recommended to use squared distance
        return std::sqrt(distance_sq(p0, p1));
    }

    yaw_pitch_256 to_yaw_pitch_256(angle_deg val);
    yaw_pitch_256 to_yaw_pitch_256(angle_rad val);
    yaw_pitch_256 to_yaw_pitch_256(vector val);

    namespace minecraft {
        vector velocity(angle_deg rot, angle_deg speed);
        vector velocity(vector pos, vector target, double speed);

        namespace packets {
            double velocity_clamp(double value);
            double velocity_round(double value);
            double velocity_deround(int64_t value);
            xyz<int16_t> velocity(vector rot);
            xyz<int16_t> delta_move(xyz<float> pos);
            xy<int16_t> delta_move(xy<float> pos);
        }
    }
}

namespace std {
    template <class T>
    struct hash<copper_server::util::xyz<T>> {
        size_t operator()(const copper_server::util::xyz<T>& val) const {
            return hash<T>()(val.x) ^ hash<T>()(val.y) ^ hash<T>()(val.z);
        }
    };

    template <class T>
    struct hash<copper_server::util::xy<T>> {
        size_t operator()(const copper_server::util::xy<T>& val) const {
            return hash<T>()(val.x) ^ hash<T>()(val.y);
        }
    };

    template <>
    struct hash<copper_server::util::angle_deg> {
        size_t operator()(const copper_server::util::angle_deg& val) const {
            return hash<double>()(val.pitch) ^ hash<double>()(val.yaw);
        }
    };

    template <>
    struct hash<copper_server::util::angle_rad> {
        size_t operator()(const copper_server::util::angle_rad& val) const {
            return hash<double>()(val.pitch) ^ hash<double>()(val.yaw);
        }
    };
}


#endif /* SRC_UTIL_CALCULATIONS */
