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
#include <VEK/Core/Container/VCO_Vector.hpp>
#include <cstdint>

namespace VEK::Platform {

    // Keyboard key codes (based on scancode for cross-platform compatibility)
    enum class KeyCode : uint16_t {
        // Letters
        A = 0x1E, B = 0x30, C = 0x2E, D = 0x20, E = 0x12, F = 0x21, G = 0x22, H = 0x23,
        I = 0x17, J = 0x24, K = 0x25, L = 0x26, M = 0x32, N = 0x31, O = 0x18, P = 0x19,
        Q = 0x10, R = 0x13, S = 0x1F, T = 0x14, U = 0x16, V = 0x2F, W = 0x11, X = 0x2D,
        Y = 0x15, Z = 0x2C,
        
        // Numbers
        Num0 = 0x0B, Num1 = 0x02, Num2 = 0x03, Num3 = 0x04, Num4 = 0x05,
        Num5 = 0x06, Num6 = 0x07, Num7 = 0x08, Num8 = 0x09, Num9 = 0x0A,
        
        // Function keys
        F1 = 0x3B, F2 = 0x3C, F3 = 0x3D, F4 = 0x3E, F5 = 0x3F, F6 = 0x40,
        F7 = 0x41, F8 = 0x42, F9 = 0x43, F10 = 0x44, F11 = 0x57, F12 = 0x58,
        
        // Arrow keys
        Left = 0x4B, Right = 0x4D, Up = 0x48, Down = 0x50,
        
        // Special keys
        Escape = 0x01, Tab = 0x0F, CapsLock = 0x3A, LeftShift = 0x2A, RightShift = 0x36,
        LeftCtrl = 0x1D, RightCtrl = 0x9D, LeftAlt = 0x38, RightAlt = 0xB8,
        Space = 0x39, Enter = 0x1C, Backspace = 0x0E, Delete = 0x53,
        
        // Navigation
        Home = 0x47, End = 0x4F, PageUp = 0x49, PageDown = 0x51, Insert = 0x52,
        
        // Numpad
        Numpad0 = 0x52, Numpad1 = 0x4F, Numpad2 = 0x50, Numpad3 = 0x51, Numpad4 = 0x4B,
        Numpad5 = 0x4C, Numpad6 = 0x4D, Numpad7 = 0x47, Numpad8 = 0x48, Numpad9 = 0x49,
        NumpadAdd = 0x4E, NumpadSubtract = 0x4A, NumpadMultiply = 0x37, NumpadDivide = 0xB5,
        NumpadEnter = 0x9C, NumpadDecimal = 0x53,
        
        // Punctuation
        Semicolon = 0x27, Equals = 0x0D, Comma = 0x33, Minus = 0x0C, Period = 0x34,
        Slash = 0x35, Grave = 0x29, LeftBracket = 0x1A, Backslash = 0x2B,
        RightBracket = 0x1B, Apostrophe = 0x28,
        
        // System keys
        PrintScreen = 0xB7, ScrollLock = 0x46, Pause = 0xC5,
        LeftSuper = 0xDB, RightSuper = 0xDC, Menu = 0xDD,
        
        // Special
        Unknown = 0x00
    };

    // Mouse button codes
    enum class MouseButton : uint8_t {
        Left = 0,
        Right = 1,
        Middle = 2,
        X1 = 3,
        X2 = 4,
        Count = 5
    };

    // Gamepad button codes
    enum class GamepadButton : uint8_t {
        A = 0, B = 1, X = 2, Y = 3,
        LeftBumper = 4, RightBumper = 5,
        Back = 6, Start = 7, Guide = 8,
        LeftThumb = 9, RightThumb = 10,
        DpadUp = 11, DpadRight = 12, DpadDown = 13, DpadLeft = 14,
        Count = 15
    };

    // Gamepad axis codes
    enum class GamepadAxis : uint8_t {
        LeftX = 0, LeftY = 1,
        RightX = 2, RightY = 3,
        LeftTrigger = 4, RightTrigger = 5,
        Count = 6
    };

    // Input state enums
    enum class InputState : uint8_t {
        Released = 0,
        Pressed = 1,
        Held = 2
    };

    // Input event structures
    struct KeyEvent {
        KeyCode key;
        InputState state;
        bool shift;
        bool ctrl;
        bool alt;
        bool super;
        uint32_t scancode;
        uint32_t timestamp;
    };

    struct MouseButtonEvent {
        MouseButton button;
        InputState state;
        int32_t x, y;
        uint32_t timestamp;
    };

    struct MouseMoveEvent {
        int32_t x, y;
        int32_t deltaX, deltaY;
        uint32_t timestamp;
    };

    struct MouseScrollEvent {
        float deltaX, deltaY;
        int32_t x, y;
        uint32_t timestamp;
    };

    struct GamepadConnectionEvent {
        uint8_t gamepadId;
        bool connected;
        Core::KSafeString<> name;
        uint32_t timestamp;
    };

    struct GamepadButtonEvent {
        uint8_t gamepadId;
        GamepadButton button;
        InputState state;
        uint32_t timestamp;
    };

    struct GamepadAxisEvent {
        uint8_t gamepadId;
        GamepadAxis axis;
        float value;    // -1.0 to 1.0 for sticks, 0.0 to 1.0 for triggers
        uint32_t timestamp;
    };

    // Gamepad state structure
    struct GamepadState {
        bool connected;
        Core::KSafeString<> name;
        bool buttons[static_cast<size_t>(GamepadButton::Count)];
        float axes[static_cast<size_t>(GamepadAxis::Count)];
        float deadzone;
        uint32_t lastUpdateTime;
    };

    // Input interface
    class IInput {
    public:
        virtual ~IInput() = default;

        // Initialization
        virtual bool Initialize() = 0;
        virtual void Shutdown() = 0;
        virtual void Update() = 0;

        // Keyboard input
        virtual bool IsKeyPressed(KeyCode key) const = 0;
        virtual bool IsKeyReleased(KeyCode key) const = 0;
        virtual bool IsKeyHeld(KeyCode key) const = 0;
        virtual InputState GetKeyState(KeyCode key) const = 0;

        // Mouse input
        virtual bool IsMouseButtonPressed(MouseButton button) const = 0;
        virtual bool IsMouseButtonReleased(MouseButton button) const = 0;
        virtual bool IsMouseButtonHeld(MouseButton button) const = 0;
        virtual InputState GetMouseButtonState(MouseButton button) const = 0;
        
        virtual void GetMousePosition(int32_t& x, int32_t& y) const = 0;
        virtual void GetMouseDelta(int32_t& deltaX, int32_t& deltaY) const = 0;
        virtual void SetMousePosition(int32_t x, int32_t y) = 0;
        virtual void SetMouseVisible(bool visible) = 0;
        virtual bool IsMouseVisible() const = 0;

        // Gamepad input
        virtual uint8_t GetConnectedGamepadCount() const = 0;
        virtual bool IsGamepadConnected(uint8_t gamepadId) const = 0;
        virtual const GamepadState* GetGamepadState(uint8_t gamepadId) const = 0;
        
        virtual bool IsGamepadButtonPressed(uint8_t gamepadId, GamepadButton button) const = 0;
        virtual bool IsGamepadButtonReleased(uint8_t gamepadId, GamepadButton button) const = 0;
        virtual bool IsGamepadButtonHeld(uint8_t gamepadId, GamepadButton button) const = 0;
        virtual InputState GetGamepadButtonState(uint8_t gamepadId, GamepadButton button) const = 0;
        
        virtual float GetGamepadAxis(uint8_t gamepadId, GamepadAxis axis) const = 0;
        virtual void SetGamepadDeadzone(uint8_t gamepadId, float deadzone) = 0;

        // Clear event queues
        virtual void ClearEvents() = 0;

        // Utility functions
        virtual const char* GetKeyName(KeyCode key) const = 0;
        virtual const char* GetMouseButtonName(MouseButton button) const = 0;
        virtual const char* GetGamepadButtonName(GamepadButton button) const = 0;
    };

} // namespace VEK::Platform
