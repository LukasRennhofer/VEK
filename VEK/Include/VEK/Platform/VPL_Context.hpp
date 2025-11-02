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
#include <cstdint>
#include <functional>

namespace VEK::Platform {

    /// Low-level platform context interface for window and graphics operations
    class IContext {
    public:
        virtual ~IContext() = default;

        // Window Management
        virtual bool CreateWindow(int width, int height, const Core::KSafeString<>& title) = 0;
        virtual void DestroyWindow() = 0;
        
        virtual void GetWindowSize(int& width, int& height) const = 0;
        virtual void SetWindowSize(int width, int height) = 0;
        
        virtual void GetWindowPos(int& x, int& y) const = 0;
        virtual void SetWindowPos(int x, int y) = 0;
        
        virtual void SetWindowTitle(const Core::KSafeString<>& title) = 0;
        
        virtual bool IsWindowFullscreen() const = 0;
        virtual void SetWindowFullscreen(bool fullscreen) = 0;
        
        virtual bool IsWindowFocused() const = 0;
        virtual bool IsWindowMinimized() const = 0;
        virtual bool IsWindowVisible() const = 0;
        
        virtual void ShowWindow() = 0;
        virtual void HideWindow() = 0;
        virtual void MinimizeWindow() = 0;
        virtual void MaximizeWindow() = 0;
        virtual void RestoreWindow() = 0;

        // Graphics Context - OpenGL Only for now
        virtual bool InitializeGraphicsContext() = 0;
        virtual void DestroyGraphicsContext() = 0;
        virtual void SwapBuffers() = 0;
        virtual void SetVSync(bool enabled) = 0;
        virtual bool IsVSyncEnabled() const = 0;

        // Event Processing
        virtual bool PollEvents() = 0;  // Returns false when should quit
        virtual void WaitEvents() = 0;

        // Platform-specific handles (for advanced usage)
        virtual void* GetNativeWindowHandle() const = 0;
        virtual void* GetNativeDisplayHandle() const = 0;
        virtual void* GetGraphicsContextHandle() const = 0;

        // Utility
        virtual void ProcessMessages() = 0;
        virtual bool ShouldClose() const = 0;
        virtual void SetShouldClose(bool shouldClose) = 0;
    };

} // namespace VEK::Platform