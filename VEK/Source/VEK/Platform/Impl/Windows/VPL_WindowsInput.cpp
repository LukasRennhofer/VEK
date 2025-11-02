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

#include <VEK/Platform/Impl/Windows/VPL_WindowsInput.hpp>

#include <chrono>
#include <cmath>

namespace VEK::Platform {

    WindowsInput::WindowsInput() {
        InitializeKeyMappings();
        
        // Initialize XInput
        XInputEnable(TRUE);
    }

    WindowsInput::~WindowsInput() {
        if (m_initialized) {
            Shutdown();
        }
        
        // Disable XInput
        XInputEnable(FALSE);
    }

    bool WindowsInput::Initialize() {
        if (m_initialized) {
            return true;
        }

        // Initialize DirectInput (optional, can fail safely)
        try {
            if (!InitializeDirectInput()) {
                // DirectInput failed, but we can still work with window messages
            }
        } catch (...) {
            // DirectInput initialization failed, continue without it
        }

        // Start input processing thread
        m_shouldStop = false;
        try {
            m_inputThread = std::thread(&WindowsInput::InputThreadFunction, this);
        } catch (...) {
            // Thread creation failed
            ShutdownDirectInput();
            return false;
        }

        // Check for connected gamepads (optional, can fail safely)
        try {
            CheckGamepadConnections();
        } catch (...) {
            // Gamepad checking failed, continue without gamepads
        }

        m_initialized = true;
        return true;
    }

    void WindowsInput::Shutdown() {
        if (!m_initialized) {
            return;
        }

        // Stop input thread
        m_shouldStop = true;
        if (m_inputThread.joinable()) {
            m_inputThread.join();
        }

        // Shutdown DirectInput
        ShutdownDirectInput();

        m_initialized = false;
    }

    void WindowsInput::Update() {
        if (!m_initialized) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        
        // Convert Pressed states from PREVIOUS frame to Held states
        // This allows the current frame to see Pressed states, but converts
        // them to Held for the next frame
        for (size_t i = 0; i < m_keyboardState.keys.size(); ++i) {
            if (m_keyboardState.previousKeys[i] == InputState::Pressed) {
                // If key was pressed last frame, check if it's still pressed
                if (m_keyboardState.keys[i] == InputState::Pressed) {
                    m_keyboardState.keys[i] = InputState::Held;
                }
            }
        }
        
        for (size_t i = 0; i < m_mouseState.buttons.size(); ++i) {
            if (m_mouseState.previousButtons[i] == InputState::Pressed) {
                // If button was pressed last frame, check if it's still pressed
                if (m_mouseState.buttons[i] == InputState::Pressed) {
                    m_mouseState.buttons[i] = InputState::Held;
                }
            }
        }
        
        // Copy current states to previous states for next frame
        m_keyboardState.previousKeys = m_keyboardState.keys;
        m_mouseState.previousButtons = m_mouseState.buttons;
        
        // Update mouse delta
        POINT cursorPos;
        if (GetCursorPos(&cursorPos) && m_hwnd) {
            ScreenToClient(m_hwnd, &cursorPos);
            m_mouseState.deltaX = cursorPos.x - m_mouseState.lastX;
            m_mouseState.deltaY = cursorPos.y - m_mouseState.lastY;
            m_mouseState.x = cursorPos.x;
            m_mouseState.y = cursorPos.y;
            m_mouseState.lastX = cursorPos.x;
            m_mouseState.lastY = cursorPos.y;
        } else {
            m_mouseState.deltaX = 0;
            m_mouseState.deltaY = 0;
        }
    }

    // Keyboard input implementation
    bool WindowsInput::IsKeyPressed(KeyCode key) const {
        return GetKeyState(key) == InputState::Pressed;
    }

    bool WindowsInput::IsKeyReleased(KeyCode key) const {
        return GetKeyState(key) == InputState::Released;
    }

    bool WindowsInput::IsKeyHeld(KeyCode key) const {
        return GetKeyState(key) == InputState::Held;
    }

    InputState WindowsInput::GetKeyState(KeyCode key) const {
        if (!m_initialized) {
            return InputState::Released;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        uint16_t keyIndex = static_cast<uint16_t>(key);
        if (keyIndex >= MAX_KEYS) {
            return InputState::Released;
        }

        return m_keyboardState.keys[keyIndex];
    }

    // Mouse input implementation
    bool WindowsInput::IsMouseButtonPressed(MouseButton button) const {
        return GetMouseButtonState(button) == InputState::Pressed;
    }

    bool WindowsInput::IsMouseButtonReleased(MouseButton button) const {
        return GetMouseButtonState(button) == InputState::Released;
    }

    bool WindowsInput::IsMouseButtonHeld(MouseButton button) const {
        return GetMouseButtonState(button) == InputState::Held;
    }

    InputState WindowsInput::GetMouseButtonState(MouseButton button) const {
        if (!m_initialized) {
            return InputState::Released;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        size_t buttonIndex = static_cast<size_t>(button);
        if (buttonIndex >= MAX_MOUSE_BUTTONS) {
            return InputState::Released;
        }

        return m_mouseState.buttons[buttonIndex];
    }

    void WindowsInput::GetMousePosition(int32_t& x, int32_t& y) const {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        x = m_mouseState.x;
        y = m_mouseState.y;
    }

    void WindowsInput::GetMouseDelta(int32_t& deltaX, int32_t& deltaY) const {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        deltaX = m_mouseState.deltaX;
        deltaY = m_mouseState.deltaY;
    }

    void WindowsInput::SetMousePosition(int32_t x, int32_t y) {
        if (!m_initialized || !m_hwnd) {
            return;
        }

        POINT point = {x, y};
        ClientToScreen(m_hwnd, &point);
        SetCursorPos(point.x, point.y);

        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_mouseState.x = x;
        m_mouseState.y = y;
    }

    void WindowsInput::SetMouseVisible(bool visible) {
        if (visible != m_mouseState.visible) {
            ShowCursor(visible ? TRUE : FALSE);
            m_mouseState.visible = visible;
        }
    }

    bool WindowsInput::IsMouseVisible() const {
        return m_mouseState.visible;
    }

    // Gamepad input implementation
    uint8_t WindowsInput::GetConnectedGamepadCount() const {
        return m_connectedGamepadCount;
    }

    bool WindowsInput::IsGamepadConnected(uint8_t gamepadId) const {
        if (gamepadId >= MAX_GAMEPADS) {
            return false;
        }
        return m_gamepads[gamepadId].connected;
    }

    const GamepadState* WindowsInput::GetGamepadState(uint8_t gamepadId) const {
        if (gamepadId >= MAX_GAMEPADS || !m_gamepads[gamepadId].connected) {
            return nullptr;
        }
        return &m_gamepads[gamepadId].state;
    }

    bool WindowsInput::IsGamepadButtonPressed(uint8_t gamepadId, GamepadButton button) const {
        return GetGamepadButtonState(gamepadId, button) == InputState::Pressed;
    }

    bool WindowsInput::IsGamepadButtonReleased(uint8_t gamepadId, GamepadButton button) const {
        return GetGamepadButtonState(gamepadId, button) == InputState::Released;
    }

    bool WindowsInput::IsGamepadButtonHeld(uint8_t gamepadId, GamepadButton button) const {
        return GetGamepadButtonState(gamepadId, button) == InputState::Held;
    }

    InputState WindowsInput::GetGamepadButtonState(uint8_t gamepadId, GamepadButton button) const {
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

    float WindowsInput::GetGamepadAxis(uint8_t gamepadId, GamepadAxis axis) const {
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

    void WindowsInput::SetGamepadDeadzone(uint8_t gamepadId, float deadzone) {
        if (gamepadId >= MAX_GAMEPADS || !m_gamepads[gamepadId].connected) {
            return;
        }

        m_gamepads[gamepadId].state.deadzone = deadzone;
    }

    void WindowsInput::ClearEvents() {
        // Events are processed continuously, no need to clear
    }

    // Utility functions
    const char* WindowsInput::GetKeyName(KeyCode key) const {
        auto it = m_keyNames.find(key);
        return (it != m_keyNames.end()) ? it->second : "Unknown";
    }

    const char* WindowsInput::GetMouseButtonName(MouseButton button) const {
        auto it = m_mouseButtonNames.find(button);
        return (it != m_mouseButtonNames.end()) ? it->second : "Unknown";
    }

    const char* WindowsInput::GetGamepadButtonName(GamepadButton button) const {
        auto it = m_gamepadButtonNames.find(button);
        return (it != m_gamepadButtonNames.end()) ? it->second : "Unknown";
    }

    // Windows-specific methods
    void WindowsInput::SetWindowHandle(HWND hwnd) {
        m_hwnd = hwnd;
    }

    bool WindowsInput::ProcessWindowMessage(UINT message, WPARAM wParam, LPARAM lParam) {
        std::lock_guard<std::mutex> lock(m_stateMutex);

        switch (message) {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN: {
                KeyCode keyCode = VirtualKeyToKeyCode(static_cast<uint8_t>(wParam));
                UpdateKeyState(keyCode, true);
                
                // Update modifier states
                m_keyboardState.modifierStates[0] = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;   // Shift
                m_keyboardState.modifierStates[1] = (::GetKeyState(VK_CONTROL) & 0x8000) != 0; // Ctrl
                m_keyboardState.modifierStates[2] = (::GetKeyState(VK_MENU) & 0x8000) != 0;    // Alt
                m_keyboardState.modifierStates[3] = (::GetKeyState(VK_LWIN) & 0x8000) != 0 ||  // Super
                                                   (::GetKeyState(VK_RWIN) & 0x8000) != 0;
                return true;
            }

            case WM_KEYUP:
            case WM_SYSKEYUP: {
                KeyCode keyCode = VirtualKeyToKeyCode(static_cast<uint8_t>(wParam));
                UpdateKeyState(keyCode, false);
                
                // Update modifier states
                m_keyboardState.modifierStates[0] = (::GetKeyState(VK_SHIFT) & 0x8000) != 0;   // Shift
                m_keyboardState.modifierStates[1] = (::GetKeyState(VK_CONTROL) & 0x8000) != 0; // Ctrl
                m_keyboardState.modifierStates[2] = (::GetKeyState(VK_MENU) & 0x8000) != 0;    // Alt
                m_keyboardState.modifierStates[3] = (::GetKeyState(VK_LWIN) & 0x8000) != 0 ||  // Super
                                                   (::GetKeyState(VK_RWIN) & 0x8000) != 0;
                return true;
            }

            case WM_LBUTTONDOWN:
                UpdateMouseButtonState(MouseButton::Left, true);
                return true;

            case WM_LBUTTONUP:
                UpdateMouseButtonState(MouseButton::Left, false);
                return true;

            case WM_RBUTTONDOWN:
                UpdateMouseButtonState(MouseButton::Right, true);
                return true;

            case WM_RBUTTONUP:
                UpdateMouseButtonState(MouseButton::Right, false);
                return true;

            case WM_MBUTTONDOWN:
                UpdateMouseButtonState(MouseButton::Middle, true);
                return true;

            case WM_MBUTTONUP:
                UpdateMouseButtonState(MouseButton::Middle, false);
                return true;

            case WM_XBUTTONDOWN: {
                WORD button = GET_XBUTTON_WPARAM(wParam);
                if (button == XBUTTON1) {
                    UpdateMouseButtonState(MouseButton::X1, true);
                } else if (button == XBUTTON2) {
                    UpdateMouseButtonState(MouseButton::X2, true);
                }
                return true;
            }

            case WM_XBUTTONUP: {
                WORD button = GET_XBUTTON_WPARAM(wParam);
                if (button == XBUTTON1) {
                    UpdateMouseButtonState(MouseButton::X1, false);
                } else if (button == XBUTTON2) {
                    UpdateMouseButtonState(MouseButton::X2, false);
                }
                return true;
            }

            case WM_MOUSEMOVE: {
                m_mouseState.x = GET_X_LPARAM(lParam);
                m_mouseState.y = GET_Y_LPARAM(lParam);
                return true;
            }

            case WM_MOUSEWHEEL: {
                float wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam) / static_cast<float>(WHEEL_DELTA);
                m_mouseState.wheelDelta = wheelDelta;
                return true;
            }

            default:
                return false;
        }
    }

    void WindowsInput::SetMouseCapture(bool capture) {
        if (capture && !m_mouseState.captured) {
            SetCapture(m_hwnd);
            m_mouseState.captured = true;
        } else if (!capture && m_mouseState.captured) {
            ReleaseCapture();
            m_mouseState.captured = false;
        }
    }

    // Private methods implementation
    void WindowsInput::InitializeKeyMappings() {
        // Initialize virtual key to KeyCode mapping
        m_virtualKeyToKeyCode[0x41] = KeyCode::A;    // A
        m_virtualKeyToKeyCode[0x42] = KeyCode::B;    // B
        m_virtualKeyToKeyCode[0x43] = KeyCode::C;    // C
        m_virtualKeyToKeyCode[0x44] = KeyCode::D;    // D
        m_virtualKeyToKeyCode[0x45] = KeyCode::E;    // E
        m_virtualKeyToKeyCode[0x46] = KeyCode::F;    // F
        m_virtualKeyToKeyCode[0x47] = KeyCode::G;    // G
        m_virtualKeyToKeyCode[0x48] = KeyCode::H;    // H
        m_virtualKeyToKeyCode[0x49] = KeyCode::I;    // I
        m_virtualKeyToKeyCode[0x4A] = KeyCode::J;    // J
        m_virtualKeyToKeyCode[0x4B] = KeyCode::K;    // K
        m_virtualKeyToKeyCode[0x4C] = KeyCode::L;    // L
        m_virtualKeyToKeyCode[0x4D] = KeyCode::M;    // M
        m_virtualKeyToKeyCode[0x4E] = KeyCode::N;    // N
        m_virtualKeyToKeyCode[0x4F] = KeyCode::O;    // O
        m_virtualKeyToKeyCode[0x50] = KeyCode::P;    // P
        m_virtualKeyToKeyCode[0x51] = KeyCode::Q;    // Q
        m_virtualKeyToKeyCode[0x52] = KeyCode::R;    // R
        m_virtualKeyToKeyCode[0x53] = KeyCode::S;    // S
        m_virtualKeyToKeyCode[0x54] = KeyCode::T;    // T
        m_virtualKeyToKeyCode[0x55] = KeyCode::U;    // U
        m_virtualKeyToKeyCode[0x56] = KeyCode::V;    // V
        m_virtualKeyToKeyCode[0x57] = KeyCode::W;    // W
        m_virtualKeyToKeyCode[0x58] = KeyCode::X;    // X
        m_virtualKeyToKeyCode[0x59] = KeyCode::Y;    // Y
        m_virtualKeyToKeyCode[0x5A] = KeyCode::Z;    // Z
        
        // Numbers
        m_virtualKeyToKeyCode[0x30] = KeyCode::Num0;
        m_virtualKeyToKeyCode[0x31] = KeyCode::Num1;
        m_virtualKeyToKeyCode[0x32] = KeyCode::Num2;
        m_virtualKeyToKeyCode[0x33] = KeyCode::Num3;
        m_virtualKeyToKeyCode[0x34] = KeyCode::Num4;
        m_virtualKeyToKeyCode[0x35] = KeyCode::Num5;
        m_virtualKeyToKeyCode[0x36] = KeyCode::Num6;
        m_virtualKeyToKeyCode[0x37] = KeyCode::Num7;
        m_virtualKeyToKeyCode[0x38] = KeyCode::Num8;
        m_virtualKeyToKeyCode[0x39] = KeyCode::Num9;
        
        // Function keys
        m_virtualKeyToKeyCode[VK_F1] = KeyCode::F1;
        m_virtualKeyToKeyCode[VK_F2] = KeyCode::F2;
        m_virtualKeyToKeyCode[VK_F3] = KeyCode::F3;
        m_virtualKeyToKeyCode[VK_F4] = KeyCode::F4;
        m_virtualKeyToKeyCode[VK_F5] = KeyCode::F5;
        m_virtualKeyToKeyCode[VK_F6] = KeyCode::F6;
        m_virtualKeyToKeyCode[VK_F7] = KeyCode::F7;
        m_virtualKeyToKeyCode[VK_F8] = KeyCode::F8;
        m_virtualKeyToKeyCode[VK_F9] = KeyCode::F9;
        m_virtualKeyToKeyCode[VK_F10] = KeyCode::F10;
        m_virtualKeyToKeyCode[VK_F11] = KeyCode::F11;
        m_virtualKeyToKeyCode[VK_F12] = KeyCode::F12;
        
        // Arrow keys
        m_virtualKeyToKeyCode[VK_LEFT] = KeyCode::Left;
        m_virtualKeyToKeyCode[VK_RIGHT] = KeyCode::Right;
        m_virtualKeyToKeyCode[VK_UP] = KeyCode::Up;
        m_virtualKeyToKeyCode[VK_DOWN] = KeyCode::Down;
        
        // Special keys
        m_virtualKeyToKeyCode[VK_ESCAPE] = KeyCode::Escape;
        m_virtualKeyToKeyCode[VK_TAB] = KeyCode::Tab;
        m_virtualKeyToKeyCode[VK_CAPITAL] = KeyCode::CapsLock;
        m_virtualKeyToKeyCode[VK_LSHIFT] = KeyCode::LeftShift;
        m_virtualKeyToKeyCode[VK_RSHIFT] = KeyCode::RightShift;
        m_virtualKeyToKeyCode[VK_LCONTROL] = KeyCode::LeftCtrl;
        m_virtualKeyToKeyCode[VK_RCONTROL] = KeyCode::RightCtrl;
        m_virtualKeyToKeyCode[VK_LMENU] = KeyCode::LeftAlt;
        m_virtualKeyToKeyCode[VK_RMENU] = KeyCode::RightAlt;
        m_virtualKeyToKeyCode[VK_SPACE] = KeyCode::Space;
        m_virtualKeyToKeyCode[VK_RETURN] = KeyCode::Enter;
        m_virtualKeyToKeyCode[VK_BACK] = KeyCode::Backspace;
        m_virtualKeyToKeyCode[VK_DELETE] = KeyCode::Delete;
        
        // Navigation
        m_virtualKeyToKeyCode[VK_HOME] = KeyCode::Home;
        m_virtualKeyToKeyCode[VK_END] = KeyCode::End;
        m_virtualKeyToKeyCode[VK_PRIOR] = KeyCode::PageUp;
        m_virtualKeyToKeyCode[VK_NEXT] = KeyCode::PageDown;
        m_virtualKeyToKeyCode[VK_INSERT] = KeyCode::Insert;

        // Initialize key names (same as Linux implementation)
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

    bool WindowsInput::InitializeDirectInput() {
        HRESULT hr = DirectInput8Create(GetModuleHandle(nullptr), DIRECTINPUT_VERSION,
                                       IID_IDirectInput8, (void**)&m_directInput, nullptr);
        if (FAILED(hr)) {
            return false;
        }

        // Create keyboard device (optional)
        hr = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, nullptr);
        if (SUCCEEDED(hr)) {
            hr = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
            if (SUCCEEDED(hr) && m_hwnd) {
                hr = m_keyboard->SetCooperativeLevel(m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
            }
            if (FAILED(hr)) {
                m_keyboard->Release();
                m_keyboard = nullptr;
            }
        }

        // Create mouse device (optional)
        hr = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, nullptr);
        if (SUCCEEDED(hr)) {
            hr = m_mouse->SetDataFormat(&c_dfDIMouse);
            if (SUCCEEDED(hr) && m_hwnd) {
                hr = m_mouse->SetCooperativeLevel(m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
            }
            if (FAILED(hr)) {
                m_mouse->Release();
                m_mouse = nullptr;
            }
        }

        return true; // Return true even if some devices failed
    }

    void WindowsInput::ShutdownDirectInput() {
        if (m_keyboard) {
            m_keyboard->Unacquire();
            m_keyboard->Release();
            m_keyboard = nullptr;
        }

        if (m_mouse) {
            m_mouse->Unacquire();
            m_mouse->Release();
            m_mouse = nullptr;
        }

        if (m_directInput) {
            m_directInput->Release();
            m_directInput = nullptr;
        }
    }

    void WindowsInput::InputThreadFunction() {
        while (!m_shouldStop) {
            try {
                UpdateKeyboardState();
                UpdateMouseState();
                UpdateGamepadStates();
            } catch (...) {
                // If any input processing fails, continue running
                // This prevents crashes from bringing down the entire input system
            }
            
            // Small sleep to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void WindowsInput::UpdateKeyboardState() {
        if (!m_keyboard) {
            return;
        }

        // Acquire keyboard if lost
        HRESULT hr = m_keyboard->Poll();
        if (FAILED(hr)) {
            hr = m_keyboard->Acquire();
            if (FAILED(hr)) {
                return;
            }
        }

        // Get keyboard state
        BYTE keyboardState[256];
        hr = m_keyboard->GetDeviceState(sizeof(keyboardState), keyboardState);
        if (FAILED(hr)) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        
        // Update key states
        for (auto& mapping : m_virtualKeyToKeyCode) {
            uint8_t vKey = mapping.first;
            KeyCode keyCode = mapping.second;
            
            bool pressed = (keyboardState[vKey] & 0x80) != 0;
            UpdateKeyState(keyCode, pressed);
        }
    }

    void WindowsInput::UpdateMouseState() {
        if (!m_mouse) {
            return;
        }

        // Acquire mouse if lost
        HRESULT hr = m_mouse->Poll();
        if (FAILED(hr)) {
            hr = m_mouse->Acquire();
            if (FAILED(hr)) {
                return;
            }
        }

        // Get mouse state
        DIMOUSESTATE mouseState;
        hr = m_mouse->GetDeviceState(sizeof(mouseState), &mouseState);
        if (FAILED(hr)) {
            return;
        }

        std::lock_guard<std::mutex> lock(m_stateMutex);
        
        // Update button states
        UpdateMouseButtonState(MouseButton::Left, (mouseState.rgbButtons[0] & 0x80) != 0);
        UpdateMouseButtonState(MouseButton::Right, (mouseState.rgbButtons[1] & 0x80) != 0);
        UpdateMouseButtonState(MouseButton::Middle, (mouseState.rgbButtons[2] & 0x80) != 0);
        if (mouseState.rgbButtons[3]) UpdateMouseButtonState(MouseButton::X1, (mouseState.rgbButtons[3] & 0x80) != 0);
        if (mouseState.rgbButtons[4]) UpdateMouseButtonState(MouseButton::X2, (mouseState.rgbButtons[4] & 0x80) != 0);
    }

    void WindowsInput::UpdateGamepadStates() {
        CheckGamepadConnections();
        
        for (uint8_t i = 0; i < MAX_GAMEPADS; ++i) {
            if (m_gamepads[i].connected) {
                XINPUT_STATE state;
                DWORD result = XInputGetState(i, &state);
                
                if (result == ERROR_SUCCESS) {
                    ProcessXInputGamepad(i, state);
                } else {
                    // Controller disconnected
                    m_gamepads[i].connected = false;
                    m_gamepads[i].state.connected = false;
                    if (m_connectedGamepadCount > 0) {
                        --m_connectedGamepadCount;
                    }
                }
            }
        }
    }

    void WindowsInput::CheckGamepadConnections() {
        m_connectedGamepadCount = 0;
        
        for (uint8_t i = 0; i < MAX_GAMEPADS; ++i) {
            XINPUT_STATE state;
            DWORD result = XInputGetState(i, &state);
            
            bool wasConnected = m_gamepads[i].connected;
            bool isConnected = (result == ERROR_SUCCESS);
            
            if (isConnected && !wasConnected) {
                // Controller connected
                m_gamepads[i].connected = true;
                m_gamepads[i].state.connected = true;
                m_gamepads[i].state.name = "Xbox Controller";
                m_gamepads[i].state.deadzone = 0.15f;
                m_gamepads[i].lastPacketNumber = state.dwPacketNumber;
                
                // Initialize button and axis states
                memset(m_gamepads[i].state.buttons, 0, sizeof(m_gamepads[i].state.buttons));
                memset(m_gamepads[i].state.axes, 0, sizeof(m_gamepads[i].state.axes));
            } else if (!isConnected && wasConnected) {
                // Controller disconnected
                m_gamepads[i].connected = false;
                m_gamepads[i].state.connected = false;
                m_gamepads[i].state.name.clear();
            }
            
            if (m_gamepads[i].connected) {
                ++m_connectedGamepadCount;
            }
        }
    }

    KeyCode WindowsInput::VirtualKeyToKeyCode(uint8_t virtualKey) const {
        auto it = m_virtualKeyToKeyCode.find(virtualKey);
        return (it != m_virtualKeyToKeyCode.end()) ? it->second : KeyCode::Unknown;
    }

    MouseButton WindowsInput::Win32ButtonToMouseButton(uint8_t button) const {
        switch (button) {
            case 0: return MouseButton::Left;
            case 1: return MouseButton::Right;
            case 2: return MouseButton::Middle;
            case 3: return MouseButton::X1;
            case 4: return MouseButton::X2;
            default: return MouseButton::Count; // Invalid
        }
    }

    GamepadButton WindowsInput::XInputButtonToGamepadButton(WORD xinputButton) const {
        if (xinputButton & XINPUT_GAMEPAD_A) return GamepadButton::A;
        if (xinputButton & XINPUT_GAMEPAD_B) return GamepadButton::B;
        if (xinputButton & XINPUT_GAMEPAD_X) return GamepadButton::X;
        if (xinputButton & XINPUT_GAMEPAD_Y) return GamepadButton::Y;
        if (xinputButton & XINPUT_GAMEPAD_LEFT_SHOULDER) return GamepadButton::LeftBumper;
        if (xinputButton & XINPUT_GAMEPAD_RIGHT_SHOULDER) return GamepadButton::RightBumper;
        if (xinputButton & XINPUT_GAMEPAD_BACK) return GamepadButton::Back;
        if (xinputButton & XINPUT_GAMEPAD_START) return GamepadButton::Start;
        if (xinputButton & XINPUT_GAMEPAD_LEFT_THUMB) return GamepadButton::LeftThumb;
        if (xinputButton & XINPUT_GAMEPAD_RIGHT_THUMB) return GamepadButton::RightThumb;
        if (xinputButton & XINPUT_GAMEPAD_DPAD_UP) return GamepadButton::DpadUp;
        if (xinputButton & XINPUT_GAMEPAD_DPAD_DOWN) return GamepadButton::DpadDown;
        if (xinputButton & XINPUT_GAMEPAD_DPAD_LEFT) return GamepadButton::DpadLeft;
        if (xinputButton & XINPUT_GAMEPAD_DPAD_RIGHT) return GamepadButton::DpadRight;
        
        return GamepadButton::Count; // Invalid
    }

    void WindowsInput::UpdateKeyState(KeyCode key, bool pressed) {
        uint16_t keyIndex = static_cast<uint16_t>(key);
        if (keyIndex >= MAX_KEYS) {
            return;
        }

        InputState previousState = m_keyboardState.previousKeys[keyIndex];

        if (pressed) {
            if (previousState == InputState::Released) {
                m_keyboardState.keys[keyIndex] = InputState::Pressed;
            } else {
                m_keyboardState.keys[keyIndex] = InputState::Held;
            }
        } else {
            if (previousState != InputState::Released) {
                m_keyboardState.keys[keyIndex] = InputState::Released;
            }
        }
    }

    void WindowsInput::UpdateMouseButtonState(MouseButton button, bool pressed) {
        size_t buttonIndex = static_cast<size_t>(button);
        if (buttonIndex >= MAX_MOUSE_BUTTONS) {
            return;
        }

        InputState previousState = m_mouseState.previousButtons[buttonIndex];

        if (pressed) {
            if (previousState == InputState::Released) {
                m_mouseState.buttons[buttonIndex] = InputState::Pressed;
            } else {
                m_mouseState.buttons[buttonIndex] = InputState::Held;
            }
        } else {
            if (previousState != InputState::Released) {
                m_mouseState.buttons[buttonIndex] = InputState::Released;
            }
        }
    }

    void WindowsInput::UpdatePreviousStates() {
        // Update previous keyboard states
        m_keyboardState.previousKeys = m_keyboardState.keys;
        
        // Update previous mouse button states
        m_mouseState.previousButtons = m_mouseState.buttons;
        
        // Convert Pressed to Held for next frame
        for (auto& keyState : m_keyboardState.keys) {
            if (keyState == InputState::Pressed) {
                keyState = InputState::Held;
            }
        }
        
        for (auto& buttonState : m_mouseState.buttons) {
            if (buttonState == InputState::Pressed) {
                buttonState = InputState::Held;
            }
        }
    }

    void WindowsInput::ProcessXInputGamepad(uint8_t gamepadId, const XINPUT_STATE& state) {
        if (gamepadId >= MAX_GAMEPADS) {
            return;
        }

        auto& gamepad = m_gamepads[gamepadId];
        
        // Only update if packet number changed
        if (state.dwPacketNumber == gamepad.lastPacketNumber) {
            return;
        }
        
        gamepad.lastPacketNumber = state.dwPacketNumber;
        gamepad.lastState = state;

        // Update button states
        const XINPUT_GAMEPAD& gp = state.Gamepad;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::A)] = (gp.wButtons & XINPUT_GAMEPAD_A) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::B)] = (gp.wButtons & XINPUT_GAMEPAD_B) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::X)] = (gp.wButtons & XINPUT_GAMEPAD_X) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::Y)] = (gp.wButtons & XINPUT_GAMEPAD_Y) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::LeftBumper)] = (gp.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::RightBumper)] = (gp.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::Back)] = (gp.wButtons & XINPUT_GAMEPAD_BACK) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::Start)] = (gp.wButtons & XINPUT_GAMEPAD_START) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::LeftThumb)] = (gp.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::RightThumb)] = (gp.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::DpadUp)] = (gp.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::DpadDown)] = (gp.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::DpadLeft)] = (gp.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
        gamepad.state.buttons[static_cast<size_t>(GamepadButton::DpadRight)] = (gp.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;

        // Update axis states
        gamepad.state.axes[static_cast<size_t>(GamepadAxis::LeftX)] = NormalizeXInputStick(gp.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        gamepad.state.axes[static_cast<size_t>(GamepadAxis::LeftY)] = NormalizeXInputStick(gp.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
        gamepad.state.axes[static_cast<size_t>(GamepadAxis::RightX)] = NormalizeXInputStick(gp.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        gamepad.state.axes[static_cast<size_t>(GamepadAxis::RightY)] = NormalizeXInputStick(gp.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
        gamepad.state.axes[static_cast<size_t>(GamepadAxis::LeftTrigger)] = NormalizeXInputTrigger(gp.bLeftTrigger);
        gamepad.state.axes[static_cast<size_t>(GamepadAxis::RightTrigger)] = NormalizeXInputTrigger(gp.bRightTrigger);

        // Update timestamp
        auto now = std::chrono::steady_clock::now();
        gamepad.state.lastUpdateTime = static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
    }

    float WindowsInput::NormalizeXInputTrigger(BYTE triggerValue) const {
        // Normalize from 0-255 to 0.0-1.0
        return triggerValue / 255.0f;
    }

    float WindowsInput::NormalizeXInputStick(SHORT stickValue, SHORT deadzone) const {
        // Apply deadzone
        if (abs(stickValue) < deadzone) {
            return 0.0f;
        }
        
        // Normalize from -32768 to 32767 to -1.0 to 1.0
        if (stickValue < 0) {
            return stickValue / 32768.0f;
        } else {
            return stickValue / 32767.0f;
        }
    }

    float WindowsInput::ApplyDeadzone(float value, float deadzone) const {
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

#endif // VEK_WINDOWS
