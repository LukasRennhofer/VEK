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

// For now just stdio, because we dont have a platform yet
#include <stdio.h>

namespace VEK::Core {

    void KConsoleStream::Write(const KSafeString<>& text, KConsoleColor color) {
        // TODO: Use Color

        #ifndef VEK_NSX
        printf(text.c_str());
        #endif
    }
}
