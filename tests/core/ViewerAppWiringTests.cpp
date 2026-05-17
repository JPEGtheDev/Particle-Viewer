/*
 * ViewerAppWiringTests.cpp
 *
 * Structural unit tests verifying that the new fields and getters introduced
 * by the viewer-app-wiring feature exist and default-initialise correctly.
 * These tests do NOT instantiate ViewerApp; they test components in isolation.
 */

// clang-format off
#include <glad/glad.h>  // NOLINT(llvm-include-order) — must precede GL headers
// clang-format on

#include <gtest/gtest.h>

#include "MockOpenGL.hpp"
#include "camera.hpp"
#include "ui/imgui_menu.hpp"

// ============================================================================
// MenuState — new fields default-initialise correctly
// ============================================================================

TEST(ViewerAppWiring_MenuState, AutoComComputeDefaultsFalse)
{
    // Arrange & Act
    MenuState state;

    // Assert
    EXPECT_FALSE(state.auto_com_compute);
}

TEST(ViewerAppWiring_MenuState, CacheStatusFramesCachedDefaultsZero)
{
    // Arrange & Act
    MenuState state;

    // Assert
    EXPECT_EQ(state.cache_status.frames_cached, 0);
}

TEST(ViewerAppWiring_MenuState, CacheStatusBytesUsedDefaultsZero)
{
    // Arrange & Act
    MenuState state;

    // Assert
    EXPECT_EQ(state.cache_status.bytes_used, static_cast<std::size_t>(0));
}

// ============================================================================
// MenuActions — new field defaults to false
// ============================================================================

TEST(ViewerAppWiring_MenuActions, ToggleAutoComDefaultsFalse)
{
    // Arrange & Act
    MenuActions actions;

    // Assert
    EXPECT_FALSE(actions.toggle_auto_com);
}

// ============================================================================
// MenuState — Settings panel fields default-initialise correctly
// ============================================================================

TEST(ViewerAppWiring_MenuState, UiScaleDefaultsZero)
{
    MenuState state;
    EXPECT_FLOAT_EQ(state.ui_scale, 0.0f);
}

TEST(ViewerAppWiring_MenuState, SettingsOpenDefaultsFalse)
{
    MenuState state;
    EXPECT_FALSE(state.settings_open);
}

// ============================================================================
// MenuActions — Settings panel fields default-initialise correctly
// ============================================================================

TEST(ViewerAppWiring_MenuActions, ScaleChangedDefaultsFalse)
{
    MenuActions actions;
    EXPECT_FALSE(actions.scale_changed);
}

TEST(ViewerAppWiring_MenuActions, NewScaleDefaultsOnePointFive)
{
    MenuActions actions;
    EXPECT_FLOAT_EQ(actions.new_scale, 1.5f);
}

// ============================================================================
// Camera — isComLocked() returns false on construction
// ============================================================================

TEST(ViewerAppWiring_Camera, IsComLockedReturnsFalseInitially)
{
    // Arrange
    MockOpenGL::reset();
    Camera camera(800, 600);

    // Act & Assert — comLock starts false; getter must reflect that
    EXPECT_FALSE(camera.isComLocked());
}
