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

#include <VEK/Platform/Impl/Linux/VPL_LinuxContext.hpp>
#include <VEK/Platform/Impl/Linux/VPL_LinuxInput.hpp>
#include <glad/glad.h>
#include <GL/glx.h>
#include <iostream>
#include <cstring>
#include <cassert>

namespace VEK::Platform {

    LinuxContext::LinuxContext() {
        // Initialize X11 display
        m_display = XOpenDisplay(nullptr);
        if (!m_display) {
            std::cerr << "[OS_MESSAGE] Failed to open X11 display\n";
            return;
        }
        
        m_screen = DefaultScreen(m_display);
        m_rootWindow = RootWindow(m_display, m_screen);
        
        // Initialize window manager atoms
        m_wmDeleteWindow = XInternAtom(m_display, "WM_DELETE_WINDOW", False);
        m_wmState = XInternAtom(m_display, "_NET_WM_STATE", False);
        m_wmStateFullscreen = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);
    }

    LinuxContext::~LinuxContext() {
        DestroyWindow();
        
        if (m_display) {
            XCloseDisplay(m_display);
            m_display = nullptr;
        }
    }

    bool LinuxContext::SetupVisual() {
        // OpenGL attributes
        int attribs[] = {
            GLX_RGBA,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            GLX_DOUBLEBUFFER,
            None
        };
        
        m_visualInfo = glXChooseVisual(m_display, m_screen, attribs);
        if (!m_visualInfo) {
            std::cerr << "[OS_MESSAGE] Failed to choose visual\n";
            return false;
        }
        
        return true;
    }

    bool LinuxContext::CreateWindow(int width, int height, const Core::KSafeString<>& title) {
        if (!m_display) {
            return false;
        }
        
        m_width = width;
        m_height = height;
        m_windowTitle = title;
        
        // Window needs to be created with proper visual for OpenGL
        if (!SetupVisual()) {
            return false;
        }
        
        // Create colormap
        m_colormap = XCreateColormap(m_display, m_rootWindow, m_visualInfo->visual, AllocNone);
        
        // Window attributes
        XSetWindowAttributes swa;
        swa.colormap = m_colormap;
        swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | 
                        ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                        StructureNotifyMask | FocusChangeMask | VisibilityChangeMask;
        swa.background_pixmap = None;
        swa.border_pixel = 0;
        
        // Create window with OpenGL visual
        m_window = XCreateWindow(
            m_display, m_rootWindow,
            100, 100,  // Initial position
            width, height,
            0,  // Border width
            m_visualInfo->depth,
            InputOutput,
            m_visualInfo->visual,
            CWColormap | CWEventMask | CWBackPixmap | CWBorderPixel,
            &swa
        );
        
        if (!m_window) {
            std::cerr << "[OS_MESSAGE] Failed to create X11 window\n";
            return false;
        }
        
        // Set window to accept focus and keyboard input
        XSelectInput(m_display, m_window, 
                    ExposureMask | KeyPressMask | KeyReleaseMask | 
                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                    StructureNotifyMask | FocusChangeMask | VisibilityChangeMask |
                    EnterWindowMask | LeaveWindowMask);
        
        // Make sure the window can receive keyboard focus
        Atom wmTakeFocus = XInternAtom(m_display, "WM_TAKE_FOCUS", False);
        XSetWMProtocols(m_display, m_window, &wmTakeFocus, 1);
        
        // Set window title
        XStoreName(m_display, m_window, title.c_str());
        XSetIconName(m_display, m_window, title.c_str());
        
        // Set window manager protocols
        SetupWindowManager();
        
        // Show window
        XMapWindow(m_display, m_window);
        XFlush(m_display);
        
        return InitializeGraphicsContext();
        
        return true;
    }

    void LinuxContext::SetupWindowManager() {
        // Set WM_DELETE_WINDOW protocol
        XSetWMProtocols(m_display, m_window, &m_wmDeleteWindow, 1);
        
        // Set window class
        XClassHint classHint;
        classHint.res_name = const_cast<char*>("VEK");
        classHint.res_class = const_cast<char*>("VEK");
        XSetClassHint(m_display, m_window, &classHint);
        
        // Set size hints
        XSizeHints sizeHints;
        sizeHints.flags = PPosition | PSize;
        sizeHints.x = 100;
        sizeHints.y = 100;
        sizeHints.width = m_width;
        sizeHints.height = m_height;
        XSetWMNormalHints(m_display, m_window, &sizeHints);
    }

    void LinuxContext::DestroyWindow() {
        DestroyGraphicsContext();
        
        if (m_window) {
            XUnmapWindow(m_display, m_window);
            XDestroyWindow(m_display, m_window);
            m_window = 0;
        }
        
        if (m_colormap) {
            XFreeColormap(m_display, m_colormap);
            m_colormap = 0;
        }
        
        if (m_visualInfo) {
            XFree(m_visualInfo);
            m_visualInfo = nullptr;
        }
    }

    void LinuxContext::GetWindowSize(int& width, int& height) const {
        if (m_window) {
            Window root;
            int x, y;
            unsigned int w, h, border, depth;
            XGetGeometry(m_display, m_window, &root, &x, &y, &w, &h, &border, &depth);
            width = static_cast<int>(w);
            height = static_cast<int>(h);
        } else {
            width = m_width;
            height = m_height;
        }
    }

    void LinuxContext::SetWindowSize(int width, int height) {
        m_width = width;
        m_height = height;
        
        if (m_window) {
            XResizeWindow(m_display, m_window, width, height);
            XFlush(m_display);
        }
    }

    void LinuxContext::GetWindowPos(int& x, int& y) const {
        if (m_window) {
            Window child;
            XTranslateCoordinates(m_display, m_window, m_rootWindow, 0, 0, &x, &y, &child);
        } else {
            x = m_posX;
            y = m_posY;
        }
    }

    void LinuxContext::SetWindowPos(int x, int y) {
        m_posX = x;
        m_posY = y;
        
        if (m_window) {
            XMoveWindow(m_display, m_window, x, y);
            XFlush(m_display);
        }
    }

    void LinuxContext::SetWindowTitle(const Core::KSafeString<>& title) {
        m_windowTitle = title;
        
        if (m_window) {
            XStoreName(m_display, m_window, title.c_str());
            XSetIconName(m_display, m_window, title.c_str());
            XFlush(m_display);
        }
    }

    void LinuxContext::SetWindowFullscreen(bool fullscreen) {
        if (m_fullscreen == fullscreen) return;
        
        m_fullscreen = fullscreen;
        
        if (m_window) {
            XEvent xev;
            xev.type = ClientMessage;
            xev.xclient.window = m_window;
            xev.xclient.message_type = m_wmState;
            xev.xclient.format = 32;
            xev.xclient.data.l[0] = fullscreen ? 1 : 0;  // _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE
            xev.xclient.data.l[1] = m_wmStateFullscreen;
            xev.xclient.data.l[2] = 0;
            xev.xclient.data.l[3] = 1;  // Source indication (application)
            xev.xclient.data.l[4] = 0;
            
            XSendEvent(m_display, m_rootWindow, False,
                      SubstructureRedirectMask | SubstructureNotifyMask, &xev);
            XFlush(m_display);
        }
    }

    bool LinuxContext::IsWindowFocused() const {
        if (!m_window) return false;
        
        Window focused;
        int revert;
        XGetInputFocus(m_display, &focused, &revert);
        return focused == m_window;
    }

    bool LinuxContext::IsWindowMinimized() const {
        if (!m_window) return false;
        
        XWindowAttributes attrs;
        XGetWindowAttributes(m_display, m_window, &attrs);
        return attrs.map_state == IsUnmapped;
    }

    void LinuxContext::ShowWindow() {
        if (m_window) {
            XMapWindow(m_display, m_window);
            XFlush(m_display);
            m_visible = true;
        }
    }

    void LinuxContext::HideWindow() {
        if (m_window) {
            XUnmapWindow(m_display, m_window);
            XFlush(m_display);
            m_visible = false;
        }
    }

    void LinuxContext::MinimizeWindow() {
        if (m_window) {
            XIconifyWindow(m_display, m_window, m_screen);
            XFlush(m_display);
        }
    }

    void LinuxContext::MaximizeWindow() {
        if (m_window) {
            // Send maximize message to window manager
            XEvent xev;
            Atom wmStateMaxVert = XInternAtom(m_display, "_NET_WM_STATE_MAXIMIZED_VERT", False);
            Atom wmStateMaxHorz = XInternAtom(m_display, "_NET_WM_STATE_MAXIMIZED_HORZ", False);
            
            xev.type = ClientMessage;
            xev.xclient.window = m_window;
            xev.xclient.message_type = m_wmState;
            xev.xclient.format = 32;
            xev.xclient.data.l[0] = 1;  // _NET_WM_STATE_ADD
            xev.xclient.data.l[1] = wmStateMaxVert;
            xev.xclient.data.l[2] = wmStateMaxHorz;
            xev.xclient.data.l[3] = 1;
            xev.xclient.data.l[4] = 0;
            
            XSendEvent(m_display, m_rootWindow, False,
                      SubstructureRedirectMask | SubstructureNotifyMask, &xev);
            XFlush(m_display);
        }
    }

    void LinuxContext::RestoreWindow() {
        if (m_window) {
            XMapWindow(m_display, m_window);
            XRaiseWindow(m_display, m_window);
            XFlush(m_display);
        }
    }

    bool LinuxContext::InitializeGraphicsContext() {
        if (!m_display || !m_window || !m_visualInfo) {
            return false;
        }
        
        m_glContext = glXCreateContext(m_display, m_visualInfo, nullptr, GL_TRUE);
        if (!m_glContext) {
            std::cerr << "[OS_MESSAGE] Failed to create OpenGL context\n";
            return false;
        }
        
        if (!glXMakeCurrent(m_display, m_window, m_glContext)) {
            std::cerr << "[OS_MESSAGE] Failed to make OpenGL context current\n";
            glXDestroyContext(m_display, m_glContext);
            m_glContext = nullptr;
            return false;
        }
        
        // Initialize GLAD
        if (!gladLoadGL()) {
            std::cerr << "[OS_MESSAGE] Failed to initialize GLAD\n";
            glXMakeCurrent(m_display, None, nullptr);
            glXDestroyContext(m_display, m_glContext);
            m_glContext = nullptr;
            return false;
        }
        
        return true;
    }

    void LinuxContext::DestroyGraphicsContext() {
        if (m_glContext) {
            glXMakeCurrent(m_display, None, nullptr);
            glXDestroyContext(m_display, m_glContext);
            m_glContext = nullptr;
        }
    }

    void LinuxContext::SwapBuffers() {
        if (m_display && m_window) {
            glXSwapBuffers(m_display, m_window);
        }
    }

    void LinuxContext::SetVSync(bool enabled) {
        m_vsyncEnabled = enabled;
        // Note: VSync implementation would require GLX extensions
        // For now, we just store the state
    }

    bool LinuxContext::PollEvents() {
        ProcessMessages();
        return !m_shouldClose;
    }

    void LinuxContext::WaitEvents() {
        if (XPending(m_display) == 0) {
            // Wait for events if none are pending
            XEvent event;
            XNextEvent(m_display, &event);
            HandleEvent(event);
        }
        ProcessMessages();
    }

    void LinuxContext::ProcessMessages() {
        while (XPending(m_display)) {
            XEvent event;
            XNextEvent(m_display, &event);
            HandleEvent(event);
        }
    }

    void LinuxContext::HandleEvent(const XEvent& event) {
        // Forward to input system first if registered
        if (m_inputSystem) {
            m_inputSystem->ProcessX11Event(const_cast<XEvent*>(&event));
        }
        
        // Handle context-specific events
        switch (event.type) {
            case ClientMessage:
                if (static_cast<Atom>(event.xclient.data.l[0]) == m_wmDeleteWindow) {
                    m_shouldClose = true;
                }
                break;
                
            case ConfigureNotify:
                m_width = event.xconfigure.width;
                m_height = event.xconfigure.height;
                m_posX = event.xconfigure.x;
                m_posY = event.xconfigure.y;
                break;
                
            case MapNotify:
                m_visible = true;
                break;
                
            case UnmapNotify:
                m_visible = false;
                break;
                
            case DestroyNotify:
                m_shouldClose = true;
                break;
                
            default:
                // Handle other events as needed
                break;
        }
    }

}

#endif // VEK_LINUX
