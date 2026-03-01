/*
 * GamepadInputTests.cpp
 *
 * Unit tests for GamepadInput's pure utility functions:
 * normalizeAxis() and normalizeTrigger().
 *
 * These functions contain all the non-trivial mapping logic and are
 * exercised here without requiring real SDL3 hardware.
 */

#include <glad/glad.h>
#include <gtest/gtest.h>

#include "MockOpenGL.hpp"
#include "input/gamepad_input.hpp"

class GamepadInputTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
    }
};

// ============================================================================
// normalizeAxis — zero / dead zone
// ============================================================================

TEST_F(GamepadInputTest, NormalizeAxis_ZeroRaw_ReturnsZero)
{
    // Act
    float result = GamepadInput::normalizeAxis(0);

    // Assert
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(GamepadInputTest, NormalizeAxis_WithinDeadZone_ReturnsZero)
{
    // Arrange — value just inside the 15 % dead zone (< 0.15 * 32767 ≈ 4915)
    Sint16 raw = 4000;

    // Act
    float result = GamepadInput::normalizeAxis(raw);

    // Assert
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(GamepadInputTest, NormalizeAxis_NegativeWithinDeadZone_ReturnsZero)
{
    // Arrange
    Sint16 raw = -4000;

    // Act
    float result = GamepadInput::normalizeAxis(raw);

    // Assert
    EXPECT_FLOAT_EQ(result, 0.0f);
}

// ============================================================================
// normalizeAxis — max values
// ============================================================================

TEST_F(GamepadInputTest, NormalizeAxis_PositiveMax_ReturnsOne)
{
    // Arrange
    Sint16 raw = 32767;

    // Act
    float result = GamepadInput::normalizeAxis(raw);

    // Assert
    EXPECT_NEAR(result, 1.0f, 1e-4f);
}

TEST_F(GamepadInputTest, NormalizeAxis_NegativeMax_ReturnsNegativeOne)
{
    // Arrange — SDL3 minimum axis value
    Sint16 raw = -32768;

    // Act
    float result = GamepadInput::normalizeAxis(raw);

    // Assert
    EXPECT_NEAR(result, -1.0f, 1e-4f);
}

// ============================================================================
// normalizeAxis — rescaling outside dead zone
// ============================================================================

TEST_F(GamepadInputTest, NormalizeAxis_JustAboveDeadZone_ReturnsSmallPositive)
{
    // Arrange — value slightly above dead zone threshold
    constexpr float dead_zone = GamepadInput::DEFAULT_DEAD_ZONE;
    Sint16 raw = static_cast<Sint16>((dead_zone + 0.01f) * 32767.0f);
    float expected_normalized = (dead_zone + 0.01f);
    float expected_rescaled = (expected_normalized - dead_zone) / (1.0f - dead_zone);

    // Act
    float result = GamepadInput::normalizeAxis(raw);

    // Assert
    EXPECT_NEAR(result, expected_rescaled, 0.01f);
    EXPECT_GT(result, 0.0f);
}

TEST_F(GamepadInputTest, NormalizeAxis_NegativeJustAboveDeadZone_ReturnsSmallNegative)
{
    // Arrange
    constexpr float dead_zone = GamepadInput::DEFAULT_DEAD_ZONE;
    Sint16 raw = static_cast<Sint16>(-(dead_zone + 0.01f) * 32768.0f);

    // Act
    float result = GamepadInput::normalizeAxis(raw);

    // Assert
    EXPECT_LT(result, 0.0f);
}

// ============================================================================
// normalizeAxis — custom dead zone
// ============================================================================

TEST_F(GamepadInputTest, NormalizeAxis_CustomDeadZone_ZeroInsideZone)
{
    // Arrange — value is 20 % of range; custom dead zone 25 %
    Sint16 raw = static_cast<Sint16>(0.20f * 32767.0f);
    constexpr float custom_dead_zone = 0.25f;

    // Act
    float result = GamepadInput::normalizeAxis(raw, custom_dead_zone);

    // Assert
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(GamepadInputTest, NormalizeAxis_CustomDeadZone_NonZeroOutsideZone)
{
    // Arrange — value is 50 % of range; custom dead zone 25 %
    Sint16 raw = static_cast<Sint16>(0.50f * 32767.0f);
    constexpr float custom_dead_zone = 0.25f;

    // Act
    float result = GamepadInput::normalizeAxis(raw, custom_dead_zone);

    // Assert
    EXPECT_GT(result, 0.0f);
    EXPECT_LE(result, 1.0f);
}

// ============================================================================
// normalizeTrigger
// ============================================================================

TEST_F(GamepadInputTest, NormalizeTrigger_Zero_ReturnsZero)
{
    // Act
    float result = GamepadInput::normalizeTrigger(0);

    // Assert
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(GamepadInputTest, NormalizeTrigger_MaxValue_ReturnsOne)
{
    // Arrange
    Sint16 raw = 32767;

    // Act
    float result = GamepadInput::normalizeTrigger(raw);

    // Assert
    EXPECT_NEAR(result, 1.0f, 1e-4f);
}

TEST_F(GamepadInputTest, NormalizeTrigger_HalfValue_ReturnsApproxHalf)
{
    // Arrange
    Sint16 raw = 16383;

    // Act
    float result = GamepadInput::normalizeTrigger(raw);

    // Assert
    EXPECT_NEAR(result, 0.5f, 0.01f);
}

TEST_F(GamepadInputTest, NormalizeTrigger_NegativeRaw_ReturnsZero)
{
    // Arrange — triggers should never be negative, but guard against it
    Sint16 raw = -100;

    // Act
    float result = GamepadInput::normalizeTrigger(raw);

    // Assert
    EXPECT_FLOAT_EQ(result, 0.0f);
}

TEST_F(GamepadInputTest, NormalizeTrigger_ResultClamped_NeverExceedsOne)
{
    // Arrange — use max Sint16 value
    Sint16 raw = 32767;

    // Act
    float result = GamepadInput::normalizeTrigger(raw);

    // Assert
    EXPECT_LE(result, 1.0f);
}

// ============================================================================
// GamepadInput — disconnected state
// ============================================================================

TEST_F(GamepadInputTest, GamepadInput_DefaultConstruction_NotConnected)
{
    // Arrange & Act
    GamepadInput gamepad;

    // Assert
    EXPECT_FALSE(gamepad.isConnected());
}

TEST_F(GamepadInputTest, GamepadInput_WhenDisconnected_AllAxesReturnZero)
{
    // Arrange
    GamepadInput gamepad;

    // Act & Assert
    EXPECT_FLOAT_EQ(gamepad.getLeftStickX(), 0.0f);
    EXPECT_FLOAT_EQ(gamepad.getLeftStickY(), 0.0f);
    EXPECT_FLOAT_EQ(gamepad.getRightStickX(), 0.0f);
    EXPECT_FLOAT_EQ(gamepad.getRightStickY(), 0.0f);
    EXPECT_FLOAT_EQ(gamepad.getLeftTrigger(), 0.0f);
    EXPECT_FLOAT_EQ(gamepad.getRightTrigger(), 0.0f);
}

TEST_F(GamepadInputTest, GamepadInput_WhenDisconnected_AllButtonsNotHeld)
{
    // Arrange
    GamepadInput gamepad;

    // Act & Assert — spot-check a few buttons
    EXPECT_FALSE(gamepad.isButtonHeld(SDL_GAMEPAD_BUTTON_SOUTH));
    EXPECT_FALSE(gamepad.isButtonHeld(SDL_GAMEPAD_BUTTON_NORTH));
    EXPECT_FALSE(gamepad.isButtonHeld(SDL_GAMEPAD_BUTTON_BACK));
}

TEST_F(GamepadInputTest, GamepadInput_WhenDisconnected_NoButtonsJustPressed)
{
    // Arrange
    GamepadInput gamepad;

    // Act & Assert
    EXPECT_FALSE(gamepad.isButtonJustPressed(SDL_GAMEPAD_BUTTON_SOUTH));
    EXPECT_FALSE(gamepad.isButtonJustPressed(SDL_GAMEPAD_BUTTON_NORTH));
}

TEST_F(GamepadInputTest, GamepadInput_Poll_WhenDisconnected_DoesNotCrash)
{
    // Arrange
    GamepadInput gamepad;

    // Act & Assert
    EXPECT_NO_THROW({ gamepad.poll(); });
}
