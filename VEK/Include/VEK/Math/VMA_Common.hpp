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

#include <cmath>

namespace VEK::Math
{
    constexpr float PI  = 3.14159265358979323846f;
    constexpr float TAU = 6.28318530717958647692f; // TAU = 2 * PI

    // Convert degrees to radians
    constexpr float DegToRad(float degrees) { return degrees * PI / 180.0f; }

    // Convert radians to degrees
    constexpr float RadToDeg(float radians) { return radians * 180.0f / PI; }

    // Rounds up a given value to the nearest multiple of a specified alignment
    template <typename T> constexpr T align(T value, T alignment) { return ((value + alignment - T(1)) / alignment) * alignment; }
}