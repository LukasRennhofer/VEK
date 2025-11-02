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

#include <VEK/Platform/Impl/Windows/VPL_WindowsContext.hpp>
#include <VEK/Platform/Impl/Windows/VPL_WindowsInput.hpp>

#include <glad/glad.h>
#include <iostream>
#include <cassert>

namespace VEK::Platform {

    WindowsContext::WindowsContext() {
        m_hInstance = GetModuleHandle(nullptr);
    }

    WindowsContext::~WindowsContext() {
        DestroyWindow();
    }

    LRESULT CALLBACK WindowsContext::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        WindowsContext* context = reinterpret_cast<WindowsContext*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        
        // Forward input messages to input system first
        if (context && context->m_inputSystem) {
            if (context->m_inputSystem->ProcessWindowMessage(uMsg, wParam, lParam)) {
                // Message was handled by input system
                // Continue to let context handle it too for compatibility
            }
        }
        
        switch (uMsg) {
            case WM_CLOSE:
                if (context) {
                    context->SetShouldClose(true);
                }
                return 0;
                
            case WM_SIZE:
                if (context) {
                    context->m_width = LOWORD(lParam);
                    context->m_height = HIWORD(lParam);
                }
                return 0;
                
            case WM_SHOWWINDOW:
                if (context) {
                    context->m_visible = (wParam == TRUE);
                }
                return 0;
                
            default:
                return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
    }

    bool WindowsContext::CreateWindow(int width, int height, const Core::KSafeString<>& title) {
        m_width = width;
        m_height = height;
        m_windowTitle = title;

        // Register window class
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = m_hInstance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.lpszClassName = "VEKWindow";

        if (!RegisterClassEx(&wc)) {
            std::cerr << "[OS_MESSAGE] Failed to register window class\n";
            return false;
        }

        // Calculate window size including borders
        RECT windowRect = { 0, 0, width, height };
        AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, 0);

        // Create window
        m_hwnd = ::CreateWindowEx(
            0,
            "VEKWindow",
            title.c_str(),
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT,
            windowRect.right - windowRect.left,
            windowRect.bottom - windowRect.top,
            nullptr, nullptr,
            m_hInstance,
            nullptr
        );

        if (!m_hwnd) {
            std::cerr << "[OS_MESSAGE] Failed to create window\n";
            return false;
        }

        // Store this instance in window user data
        SetWindowLongPtr(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

        // Get device context
        m_hdc = GetDC(m_hwnd);
        if (!m_hdc) {
            std::cerr << "[OS_MESSAGE] Failed to get device context\n";
            return false;
        }

        return InitializeGraphicsContext();
    }

    void WindowsContext::DestroyWindow() {
        DestroyGraphicsContext();
        
        if (m_hdc) {
            ReleaseDC(m_hwnd, m_hdc);
            m_hdc = nullptr;
        }
        
        if (m_hwnd) {
            ::DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
        
        UnregisterClass("VEKWindow", m_hInstance);
    }

    void WindowsContext::GetWindowSize(int& width, int& height) const {
        if (m_hwnd) {
            RECT rect;
            GetClientRect(m_hwnd, &rect);
            width = rect.right - rect.left;
            height = rect.bottom - rect.top;
        } else {
            width = m_width;
            height = m_height;
        }
    }

    void WindowsContext::SetWindowSize(int width, int height) {
        if (m_hwnd) {
            RECT windowRect = { 0, 0, width, height };
            AdjustWindowRectEx(&windowRect, GetWindowLong(m_hwnd, GWL_STYLE), FALSE, 0);
            
            ::SetWindowPos(m_hwnd, nullptr, 0, 0, 
                windowRect.right - windowRect.left,
                windowRect.bottom - windowRect.top,
                SWP_NOMOVE | SWP_NOZORDER);
        }
        m_width = width;
        m_height = height;
    }

    void WindowsContext::GetWindowPos(int& x, int& y) const {
        if (m_hwnd) {
            RECT rect;
            GetWindowRect(m_hwnd, &rect);
            x = rect.left;
            y = rect.top;
        } else {
            x = 0;
            y = 0;
        }
    }

    void WindowsContext::SetWindowPos(int x, int y) {
        if (m_hwnd) {
            ::SetWindowPos(m_hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
        }
    }

    void WindowsContext::SetWindowTitle(const Core::KSafeString<>& title) {
        m_windowTitle = title;
        if (m_hwnd) {
            SetWindowText(m_hwnd, title.c_str());
        }
    }

    void WindowsContext::SetWindowFullscreen(bool fullscreen) {
        if (m_fullscreen == fullscreen) return;
        
        m_fullscreen = fullscreen;
        
        if (fullscreen) {
            // Store current window position and size
            GetWindowPos(m_width, m_height);  // Reuse for storage
            
            // Set fullscreen style
            SetWindowLong(m_hwnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
            
            // Get monitor dimensions
            HMONITOR hmon = MonitorFromWindow(m_hwnd, MONITOR_DEFAULTTONEAREST);
            MONITORINFO mi = { sizeof(mi) };
            GetMonitorInfo(hmon, &mi);
            
            ::SetWindowPos(m_hwnd, HWND_TOP,
                mi.rcMonitor.left, mi.rcMonitor.top,
                mi.rcMonitor.right - mi.rcMonitor.left,
                mi.rcMonitor.bottom - mi.rcMonitor.top,
                SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        } else {
            // Restore windowed style
            SetWindowLong(m_hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
            
            RECT windowRect = { 0, 0, m_width, m_height };
            AdjustWindowRectEx(&windowRect, WS_OVERLAPPEDWINDOW, FALSE, 0);
            
            ::SetWindowPos(m_hwnd, nullptr, 100, 100,
                windowRect.right - windowRect.left,
                windowRect.bottom - windowRect.top,
                SWP_NOZORDER | SWP_FRAMECHANGED);
        }
    }

    bool WindowsContext::IsWindowFocused() const {
        return m_hwnd && (GetForegroundWindow() == m_hwnd);
    }

    bool WindowsContext::IsWindowMinimized() const {
        return m_hwnd && IsIconic(m_hwnd);
    }

    void WindowsContext::ShowWindow() {
        if (m_hwnd) {
            ::ShowWindow(m_hwnd, SW_SHOW);
            m_visible = true;
        }
    }

    void WindowsContext::HideWindow() {
        if (m_hwnd) {
            ::ShowWindow(m_hwnd, SW_HIDE);
            m_visible = false;
        }
    }

    void WindowsContext::MinimizeWindow() {
        if (m_hwnd) {
            ::ShowWindow(m_hwnd, SW_MINIMIZE);
        }
    }

    void WindowsContext::MaximizeWindow() {
        if (m_hwnd) {
            ::ShowWindow(m_hwnd, SW_MAXIMIZE);
        }
    }

    void WindowsContext::RestoreWindow() {
        if (m_hwnd) {
            ::ShowWindow(m_hwnd, SW_RESTORE);
        }
    }

    bool WindowsContext::SetupPixelFormat() {
        PIXELFORMATDESCRIPTOR pfd = {};
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.nVersion = 1;
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cDepthBits = 24;
        pfd.cStencilBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int pixelFormat = ChoosePixelFormat(m_hdc, &pfd);
        if (pixelFormat == 0) {
            std::cerr << "[OS_MESSAGE] Failed to choose pixel format\n";
            return false;
        }

        if (!SetPixelFormat(m_hdc, pixelFormat, &pfd)) {
            std::cerr << "[OS_MESSAGE] Failed to set pixel format\n";
            return false;
        }

        return true;
    }

    bool WindowsContext::InitializeGraphicsContext() {
        if (!m_hdc || !SetupPixelFormat()) {
            return false;
        }

        m_glContext = wglCreateContext(m_hdc);
        if (!m_glContext) {
            std::cerr << "[OS_MESSAGE] Failed to create OpenGL context\n";
            return false;
        }

        if (!wglMakeCurrent(m_hdc, m_glContext)) {
            std::cerr << "[OS_MESSAGE] Failed to make OpenGL context current\n";
            wglDeleteContext(m_glContext);
            m_glContext = nullptr;
            return false;
        }

        // Initialize GLAD
        if (!gladLoadGL()) {
            std::cerr << "[OS_MESSAGE] Failed to initialize GLAD\n";
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(m_glContext);
            m_glContext = nullptr;
            return false;
        }

        return true;
    }

    void WindowsContext::DestroyGraphicsContext() {
        if (m_glContext) {
            wglMakeCurrent(nullptr, nullptr);
            wglDeleteContext(m_glContext);
            m_glContext = nullptr;
        }
    }

    void WindowsContext::SwapBuffers() {
        if (m_hdc) {
            ::SwapBuffers(m_hdc);
        }
    }

    void WindowsContext::SetVSync(bool enabled) {
        m_vsyncEnabled = enabled;
        // Note: VSync implementation would require WGL extensions
        // For now, we just store the state
    }

    bool WindowsContext::PollEvents() {
        ProcessMessages();
        return !m_shouldClose;
    }

    void WindowsContext::WaitEvents() {
        WaitMessage();
        ProcessMessages();
    }

    void WindowsContext::ProcessMessages() {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                m_shouldClose = true;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

} // namespace VEK::Platform

#endif // VEK_WINDOWS
