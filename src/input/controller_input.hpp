/*
 * controller_input.hpp
 *
 * Controller input handler for gamepad/Xbox controller support.
 * Uses GLFW 3.3+ gamepad API to provide standardized controller input.
 *
 * Architecture:
 * - Polls gamepad state each frame via GLFW
 * - Converts stick axes and button states to camera/playback commands
 * - Auto-detects first available gamepad from all joystick slots (0-15)
 * - Supports Steam Deck, Xbox controllers, and generic xinput gamepads
 * - Hardcoded Xbox-style button mapping (no customization in v1)
 *
 * Button Mapping (Xbox 360/One layout):
 * - Left Stick: Camera movement (forward/backward/strafe)
 * - Right Stick: Camera rotation (pan/tilt) or zoom when point lock active
 * - Left Trigger: Frame rewind
 * - Right Trigger: Frame fast-forward
 * - Left Bumper: Previous frame
 * - Right Bumper: Next frame
 * - A Button: Toggle play/pause
 * - B Button: (reserved)
 * - X Button: Toggle point lock/bracket
 * - Y Button: Toggle COM lock
 * - Start: (reserved)
 * - Back/Select: Open file load dialog
 * - L3 (Left Stick Button): Move sphere closer
 * - R3 (Right Stick Button): Move sphere farther
 *
 * Limitations:
 * - No haptic feedback (GLFW 3.3 limitation)
 * - Single controller only (first detected gamepad is used)
 * - No user-customizable mapping (future enhancement)
 */

#ifndef PARTICLE_VIEWER_CONTROLLER_INPUT_H
#define PARTICLE_VIEWER_CONTROLLER_INPUT_H

#include <cmath>
#include <iostream>

#include <GLFW/glfw3.h>

/*
 * Dead zone for analog stick inputs to prevent drift.
 * Typical values: 0.1 - 0.2 (10% - 20% of full range)
 */
constexpr float STICK_DEADZONE = 0.15f;

/*
 * Trigger threshold for digital button behavior.
 * Triggers report 0.0-1.0, treat as "pressed" above this value.
 */
constexpr float TRIGGER_THRESHOLD = 0.2f;

/*
 * Controller state captured from GLFW gamepad API.
 * All axis values are in range [-1.0, 1.0] (already normalized by GLFW).
 * Button states are boolean (GLFW_PRESS or GLFW_RELEASE).
 */
struct ControllerState
{
    // Left stick (movement)
    float left_stick_x = 0.0f;
    float left_stick_y = 0.0f;

    // Right stick (camera rotation or zoom)
    float right_stick_x = 0.0f;
    float right_stick_y = 0.0f;

    // Triggers (analog, 0.0 to 1.0)
    float left_trigger = 0.0f;
    float right_trigger = 0.0f;

    // Face buttons
    bool button_a = false;
    bool button_b = false;
    bool button_x = false;
    bool button_y = false;

    // Shoulder buttons
    bool left_bumper = false;
    bool right_bumper = false;

    // System buttons
    bool button_start = false;
    bool button_back = false;

    // Stick buttons
    bool left_stick_button = false;
    bool right_stick_button = false;

    // D-pad (optional, not used in initial mapping)
    bool dpad_up = false;
    bool dpad_down = false;
    bool dpad_left = false;
    bool dpad_right = false;
};

/*
 * Controller input handler.
 * Polls gamepad state and provides high-level input queries.
 */
class ControllerInput
{
  public:
    /*
     * Initialize controller input system.
     * Scans all joystick slots to find the first available gamepad.
     * Starts with slot 0 (most common, including Steam Deck).
     */
    ControllerInput() : joystick_id_(GLFW_JOYSTICK_1), was_connected_(false)
    {
        // Scan all joystick slots to find first available gamepad
        // GLFW_JOYSTICK_1 is 0, GLFW_JOYSTICK_2 is 1, etc.
        for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i) && glfwJoystickIsGamepad(i)) {
                joystick_id_ = i;
                std::cout << "Controller detected at slot " << i << ": " << glfwGetGamepadName(i) << std::endl;
                break;
            }
        }
    }

    /*
     * Check if a gamepad is connected and available.
     * Uses GLFW_JOYSTICK_1 (first controller).
     */
    bool isConnected() const
    {
        return glfwJoystickPresent(joystick_id_) && glfwJoystickIsGamepad(joystick_id_);
    }

    /*
     * Get human-readable controller name (e.g., "Xbox 360 Controller").
     * Returns empty string if not connected.
     */
    const char* getControllerName() const
    {
        if (isConnected()) {
            return glfwGetGamepadName(joystick_id_);
        }
        return "";
    }

    /*
     * Poll controller state and update internal state.
     * Call once per frame before processing input.
     * Returns true if controller is connected and state was updated.
     */
    bool poll()
    {
        bool connected = isConnected();

        // Detect connection/disconnection events
        if (connected && !was_connected_) {
            // Controller just connected
            was_connected_ = true;
        } else if (!connected && was_connected_) {
            // Controller just disconnected - reset state
            was_connected_ = false;
            state_ = ControllerState(); // reset to defaults
            return false;
        }

        if (!connected) {
            return false;
        }

        // Get gamepad state from GLFW
        GLFWgamepadstate glfw_state;
        if (glfwGetGamepadState(joystick_id_, &glfw_state) != GLFW_TRUE) {
            return false;
        }

        // Update state structure
        // Left stick (note: GLFW Y axis is inverted for movement)
        state_.left_stick_x = applyDeadzone(glfw_state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
        state_.left_stick_y = applyDeadzone(-glfw_state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]); // invert Y

        // Right stick (note: GLFW Y axis is inverted for camera look)
        state_.right_stick_x = applyDeadzone(glfw_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]);
        state_.right_stick_y = applyDeadzone(-glfw_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]); // invert Y

        // Triggers (convert from [-1,1] to [0,1] range)
        state_.left_trigger = (glfw_state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1.0f) * 0.5f;
        state_.right_trigger = (glfw_state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) * 0.5f;

        // Face buttons
        state_.button_a = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_A] == GLFW_PRESS;
        state_.button_b = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_B] == GLFW_PRESS;
        state_.button_x = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_X] == GLFW_PRESS;
        state_.button_y = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_Y] == GLFW_PRESS;

        // Shoulder buttons
        state_.left_bumper = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS;
        state_.right_bumper = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS;

        // System buttons
        state_.button_start = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS;
        state_.button_back = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_BACK] == GLFW_PRESS;

        // Stick buttons
        state_.left_stick_button = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB] == GLFW_PRESS;
        state_.right_stick_button = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_THUMB] == GLFW_PRESS;

        // D-pad
        state_.dpad_up = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS;
        state_.dpad_down = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS;
        state_.dpad_left = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS;
        state_.dpad_right = glfw_state.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS;

        return true;
    }

    /*
     * Get current controller state.
     * Only valid after poll() returns true.
     */
    const ControllerState& getState() const
    {
        return state_;
    }

    /*
     * Check if controller was just connected this frame.
     */
    bool wasJustConnected() const
    {
        return isConnected() && was_connected_;
    }

  private:
    int joystick_id_;
    bool was_connected_;
    ControllerState state_;

    /*
     * Apply circular deadzone to analog stick input.
     * Returns 0.0 if magnitude is below threshold, otherwise returns input.
     */
    float applyDeadzone(float value) const
    {
        if (std::abs(value) < STICK_DEADZONE) {
            return 0.0f;
        }
        // Scale to remove deadzone (map [deadzone, 1.0] to [0.0, 1.0])
        float sign = (value > 0.0f) ? 1.0f : -1.0f;
        float magnitude = std::abs(value);
        return sign * ((magnitude - STICK_DEADZONE) / (1.0f - STICK_DEADZONE));
    }
};

#endif // PARTICLE_VIEWER_CONTROLLER_INPUT_H
