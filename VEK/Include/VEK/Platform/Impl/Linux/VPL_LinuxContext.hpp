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

#include <VEK/Platform/VPL_Context.hpp>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

// Forward declarations for GLX types
typedef struct __GLXcontextRec *GLXContext;
typedef XID GLXDrawable;

// Forward declaration
namespace VEK::Platform {
    class LinuxInput;
}

namespace VEK::Platform {

    class LinuxContext : public IContext {
    private:
        Display* m_display = nullptr;
        Window m_window = 0;
        Window m_rootWindow = 0;
        GLXContext m_glContext = nullptr;
        XVisualInfo* m_visualInfo = nullptr;
        Colormap m_colormap = 0;
        
        int m_screen = 0;
        int m_width = 0;
        int m_height = 0;
        int m_posX = 0;
        int m_posY = 0;
        
        bool m_fullscreen = false;
        bool m_vsyncEnabled = false;
        bool m_shouldClose = false;
        bool m_visible = true;
        
        Core::KSafeString<> m_windowTitle;
        
        // Window state tracking
        Atom m_wmDeleteWindow;
        Atom m_wmState;
        Atom m_wmStateFullscreen;
        
        bool SetupVisual();
        bool CreateGLContext();
        void SetupWindowManager();
        
    public:
        LinuxContext();
        virtual ~LinuxContext();

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
        void* GetNativeWindowHandle() const override { return reinterpret_cast<void*>(m_window); }
        void* GetNativeDisplayHandle() const override { return m_display; }
        void* GetGraphicsContextHandle() const override { return m_glContext; }

        // Utility
        void ProcessMessages() override;
        bool ShouldClose() const override { return m_shouldClose; }
        void SetShouldClose(bool shouldClose) override { m_shouldClose = shouldClose; }
        
        // Input system integration
        void RegisterInputSystem(class LinuxInput* input) { m_inputSystem = input; }
        
    private:
        void HandleEvent(const XEvent& event);
        
        // Input system reference for forwarding events
        class LinuxInput* m_inputSystem = nullptr;
    };

} // namespace VEK::Platform

#endif // VEK_LINUX
