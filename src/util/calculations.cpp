/*
 * Copyright 2024-Present Danyil Melnytskyi. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License"). You may not use
 * this file except in compliance with the License. You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * http://www.apache.org/licenses/LICENSE-2.0
 */
#include <algorithm>
#include <src/util/calculations.hpp>

namespace copper_server::util {
    const double pi = 3.14159265358979323846;

    vector convert(angle_deg rot) {
        vector res;
        double x_cos = cos(rot.pitch);
        res.x = cos(rot.yaw) * x_cos;
        res.y = sin(rot.yaw) * x_cos;
        res.z = sin(rot.pitch);
        return res;
    }

    angle_deg convert(vector val) {
        double yaw = atan2(val.z, val.x);
        double pitch = atan2(sqrt(val.z * val.z + val.x * val.x), val.y);
        return {yaw, pitch}; //xy
    }

    vector dif(vector p0, vector p1) {
        return vector{p0.x - p1.x, p0.y - p1.y, p0.z - p1.z};
    }

    vector normalize(vector val) {
        return convert(convert(val));
    }

    vector strength(vector val, double mult) {
        val.x *= mult;
        val.y *= mult;
        val.z *= mult;
        return val;
    }

    vector weak(vector val, double div) {
        val.x /= div;
        val.y /= div;
        val.z /= div;
        return val;
    }

    angle_deg direction(vector pos, vector target) {
        return convert(normalize(dif(target, pos)));
    }

    double rad_to_deg180(double val) {
        return (val * pi) / 180;
    }

    double deg_to_rad180(double val) {
        return (val * 180) / pi;
    }

    double rad_to_deg360(double val) {
        return (val * pi * 2) / 360;
    }

    double deg_to_rad360(double val) {
        return (val * 360) / pi * 2;
    }

    yaw_pitch_256 to_yaw_pitch_256(angle_deg val) {
        return {(uint8_t)val.pitch, (uint8_t)val.yaw};
    }

    yaw_pitch_256 to_yaw_pitch_256(angle_rad val) {
        return {(uint8_t)rad_to_deg180(val.pitch), (uint8_t)rad_to_deg180(val.yaw)};
    }

    yaw_pitch_256 to_yaw_pitch_256(vector val) {
        return to_yaw_pitch_256(convert(val));
    }

    namespace minecraft {
        vector velocity(angle_deg rot, angle_deg speed) {
            return strength(convert(rot), speed.yaw);
        }

        vector velocity(vector pos, vector target, double speed) {
            return strength(normalize(dif(target, pos)), speed);
        }

        namespace packets {
            double velocity_round(double value) {
                return std::round((value * 0.5 + 0.5) * 32766.0);
            }

            double velocity_deround(int64_t value) {
                return std::min((double)(value & 32767L), 32766.0) * 2.0 / 32766.0 - 1.0;
            }

            double velocity_clamp(double value) {
                static constexpr double max = double(1ui64 << 34) - 1;
                static constexpr double min = -(max);
                return isnan(value) ? 0.0 : std::clamp(value, min, max);
            }

            xyz<int16_t> velocity(vector rot) {
                return {(int16_t)(rot.x * 8000), (int16_t)(rot.y * 8000), (int16_t)(rot.z * 8000)};
            }

            xyz<int16_t> delta_move(xyz<float> pos) {
                int64_t x = (int64_t)pos.x * (4096);
                int64_t y = (int64_t)pos.y * (4096);
                int64_t z = (int64_t)pos.z * (4096);
                return {
                    (int16_t)std::clamp<int64_t>(x, INT16_MIN, INT16_MAX),
                    (int16_t)std::clamp<int64_t>(y, INT16_MIN, INT16_MAX),
                    (int16_t)std::clamp<int64_t>(z, INT16_MIN, INT16_MAX)
                };
            }

            xy<int16_t> delta_move(xy<float> pos) {
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