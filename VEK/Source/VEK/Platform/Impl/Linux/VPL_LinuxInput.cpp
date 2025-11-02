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

#include <VEK/Platform/Impl/Linux/VPL_LinuxInput.hpp>

#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cstdio>
#include <chrono>

namespace VEK::Platform {

    LinuxInput::LinuxInput() {
        InitializeKeyMappings();
    }

    LinuxInput::~LinuxInput() {
        if (m_initialized) {
            Shutdown();
        }
    }

    bool LinuxInput::Initialize() {
        if (m_initialized) {
            return true;
        }

        // Initialize X11 connection first (this is safer)
        if (!InitializeX11()) {
            return false;
        }

        // Initialize input devices (optional, can fail safely)
        InitializeDevices();

        // Start input processing thread only if we have some form of input
        m_shouldStop = false;
        m_inputThread = std::thread(&LinuxInput::InputThreadFunction, this);

        // Scan for gamepads (optional, can fail safely)
        try {
            ScanForGamepads();
        } catch (...) {
            // Gamepad scanning failed, continue without gamepads
        }

        m_initialized = true;
        return true;
    }

    void LinuxInput::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Stop input thread
        m_shouldStop = true;
        if (m_inputThread.joinable()) {
            m_inputThread.join();
        }

        // Shutdown devices
        ShutdownDevices();

        // Shutdown X11
        ShutdownX11();

        m_initialized = false;
    }

    void LinuxInput::Update() {
        if (!m_initialized) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        
        // Convert Pressed states from PREVIOUS frame to Held states
        // This allows the current frame to see Pressed states, but converts
        // them to Held for the next frame
        for (size_t i = 0; i < m_keyboard.keys.size(); ++i) {
            if (m_keyboard.previousKeys[i] == InputState::Pressed) {
                // If key was pressed last frame, check if it's still pressed
                if (m_keyboard.keys[i] == InputState::Pressed) {
                    m_keyboard.keys[i] = InputState::Held;
                }
            }
        }
        
        for (size_t i = 0; i < m_mouse.buttons.size(); ++i) {
            if (m_mouse.previousButtons[i] == InputState::Pressed) {
                // If button was pressed last frame, check if it's still pressed
                if (m_mouse.buttons[i] == InputState::Pressed) {
                    m_mouse.buttons[i] = InputState::Held;
                }
            }
        }
        
        // Copy current states to previous states for next frame
        m_keyboard.previousKeys = m_keyboard.keys;
        m_mouse.previousButtons = m_mouse.buttons;
        
        // Update mouse delta
        m_mouse.deltaX = m_mouse.x - m_mouse.lastX;
        m_mouse.deltaY = m_mouse.y - m_mouse.lastY;
        m_mouse.lastX = m_mouse.x;
        m_mouse.lastY = m_mouse.y;
    }

    // Keyboard input implementation
    bool LinuxInput::IsKeyPressed(KeyCode key) const {
        return GetKeyState(key) == InputState::Pressed;
    }

    bool LinuxInput::IsKeyReleased(KeyCode key) const {
        return GetKeyState(key) == InputState::Released;
    }

    bool LinuxInput::IsKeyHeld(KeyCode key) const {
        return GetKeyState(key) == InputState::Held;
    }

    InputState LinuxInput::GetKeyState(KeyCode key) const {
        if (!m_initialized) {
            return InputState::Released;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        uint16_t keyIndex = static_cast<uint16_t>(key);
        if (keyIndex >= MAX_KEYS) {
            return InputState::Released;
        }

        return m_keyboard.keys[keyIndex];
    }

    // Mouse input implementation
    bool LinuxInput::IsMouseButtonPressed(MouseButton button) const {
        return GetMouseButtonState(button) == InputState::Pressed;
    }

    bool LinuxInput::IsMouseButtonReleased(MouseButton button) const {
        return GetMouseButtonState(button) == InputState::Released;
    }

    bool LinuxInput::IsMouseButtonHeld(MouseButton button) const {
        return GetMouseButtonState(button) == InputState::Held;
    }

    InputState LinuxInput::GetMouseButtonState(MouseButton button) const {
        if (!m_initialized) {
            return InputState::Released;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        size_t buttonIndex = static_cast<size_t>(button);
        if (buttonIndex >= MAX_MOUSE_BUTTONS) {
            return InputState::Released;
        }

        return m_mouse.buttons[buttonIndex];
    }

    void LinuxInput::GetMousePosition(int32_t& x, int32_t& y) const {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        x = m_mouse.x;
        y = m_mouse.y;
    }

    void LinuxInput::GetMouseDelta(int32_t& deltaX, int32_t& deltaY) const {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        deltaX = m_mouse.deltaX;
        deltaY = m_mouse.deltaY;
    }

    void LinuxInput::SetMousePosition(int32_t x, int32_t y) {
        if (!m_initialized || !m_display || m_window == None) {
            return;
        }

        XWarpPointer(m_display, None, m_window, 0, 0, 0, 0, x, y);
        XFlush(m_display);

        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_mouse.x = x;
        m_mouse.y = y;
    }

    void LinuxInput::SetMouseVisible(bool visible) {
        if (!m_initialized || !m_display || m_window == None) {
            // Can't change cursor visibility without X11 connection
            m_mouse.visible = visible; // Store the desired state
            return;
        }

        if (visible != m_mouse.visible) {
            if (visible) {
                XUndefineCursor(m_display, m_window);
            } else {
                // Create invisible cursor
                Pixmap pixmap = XCreatePixmap(m_display, m_window, 1, 1, 1);
                XColor color = {0, 0, 0, 0, 0, 0};
                Cursor cursor = XCreatePixmapCursor(m_display, pixmap, pixmap, &color, &color, 0, 0);
                XDefineCursor(m_display, m_window, cursor);
                XFreeCursor(m_display, cursor);
                XFreePixmap(m_display, pixmap);
            }
            XFlush(m_display);
            m_mouse.visible = visible;
        }
    }

    bool LinuxInput::IsMouseVisible() const {
        return m_mouse.visible;
    }

    // Gamepad input implementation
    uint8_t LinuxInput::GetConnectedGamepadCount() const {
        return m_connectedGamepadCount;
    }

    bool LinuxInput::IsGamepadConnected(uint8_t gamepadId) const {
        if (gamepadId >= MAX_GAMEPADS) {
            return false;
        }
        return m_gamepads[gamepadId].connected;
    }

    const GamepadState* LinuxInput::GetGamepadState(uint8_t gamepadId) const {
        if (gamepadId >= MAX_GAMEPADS || !m_gamepads[gamepadId].connected) {
            return nullptr;
        }
        return &m_gamepads[gamepadId].state;
    }

    bool LinuxInput::IsGamepadButtonPressed(uint8_t gamepadId, GamepadButton button) const {
        return GetGamepadButtonState(gamepadId, button) == InputState::Pressed;
    }

    bool LinuxInput::IsGamepadButtonReleased(uint8_t gamepadId, GamepadButton button) const {
        return GetGamepadButtonState(gamepadId, button) == InputState::Released;
    }

    bool LinuxInput::IsGamepadButtonHeld(uint8_t gamepadId, GamepadButton button) const {
        return GetGamepadButtonState(gamepadId, button) == InputState::Held;
    }

    InputState LinuxInput::GetGamepadButtonState(uint8_t gamepadId, GamepadButton button) const {
        if (gamepadId >= MAX_GAMEPADS || !m_gamepads[gamepadId].connected) {
            return InputState::Released;
        }

        size_t buttonIndex = static_cast<size_t>(button);
        if (buttonIndex >= static_cast<size_t>(GamepadButton::Count)) {
            return InputState::Released;
        }

        bool isPressed = m_gamepads[gamepadId].state.buttons[buttonIndex];
        return isPressed ? InputState::Held : InputState::Released;
    }

    float LinuxInput::GetGamepadAxis(uint8_t gamepadId, GamepadAxis axis) const {
        if (gamepadId >= MAX_GAMEPADS || !m_gamepads[gamepadId].connected) {
            return 0.0f;
        }

        size_t axisIndex = static_cast<size_t>(axis);
        if (axisIndex >= static_cast<size_t>(GamepadAxis::Count)) {
            return 0.0f;
        }

        float value = m_gamepads[gamepadId].state.axes[axisIndex];
        return ApplyDeadzone(value, m_gamepads[gamepadId].state.deadzone);
    }

    void LinuxInput::SetGamepadDeadzone(uint8_t gamepadId, float deadzone) {
        if (gamepadId >= MAX_GAMEPADS || !m_gamepads[gamepadId].connected) {
            return;
        }

        m_gamepads[gamepadId].state.deadzone = deadzone;
    }

    void LinuxInput::ClearEvents() {
        // Events are processed continuously, no need to clear
    }

    // Utility functions
    const char* LinuxInput::GetKeyName(KeyCode key) const {
        auto it = m_keyNames.find(key);
        return (it != m_keyNames.end()) ? it->second : "Unknown";
    }

    const char* LinuxInput::GetMouseButtonName(MouseButton button) const {
        auto it = m_mouseButtonNames.find(button);
        return (it != m_mouseButtonNames.end()) ? it->second : "Unknown";
    }

    const char* LinuxInput::GetGamepadButtonName(GamepadButton button) const {
        auto it = m_gamepadButtonNames.find(button);
        return (it != m_gamepadButtonNames.end()) ? it->second : "Unknown";
    }

    // Linux-specific methods
    void LinuxInput::SetX11Window(Display* display, Window window) {
        m_display = display;
        m_window = window;
        if (m_display) {
            m_screen = DefaultScreen(m_display);
        }
    }

    bool LinuxInput::ProcessX11Event(XEvent* event) {
        if (!event) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);

        switch (event->type) {
            case KeyPress:
            case KeyRelease: {
                KeyCode keyCode = LinuxScanCodeToKeyCode(event->xkey.keycode);
                bool pressed = (event->type == KeyPress);
                
                UpdateKeyState(keyCode, pressed);
                
                // Update modifier states
                m_keyboard.modifierStates[0] = (event->xkey.state & ShiftMask) != 0;    // Shift
                m_keyboard.modifierStates[1] = (event->xkey.state & ControlMask) != 0;  // Ctrl
                m_keyboard.modifierStates[2] = (event->xkey.state & Mod1Mask) != 0;     // Alt
                m_keyboard.modifierStates[3] = (event->xkey.state & Mod4Mask) != 0;     // Super
                break;
            }
            
            case ButtonPress:
            case ButtonRelease: {
                MouseButton button = LinuxButtonToMouseButton(event->xbutton.button);
                bool pressed = (event->type == ButtonPress);
                
                if (button != MouseButton::Count) {
                    UpdateMouseButtonState(button, pressed);
                }
                
                m_mouse.x = event->xbutton.x;
                m_mouse.y = event->xbutton.y;
                break;
            }
            
            case MotionNotify: {
                m_mouse.x = event->xmotion.x;
                m_mouse.y = event->xmotion.y;
                break;
            }
            
            default:
                return false;
        }

        return true;
    }

    // Private methods implementation
    void LinuxInput::InitializeKeyMappings() {
        // Initialize X11 keycode to KeyCode mapping (these are X11 keycodes, not scancodes)
        m_scancodeToKeyCode[38] = KeyCode::A;    // A
        m_scancodeToKeyCode[56] = KeyCode::B;    // B
        m_scancodeToKeyCode[54] = KeyCode::C;    // C
        m_scancodeToKeyCode[40] = KeyCode::D;    // D
        m_scancodeToKeyCode[26] = KeyCode::E;    // E
        m_scancodeToKeyCode[41] = KeyCode::F;    // F
        m_scancodeToKeyCode[42] = KeyCode::G;    // G
        m_scancodeToKeyCode[43] = KeyCode::H;    // H
        m_scancodeToKeyCode[31] = KeyCode::I;    // I
        m_scancodeToKeyCode[44] = KeyCode::J;    // J
        m_scancodeToKeyCode[45] = KeyCode::K;    // K
        m_scancodeToKeyCode[46] = KeyCode::L;    // L
        m_scancodeToKeyCode[58] = KeyCode::M;    // M
        m_scancodeToKeyCode[57] = KeyCode::N;    // N
        m_scancodeToKeyCode[32] = KeyCode::O;    // O
        m_scancodeToKeyCode[33] = KeyCode::P;    // P
        m_scancodeToKeyCode[24] = KeyCode::Q;    // Q
        m_scancodeToKeyCode[27] = KeyCode::R;    // R
        m_scancodeToKeyCode[39] = KeyCode::S;    // S
        m_scancodeToKeyCode[28] = KeyCode::T;    // T
        m_scancodeToKeyCode[30] = KeyCode::U;    // U
        m_scancodeToKeyCode[55] = KeyCode::V;    // V
        m_scancodeToKeyCode[25] = KeyCode::W;    // W
        m_scancodeToKeyCode[53] = KeyCode::X;    // X
        m_scancodeToKeyCode[29] = KeyCode::Y;    // Y
        m_scancodeToKeyCode[52] = KeyCode::Z;    // Z
        
        // Numbers
        m_scancodeToKeyCode[19] = KeyCode::Num0;
        m_scancodeToKeyCode[10] = KeyCode::Num1;
        m_scancodeToKeyCode[11] = KeyCode::Num2;
        m_scancodeToKeyCode[12] = KeyCode::Num3;
        m_scancodeToKeyCode[13] = KeyCode::Num4;
        m_scancodeToKeyCode[14] = KeyCode::Num5;
        m_scancodeToKeyCode[15] = KeyCode::Num6;
        m_scancodeToKeyCode[16] = KeyCode::Num7;
        m_scancodeToKeyCode[17] = KeyCode::Num8;
        m_scancodeToKeyCode[18] = KeyCode::Num9;
        
        // Special keys
        m_scancodeToKeyCode[9] = KeyCode::Escape;
        m_scancodeToKeyCode[65] = KeyCode::Space;
        m_scancodeToKeyCode[36] = KeyCode::Enter;
        m_scancodeToKeyCode[22] = KeyCode::Backspace;
        
        // Arrow keys
        m_scancodeToKeyCode[113] = KeyCode::Left;
        m_scancodeToKeyCode[114] = KeyCode::Right;
        m_scancodeToKeyCode[111] = KeyCode::Up;
        m_scancodeToKeyCode[116] = KeyCode::Down;

        // Initialize key names
        m_keyNames[KeyCode::A] = "A";
        m_keyNames[KeyCode::B] = "B";
        m_keyNames[KeyCode::C] = "C";
        m_keyNames[KeyCode::D] = "D";
        m_keyNames[KeyCode::E] = "E";
        m_keyNames[KeyCode::F] = "F";
        m_keyNames[KeyCode::G] = "G";
        m_keyNames[KeyCode::H] = "H";
        m_keyNames[KeyCode::I] = "I";
        m_keyNames[KeyCode::J] = "J";
        m_keyNames[KeyCode::K] = "K";
        m_keyNames[KeyCode::L] = "L";
        m_keyNames[KeyCode::M] = "M";
        m_keyNames[KeyCode::N] = "N";
        m_keyNames[KeyCode::O] = "O";
        m_keyNames[KeyCode::P] = "P";
        m_keyNames[KeyCode::Q] = "Q";
        m_keyNames[KeyCode::R] = "R";
        m_keyNames[KeyCode::S] = "S";
        m_keyNames[KeyCode::T] = "T";
        m_keyNames[KeyCode::U] = "U";
        m_keyNames[KeyCode::V] = "V";
        m_keyNames[KeyCode::W] = "W";
        m_keyNames[KeyCode::X] = "X";
        m_keyNames[KeyCode::Y] = "Y";
        m_keyNames[KeyCode::Z] = "Z";
        m_keyNames[KeyCode::Space] = "Space";
        m_keyNames[KeyCode::Enter] = "Enter";
        m_keyNames[KeyCode::Escape] = "Escape";
        m_keyNames[KeyCode::Left] = "Left Arrow";
        m_keyNames[KeyCode::Right] = "Right Arrow";
        m_keyNames[KeyCode::Up] = "Up Arrow";
        m_keyNames[KeyCode::Down] = "Down Arrow";

        // Initialize mouse button names
        m_mouseButtonNames[MouseButton::Left] = "Left Mouse Button";
        m_mouseButtonNames[MouseButton::Right] = "Right Mouse Button";
        m_mouseButtonNames[MouseButton::Middle] = "Middle Mouse Button";
        m_mouseButtonNames[MouseButton::X1] = "Mouse Button 4";
        m_mouseButtonNames[MouseButton::X2] = "Mouse Button 5";

        // Initialize gamepad button names
        m_gamepadButtonNames[GamepadButton::A] = "A";
        m_gamepadButtonNames[GamepadButton::B] = "B";
        m_gamepadButtonNames[GamepadButton::X] = "X";
        m_gamepadButtonNames[GamepadButton::Y] = "Y";
        m_gamepadButtonNames[GamepadButton::LeftBumper] = "Left Bumper";
        m_gamepadButtonNames[GamepadButton::RightBumper] = "Right Bumper";
        m_gamepadButtonNames[GamepadButton::Back] = "Back";
        m_gamepadButtonNames[GamepadButton::Start] = "Start";
        m_gamepadButtonNames[GamepadButton::Guide] = "Guide";
        m_gamepadButtonNames[GamepadButton::LeftThumb] = "Left Stick";
        m_gamepadButtonNames[GamepadButton::RightThumb] = "Right Stick";
        m_gamepadButtonNames[GamepadButton::DpadUp] = "D-Pad Up";
        m_gamepadButtonNames[GamepadButton::DpadRight] = "D-Pad Right";
        m_gamepadButtonNames[GamepadButton::DpadDown] = "D-Pad Down";
        m_gamepadButtonNames[GamepadButton::DpadLeft] = "D-Pad Left";
    }

    void LinuxInput::InitializeDevices() {
        // TODO: Write fallbacks
        m_keyboardFd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
        if (m_keyboardFd == -1) {
            // Keyboard device not available, input will still work via X11
        }
        
        m_mouseFd = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);
        if (m_mouseFd == -1) {
            // Mouse device not available, input will still work via X11
        }
    }

    void LinuxInput::ShutdownDevices() {
        // Close device file descriptors
        if (m_keyboardFd != -1) {
            close(m_keyboardFd);
            m_keyboardFd = -1;
        }
        
        if (m_mouseFd != -1) {
            close(m_mouseFd);
            m_mouseFd = -1;
        }

        // Disconnect all gamepads
        for (uint8_t i = 0; i < MAX_GAMEPADS; ++i) {
            if (m_gamepads[i].connected) {
                DisconnectGamepad(i);
            }
        }
    }

    void LinuxInput::InputThreadFunction() {
        while (!m_shouldStop) {
            try {
                ProcessKeyboardEvents();
                ProcessMouseEvents();
                UpdateGamepadStates();
            } catch (...) {
                // If any input processing fails, continue running
                // This prevents crashes from bringing down the entire input system
            }
            
            // Small sleep to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void LinuxInput::ProcessKeyboardEvents() {
        if (m_keyboardFd == -1) {
            return;
        }

        struct input_event ev;
        while (read(m_keyboardFd, &ev, sizeof(ev)) == sizeof(ev)) {
            if (ev.type == EV_KEY) {
                KeyCode keyCode = LinuxScanCodeToKeyCode(ev.code);
                bool pressed = (ev.value == 1);
                
                std::lock_guard<std::mutex> lock(m_stateMutex);
                UpdateKeyState(keyCode, pressed);
            }
        }
    }

    void LinuxInput::ProcessMouseEvents() {
        if (m_mouseFd == -1) {
            return;
        }

        struct input_event ev;
        while (read(m_mouseFd, &ev, sizeof(ev)) == sizeof(ev)) {
            std::lock_guard<std::mutex> lock(m_stateMutex);
            
            if (ev.type == EV_KEY) {
                MouseButton button = LinuxButtonToMouseButton(ev.code - BTN_MOUSE);
                if (button != MouseButton::Count) {
                    bool pressed = (ev.value == 1);
                    UpdateMouseButtonState(button, pressed);
                }
            } else if (ev.type == EV_REL) {
                if (ev.code == REL_X) {
                    m_mouse.x += ev.value;
                } else if (ev.code == REL_Y) {
                    m_mouse.y += ev.value;
                }
            }
        }
    }

    void LinuxInput::UpdateGamepadStates() {
        // Update timestamp for connected gamepads
        auto now = std::chrono::steady_clock::now();
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
        
        for (uint8_t i = 0; i < MAX_GAMEPADS; ++i) {
            if (m_gamepads[i].connected) {
                m_gamepads[i].state.lastUpdateTime = static_cast<uint32_t>(timestamp);
            }
        }
    }

    void LinuxInput::ScanForGamepads() {
        const char* inputPath = "/dev/input";
        DIR* dir = opendir(inputPath);
        if (!dir) {
            // Can't access input directory, skip gamepad detection
            return;
        }

        struct dirent* entry;
        uint8_t gamepadIndex = 0;
        
        while ((entry = readdir(dir)) && gamepadIndex < MAX_GAMEPADS) {
            if (entry && strncmp(entry->d_name, "js", 2) == 0) {
                char devicePath[256];
                int result = snprintf(devicePath, sizeof(devicePath), "%s/%s", inputPath, entry->d_name);
                if (result > 0 && result < static_cast<int>(sizeof(devicePath))) {
                    ConnectGamepad(gamepadIndex, devicePath);
                    ++gamepadIndex;
                }
            }
        }
        
        closedir(dir);
        m_connectedGamepadCount = gamepadIndex;
    }

    void LinuxInput::ConnectGamepad(uint8_t id, const char* devicePath) {
        if (id >= MAX_GAMEPADS || !devicePath) {
            return;
        }

        auto& gamepad = m_gamepads[id];
        gamepad.fd = open(devicePath, O_RDONLY | O_NONBLOCK);
        
        if (gamepad.fd != -1) {
            // Get gamepad name
            char name[256] = {0};
            if (ioctl(gamepad.fd, JSIOCGNAME(sizeof(name)), name) >= 0) {
                gamepad.name = name;
            } else {
                gamepad.name = "Unknown Gamepad";
            }
            
            gamepad.connected = true;
            gamepad.state.connected = true;
            gamepad.state.name = gamepad.name;
            gamepad.state.deadzone = 0.15f;
            
            // Initialize button and axis states
            memset(gamepad.state.buttons, 0, sizeof(gamepad.state.buttons));
            memset(gamepad.state.axes, 0, sizeof(gamepad.state.axes));
            
            // Start gamepad input thread
            gamepad.shouldStop = false;
            try {
                gamepad.inputThread = std::thread(&LinuxInput::GamepadThreadFunction, this, id);
            } catch (...) {
                // Thread creation failed, close the gamepad
                close(gamepad.fd);
                gamepad.fd = -1;
                gamepad.connected = false;
                gamepad.state.connected = false;
            }
        }
    }

    void LinuxInput::DisconnectGamepad(uint8_t id) {
        if (id >= MAX_GAMEPADS || !m_gamepads[id].connected) {
            return;
        }

        auto& gamepad = m_gamepads[id];
        
        // Stop input thread
        gamepad.shouldStop = true;
        if (gamepad.inputThread.joinable()) {
            gamepad.inputThread.join();
        }
        
        // Close device
        if (gamepad.fd != -1) {
            close(gamepad.fd);
            gamepad.fd = -1;
        }
        
        gamepad.connected = false;
        gamepad.state.connected = false;
        gamepad.name.clear();
        
        if (m_connectedGamepadCount > 0) {
            --m_connectedGamepadCount;
        }
    }

    void LinuxInput::GamepadThreadFunction(uint8_t gamepadId) {
        if (gamepadId >= MAX_GAMEPADS) {
            return;
        }

        auto& gamepad = m_gamepads[gamepadId];
        struct js_event event;
        
        while (!gamepad.shouldStop && gamepad.connected) {
            ssize_t bytesRead = read(gamepad.fd, &event, sizeof(event));
            
            if (bytesRead == sizeof(event)) {
                switch (event.type & ~JS_EVENT_INIT) {
                    case JS_EVENT_BUTTON:
                        if (event.number < static_cast<size_t>(GamepadButton::Count)) {
                            gamepad.state.buttons[event.number] = (event.value != 0);
                        }
                        break;
                        
                    case JS_EVENT_AXIS:
                        if (event.number < static_cast<size_t>(GamepadAxis::Count)) {
                            // Convert from -32767 to 32767 range to -1.0 to 1.0
                            gamepad.state.axes[event.number] = event.value / 32767.0f;
                        }
                        break;
                }
            } else if (bytesRead == -1) {
                // No data available, small sleep
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }

    bool LinuxInput::InitializeX11() {
        // For now, just return true to allow initialization to continue
        return true;
    }

    void LinuxInput::ShutdownX11() {
        // Only close display if we opened it ourselves
        if (m_display && m_window == None) {
            // Only close if we don't have a window set (meaning we opened it ourselves)
            XCloseDisplay(m_display);
            m_display = nullptr;
        }
    }

    KeyCode LinuxInput::LinuxScanCodeToKeyCode(uint16_t scancode) const {
        auto it = m_scancodeToKeyCode.find(scancode);
        return (it != m_scancodeToKeyCode.end()) ? it->second : KeyCode::Unknown;
    }

    MouseButton LinuxInput::LinuxButtonToMouseButton(uint8_t button) const {
        switch (button) {
            case 1: return MouseButton::Left;
            case 2: return MouseButton::Middle;
            case 3: return MouseButton::Right;
            case 4: return MouseButton::X1;
            case 5: return MouseButton::X2;
            default: return MouseButton::Count; // Invalid
        }
    }

    void LinuxInput::UpdateKeyState(KeyCode key, bool pressed) {
        uint16_t keyIndex = static_cast<uint16_t>(key);
        if (keyIndex >= MAX_KEYS) {
            return;
        }

        InputState previousState = m_keyboard.previousKeys[keyIndex];

        if (pressed) {
            if (previousState == InputState::Released) {
                m_keyboard.keys[keyIndex] = InputState::Pressed;
            } else {
                m_keyboard.keys[keyIndex] = InputState::Held;
            }
        } else {
            if (previousState != InputState::Released) {
                m_keyboard.keys[keyIndex] = InputState::Released;
            }
        }
    }

    void LinuxInput::UpdateMouseButtonState(MouseButton button, bool pressed) {
        size_t buttonIndex = static_cast<size_t>(button);
        if (buttonIndex >= MAX_MOUSE_BUTTONS) {
            return;
        }

        InputState previousState = m_mouse.previousButtons[buttonIndex];

        if (pressed) {
            if (previousState == InputState::Released) {
                m_mouse.buttons[buttonIndex] = InputState::Pressed;
            } else {
                m_mouse.buttons[buttonIndex] = InputState::Held;
            }
        } else {
            if (previousState != InputState::Released) {
                m_mouse.buttons[buttonIndex] = InputState::Released;
            }
        }
    }

    void LinuxInput::UpdatePreviousStates() {
        // Update previous keyboard states
        m_keyboard.previousKeys = m_keyboard.keys;
        
        // Update previous mouse button states
        m_mouse.previousButtons = m_mouse.buttons;
        
        // Convert Pressed to Held for next frame
        for (auto& keyState : m_keyboard.keys) {
            if (keyState == InputState::Pressed) {
                keyState = InputState::Held;
            }
        }
        
        for (auto& buttonState : m_mouse.buttons) {
            if (buttonState == InputState::Pressed) {
                buttonState = InputState::Held;
            }
        }
    }

    float LinuxInput::ApplyDeadzone(float value, float deadzone) const {
        if (deadzone <= 0.0f) {
            return value;
        }
        
        float absValue = std::abs(value);
        if (absValue < deadzone) {
            return 0.0f;
        }
        
        // Scale the value to maintain full range after deadzone
        float scaledValue = (absValue - deadzone) / (1.0f - deadzone);
        return (value < 0.0f) ? -scaledValue : scaledValue;
    }

} // namespace VEK::Platform

#endif // VEK_LINUX
