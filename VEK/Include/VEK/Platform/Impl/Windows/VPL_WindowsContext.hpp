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

#ifdef VEK_WINDOWS

#include <VEK/Platform/VPL_Context.hpp>

#include <windows.h>

// Undefine problematic Windows macros
#ifdef CreateWindow
#undef CreateWindow
#endif

// Forward declaration
namespace VEK::Platform {
    class WindowsInput;
}

namespace VEK::Platform {

    class WindowsContext : public IContext {
    private:
        HWND m_hwnd = nullptr;
        HDC m_hdc = nullptr;
        HGLRC m_glContext = nullptr;
        HINSTANCE m_hInstance = nullptr;
        
        int m_width = 0;
        int m_height = 0;
        bool m_fullscreen = false;
        bool m_vsyncEnabled = false;
        bool m_shouldClose = false;
        bool m_visible = true;
        
        Core::KSafeString<> m_windowTitle;
        
        static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
        bool SetupPixelFormat();
        
    public:
        WindowsContext();
        virtual ~WindowsContext();

        // Window Management
        bool CreateWindow(int width, int height, const Core::KSafeString<>& title) override;
        void DestroyWindow() override;
        
        void GetWindowSize(int& width, int& height) const override;
        void SetWindowSize(int width, int height) override;
        
        void GetWindowPos(int& x, int& y) const override;
        void SetWindowPos(int x, int y) override;
        
        void SetWindowTitle(const Core::KSafeString<>& title) override;
        
        bool IsWindowFullscreen() const override { return m_fullscreen; }
        void SetWindowFullscreen(bool fullscreen) override;
        
        bool IsWindowFocused() const override;
        bool IsWindowMinimized() const override;
        bool IsWindowVisible() const override { return m_visible; }
        
        void ShowWindow() override;
        void HideWindow() override;
        void MinimizeWindow() override;
        void MaximizeWindow() override;
        void RestoreWindow() override;

        // Graphics Context
        bool InitializeGraphicsContext() override;
        void DestroyGraphicsContext() override;
        void SwapBuffers() override;
        void SetVSync(bool enabled) override;
        bool IsVSyncEnabled() const override { return m_vsyncEnabled; }

        // Event Processing
        bool PollEvents() override;
        void WaitEvents() override;

        // Platform-specific handles
        void* GetNativeWindowHandle() const override { return m_hwnd; }
        void* GetNativeDisplayHandle() const override { return m_hdc; }
        void* GetGraphicsContextHandle() const override { return m_glContext; }

        // Utility
        void ProcessMessages() override;
        bool ShouldClose() const override { return m_shouldClose; }
        void SetShouldClose(bool shouldClose) override { m_shouldClose = shouldClose; }
        
        // Input system integration
        void RegisterInputSystem(class WindowsInput* input) { m_inputSystem = input; }

    private:
        // Input system reference
        class WindowsInput* m_inputSystem = nullptr;
    };

} // namespace VEK::Platform

#endif // VEK_WINDOWS
