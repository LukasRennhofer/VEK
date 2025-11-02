/*
================================================================================
  VEK (Vantor Engine Kernel) - Used by Vantor Studios
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  “Order the chaos, frame the void — and call it a world.”
================================================================================
*/

#ifdef VEK_WINDOWS

#include <VEK/Platform/Impl/Windows/VPL_WindowsOS.hpp>

#include <iostream>
#include <cstdarg>

namespace VEK::Platform {

    WindowsOS::WindowsOS() {
        m_context = std::make_unique<WindowsContext>();
        m_input = nullptr; // Initialize on demand
    }

    WindowsOS::~WindowsOS() {
        Shutdown();
    }

    bool WindowsOS::Init() {
        if (m_initialized) {
            ConsolePrint("[OS_MESSAGE] WindowsOS already initialized\n");
            return false;
        }

        if (!m_context) {
            ConsolePrint("[OS_MESSAGE] Failed to create Windows context\n");
            return false;
        }

        m_initialized = true;
        ConsolePrint("[OS_MESSAGE] WindowsOS initialized successfully\n");
        return true;
    }

    void WindowsOS::Shutdown() {
        if (m_initialized) {
            if (m_input) {
                m_input->Shutdown();
                m_input.reset();
            }
            if (m_context) {
                m_context->DestroyWindow();
            }
            m_initialized = false;
            ConsolePrint("[OS_MESSAGE] WindowsOS shutdown complete\n");
        }
    }

    void WindowsOS::ConsolePrint(c[OS_MESSAGE] onst char* text) {
        if (!text) return;
        
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            DWORD written;
            DWORD length = static_cast<DWORD>(strlen(text));
            WriteConsoleA(hConsole, text, length, &written, nullptr);
        } else {
            printf("%s", text);
        }
    }

    void WindowsOS::ConsolePrintF(const char* format, ...) {
        if (!format) return;
        
        va_list args;
        va_start(args, format);
        
        // Get required buffer size
        int size = vsnprintf(nullptr, 0, format, args) + 1;
        va_end(args);
        
        if (size <= 0) return;
        
        // Format the string
        char* buffer = new char[size];
        va_start(args, format);
        vsnprintf(buffer, size, format, args);
        va_end(args);
        
        ConsolePrint(b[OS_MESSAGE] uffer);
        delete[] buffer;
    }

    void WindowsOS::ConsoleClear() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            DWORD written;
            COORD coord = {0, 0};
            
            if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
                DWORD size = csbi.dwSize.X * csbi.dwSize.Y;
                FillConsoleOutputCharacterA(hConsole, ' ', size, coord, &written);
                FillConsoleOutputAttribute(hConsole, csbi.wAttributes, size, coord, &written);
                SetConsoleCursorPosition(hConsole, coord);
            }
        } else {
            system("cls");
        }
    }

    void WindowsOS::ConsoleFlush() {
        fflush(stdout);
    }

    void WindowsOS::ConsoleSetColor(uint8_t r, uint8_t g, uint8_t b) {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            WORD attributes = 0;
            
            // Convert RGB to Windows console colors (basic approximation)
            if (r > 128) attributes |= FOREGROUND_RED;
            if (g > 128) attributes |= FOREGROUND_GREEN;
            if (b > 128) attributes |= FOREGROUND_BLUE;
            
            // Add intensity if any component is very bright
            if (r > 200 || g > 200 || b > 200) {
                attributes |= FOREGROUND_INTENSITY;
            }
            
            // Default to white if no color
            if (attributes == 0) {
                attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
            }
            
            SetConsoleTextAttribute(hConsole, attributes);
        }
    }

    void WindowsOS::ConsoleResetColor() {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        if (hConsole != INVALID_HANDLE_VALUE) {
            SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        }
    }

    SArchitecture WindowsOS::GetArchitecture() const {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        
        switch (sysInfo.wProcessorArchitecture) {
            case PROCESSOR_ARCHITECTURE_AMD64:
                return SArchitecture::x64;
            case PROCESSOR_ARCHITECTURE_INTEL:
                return SArchitecture::x86;
            case PROCESSOR_ARCHITECTURE_ARM:
                return SArchitecture::ARM32;
            case PROCESSOR_ARCHITECTURE_ARM64:
                return SArchitecture::ARM64;
            default:
                return SArchitecture::Unknown;
        }
    }

    uint64_t WindowsOS::GetTotalMemory() const {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            return static_cast<uint64_t>(memInfo.ullTotalPhys);
        }
        return 0;
    }

    uint64_t WindowsOS::GetAvailableMemory() const {
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        if (GlobalMemoryStatusEx(&memInfo)) {
            return static_cast<uint64_t>(memInfo.ullAvailPhys);
        }
        return 0;
    }

    uint32_t WindowsOS::GetCpuCoreCount() const {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        return static_cast<uint32_t>(sysInfo.dwNumberOfProcessors);
    }

    uint64_t WindowsOS::GetTicks() const {
        LARGE_INTEGER frequency;
        LARGE_INTEGER counter;
        
        if (QueryPerformanceFrequency(&frequency) && QueryPerformanceCounter(&counter)) {
            // Convert to milliseconds
            return static_cast<uint64_t>((counter.QuadPart * 1000) / frequency.QuadPart);
        }
        
        // Fallback to GetTickCount64
        return GetTickCount64();
    }

    uint64_t WindowsOS::GetTicksMicro() const {
        LARGE_INTEGER frequency;
        LARGE_INTEGER counter;
        
        if (QueryPerformanceFrequency(&frequency) && QueryPerformanceCounter(&counter)) {
            // Convert to microseconds
            return static_cast<uint64_t>((counter.QuadPart * 1000000) / frequency.QuadPart);
        }
        
        // Fallback
        return GetTicks() * 1000;
    }

    uint64_t WindowsOS::GetTicksNano() const {
        LARGE_INTEGER frequency;
        LARGE_INTEGER counter;
        
        if (QueryPerformanceFrequency(&frequency) && QueryPerformanceCounter(&counter)) {
            // Convert to nanoseconds
            return static_cast<uint64_t>((counter.QuadPart * 1000000000ULL) / frequency.QuadPart);
        }
        
        // Fallback
        return GetTicksMicro() * 1000;
    }

    uint64_t WindowsOS::GetUnixTime() const {
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        
        // Convert FILETIME to Unix timestamp
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        
        // FILETIME is in 100-nanosecond intervals since January 1, 1601
        // Unix time is seconds since January 1, 1970
        // Difference is 11644473600 seconds
        return (uli.QuadPart / 10000000ULL) - 11644473600ULL;
    }

    uint64_t WindowsOS::GetUnixTimeMs() const {
        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        
        ULARGE_INTEGER uli;
        uli.LowPart = ft.dwLowDateTime;
        uli.HighPart = ft.dwHighDateTime;
        
        // Convert to milliseconds and adjust for Unix epoch
        return (uli.QuadPart / 10000ULL) - 11644473600000ULL;
    }

    void WindowsOS::Sleep(uint32_t milliseconds) {
        ::Sleep(milliseconds);
    }

    void WindowsOS::SleepMicro(uint32_t microseconds) {
        // Windows doesn't have microsecond sleep, use multimedia timer for better precision
        HANDLE timer = CreateWaitableTimer(nullptr, TRUE, nullptr);
        if (timer) {
            LARGE_INTEGER dueTime;
            dueTime.QuadPart = -static_cast<LONGLONG>(microseconds * 10); // Convert to 100ns intervals
            SetWaitableTimer(timer, &dueTime, 0, nullptr, nullptr, FALSE);
            WaitForSingleObject(timer, INFINITE);
            CloseHandle(timer);
        } else {
            // Fallback to millisecond sleep
            Sleep(microseconds / 1000);
        }
    }

    uint64_t WindowsOS::GetCpuFrequency() const {
        LARGE_INTEGER frequency;
        if (QueryPerformanceFrequency(&frequency)) {
            return static_cast<uint64_t>(frequency.QuadPart);
        }
        return 0;
    }

    IInput* WindowsOS::GetInput() {
        if (!m_input) {
            if (!InitializeInput()) {
                ConsolePrint("[OS_MESSAGE] Failed to get input system!\n");
                return nullptr;
            }
        }
        return m_input.get();
    }

    bool WindowsOS::InitializeInput() {
        if (m_input) {
            return true; // Already initialized
        }

        if (!m_context) {
            ConsolePrint("[OS_MESSAGE] Cannot initialize input without context\n");
            return false;
        }

        try {
            m_input = std::make_unique<WindowsInput>();
            
            // Get the HWND from the context and set it
            HWND hwnd = static_cast<HWND>(m_context->GetNativeWindowHandle());
            if (hwnd) {
                m_input->SetWindowHandle(hwnd);
            }
            
            if (!m_input->Initialize()) {
                ConsolePrint("[OS_MESSAGE] Failed to initialize Windows input system\n");
                m_input.reset();
                return false;
            }
            
            // IMPORTANT: Register the input system with the context so it receives events
            m_context->RegisterInputSystem(m_input.get());
            
            ConsolePrint("[OS_MESSAGE] Windows input system initialized successfully\n");
            return true;
        } catch (const std::exception& e) {
            ConsolePrint("[OS_MESSAGE] Exception during input initialization: ");
            ConsolePrint(e[OS_MESSAGE] .what());
            ConsolePrint("[OS_MESSAGE] \n");
            m_input.reset();
            return false;
        }
    }

    // Method to process window messages through input system
    bool WindowsOS::ProcessInputMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        if (m_input) {
            return m_input->ProcessWindowMessage(message, wParam, lParam);
        }
        return false;
    }

    // Method to update input state
    void WindowsOS::UpdateInput() {
        if (m_input) {
            m_input->Update();
        }
    }

}

#endif // VEK_WINDOWS
