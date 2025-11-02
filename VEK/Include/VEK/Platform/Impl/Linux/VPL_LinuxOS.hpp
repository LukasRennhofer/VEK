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

#ifdef VEK_LINUX

#include <VEK/Platform/VPL_Platform.hpp>
#include <VEK/Platform/Impl/Linux/VPL_LinuxContext.hpp>
#include <VEK/Platform/Impl/Linux/VPL_LinuxInput.hpp>

#include <memory>
#include <sys/utsname.h>
#include <unistd.h>
#include <ctime>

// Forward declaration
struct _XDisplay;
typedef struct _XDisplay Display;
typedef union _XEvent XEvent;

namespace VEK::Platform {

    class LinuxOS : public IOS {
    private:
        std::unique_ptr<LinuxContext> m_context;
        std::unique_ptr<LinuxInput> m_input;
        bool m_initialized = false;
        
    public:
        LinuxOS();
        virtual ~LinuxOS();

        // IOS implementation
        bool Init() override;
        void Shutdown() override;

        IContext* GetContext() override { return m_context.get(); }

        // Get the input system for handling keyboard, mouse, and gamepad input
        IInput* GetInput() override;

        // Initialize input system (called after window creation)
        bool InitializeInput();

        // Input event processing  
        bool ProcessInputEvent(XEvent* event);
        void UpdateInput();

        // Platform information
        SPlatformType GetPlatformType() const override { return SPlatformType::Linux; }
        SArchitecture GetArchitecture() const override;

        // Basic console functions (Linux-specific)
        void ConsolePrint(const char* text) override;
        void ConsolePrintF(const char* format, ...) override;
        void ConsoleClear() override;
        void ConsoleFlush() override;
        void ConsoleSetColor(uint8_t r, uint8_t g, uint8_t b) override;
        void ConsoleResetColor() override;

        // System information
        uint64_t GetTotalMemory() const override;
        uint64_t GetAvailableMemory() const override;
        uint32_t GetCpuCoreCount() const override;

        // Time functions (Linux-specific implementations)
        uint64_t GetTicks() const override;
        uint64_t GetTicksMicro() const override;
        uint64_t GetTicksNano() const override;
        uint64_t GetUnixTime() const override;
        uint64_t GetUnixTimeMs() const override;
        void Sleep(uint32_t milliseconds) override;
        void SleepMicro(uint32_t microseconds) override;
        uint64_t GetCpuFrequency() const override;
    };

} // namespace VEK::Platform

#endif // VEK_LINUX
