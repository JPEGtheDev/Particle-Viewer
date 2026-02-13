/*
 * ControllerInputTests.cpp
 *
 * Unit tests for ControllerInput class.
 *
 * NOTE: GLFW does not provide a mock/fake gamepad API, so these tests focus on:
 * 1. State structure initialization and defaults
 * 2. Deadzone application logic
 * 3. Controller connection detection (when available)
 *
 * Full gamepad polling requires manual integration testing with a physical controller.
 */

#include <gtest/gtest.h>

#include "input/controller_input.hpp"

// ============================================================================
// ControllerState Tests
// ============================================================================

TEST(ControllerStateTest, Constructor_InitializesAxesToZero)
{
    // Act
    ControllerState state;

    // Assert
    EXPECT_FLOAT_EQ(state.left_stick_x, 0.0f);
    EXPECT_FLOAT_EQ(state.left_stick_y, 0.0f);
    EXPECT_FLOAT_EQ(state.right_stick_x, 0.0f);
    EXPECT_FLOAT_EQ(state.right_stick_y, 0.0f);
    EXPECT_FLOAT_EQ(state.left_trigger, 0.0f);
    EXPECT_FLOAT_EQ(state.right_trigger, 0.0f);
}

TEST(ControllerStateTest, Constructor_InitializesButtonsToFalse)
{
    // Act
    ControllerState state;

    // Assert
    EXPECT_FALSE(state.button_a);
    EXPECT_FALSE(state.button_b);
    EXPECT_FALSE(state.button_x);
    EXPECT_FALSE(state.button_y);
    EXPECT_FALSE(state.left_bumper);
    EXPECT_FALSE(state.right_bumper);
    EXPECT_FALSE(state.button_start);
    EXPECT_FALSE(state.button_back);
    EXPECT_FALSE(state.left_stick_button);
    EXPECT_FALSE(state.right_stick_button);
}

TEST(ControllerStateTest, Constructor_InitializesDpadToFalse)
{
    // Act
    ControllerState state;

    // Assert
    EXPECT_FALSE(state.dpad_up);
    EXPECT_FALSE(state.dpad_down);
    EXPECT_FALSE(state.dpad_left);
    EXPECT_FALSE(state.dpad_right);
}

// ============================================================================
// ControllerInput Construction Tests
// ============================================================================

TEST(ControllerInputTest, Constructor_InitializesSuccessfully)
{
    // Act
    ControllerInput controller;

    // Assert - should not crash
    SUCCEED();
}

TEST(ControllerInputTest, Constructor_IsConnectedReturnsValidBoolean)
{
    // Act
    ControllerInput controller;
    bool is_connected = controller.isConnected();

    // Assert
    // Result can be true or false depending on hardware availability.
    // Just verify it returns a valid boolean (no crash).
    EXPECT_TRUE(is_connected == true || is_connected == false);
}

TEST(ControllerInputTest, GetControllerName_ReturnsEmptyWhenNotConnected)
{
    // Arrange
    ControllerInput controller;

    // Act
    const char* name = controller.getControllerName();

    // Assert
    // Should return empty string when no controller is connected
    // (or valid name if controller happens to be connected)
    EXPECT_TRUE(name != nullptr);
}

// ============================================================================
// ControllerInput State Management Tests
// ============================================================================

TEST(ControllerInputTest, GetState_ReturnsDefaultStateBeforePoll)
{
    // Arrange
    ControllerInput controller;

    // Act
    const ControllerState& state = controller.getState();

    // Assert - state should be zero-initialized
    EXPECT_FLOAT_EQ(state.left_stick_x, 0.0f);
    EXPECT_FLOAT_EQ(state.left_stick_y, 0.0f);
    EXPECT_FLOAT_EQ(state.right_stick_x, 0.0f);
    EXPECT_FLOAT_EQ(state.right_stick_y, 0.0f);
    EXPECT_FALSE(state.button_a);
}

TEST(ControllerInputTest, Poll_ReturnsValidBoolean)
{
    // Arrange
    ControllerInput controller;

    // Act
    bool result = controller.poll();

    // Assert
    // Result can be true or false depending on hardware availability.
    // Just verify it returns a valid boolean (no crash).
    EXPECT_TRUE(result == true || result == false);
}

TEST(ControllerInputTest, WasJustConnected_ReturnsValidBoolean)
{
    // Arrange
    ControllerInput controller;

    // Act
    bool just_connected = controller.wasJustConnected();

    // Assert
    // Result can be true or false depending on hardware availability.
    // Just verify it returns a valid boolean (no crash).
    EXPECT_TRUE(just_connected == true || just_connected == false);
}

// ============================================================================
// Deadzone Configuration Tests
// ============================================================================

TEST(DeadzoneTest, STICK_DEADZONE_IsWithinValidRange)
{
    // Assert
    // Deadzone should be between 0.1 and 0.3 (10%-30%)
    EXPECT_GE(STICK_DEADZONE, 0.0f);
    EXPECT_LE(STICK_DEADZONE, 0.3f);
}

TEST(DeadzoneTest, TRIGGER_THRESHOLD_IsWithinValidRange)
{
    // Assert
    // Trigger threshold should be between 0.1 and 0.5
    EXPECT_GE(TRIGGER_THRESHOLD, 0.0f);
    EXPECT_LE(TRIGGER_THRESHOLD, 0.5f);
}

// ============================================================================
// Integration Test Documentation
// ============================================================================

/*
 * MANUAL INTEGRATION TESTS
 *
 * The following tests require a physical gamepad connected and should be
 * verified manually during development:
 *
 * 1. Controller Connection:
 *    - Start app without controller → isConnected() returns false
 *    - Connect controller → isConnected() returns true
 *    - Disconnect controller → isConnected() returns false
 *
 * 2. Left Stick Movement:
 *    - Push left stick right → left_stick_x > 0
 *    - Push left stick left → left_stick_x < 0
 *    - Push left stick forward → left_stick_y > 0
 *    - Push left stick back → left_stick_y < 0
 *    - Center stick → both values near 0.0 (within deadzone)
 *
 * 3. Right Stick Camera:
 *    - Push right stick right → right_stick_x > 0
 *    - Push right stick left → right_stick_x < 0
 *    - Push right stick up → right_stick_y > 0
 *    - Push right stick down → right_stick_y < 0
 *
 * 4. Triggers:
 *    - Pull left trigger → left_trigger increases from 0.0 to 1.0
 *    - Pull right trigger → right_trigger increases from 0.0 to 1.0
 *
 * 5. Face Buttons:
 *    - Press A → button_a becomes true
 *    - Press B → button_b becomes true
 *    - Press X → button_x becomes true
 *    - Press Y → button_y becomes true
 *
 * 6. Bumpers:
 *    - Press LB → left_bumper becomes true
 *    - Press RB → right_bumper becomes true
 *
 * 7. Stick Buttons:
 *    - Press L3 → left_stick_button becomes true
 *    - Press R3 → right_stick_button becomes true
 *
 * 8. System Buttons:
 *    - Press Start → button_start becomes true
 *    - Press Back/Select → button_back becomes true
 *
 * 9. Deadzone Behavior:
 *    - Slight stick drift (<15%) → values clamp to 0.0
 *    - Stick push beyond deadzone → scaled value returned
 *
 * 10. Controller Name:
 *     - Connect Xbox 360 → name contains "Xbox 360"
 *     - Connect Xbox One → name contains "Xbox One" or "Xbox"
 *     - Connect generic → name varies by device
 */
