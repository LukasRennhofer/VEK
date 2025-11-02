/*
================================================================================
  VEK (Vantor Engine Kernel) - Input System Test Demo
--------------------------------------------------------------------------------
  Author  : Lukas Rennhofer (lukas.renn@aon.at)
  License : GNU General Public License v3.0

  "God made the world. I just made a computer kingdom in it."
      — Terry A. Davis
================================================================================
*/

#include <VEK/VEK.hpp>
#include <glad/glad.h>
#include <cmath>

int main(int argc, char** argv) {
    // Create and initialize platform
    auto os = VEK::Platform::IOS::Create();
    if (!os || !os->Init()) {
        return -1;
    }

    // Initialize console stream
    VEK::Core::KConsoleStream::Initialize(os.get());
    VEK::Core::KConsoleStream::WriteLine("=== VEK Input System Test Demo ===", VEK::Core::KConsoleColor::BrightWhite);

    // Create window
    auto* context = os->GetContext();
    if (!context) {
        VEK::Core::KConsoleStream::WriteLine("Failed to get context!", VEK::Core::KConsoleColor::BrightRed);
        return -1;
    }
    
    VEK::Core::KConsoleStream::WriteLine("Got context, creating window...", VEK::Core::KConsoleColor::Yellow);
    
    // Temporarily undefine Windows CreateWindow macro to avoid conflicts
    #ifdef CreateWindow
    #undef CreateWindow
    #endif
    
    if (!context->CreateWindow(800, 600, "VEK Input Test Demo")) {
        VEK::Core::KConsoleStream::WriteLine("Failed to create window!", VEK::Core::KConsoleColor::BrightRed);
        return -1;
    }

    VEK::Core::KConsoleStream::WriteLine("Window created. GLAD initialized successfully!", VEK::Core::KConsoleColor::Green);

    context->SetWindowFullscreen(true);

    os->ConsolePrintF("This is a wonderfull message, directly from the OS layer!\n");

    // Get input system
    auto* input = os->GetInput();
    if (!input) {
        VEK::Core::KConsoleStream::WriteLine("Failed to get input system!", VEK::Core::KConsoleColor::BrightRed);
        return -1;
    }

    VEK_LOG_INFO("Main", "This is a Test Log!");

    // Check for gamepads
    uint8_t gamepadCount = input->GetConnectedGamepadCount();
    if (gamepadCount > 0) {
        VEK::Core::KConsoleStream::WriteLine("Gamepads detected!", VEK::Core::KConsoleColor::Green);
        for (uint8_t i = 0; i < gamepadCount; ++i) {
            if (input->IsGamepadConnected(i)) {
                const auto* gamepadState = input->GetGamepadState(i);
                if (gamepadState) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad found: Ready for testing!", VEK::Core::KConsoleColor::Green);
                }
            }
        }
    } else {
        VEK::Core::KConsoleStream::WriteLine("No gamepads detected", VEK::Core::KConsoleColor::Yellow);
    }

    // Initialize input tracking variables
    bool mouseVisible = true;
    int32_t lastMouseX = 0, lastMouseY = 0;
    uint32_t frameCount = 0;

    // Main render loop
    while (!context->ShouldClose()) {
        // Poll events
        context->PollEvents();
        
        // Update input
        input->Update();
        frameCount++;

        // Check for exit key
        if (input->IsKeyPressed(VEK::Platform::KeyCode::Escape)) {
            VEK::Core::KConsoleStream::WriteLine("ESC pressed - exiting!", VEK::Core::KConsoleColor::Yellow);
            break;
        }
        
        // Test movement keys
        if (input->IsKeyPressed(VEK::Platform::KeyCode::W)) {
            VEK::Core::KConsoleStream::WriteLine("W pressed - Move Forward!", VEK::Core::KConsoleColor::BrightGreen);
        }
        if (input->IsKeyPressed(VEK::Platform::KeyCode::A)) {
            VEK::Core::KConsoleStream::WriteLine("A pressed - Move Left!", VEK::Core::KConsoleColor::BrightGreen);
        }
        if (input->IsKeyPressed(VEK::Platform::KeyCode::S)) {
            VEK::Core::KConsoleStream::WriteLine("S pressed - Move Backward!", VEK::Core::KConsoleColor::BrightGreen);
        }
        if (input->IsKeyPressed(VEK::Platform::KeyCode::D)) {
            VEK::Core::KConsoleStream::WriteLine("D pressed - Move Right!", VEK::Core::KConsoleColor::BrightGreen);
        }

        // Test arrow keys
        if (input->IsKeyPressed(VEK::Platform::KeyCode::Up)) {
            VEK::Core::KConsoleStream::WriteLine("Up Arrow pressed!", VEK::Core::KConsoleColor::BrightBlue);
        }
        if (input->IsKeyPressed(VEK::Platform::KeyCode::Down)) {
            VEK::Core::KConsoleStream::WriteLine("Down Arrow pressed!", VEK::Core::KConsoleColor::BrightBlue);
        }
        if (input->IsKeyPressed(VEK::Platform::KeyCode::Left)) {
            VEK::Core::KConsoleStream::WriteLine("Left Arrow pressed!", VEK::Core::KConsoleColor::BrightBlue);
        }
        if (input->IsKeyPressed(VEK::Platform::KeyCode::Right)) {
            VEK::Core::KConsoleStream::WriteLine("Right Arrow pressed!", VEK::Core::KConsoleColor::BrightBlue);
        }

        // Test space key
        if (input->IsKeyPressed(VEK::Platform::KeyCode::Space)) {
            VEK::Core::KConsoleStream::WriteLine("SPACE pressed - Jump!", VEK::Core::KConsoleColor::BrightYellow);
        }

        // Test mouse visibility toggle
        if (input->IsKeyPressed(VEK::Platform::KeyCode::M)) {
            mouseVisible = !mouseVisible;
            input->SetMouseVisible(mouseVisible);
            if (mouseVisible) {
                VEK::Core::KConsoleStream::WriteLine("Mouse cursor shown", VEK::Core::KConsoleColor::Magenta);
            } else {
                VEK::Core::KConsoleStream::WriteLine("Mouse cursor hidden", VEK::Core::KConsoleColor::Magenta);
            }
        }

        // Test gamepad detection
        if (input->IsKeyPressed(VEK::Platform::KeyCode::G)) {
            gamepadCount = input->GetConnectedGamepadCount();
            VEK::Core::KConsoleStream::WriteLine("Gamepad check - connected:", VEK::Core::KConsoleColor::Green);
        }
        
        // Test mouse buttons
        if (input->IsMouseButtonPressed(VEK::Platform::MouseButton::Left)) {
            int32_t mouseX, mouseY;
            input->GetMousePosition(mouseX, mouseY);
            VEK::Core::KConsoleStream::WriteLine("Left mouse clicked!", VEK::Core::KConsoleColor::BrightMagenta);
        }
        
        if (input->IsMouseButtonPressed(VEK::Platform::MouseButton::Right)) {
            VEK::Core::KConsoleStream::WriteLine("Right mouse clicked!", VEK::Core::KConsoleColor::BrightMagenta);
        }
        
        if (input->IsMouseButtonPressed(VEK::Platform::MouseButton::Middle)) {
            VEK::Core::KConsoleStream::WriteLine("Middle mouse clicked!", VEK::Core::KConsoleColor::BrightMagenta);
        }

        // Track mouse movement (only report every 30 frames to avoid spam)
        if (frameCount % 30 == 0) {
            int32_t mouseX, mouseY;
            input->GetMousePosition(mouseX, mouseY);
            int32_t deltaX, deltaY;
            input->GetMouseDelta(deltaX, deltaY);
            
            if (mouseX != lastMouseX || mouseY != lastMouseY) {
                lastMouseX = mouseX;
                lastMouseY = mouseY;
            }
        }
        
        for (uint8_t i = 0; i < gamepadCount; ++i) {
            if (input->IsGamepadConnected(i)) {
                // Test gamepad buttons
                if (input->IsGamepadButtonPressed(i, VEK::Platform::GamepadButton::A)) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad A button pressed!", VEK::Core::KConsoleColor::BrightRed);
                }
                if (input->IsGamepadButtonPressed(i, VEK::Platform::GamepadButton::B)) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad B button pressed!", VEK::Core::KConsoleColor::BrightRed);
                }
                if (input->IsGamepadButtonPressed(i, VEK::Platform::GamepadButton::X)) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad X button pressed!", VEK::Core::KConsoleColor::BrightRed);
                }
                if (input->IsGamepadButtonPressed(i, VEK::Platform::GamepadButton::Y)) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad Y button pressed!", VEK::Core::KConsoleColor::BrightRed);
                }

                // Test D-pad
                if (input->IsGamepadButtonPressed(i, VEK::Platform::GamepadButton::DpadUp)) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad D-Pad Up!", VEK::Core::KConsoleColor::BrightBlue);
                }
                if (input->IsGamepadButtonPressed(i, VEK::Platform::GamepadButton::DpadDown)) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad D-Pad Down!", VEK::Core::KConsoleColor::BrightBlue);
                }
                if (input->IsGamepadButtonPressed(i, VEK::Platform::GamepadButton::DpadLeft)) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad D-Pad Left!", VEK::Core::KConsoleColor::BrightBlue);
                }
                if (input->IsGamepadButtonPressed(i, VEK::Platform::GamepadButton::DpadRight)) {
                    VEK::Core::KConsoleStream::WriteLine("Gamepad D-Pad Right!", VEK::Core::KConsoleColor::BrightBlue);
                }

                // Test analog sticks (only report significant movement)
                float leftX = input->GetGamepadAxis(i, VEK::Platform::GamepadAxis::LeftX);
                float leftY = input->GetGamepadAxis(i, VEK::Platform::GamepadAxis::LeftY);
                float rightX = input->GetGamepadAxis(i, VEK::Platform::GamepadAxis::RightX);
                float rightY = input->GetGamepadAxis(i, VEK::Platform::GamepadAxis::RightY);
                
                if (std::abs(leftX) > 0.5f || std::abs(leftY) > 0.5f) {
                    VEK::Core::KConsoleStream::WriteLine("Left stick moved!", VEK::Core::KConsoleColor::Cyan);
                }
                if (std::abs(rightX) > 0.5f || std::abs(rightY) > 0.5f) {
                    VEK::Core::KConsoleStream::WriteLine("Right stick moved!", VEK::Core::KConsoleColor::Cyan);
                }

                // Test triggers
                float leftTrigger = input->GetGamepadAxis(i, VEK::Platform::GamepadAxis::LeftTrigger);
                float rightTrigger = input->GetGamepadAxis(i, VEK::Platform::GamepadAxis::RightTrigger);
                
                if (leftTrigger > 0.5f) {
                    VEK::Core::KConsoleStream::WriteLine("Left trigger pressed!", VEK::Core::KConsoleColor::BrightYellow);
                }
                if (rightTrigger > 0.5f) {
                    VEK::Core::KConsoleStream::WriteLine("Right trigger pressed!", VEK::Core::KConsoleColor::BrightYellow);
                }
            }
        }

        // Change background color based on held keys
        float r = 0.1f;
        float g = 0.1f;
        float b = 0.2f;
        
        if (input->IsKeyHeld(VEK::Platform::KeyCode::W)) r += 0.4f;
        if (input->IsKeyHeld(VEK::Platform::KeyCode::A)) g += 0.4f;
        if (input->IsKeyHeld(VEK::Platform::KeyCode::S)) b += 0.4f;
        if (input->IsKeyHeld(VEK::Platform::KeyCode::D)) {
            r += 0.3f;
            g += 0.3f;
        }
        
        // Add mouse click effects
        if (input->IsMouseButtonHeld(VEK::Platform::MouseButton::Left)) {
            r += 0.2f;
        }
        if (input->IsMouseButtonHeld(VEK::Platform::MouseButton::Right)) {
            b += 0.2f;
        }
        
        // Add gamepad effects
        for (uint8_t i = 0; i < gamepadCount; ++i) {
            if (input->IsGamepadConnected(i)) {
                if (input->IsGamepadButtonHeld(i, VEK::Platform::GamepadButton::A)) {
                    r += 0.3f;
                }
                if (input->IsGamepadButtonHeld(i, VEK::Platform::GamepadButton::B)) {
                    g += 0.3f;
                }
                if (input->IsGamepadButtonHeld(i, VEK::Platform::GamepadButton::X)) {
                    b += 0.3f;
                }
            }
        }
        
        // Clamp colors
        r = std::min(1.0f, r);
        g = std::min(1.0f, g);
        b = std::min(1.0f, b);
        
        glClearColor(r, g, b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Present frame
        context->SwapBuffers();

        // Limit to around 60 FPS
        os->Sleep(16);
    }

    VEK::Core::KConsoleStream::WriteLine("Demo finished!", VEK::Core::KConsoleColor::Green);

    // Cleanup
    VEK::Core::KConsoleStream::Shutdown();
    context->DestroyWindow();
    os->Shutdown();

    return 0;
}
