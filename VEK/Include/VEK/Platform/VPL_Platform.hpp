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
#include <VEK/Core/Container/VCO_Vector.hpp>

#include <VEK/Platform/VPL_Context.hpp>
#include <VEK/Platform/VPL_Input.hpp>

#include <memory>
#include <functional>
#include <cstdint>

namespace VEK::Platform {

    enum class SPlatformType : uint8_t {
        Windows,
        Linux, 
        MacOS, // Unused
        NintendoSwitch,
        PS5, // Unused
        XBOX, // Unused
        Unknown
    };

    enum class SArchitecture : uint8_t {
        x86,
        x64,
        ARM32,
        ARM64,
        Unknown
    };

    class IOS {
    public:
        virtual ~IOS() = default;

        // Initialize the OS and create a context 
        virtual bool Init() = 0;
        virtual void Shutdown() = 0;

        // Get the context for window and graphics operations
        virtual IContext* GetContext() = 0;

        // Get the input system for handling keyboard, mouse, and gamepad input
        virtual IInput* GetInput() = 0;

        // Platform information
        virtual SPlatformType GetPlatformType() const = 0;
        virtual SArchitecture GetArchitecture() const = 0;

        // Basic console functions (low-level)
        virtual void ConsolePrint(const char* text) = 0;                    // Print raw text
        virtual void ConsolePrintF(const char* format, ...) = 0;           // Printf-style with args
        virtual void ConsoleClear() = 0;                                    // Clear console
        virtual void ConsoleFlush() = 0;                                    // Flush output buffer
        virtual void ConsoleSetColor(uint8_t r, uint8_t g, uint8_t b) = 0; // Set text color (if supported)
        virtual void ConsoleResetColor() = 0;                              // Reset to default color

        // System information
        virtual uint64_t GetTotalMemory() const = 0;
        virtual uint64_t GetAvailableMemory() const = 0;
        virtual uint32_t GetCpuCoreCount() const = 0;

        // Time functions (OS-specific implementations)
        virtual uint64_t GetTicks() const = 0;              // High-resolution timer in milliseconds
        virtual uint64_t GetTicksMicro() const = 0;         // High-resolution timer in microseconds
        virtual uint64_t GetTicksNano() const = 0;          // High-resolution timer in nanoseconds
        virtual uint64_t GetUnixTime() const = 0;           // Unix timestamp in seconds
        virtual uint64_t GetUnixTimeMs() const = 0;         // Unix timestamp in milliseconds

        virtual void Sleep(uint32_t milliseconds) = 0;      // Sleep for milliseconds
        virtual void SleepMicro(uint32_t microseconds) = 0; // Sleep for microseconds
        virtual uint64_t GetCpuFrequency() const = 0;       // CPU frequency if available

        // Factory method to create platform-specific OS instance
        static std::unique_ptr<IOS> Create();
    };

} // namespace VEK::Platform
