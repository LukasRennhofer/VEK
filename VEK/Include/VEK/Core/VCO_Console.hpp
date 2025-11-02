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

#include <VEK/Core/Container/VCO_String.hpp>
#include <memory>
#include <mutex>

// Forward declaration
namespace VEK::Platform {
    class IOS;
}

// Configuration macros
#ifndef VEK_CONSOLE_ENABLED
    #define VEK_CONSOLE_ENABLED 1
#endif

namespace VEK::Core {

    enum class KConsoleColor {
        Default,
        Red,
        Green,
        Yellow,
        Blue,
        Cyan,
        Magenta,
        White,
        Black,
        BrightRed,
        BrightGreen,
        BrightYellow,
        BrightBlue,
        BrightCyan,
        BrightMagenta,
        BrightWhite
    };

    class KConsoleStream {
        public:
            // Initialize with OS instance
            static void Initialize(VEK::Platform::IOS* osInstance);
            static void Shutdown();
            
            // Main output functions
            static void Write(const KSafeString<>& text, KConsoleColor color = KConsoleColor::Default);
            static void WriteLine(const KSafeString<>& text, KConsoleColor color = KConsoleColor::Default);
            
            // Console control functions
            static void Clear();
            static void Flush();
            
            // Enable/disable console output
            static void SetEnabled(bool enabled) { s_Enabled = enabled; }
            static bool IsEnabled() { return s_Enabled; }

        private:
            static void SetColor(KConsoleColor color);
            static void ResetColor();
            static uint8_t ColorToRGB_R(KConsoleColor color);
            static uint8_t ColorToRGB_G(KConsoleColor color);
            static uint8_t ColorToRGB_B(KConsoleColor color);

        private:
            static std::mutex s_Mutex;
            static bool s_Enabled;
            static VEK::Platform::IOS* s_OSInstance;
    };
}

// Macros
// Console Stream Write Line
#define VEK_CONSOLE_WL(text, ...) \
    VEK::Core::KConsoleStream::WriteLine(text, ##__VA_ARGS__)


