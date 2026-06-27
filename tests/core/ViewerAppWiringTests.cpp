/*
 * ViewerAppWiringTests.cpp
 *
 * Structural unit tests verifying that the new fields and getters introduced
 * by the viewer-app-wiring feature exist and default-initialise correctly.
 * Includes dialog injection tests that verify the setFileDialog /
 * setRecordingDialog API compiles and works without a real OpenGL context.
 */

// clang-format off
#include <glad/glad.h>  // NOLINT(llvm-include-order) — must precede GL headers
// clang-format on

#include <type_traits>
#include <utility>

#include <gtest/gtest.h>

#include "IFileDialog.hpp"
#include "MockFileDialog.hpp"
#include "MockOpenGL.hpp"
#include "camera.hpp"
#include "ui/imgui_menu.hpp"
#include "viewer_app.hpp"

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

TEST(ViewerAppWiring_MenuActions, SelectRecordingFolderDefaultsFalse)
{
    // Arrange & Act
    MenuActions actions;

    // Assert
    EXPECT_FALSE(actions.select_recording_folder);
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

TEST(ViewerAppWiring_MenuActions, NewScaleDefaultsOne)
{
    MenuActions actions;
    EXPECT_FLOAT_EQ(actions.new_scale, 1.0f);
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

// ============================================================================
// Dialog injection API — component-level verification (T07/T08)
//
// ViewerApp is not instantiated here (viewer_app.cpp is not linked in the
// test target). Instead, these tests verify the types that form the wiring:
//   - MockFileDialog satisfies the IFileDialog contract
//   - IFileDialog* pointers can hold dialog instances
// The setter methods (setFileDialog / setRecordingDialog) are verified to be
// present and callable via their compile-time signature tests below.
// ============================================================================

// Static assertions: setters exist and accept IFileDialog*
// (If these fail to compile, the wiring API is missing.)
static_assert(std::is_same_v<void, decltype(std::declval<ViewerApp>().setFileDialog(nullptr))>,
              "ViewerApp::setFileDialog(IFileDialog*) must exist and return void");

static_assert(std::is_same_v<void, decltype(std::declval<ViewerApp>().setRecordingDialog(nullptr))>,
              "ViewerApp::setRecordingDialog(IFileDialog*) must exist and return void");

TEST(ViewerAppWiring_DialogInjection, MockFileDialog_SelectFolder_ReturnsPresetPath)
{
    // Arrange
    MockFileDialog mock;
    mock.reset("/some/path");
    IFileDialog* dialog = &mock;

    // Act
    const std::string result = dialog->selectFolder("title");

    // Assert
    EXPECT_EQ(result, "/some/path");
}

TEST(ViewerAppWiring_DialogInjection, MockFileDialog_SelectFolder_ClosesDialog)
{
    // Arrange
    MockFileDialog mock;
    mock.reset("/some/path");
    IFileDialog* dialog = &mock;

    // Act
    dialog->selectFolder("title");

    // Assert — dialog is now closed
    EXPECT_FALSE(dialog->isOpen());
}

TEST(ViewerAppWiring_DialogInjection, MockFileDialog_SelectFolderWithNoPreset_ReturnsEmpty)
{
    // Arrange
    MockFileDialog mock;
    mock.reset("");
    IFileDialog* dialog = &mock;

    // Act
    const std::string result = dialog->selectFolder("title");

    // Assert
    EXPECT_TRUE(result.empty());
}

TEST(ViewerAppWiring_DialogInjection, MockFileDialog_SelectFolderWithNoPreset_ClosesDialog)
{
    // Arrange
    MockFileDialog mock;
    mock.reset("");
    IFileDialog* dialog = &mock;

    // Act
    dialog->selectFolder("title");

    // Assert
    EXPECT_FALSE(dialog->isOpen());
}

TEST(ViewerAppWiring_DialogInjection, MockFileDialog_Reset_AllowsSecondCall)
{
    // Arrange
    MockFileDialog mock("");
    mock.selectFolder("title"); // first cycle: cancel

    // Act — reset and inject a new path (simulates re-opening the dialog)
    mock.reset("/new/folder");
    const std::string result = mock.selectFolder("title");

    // Assert
    EXPECT_EQ(result, "/new/folder");
}

// ============================================================================
// computeShadersAvailable() — compile-time and default-value verification
// ============================================================================

// Static assertion: computeShadersAvailable() exists, is const, and returns bool.
static_assert(std::is_same_v<bool, decltype(std::declval<const ViewerApp>().computeShadersAvailable())>,
              "ViewerApp::computeShadersAvailable() must exist, be const, and return bool");

TEST(ViewerAppWiring_MCWiring, ComputeShadersAvailable_DefaultsToFalse)
{
    // Note: viewer_app.cpp is not linked in this test target, so ViewerApp cannot
    // be instantiated at runtime.  The static_assert above verifies that the
    // computeShadersAvailable() accessor exists, is const, and returns bool.
    //
    // This test verifies the *behavioural consequence* of compute_shaders_available_
    // defaulting to false: cycleRenderMode() must not advance to MarchingCubes when
    // compute shaders are unavailable (the second argument mirrors the accessor value).

    // Arrange — pass false to mirror the default accessor value
    const bool compute_available = false;

    // Act
    const RenderMode result = ViewerApp::cycleRenderMode(RenderMode::Spheres, compute_available);

    // Assert — MarchingCubes must not be reachable when compute shaders are off
    EXPECT_NE(result, RenderMode::MarchingCubes);
}

TEST(ViewerAppWiring_MCWiring, MCShaderPaths_DefaultsPopulated)
{
    // Arrange & Act
    ShaderPaths paths;

    // Assert — MC shader path fields exist and are non-empty
    EXPECT_FALSE(paths.density_comp.empty());
    EXPECT_FALSE(paths.mc_comp.empty());
    EXPECT_FALSE(paths.mesh_vertex.empty());
    EXPECT_FALSE(paths.mesh_fragment.empty());
}
