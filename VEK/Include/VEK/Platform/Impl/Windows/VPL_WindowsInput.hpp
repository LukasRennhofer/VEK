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

#include <VEK/Platform/VPL_Input.hpp>
#include <VEK/Core/Container/VCO_Vector.hpp>
#include <VEK/Core/Container/VCO_String.hpp>

#include <windows.h>
#include <windowsx.h>  // For GET_X_LPARAM and GET_Y_LPARAM
#include <dinput.h>
#include <xinput.h>
#include <unordered_map>
#include <array>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

// Note: Libraries linked via CMake for cross-compilation compatibility

namespace VEK::Platform {

    class WindowsInput : public IInput {
    private:
        static constexpr uint8_t MAX_GAMEPADS = 4; // XInput supports max 4 controllers
        static constexpr size_t MAX_KEYS = 256;
        static constexpr size_t MAX_MOUSE_BUTTONS = static_cast<size_t>(MouseButton::Count);

        // Input state tracking
        struct KeyboardState {
            std::array<InputState, MAX_KEYS> keys{};
            std::array<InputState, MAX_KEYS> previousKeys{};
            std::array<bool, MAX_KEYS> keyStates{};
            bool modifierStates[4] = {false, false, false, false}; // shift, ctrl, alt, super
        };

        struct MouseState {
            std::array<InputState, MAX_MOUSE_BUTTONS> buttons{};
            std::array<InputState, MAX_MOUSE_BUTTONS> previousButtons{};
            int32_t x = 0, y = 0;
            int32_t deltaX = 0, deltaY = 0;
            int32_t lastX = 0, lastY = 0;
            float wheelDelta = 0.0f;
            bool visible = true;
            bool captured = false;
        };

        struct GamepadDevice {
            bool connected = false;
            XINPUT_STATE lastState{};
            GamepadState state{};
            uint32_t lastPacketNumber = 0;
        };

        // Member variables
        bool m_initialized = false;
        
        // Window handle for input processing
        HWND m_hwnd = nullptr;
        
        // DirectInput interface
        IDirectInput8* m_directInput = nullptr;
        IDirectInputDevice8* m_keyboard = nullptr;
        IDirectInputDevice8* m_mouse = nullptr;
        
        // State tracking
        KeyboardState m_keyboardState;
        MouseState m_mouseState;
        std::array<GamepadDevice, MAX_GAMEPADS> m_gamepads;
        uint8_t m_connectedGamepadCount = 0;
        
        // Input processing thread
        std::thread m_inputThread;
        std::atomic<bool> m_shouldStop{false};
        mutable std::mutex m_stateMutex;
        
        // Key mapping
        std::unordered_map<uint8_t, KeyCode> m_virtualKeyToKeyCode;
        std::unordered_map<KeyCode, const char*> m_keyNames;
        std::unordered_map<MouseButton, const char*> m_mouseButtonNames;
        std::unordered_map<GamepadButton, const char*> m_gamepadButtonNames;

        // Private methods
        void InitializeKeyMappings();
        bool InitializeDirectInput();
        void ShutdownDirectInput();
        void InputThreadFunction();
        void UpdateKeyboardState();
        void UpdateMouseState();
        void UpdateGamepadStates();
        void CheckGamepadConnections();
        
        // Win32 helpers
        KeyCode VirtualKeyToKeyCode(uint8_t virtualKey) const;
        MouseButton Win32ButtonToMouseButton(uint8_t button) const;
        GamepadButton XInputButtonToGamepadButton(WORD xinputButton) const;
        
        // Input state helpers
        void UpdateKeyState(KeyCode key, bool pressed);
        void UpdateMouseButtonState(MouseButton button, bool pressed);
        void UpdatePreviousStates();
        
        // XInput helpers
        void ProcessXInputGamepad(uint8_t gamepadId, const XINPUT_STATE& state);
        float NormalizeXInputTrigger(BYTE triggerValue) const;
        float NormalizeXInputStick(SHORT stickValue, SHORT deadzone) const;
        
        // Utility functions
        float ApplyDeadzone(float value, float deadzone) const;

    public:
        WindowsInput();
        virtual ~WindowsInput();

        // IInput implementation
        bool Initialize() override;
        void Shutdown() override;
        void Update() override;

        // Keyboard input
        bool IsKeyPressed(KeyCode key) const override;
        bool IsKeyReleased(KeyCode key) const override;
        bool IsKeyHeld(KeyCode key) const override;
        InputState GetKeyState(KeyCode key) const override;

        // Mouse input
        bool IsMouseButtonPressed(MouseButton button) const override;
        bool IsMouseButtonReleased(MouseButton button) const override;
        bool IsMouseButtonHeld(MouseButton button) const override;
        InputState GetMouseButtonState(MouseButton button) const override;
        
        void GetMousePosition(int32_t& x, int32_t& y) const override;
        void GetMouseDelta(int32_t& deltaX, int32_t& deltaY) const override;
        void SetMousePosition(int32_t x, int32_t y) override;
        void SetMouseVisible(bool visible) override;
        bool IsMouseVisible() const override;

        // Gamepad input
        uint8_t GetConnectedGamepadCount() const override;
        bool IsGamepadConnected(uint8_t gamepadId) const override;
        const GamepadState* GetGamepadState(uint8_t gamepadId) const override;
        
        bool IsGamepadButtonPressed(uint8_t gamepadId, GamepadButton button) const override;
        bool IsGamepadButtonReleased(uint8_t gamepadId, GamepadButton button) const override;
        bool IsGamepadButtonHeld(uint8_t gamepadId, GamepadButton button) const override;
        InputState GetGamepadButtonState(uint8_t gamepadId, GamepadButton button) const override;
        
        float GetGamepadAxis(uint8_t gamepadId, GamepadAxis axis) const override;
        void SetGamepadDeadzone(uint8_t gamepadId, float deadzone) override;

        // Clear event queues
        void ClearEvents() override;

        // Utility functions
        const char* GetKeyName(KeyCode key) const override;
        const char* GetMouseButtonName(MouseButton button) const override;
        const char* GetGamepadButtonName(GamepadButton button) const override;

        // Windows-specific methods
        void SetWindowHandle(HWND hwnd);
        bool ProcessWindowMessage(UINT message, WPARAM wParam, LPARAM lParam);
        void SetMouseCapture(bool capture);
    };

} // namespace VEK::Platform

#endif // VEK_WINDOWS
