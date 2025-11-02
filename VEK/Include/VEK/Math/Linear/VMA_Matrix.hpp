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

#include <VEK/Math/Linear/VMA_Vector.hpp>

namespace VEK::Math
{

    // ----------------- MMat4 -----------------
    struct MMat4
    {

            std::array<float, 16> m{};

            // Identity matrix
            static constexpr MMat4 Identity() noexcept
            {
                MMat4 mat{};
                mat.m[0]  = 1.f;
                mat.m[5]  = 1.f;
                mat.m[10] = 1.f;
                mat.m[15] = 1.f;
                return mat;
            }

            // Multiply two matrices
            MMat4 operator*(const MMat4 &rhs) const noexcept
            {
                MMat4 result{};
                for (int row = 0; row < 4; ++row)
                {
                    for (int col = 0; col < 4; ++col)
                    {
                        float sum = 0.f;
                        for (int i = 0; i < 4; ++i)
                        {
                            sum += m[row * 4 + i] * rhs.m[i * 4 + col];
                        }
                        result.m[row * 4 + col] = sum;
                    }
                }
                return result;
            }

            // Create translation matrix
            static MMat4 Translate(const MVector3 &translation) noexcept
            {
                MMat4 mat = Identity();
                mat.m[12] = translation.x;
                mat.m[13] = translation.y;
                mat.m[14] = translation.z;
                return mat;
            }

            // Create rotation matrix around Y (yaw)
            static MMat4 RotationYaw(float degrees) noexcept
            {
                float rad = degrees * 3.14159265358979323846f / 180.f;
                float c   = std::cos(rad);
                float s   = std::sin(rad);

                MMat4 mat = Identity();
                mat.m[0]  = c;
                mat.m[2]  = s;
                mat.m[8]  = -s;
                mat.m[10] = c;
                return mat;
            }

            // Create rotation matrix around X (pitch)
            static MMat4 RotationPitch(float degrees) noexcept
            {
                float rad = degrees * 3.14159265358979323846f / 180.f;
                float c   = std::cos(rad);
                float s   = std::sin(rad);

                MMat4 mat = Identity();
                mat.m[5]  = c;
                mat.m[6]  = -s;
                mat.m[9]  = s;
                mat.m[10] = c;
                return mat;
            }

            // Combine yaw + pitch rotations (yaw first)
            static MMat4 RotationYawPitch(float yawDegrees, float pitchDegrees) noexcept { return RotationYaw(yawDegrees) * RotationPitch(pitchDegrees); }

            // Create lookAt matrix (right-handed)
            static MMat4 LookAt(const MVector3 &eye, const MVector3 &center, const MVector3 &up) noexcept
            {
                MVector3 f = (center - eye).Normalized();
                MVector3 s = f.Cross(up).Normalized();
                MVector3 u = s.Cross(f);

                MMat4 mat = Identity();
                mat.m[0]  = s.x;
                mat.m[1]  = u.x;
                mat.m[2]  = -f.x;
                mat.m[4]  = s.y;
                mat.m[5]  = u.y;
                mat.m[6]  = -f.y;
                mat.m[8]  = s.z;
                mat.m[9]  = u.z;
                mat.m[10] = -f.z;

                mat.m[12] = -s.Dot(eye);
                mat.m[13] = -u.Dot(eye);
                mat.m[14] = f.Dot(eye);
                return mat;
            }

            static MMat4 Perspective(float fovDegrees, float aspectRatio, float nearPlane, float farPlane) noexcept
            {
                float fovRad = fovDegrees * 3.14159265358979323846f / 180.f;
                float f      = 1.f / std::tan(fovRad / 2.f);

                MMat4 mat{};
                mat.m[0]  = f / aspectRatio;                                 // scale X
                mat.m[5]  = f;                                               // scale Y
                mat.m[10] = (farPlane + nearPlane) / (nearPlane - farPlane); // Z scaling
                mat.m[11] = -1.f;                                            // perspective divide
                mat.m[14] = (2.f * farPlane * nearPlane) / (nearPlane - farPlane);
                mat.m[15] = 0.f;

                return mat;
            }

            static MMat4 Scale(const MVector3 &scale) noexcept
            {
                MMat4 mat = Identity();
                mat.m[0]  = scale.x;
                mat.m[5]  = scale.y;
                mat.m[10] = scale.z;
                return mat;
            }

            static MMat4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept
            {
                MMat4 mat{};

                float rl = right - left;
                float tb = top - bottom;
                float fn = farPlane - nearPlane;

                assert(rl != 0.f && tb != 0.f && fn != 0.f);

                mat.m[0]  = 2.f / rl;
                mat.m[5]  = 2.f / tb;
                mat.m[10] = -2.f / fn;

                mat.m[12] = -(right + left) / rl;
                mat.m[13] = -(top + bottom) / tb;
                mat.m[14] = -(farPlane + nearPlane) / fn;

                mat.m[15] = 1.f;

                return mat;
            }

            constexpr const float *Data() const noexcept { return m.data(); }
            float                 *Data() noexcept { return m.data(); }
    };

    // ----------------- MMat3 -----------------
    struct MMat3
    {
            std::array<float, 9> m{}; // 3x3 matrix stored in row-major order

            // Identity matrix
            static constexpr MMat3 Identity() noexcept
            {
                MMat3 mat{};
                mat.m[0] = 1.f;
                mat.m[4] = 1.f;
                mat.m[8] = 1.f;
                return mat;
            }

            // Multiply two matrices
            MMat3 operator*(const MMat3 &rhs) const noexcept
            {
                MMat3 result{};
                for (int row = 0; row < 3; ++row)
                {
                    for (int col = 0; col < 3; ++col)
                    {
                        float sum = 0.f;
                        for (int i = 0; i < 3; ++i)
                        {
                            sum += m[row * 3 + i] * rhs.m[i * 3 + col];
                        }
                        result.m[row * 3 + col] = sum;
                    }
                }
                return result;
            }

            // Multiply matrix by vector
            MVector3 operator*(const MVector3 &v) const noexcept
            {
                return MVector3{m[0] * v.x + m[1] * v.y + m[2] * v.z, m[3] * v.x + m[4] * v.y + m[5] * v.z, m[6] * v.x + m[7] * v.y + m[8] * v.z};
            }

            // Create rotation matrix around X axis (degrees)
            static MMat3 RotationX(float degrees) noexcept
            {
                float rad = degrees * 3.14159265358979323846f / 180.f;
                float c   = std::cos(rad);
                float s   = std::sin(rad);
                MMat3 mat = Identity();
                mat.m[4]  = c;
                mat.m[5]  = -s;
                mat.m[7]  = s;
                mat.m[8]  = c;
                return mat;
            }

            // Create rotation matrix around Y axis (degrees)
            static MMat3 RotationY(float degrees) noexcept
            {
                float rad = degrees * 3.14159265358979323846f / 180.f;
                float c   = std::cos(rad);
                float s   = std::sin(rad);
                MMat3 mat = Identity();
                mat.m[0]  = c;
                mat.m[2]  = s;
                mat.m[6]  = -s;
                mat.m[8]  = c;
                return mat;
            }

            // Create rotation matrix around Z axis (degrees)
            static MMat3 RotationZ(float degrees) noexcept
            {
                float rad = degrees * 3.14159265358979323846f / 180.f;
                float c   = std::cos(rad);
                float s   = std::sin(rad);
                MMat3 mat = Identity();
                mat.m[0]  = c;
                mat.m[1]  = -s;
                mat.m[3]  = s;
                mat.m[4]  = c;
                return mat;
            }

            // Create a LookAt rotation matrix (only rotation, no translation)
            static MMat3 LookAt(const MVector3 &eye, const MVector3 &center, const MVector3 &up) noexcept
            {
                MVector3 f = (center - eye).Normalized();
                MVector3 s = f.Cross(up).Normalized();
                MVector3 u = s.Cross(f);

                MMat3 mat{};
                // Each row is a basis vector of the new orientation
                mat.m[0] = s.x;
                mat.m[1] = s.y;
                mat.m[2] = s.z;
                mat.m[3] = u.x;
                mat.m[4] = u.y;
                mat.m[5] = u.z;
                mat.m[6] = -f.x;
                mat.m[7] = -f.y;
                mat.m[8] = -f.z;

                return mat;
            }

            // Transpose the matrix
            MMat3 Transpose() const noexcept
            {
                MMat3 result{};
                for (int row = 0; row < 3; ++row)
                {
                    for (int col = 0; col < 3; ++col)
                    {
                        result.m[col * 3 + row] = m[row * 3 + col];
                    }
                }
                return result;
            }

            constexpr const float *Data() const noexcept { return m.data(); }
            float                 *Data() noexcept { return m.data(); }
    };

    // ----------------- MMat2 -----------------
    struct MMat2
    {

            // 2x2 matrix stored in column-major order:
            // [ m[0] m[2] ]
            // [ m[1] m[3] ]
            std::array<float, 4> m{};

            // Identity matrix
            static constexpr MMat2 Identity() noexcept
            {
                MMat2 mat{};
                mat.m[0] = 1.f;
                mat.m[3] = 1.f;
                return mat;
            }

            // Multiply two 2x2 matrices
            MMat2 operator*(const MMat2 &rhs) const noexcept
            {
                MMat2 result{};
                result.m[0] = m[0] * rhs.m[0] + m[2] * rhs.m[1];
                result.m[1] = m[1] * rhs.m[0] + m[3] * rhs.m[1];
                result.m[2] = m[0] * rhs.m[2] + m[2] * rhs.m[3];
                result.m[3] = m[1] * rhs.m[2] + m[3] * rhs.m[3];
                return result;
            }

            // Multiply by a vector (2D)
            MVector2 operator*(const MVector2 &vec) const noexcept { return {m[0] * vec.x + m[2] * vec.y, m[1] * vec.x + m[3] * vec.y}; }

            // Transpose the matrix
            MMat2 Transposed() const noexcept
            {
                MMat2 result{};
                result.m[0] = m[0];
                result.m[1] = m[2];
                result.m[2] = m[1];
                result.m[3] = m[3];
                return result;
            }

            // Compute determinant
            constexpr float Determinant() const noexcept { return m[0] * m[3] - m[2] * m[1]; }

            // Create a rotation matrix (in degrees)
            static MMat2 Rotation(float degrees) noexcept
            {
                float rad = degrees * 3.14159265358979323846f / 180.f;
                float c   = std::cos(rad);
                float s   = std::sin(rad);

                MMat2 mat{};
                mat.m[0] = c;
                mat.m[1] = s;
                mat.m[2] = -s;
                mat.m[3] = c;
                return mat;
            }

            // Create a scaling matrix
            static MMat2 Scale(float scaleX, float scaleY) noexcept
            {
                MMat2 mat{};
                mat.m[0] = scaleX;
                mat.m[3] = scaleY;
                return mat;
            }

            constexpr const float *Data() const noexcept { return m.data(); }
            float                 *Data() noexcept { return m.data(); }
    };
}