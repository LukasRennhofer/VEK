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
#include <VEK/Platform/VPL_Platform.hpp>
#include <iostream>
#include <cstring>

namespace VEK::Core {

    // Define static members
    std::mutex KConsoleStream::s_Mutex;
    bool KConsoleStream::s_Enabled = true;
    VEK::Platform::IOS* KConsoleStream::s_OSInstance = nullptr;

    void KConsoleStream::Initialize(VEK::Platform::IOS* osInstance) {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_OSInstance = osInstance;
    }

    void KConsoleStream::Shutdown() {
        std::lock_guard<std::mutex> lock(s_Mutex);
        s_OSInstance = nullptr;
    }

    void KConsoleStream::Write(const KSafeString<>& text, KConsoleColor color) {
        if (!s_Enabled) return;

        std::lock_guard<std::mutex> lock(s_Mutex);
        
        #if VEK_CONSOLE_ENABLED
            if (s_OSInstance) {
                SetColor(color);
                s_OSInstance->ConsolePrint(text.c_str());
                ResetColor();
            }
        #endif
    }

    void KConsoleStream::WriteLine(const KSafeString<>& text, KConsoleColor color) {
        if (!s_Enabled) return;

        std::lock_guard<std::mutex> lock(s_Mutex);
        
        #if VEK_CONSOLE_ENABLED
            if (s_OSInstance) {
                SetColor(color);
                s_OSInstance->ConsolePrint(text.c_str());
                s_OSInstance->ConsolePrint("\n");
                ResetColor();
            }
        #endif
    }

    void KConsoleStream::Clear() {
        if (!s_Enabled) return;

        std::lock_guard<std::mutex> lock(s_Mutex);
        
        #if VEK_CONSOLE_ENABLED
            if (s_OSInstance) {
                s_OSInstance->ConsoleClear();
            }
        #endif
    }

    void KConsoleStream::Flush() {
        if (!s_Enabled) return;

        std::lock_guard<std::mutex> lock(s_Mutex);
        
        #if VEK_CONSOLE_ENABLED
            if (s_OSInstance) {
                s_OSInstance->ConsoleFlush();
            }
        #endif
    }

    void KConsoleStream::SetColor(KConsoleColor color) {
        #if VEK_CONSOLE_ENABLED
            if (s_OSInstance && color != KConsoleColor::Default) {
                uint8_t r = ColorToRGB_R(color);
                uint8_t g = ColorToRGB_G(color);
                uint8_t b = ColorToRGB_B(color);
                s_OSInstance->ConsoleSetColor(r, g, b);
            }
        #endif
    }

    void KConsoleStream::ResetColor() {
        #if VEK_CONSOLE_ENABLED
            if (s_OSInstance) {
                s_OSInstance->ConsoleResetColor();
            }
        #endif
    }

    uint8_t KConsoleStream::ColorToRGB_R(KConsoleColor color) {
        switch (color) {
            case KConsoleColor::Red:
            case KConsoleColor::BrightRed:
            case KConsoleColor::Yellow:
            case KConsoleColor::BrightYellow:
            case KConsoleColor::Magenta:
            case KConsoleColor::BrightMagenta:
            case KConsoleColor::White:
            case KConsoleColor::BrightWhite:
                return color == KConsoleColor::BrightRed || 
                       color == KConsoleColor::BrightYellow || 
                       color == KConsoleColor::BrightMagenta || 
                       color == KConsoleColor::BrightWhite ? 255 : 180;
            default:
                return 0;
        }
    }

    uint8_t KConsoleStream::ColorToRGB_G(KConsoleColor color) {
        switch (color) {
            case KConsoleColor::Green:
            case KConsoleColor::BrightGreen:
            case KConsoleColor::Yellow:
            case KConsoleColor::BrightYellow:
            case KConsoleColor::Cyan:
            case KConsoleColor::BrightCyan:
            case KConsoleColor::White:
            case KConsoleColor::BrightWhite:
                return color == KConsoleColor::BrightGreen || 
                       color == KConsoleColor::BrightYellow || 
                       color == KConsoleColor::BrightCyan || 
                       color == KConsoleColor::BrightWhite ? 255 : 180;
            default:
                return 0;
        }
    }

    uint8_t KConsoleStream::ColorToRGB_B(KConsoleColor color) {
        switch (color) {
            case KConsoleColor::Blue:
            case KConsoleColor::BrightBlue:
            case KConsoleColor::Cyan:
            case KConsoleColor::BrightCyan:
            case KConsoleColor::Magenta:
            case KConsoleColor::BrightMagenta:
            case KConsoleColor::White:
            case KConsoleColor::BrightWhite:
                return color == KConsoleColor::BrightBlue || 
                       color == KConsoleColor::BrightCyan || 
                       color == KConsoleColor::BrightMagenta || 
                       color == KConsoleColor::BrightWhite ? 255 : 180;
            default:
                return 0;
        }
    }
}
