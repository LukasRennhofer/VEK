/*
================================================================================
  VEK (Vantor Engine Kernel) - Used by Vantor Studios
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  “Order the chaos, frame the void — and call it a world.”
================================================================================
*/

#ifdef VEK_LINUX

#include <VEK/Platform/Impl/Linux/VPL_LinuxOS.hpp>

#include <iostream>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace VEK::Platform {

    LinuxOS::LinuxOS() {
        m_context = std::make_unique<LinuxContext>();
        m_input = nullptr; // Initialize on demand
    }

    LinuxOS::~LinuxOS() {
        Shutdown();
    }

    bool LinuxOS::Init() {
        if (m_initialized) {
            ConsolePrint("[OS_MESSAGE] LinuxOS already initialized\n");
            return false;
        }

        if (!m_context) {
            ConsolePrint("[OS_MESSAGE] Failed to create Linux context\n");
            return false;
        }

        m_initialized = true;
        ConsolePrint("[OS_MESSAGE] LinuxOS initialized successfully\n");
        return true;
    }

    void LinuxOS::Shutdown() {
        if (m_initialized) {
            if (m_input) {
                m_input->Shutdown();
                m_input.reset();
            }
            if (m_context) {
                m_context->DestroyWindow();
            }
            m_initialized = false;
            ConsolePrint("[OS_MESSAGE] LinuxOS shutdown complete\n");
        }
    }

    IInput* LinuxOS::GetInput() {
        if (!m_input) {
            if (!InitializeInput()) {
                ConsolePrint("[OS_MESSAGE] Failed to get input system!\n");
                return nullptr;
            }
        }
        return m_input.get();
    }

    bool LinuxOS::InitializeInput() {
        if (m_input) {
            return true; // Already initialized
        }

        if (!m_context) {
            ConsolePrint("[OS_MESSAGE] Cannot initialize input without context\n");
            return false;
        }

        try {
            m_input = std::make_unique<LinuxInput>();
            
            // Set X11 display and window from context if available
            Display* display = static_cast<Display*>(m_context->GetNativeDisplayHandle());
            Window window = static_cast<Window>(reinterpret_cast<uintptr_t>(m_context->GetNativeWindowHandle()));
            
            if (display && window) {
                m_input->SetX11Window(display, window);
            }
            
            if (!m_input->Initialize()) {
                ConsolePrint("[OS_MESSAGE] Failed to initialize Linux input system\n");
                m_input.reset();
                return false;
            }
            
            // IMPORTANT: Register the input system with the context so it receives events
            m_context->RegisterInputSystem(m_input.get());
            
            ConsolePrint("[OS_MESSAGE] Linux input system initialized successfully\n");
            return true;
        } catch (const std::exception& e) {
            ConsolePrint("[OS_MESSAGE] Exception during input initialization: ");
            ConsolePrint(e.what());
            ConsolePrint("\n");
            m_input.reset();
            return false;
        }
    }

    SArchitecture LinuxOS::GetArchitecture() const {
        struct utsname unameData;
        if (uname(&unameData) == 0) {
            std::string machine(unameData.machine);
            
            if (machine == "x86_64" || machine == "amd64") {
                return SArchitecture::x64;
            } else if (machine == "i386" || machine == "i686") {
                return SArchitecture::x86;
            } else if (machine == "armv7l" || machine == "armv6l") {
                return SArchitecture::ARM32;
            } else if (machine == "aarch64" || machine == "arm64") {
                return SArchitecture::ARM64;
            }
        }
        return SArchitecture::Unknown;
    }

    void LinuxOS::ConsolePrint(const char* text) {
        if (!text) return;
        printf("%s", text);
    }

    void LinuxOS::ConsolePrintF(const char* format, ...) {
        if (!format) return;
        
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    void LinuxOS::ConsoleClear() {
        // Try to use ANSI escape codes
        printf("\033[2J\033[H");
        fflush(stdout);
    }

    void LinuxOS::ConsoleFlush() {
        fflush(stdout);
    }

    void LinuxOS::ConsoleSetColor(uint8_t r, uint8_t g, uint8_t b) {
        // Use ANSI escape codes for 24-bit color
        printf("\033[38;2;%u;%u;%um", r, g, b);
    }

    void LinuxOS::ConsoleResetColor() {
        printf("\033[0m");
    }

    uint64_t LinuxOS::GetTotalMemory() const {
        std::ifstream meminfo("/proc/meminfo");
        std::string line;
        
        while (std::getline(meminfo, line)) {
            if (line.substr(0, 8) == "MemTotal") {
                std::istringstream iss(line);
                std::string label;
                uint64_t value;
                std::string unit;
                
                if (iss >> label >> value >> unit) {
                    return value * 1024; // Convert from KB to bytes
                }
                break;
            }
        }
        return 0;
    }

    uint64_t LinuxOS::GetAvailableMemory() const {
        std::ifstream meminfo("/proc/meminfo");
        std::string line;
        
        while (std::getline(meminfo, line)) {
            if (line.substr(0, 12) == "MemAvailable") {
                std::istringstream iss(line);
                std::string label;
                uint64_t value;
                std::string unit;
                
                if (iss >> label >> value >> unit) {
                    return value * 1024; // Convert from KB to bytes
                }
                break;
            }
        }
        
        // Fallback: try MemFree
        meminfo.clear();
        meminfo.seekg(0, std::ios::beg);
        
        while (std::getline(meminfo, line)) {
            if (line.substr(0, 7) == "MemFree") {
                std::istringstream iss(line);
                std::string label;
                uint64_t value;
                std::string unit;
                
                if (iss >> label >> value >> unit) {
                    return value * 1024; // Convert from KB to bytes
                }
                break;
            }
        }
        
        return 0;
    }

    uint32_t LinuxOS::GetCpuCoreCount() const {
        return static_cast<uint32_t>(sysconf(_SC_NPROCESSORS_ONLN));
    }

    uint64_t LinuxOS::GetTicks() const {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            return static_cast<uint64_t>(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        }
        return 0;
    }

    uint64_t LinuxOS::GetTicksMicro() const {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            return static_cast<uint64_t>(ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
        }
        return 0;
    }

    uint64_t LinuxOS::GetTicksNano() const {
        struct timespec ts;
        if (clock_gettime(CLOCK_MONOTONIC, &ts) == 0) {
            return static_cast<uint64_t>(ts.tv_sec * 1000000000ULL + ts.tv_nsec);
        }
        return 0;
    }

    uint64_t LinuxOS::GetUnixTime() const {
        return static_cast<uint64_t>(time(nullptr));
    }

    uint64_t LinuxOS::GetUnixTimeMs() const {
        struct timespec ts;
        if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
            return static_cast<uint64_t>(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
        }
        return GetUnixTime() * 1000;
    }

    void LinuxOS::Sleep(uint32_t milliseconds) {
        usleep(milliseconds * 1000);
    }

    void LinuxOS::SleepMicro(uint32_t microseconds) {
        usleep(microseconds);
    }

    uint64_t LinuxOS::GetCpuFrequency() const {
        std::ifstream cpuinfo("/proc/cpuinfo");
        std::string line;
        
        while (std::getline(cpuinfo, line)) {
            if (line.substr(0, 7) == "cpu MHz") {
                size_t colonPos = line.find(':');
                if (colonPos != std::string::npos) {
                    std::string valueStr = line.substr(colonPos + 1);
                    double frequency = std::stod(valueStr);
                    return static_cast<uint64_t>(frequency * 1000000); // Convert MHz to Hz
                }
                break;
            }
        }
        return 0;
    }

    // Method to process X11 events through input system
    bool LinuxOS::ProcessInputEvent(XEvent* event) {
        if (m_input && event) {
            return m_input->ProcessX11Event(event);
        }
        return false;
    }

    // Method to update input state
    void LinuxOS::UpdateInput() {
        if (m_input) {
            m_input->Update();
        }
    }

} // namespace VEK::Platform

#endif // VEK_LINUX
