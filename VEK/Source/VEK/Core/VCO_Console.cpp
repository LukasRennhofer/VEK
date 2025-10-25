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

#include <VEK/Core/VCO_Console.hpp>

// For now just iostream, because we dont have a platform yet
#include <iostream>

namespace VEK::Core {
    void KConsoleStream::Write(const KSafeString<>& text, KConsoleColor color = KConsoleColor::Default) {
        std::cout << text.c_str();
    }
}