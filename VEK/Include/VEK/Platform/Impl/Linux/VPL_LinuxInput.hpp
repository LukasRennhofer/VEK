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

#include <VEK/Platform/VPL_Input.hpp>
#include <VEK/Core/Container/VCO_Vector.hpp>
#include <VEK/Core/Container/VCO_String.hpp>

#include <linux/input.h>
#include <linux/joystick.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <unordered_map>
#include <array>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>

namespace VEK::Platform {

    class LinuxInput : public IInput {
    private:
        static constexpr uint8_t MAX_GAMEPADS = 8;
        static constexpr size_t MAX_KEYS = 256;
        static constexpr size_t MAX_MOUSE_BUTTONS = static_cast<size_t>(MouseButton::Count);

        // Input state tracking
        struct KeyboardState {
            std::array<InputState, MAX_KEYS> keys{};
            std::array<InputState, MAX_KEYS> previousKeys{};
            bool modifierStates[4] = {false, false, false, false}; // shift, ctrl, alt, super
        };

        struct MouseState {
            std::array<InputState, MAX_MOUSE_BUTTONS> buttons{};
            std::array<InputState, MAX_MOUSE_BUTTONS> previousButtons{};
            int32_t x = 0, y = 0;
            int32_t deltaX = 0, deltaY = 0;
            int32_t lastX = 0, lastY = 0;
            bool visible = true;
            bool captured = false;
        };

        struct GamepadDevice {
            int fd = -1;
            bool connected = false;
            Core::KSafeString<> name;
            GamepadState state{};
            std::thread inputThread;
            std::atomic<bool> shouldStop{false};
        };

        // Member variables
        bool m_initialized = false;
        
        // X11 Display connection
        Display* m_display = nullptr;
        Window m_window = None;
        int m_screen = 0;
        
        // Input device file descriptors
        int m_keyboardFd = -1;
        int m_mouseFd = -1;
        
        // State tracking
        KeyboardState m_keyboard;
        MouseState m_mouse;
        std::array<GamepadDevice, MAX_GAMEPADS> m_gamepads;
        uint8_t m_connectedGamepadCount = 0;
        
        // Input processing thread
        std::thread m_inputThread;
        std::atomic<bool> m_shouldStop{false};
        mutable std::mutex m_stateMutex;
        
        // Key mapping
        std::unordered_map<uint16_t, KeyCode> m_scancodeToKeyCode;
        std::unordered_map<KeyCode, const char*> m_keyNames;
        std::unordered_map<MouseButton, const char*> m_mouseButtonNames;
        std::unordered_map<GamepadButton, const char*> m_gamepadButtonNames;

        // Private methods
        void InitializeKeyMappings();
        void InitializeDevices();
        void ShutdownDevices();
        void InputThreadFunction();
        void ProcessKeyboardEvents();
        void ProcessMouseEvents();
        void UpdateGamepadStates();
        void ScanForGamepads();
        void ConnectGamepad(uint8_t id, const char* devicePath);
        void DisconnectGamepad(uint8_t id);
        void GamepadThreadFunction(uint8_t gamepadId);
        
        // X11 helpers
        bool InitializeX11();
        void ShutdownX11();
        KeyCode LinuxScanCodeToKeyCode(uint16_t scancode) const;
        MouseButton LinuxButtonToMouseButton(uint8_t button) const;
        
        // Input state helpers
        void UpdateKeyState(KeyCode key, bool pressed);
        void UpdateMouseButtonState(MouseButton button, bool pressed);
        void UpdatePreviousStates();
        
        // Utility functions
        float ApplyDeadzone(float value, float deadzone) const;

    public:
        LinuxInput();
        virtual ~LinuxInput();

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

        // Linux-specific methods
        void SetX11Window(Display* display, Window window);
        bool ProcessX11Event(XEvent* event);
    };

} // namespace VEK::Platform

#endif // VEK_LINUX
