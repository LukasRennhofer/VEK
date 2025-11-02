/*
================================================================================
  VEK (Vantor Engine Kernel) - Used by Vantor Studios
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  “Order the chaos, frame the void — and call it a world.”
================================================================================
*/

#pragma once

#include <array>
#include <cassert>
#include <cmath>

namespace VEK::Math
{

    // ----------------- MVector2 -----------------
    struct MVector2
    {
            float x = 0.0f, y = 0.0f;

            // Constructors
            constexpr MVector2() noexcept = default;
            constexpr MVector2(float _x, float _y) noexcept : x(_x), y(_y) {}

            // Operators
            constexpr MVector2 operator+(const MVector2 &rhs) const noexcept { return {x + rhs.x, y + rhs.y}; }
            constexpr MVector2 operator-(const MVector2 &rhs) const noexcept { return {x - rhs.x, y - rhs.y}; }
            constexpr MVector2 operator*(float scalar) const noexcept { return {x * scalar, y * scalar}; }
            constexpr MVector2 operator/(float scalar) const noexcept { return {x / scalar, y / scalar}; }

            MVector2 &operator+=(const MVector2 &rhs) noexcept
            {
                x += rhs.x;
                y += rhs.y;
                return *this;
            }
            MVector2 &operator-=(const MVector2 &rhs) noexcept
            {
                x -= rhs.x;
                y -= rhs.y;
                return *this;
            }
            MVector2 &operator*=(float scalar) noexcept
            {
                x *= scalar;
                y *= scalar;
                return *this;
            }
            MVector2 &operator/=(float scalar) noexcept
            {
                x /= scalar;
                y /= scalar;
                return *this;
            }

            // Utilities
            constexpr float Dot(const MVector2 &rhs) const noexcept { return x * rhs.x + y * rhs.y; }
            float           length() const noexcept { return std::sqrt(x * x + y * y); }
            float           lengthSquared() const noexcept { return x * x + y * y; }

            MVector2 Normalized() const noexcept
            {
                float len = length();
                return len > 0 ? (*this) / len : MVector2{};
            }

            void Normalize() noexcept
            {
                float len = length();
                if (len > 0)
                {
                    x /= len;
                    y /= len;
                }
            }

            constexpr const float *Data() const noexcept { return &x; }
            float                 *Data() noexcept { return &x; }
    };

    // Scalar * vector operator
    inline MVector2 operator*(float scalar, const MVector2 &vec) noexcept { return vec * scalar; }

    // ----------------- MVector3 -----------------
    struct MVector3
    {
            float x = 0.0f, y = 0.0f, z = 0.0f;

            constexpr MVector3() noexcept = default;
            constexpr MVector3(float _x, float _y, float _z) noexcept : x(_x), y(_y), z(_z) {}

            constexpr MVector3 operator+(const MVector3 &rhs) const noexcept { return {x + rhs.x, y + rhs.y, z + rhs.z}; }
            constexpr MVector3 operator-(const MVector3 &rhs) const noexcept { return {x - rhs.x, y - rhs.y, z - rhs.z}; }
            constexpr MVector3 operator*(float scalar) const noexcept { return {x * scalar, y * scalar, z * scalar}; }
            constexpr MVector3 operator/(float scalar) const noexcept { return {x / scalar, y / scalar, z / scalar}; }

            MVector3 &operator+=(const MVector3 &rhs) noexcept
            {
                x += rhs.x;
                y += rhs.y;
                z += rhs.z;
                return *this;
            }
            MVector3 &operator-=(const MVector3 &rhs) noexcept
            {
                x -= rhs.x;
                y -= rhs.y;
                z -= rhs.z;
                return *this;
            }
            MVector3 &operator*=(float scalar) noexcept
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;
                return *this;
            }
            MVector3 &operator/=(float scalar) noexcept
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;
                return *this;
            }

            constexpr float Dot(const MVector3 &rhs) const noexcept { return x * rhs.x + y * rhs.y + z * rhs.z; }

            MVector3 Cross(const MVector3 &rhs) const noexcept { return {y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x}; }

            float length() const noexcept { return std::sqrt(x * x + y * y + z * z); }
            float lengthSquared() const noexcept { return x * x + y * y + z * z; }

            MVector3 Normalized() const noexcept
            {
                float len = length();
                return len > 0 ? (*this) / len : MVector3{};
            }

            void Normalize() noexcept
            {
                float len = length();
                if (len > 0)
                {
                    x /= len;
                    y /= len;
                    z /= len;
                }
            }

            constexpr const float *Data() const noexcept { return &x; }
            float                 *Data() noexcept { return &x; }
    };

    inline MVector3 operator*(float scalar, const MVector3 &vec) noexcept { return vec * scalar; }

    // ----------------- MVector4 -----------------
    struct MVector4
    {
            float x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f;

            constexpr MVector4() noexcept = default;
            constexpr MVector4(float _x, float _y, float _z, float _w) noexcept : x(_x), y(_y), z(_z), w(_w) {}

            constexpr MVector4 operator+(const MVector4 &rhs) const noexcept { return {x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w}; }
            constexpr MVector4 operator-(const MVector4 &rhs) const noexcept { return {x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w}; }
            constexpr MVector4 operator*(float scalar) const noexcept { return {x * scalar, y * scalar, z * scalar, w * scalar}; }
            constexpr MVector4 operator/(float scalar) const noexcept { return {x / scalar, y / scalar, z / scalar, w / scalar}; }

            MVector4 &operator+=(const MVector4 &rhs) noexcept
            {
                x += rhs.x;
                y += rhs.y;
                z += rhs.z;
                w += rhs.w;
                return *this;
            }
            MVector4 &operator-=(const MVector4 &rhs) noexcept
            {
                x -= rhs.x;
                y -= rhs.y;
                z -= rhs.z;
                w -= rhs.w;
                return *this;
            }
            MVector4 &operator*=(float scalar) noexcept
            {
                x *= scalar;
                y *= scalar;
                z *= scalar;
                w *= scalar;
                return *this;
            }
            MVector4 &operator/=(float scalar) noexcept
            {
                x /= scalar;
                y /= scalar;
                z /= scalar;
                w /= scalar;
                return *this;
            }

            constexpr float Dot(const MVector4 &rhs) const noexcept { return x * rhs.x + y * rhs.y + z * rhs.z + w * rhs.w; }

            float length() const noexcept { return std::sqrt(x * x + y * y + z * z + w * w); }
            float lengthSquared() const noexcept { return x * x + y * y + z * z + w * w; }

            MVector4 Normalized() const noexcept
            {
                float len = length();
                return len > 0 ? (*this) / len : MVector4{};
            }

            void Normalize() noexcept
            {
                float len = length();
                if (len > 0)
                {
                    x /= len;
                    y /= len;
                    z /= len;
                    w /= len;
                }
            }

            constexpr const float *Data() const noexcept { return &x; }
            float                 *Data() noexcept { return &x; }
    };

    inline MVector4 operator*(float scalar, const MVector4 &vec) noexcept { return vec * scalar; }

    // Clamp scalar
    inline float Clamp(float value, float minVal, float maxVal) noexcept { return std::fmax(minVal, std::fmin(maxVal, value)); }

    // Clamp01 scalar
    inline float Clamp01(float value) noexcept { return Clamp(value, 0.0f, 1.0f); }

    // Lerp scalar
    inline float Lerp(float a, float b, float t) noexcept { return a + (b - a) * Clamp01(t); }

    // --------- MVector2 Utils ---------
    inline MVector2 Clamp(const MVector2 &v, const MVector2 &min, const MVector2 &max) noexcept { return {Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y)}; }

    inline MVector2 Clamp01(const MVector2 &v) noexcept { return Clamp(v, MVector2(0.0f, 0.0f), MVector2(1.0f, 1.0f)); }

    inline MVector2 Lerp(const MVector2 &a, const MVector2 &b, float t) noexcept { return a + (b - a) * Clamp01(t); }

    // --------- MVector3 Utils ---------
    inline MVector3 Clamp(const MVector3 &v, const MVector3 &min, const MVector3 &max) noexcept
    {
        return {Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y), Clamp(v.z, min.z, max.z)};
    }

    inline MVector3 Clamp01(const MVector3 &v) noexcept { return Clamp(v, MVector3(0.0f, 0.0f, 0.0f), MVector3(1.0f, 1.0f, 1.0f)); }

    inline MVector3 Lerp(const MVector3 &a, const MVector3 &b, float t) noexcept { return a + (b - a) * Clamp01(t); }

    // --------- MVector4 Utils ---------
    inline MVector4 Clamp(const MVector4 &v, const MVector4 &min, const MVector4 &max) noexcept
    {
        return {Clamp(v.x, min.x, max.x), Clamp(v.y, min.y, max.y), Clamp(v.z, min.z, max.z), Clamp(v.w, min.w, max.w)};
    }

    inline MVector4 Clamp01(const MVector4 &v) noexcept { return Clamp(v, MVector4(0.0f, 0.0f, 0.0f, 0.0f), MVector4(1.0f, 1.0f, 1.0f, 1.0f)); }

    inline MVector4 Lerp(const MVector4 &a, const MVector4 &b, float t) noexcept { return a + (b - a) * Clamp01(t); }

}