/*
================================================================================
  VEK (Vantor Engine Kernel)
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  "God made the world. I just made a computer kingdom in it."
      — Terry A. Davis
================================================================================
*/

#pragma once

#include <VEK/Core/Container/VCO_String.hpp>

#include <mutex>

namespace VEK::Core {

    enum class KConsoleColor {
        Default,
        Red,
        Green,
        Yellow,
        Blue,
        Cyan,
        Magenta,
        White
    };

    class KConsoleStream {
        public:
            static void Write(const KSafeString<>& text, KConsoleColor color = KConsoleColor::Default);
            static void WriteLine(const KSafeString<>& text, KConsoleColor color = KConsoleColor::Default);

        private:
            static void SetColor(KConsoleColor color);
            static void ResetColor();

        private:
            static std::mutex s_Mutex;
    };
}