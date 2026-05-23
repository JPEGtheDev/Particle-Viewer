/*
 * ControllerPanelIntegrationTests.cpp
 *
 * Compile-probe tests for the controller panel feature (Issue #113).
 *
 * Verifies that MenuState and MenuActions types from imgui_menu.hpp compose
 * correctly with the InputMode enum from input/input_mode.hpp in a single
 * translation unit, and that all struct fields default-initialize correctly.
 *
 * Tests that require an active ImGui/OpenGL context (renderMainMenu(),
 * renderControllerPanel(), ViewerApp) are covered by visual regression tests.
 * These tests cover struct defaults and header coexistence only.
 */

#include <gtest/gtest.h>

#include "input/input_mode.hpp"
#include "ui/imgui_menu.hpp"

// --- MenuState default values ---

TEST(ControllerPanelCompileProbe, MenuState_ControllerPanelFields_DefaultsCorrect)
{
    MenuState state;
    EXPECT_FALSE(state.controller_panel_open);
    EXPECT_EQ(state.selected_panel_item, -1);
    EXPECT_EQ(state.panel_item_count, 0);
    EXPECT_FALSE(state.confirm_panel_item);
    EXPECT_FALSE(state.button_hints_visible);
    EXPECT_TRUE(state.file_loading_enabled);
}

TEST(ControllerPanelCompileProbe, MenuState_BaseFields_DefaultsCorrect)
{
    MenuState state;
    EXPECT_FALSE(state.visible);
    EXPECT_FALSE(state.debug_mode);
    EXPECT_FALSE(state.auto_com_compute);
    EXPECT_FALSE(state.settings_open);
    EXPECT_FLOAT_EQ(state.ui_scale, 0.0f);
}

// --- MenuActions default values ---

TEST(ControllerPanelCompileProbe, MenuActions_AllBoolFlags_DefaultFalse)
{
    MenuActions actions;
    EXPECT_FALSE(actions.quit);
    EXPECT_FALSE(actions.load_file);
    EXPECT_FALSE(actions.select_recording_folder);
    EXPECT_FALSE(actions.toggle_fullscreen);
    EXPECT_FALSE(actions.toggle_auto_com);
    EXPECT_FALSE(actions.toggle_debug_mode);
    EXPECT_FALSE(actions.change_resolution);
    EXPECT_FALSE(actions.scale_changed);
}

// --- Compilation probe ---

TEST(ControllerPanelCompileProbe, TypesCoexist_InSameTranslationUnit)
{
    // If this file compiles, InputMode and MenuState headers do not conflict.
    MenuState state;
    InputMode mode = InputMode::MenuMode;
    (void)state;
    (void)mode;
}
