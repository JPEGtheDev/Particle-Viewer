// Controller panel unit tests (Issue #113).
// This file grows across Todos 1–7 to cover: InputMode, MenuState/MenuActions fields,
// toggle logic rules, D-pad navigation, and auto-pause rules.

#include <gtest/gtest.h>

#include "input/input_mode.hpp"
#include "ui/imgui_menu.hpp"
#include "viewer_app.hpp"

TEST(InputMode, DefaultInitialized_EqualsViewMode)
{
    InputMode mode{};
    EXPECT_EQ(mode, InputMode::ViewMode);
}

// Guards against accidental duplicate enumerator values (e.g. ViewMode = 0, MenuMode = 0).
TEST(InputMode, TwoModes_AreDistinct)
{
    EXPECT_NE(InputMode::ViewMode, InputMode::MenuMode);
}

// --- MenuState new fields (Todo 2) ---

TEST(MenuActions, ClosePanelDefaultsFalse)
{
    MenuActions a;
    EXPECT_FALSE(a.close_panel);
}

TEST(MenuState, ControllerPanelOpen_DefaultsFalse)
{
    MenuState s;
    EXPECT_FALSE(s.controller_panel_open);
}

TEST(MenuState, ButtonHintsVisible_DefaultsFalse)
{
    MenuState s;
    EXPECT_FALSE(s.button_hints_visible);
}

TEST(MenuState, FileLoadingEnabled_DefaultsTrue)
{
    MenuState s;
    EXPECT_TRUE(s.file_loading_enabled);
}

TEST(MenuState, SelectedPanelItem_DefaultsNegativeOne)
{
    MenuState s;
    EXPECT_EQ(s.selected_panel_item, -1);
}

TEST(MenuState, PanelItemCount_DefaultsZero)
{
    MenuState s;
    EXPECT_EQ(s.panel_item_count, 0);
}

TEST(MenuState, ConfirmPanelItem_DefaultsFalse)
{
    MenuState s;
    EXPECT_FALSE(s.confirm_panel_item);
}

// --- MenuActions new field (Todo 2) ---

TEST(MenuActions, ToggleDebugMode_DefaultsFalse)
{
    MenuActions a;
    EXPECT_FALSE(a.toggle_debug_mode);
}

TEST(MenuState, IsRecording_DefaultsFalse)
{
    MenuState s;
    EXPECT_FALSE(s.is_recording);
}

TEST(MenuActions, StopRecording_DefaultsFalse)
{
    MenuActions a;
    EXPECT_FALSE(a.stop_recording);
}

// --- ViewerApp nav timer constants (Todo 3) ---
// These require: NAV_INITIAL_DELAY_MS and NAV_REPEAT_DELAY_MS to be public static constexpr on ViewerApp

TEST(ViewerApp, NavInitialDelayMs_IsThreeHundred)
{
    EXPECT_EQ(ViewerApp::NAV_INITIAL_DELAY_MS, Uint64{300});
}

TEST(ViewerApp, NavRepeatDelayMs_IsOneFifty)
{
    EXPECT_EQ(ViewerApp::NAV_REPEAT_DELAY_MS, Uint64{150});
}

TEST(ViewerApp, NavRepeatDelayMs_LessThanInitialDelay)
{
    EXPECT_LT(ViewerApp::NAV_REPEAT_DELAY_MS, ViewerApp::NAV_INITIAL_DELAY_MS);
}

// toggleControllerPanel() contract (covered by AC10 integration test in Todo 8):
// - ViewMode → MenuMode: sets menu_state_.controller_panel_open = true,
//   resets selected_panel_item = -1, auto-pauses if set_->isPlaying.
// - MenuMode → ViewMode: sets menu_state_.controller_panel_open = false,
//   does NOT auto-resume playback.
// ViewerApp requires an OpenGL context, so direct unit-testing of this private
// method is deferred to the integration test.

// --- Nav constants ---

TEST(NavConstants, StickNavThreshold_IsInValidNormalizedRange)
{
    EXPECT_GT(ViewerApp::NAV_STICK_THRESHOLD, 0.0f);
    EXPECT_LT(ViewerApp::NAV_STICK_THRESHOLD, 1.0f);
}

// --- D-pad navigation (Todo 7) ---

TEST(DpadNav, FirstDownPress_MovesFromMinusOneToZero)
{
    EXPECT_EQ(ViewerApp::applyNavMove(-1, 5, 1), 0);
}

TEST(DpadNav, AtLastItem_DownDoesNotWrap)
{
    EXPECT_EQ(ViewerApp::applyNavMove(4, 5, 1), 4);
}

TEST(DpadNav, AtFirstItem_UpDoesNotWrap)
{
    EXPECT_EQ(ViewerApp::applyNavMove(0, 5, -1), 0);
}

TEST(DpadNav, AConfirm_AtMinusOne_IsNoOp)
{
    // A button with selected=-1 triggers no navigation move
    EXPECT_EQ(ViewerApp::applyNavMove(-1, 5, 0), -1);
}

// --- PanelLayer types (add-panel-layer-types) ---

TEST(PanelLayer, TwoLayersAreDistinct)
{
    EXPECT_NE(PanelLayer::Main, PanelLayer::RenderMode);
}

TEST(MenuState, PanelLayer_DefaultsToMain)
{
    MenuState s;
    EXPECT_EQ(s.panel_layer, PanelLayer::Main);
}
