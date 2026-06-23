// RenderModeSubPanelTests.cpp
//
// Tests for Render Mode sub-panel navigation.
// The sub-panel renders kBaseRenderMode items in base mode (Spheres + MC + Back) and
// kMarchingCubesMode items when Marching Cubes is the active render mode.
// SSM was removed; Marching Cubes remains as a greyed placeholder.
//
//   - When panel_layer == RenderMode (non-MC mode), renders kBaseRenderMode items.
//   - When panel_layer == RenderMode and MC is active, renders kMarchingCubesMode items.
//   - Confirming "Back" (item 2) returns panel_layer to PanelLayer::Main.
//   - Confirming a mode selector (item 0, "Spheres") also returns
//     panel_layer to PanelLayer::Main.
//   - The B-button / back action returns panel_layer to PanelLayer::Main
//     WITHOUT setting actions.close_panel = true.

#include <gtest/gtest.h>

#include "imgui.h"
#include "ui/imgui_menu.hpp"

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

class RenderModeSubPanelTest : public ::testing::Test
{
  protected:
    ImGuiContext* ctx_ = nullptr;

    void SetUp() override
    {
        ctx_ = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(1280, 720);
        // Provide a minimal font atlas (1x1 white pixel) so NewFrame doesn't assert.
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        io.Fonts->SetTexID(static_cast<ImTextureID>(1));
        ImGui::NewFrame();
    }

    void TearDown() override
    {
        ImGui::EndFrame();
        ImGui::DestroyContext(ctx_);
    }

    // Returns a MenuState with the panel open and panel_layer == RenderMode.
    static MenuState makeSubPanelState()
    {
        MenuState state;
        state.controller_panel_open = true;
        state.panel_layer = PanelLayer::RenderMode;
        state.selected_panel_item = -1;
        return state;
    }
};

// ---------------------------------------------------------------------------
// Test 1 --item count in sub-panel is 3 (Spheres / Marching Cubes / Back)
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_ItemCount_IsBaseRenderMode)
{
    MenuState state = makeSubPanelState();

    renderControllerPanel(state);

    EXPECT_EQ(state.panel_item_count, static_cast<int>(SubPanelItemCount::kBaseRenderMode));
}

// ---------------------------------------------------------------------------
// Test 2 -- confirming "Back" (item 2) transitions to PanelLayer::Main
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_ConfirmBack_ReturnsToPanelLayerMain)
{
    MenuState state = makeSubPanelState();
    state.selected_panel_item = 2; // index 2 == "Back" in the sub-panel
    state.confirm_panel_item = true;

    renderControllerPanel(state);

    EXPECT_EQ(state.panel_layer, PanelLayer::Main);
}

// ---------------------------------------------------------------------------
// Test 3 -- confirming "Spheres" (item 0) transitions to PanelLayer::Main
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_ConfirmSpheres_ReturnsToPanelLayerMain)
{
    MenuState state = makeSubPanelState();
    state.selected_panel_item = 0; // index 0 == "Spheres" in the sub-panel
    state.confirm_panel_item = true;

    renderControllerPanel(state);

    EXPECT_EQ(state.panel_layer, PanelLayer::Main);
}

// ---------------------------------------------------------------------------
// Test 4 -- B-button / back navigation does NOT close the panel
// ---------------------------------------------------------------------------
// When the user presses B while in the sub-panel, the expected behaviour is:
//   - panel_layer resets to PanelLayer::Main  (go back, not close)
//   - actions.close_panel remains false       (panel stays open)

TEST_F(RenderModeSubPanelTest, SubPanel_BButton_ReturnsToPanelLayerMain_WithoutClosingPanel)
{
    MenuState state = makeSubPanelState();
    state.panel_back_pressed = true;

    MenuActions actions = renderControllerPanel(state);

    EXPECT_EQ(state.panel_layer, PanelLayer::Main);
    EXPECT_FALSE(actions.close_panel);
}

// ---------------------------------------------------------------------------
// Test 5 -- MC item disabled when compute_shaders_available is false
// ---------------------------------------------------------------------------
// When compute_shaders_available is false, confirming the MC item (index 1)
// must be a no-op: render_mode_changed stays false and panel_layer stays RenderMode.

TEST_F(RenderModeSubPanelTest, SubPanel_MarchingCubes_DisabledWhenComputeShadersUnavailable)
{
    MenuState state = makeSubPanelState();
    state.compute_shaders_available = false; // explicit: default is false, stated for symmetry with Test 6
    state.selected_panel_item = 1;           // index 1 == "Marching Cubes"
    state.confirm_panel_item = true;

    MenuActions actions = renderControllerPanel(state);

    EXPECT_FALSE(actions.render_mode_changed);
    EXPECT_EQ(state.panel_layer, PanelLayer::RenderMode);
}

// ---------------------------------------------------------------------------
// Test 6 -- MC item enabled and selectable when compute_shaders_available is true
// ---------------------------------------------------------------------------
// When compute_shaders_available is true, confirming the MC item (index 1)
// must set render_mode_changed = true, new_render_mode = 2, and return to Main.

TEST_F(RenderModeSubPanelTest, SubPanel_MarchingCubes_EnabledWhenComputeShadersAvailable)
{
    MenuState state = makeSubPanelState();
    state.compute_shaders_available = true;
    state.selected_panel_item = 1; // index 1 == "Marching Cubes"
    state.confirm_panel_item = true;

    MenuActions actions = renderControllerPanel(state);

    EXPECT_TRUE(actions.render_mode_changed);
    EXPECT_EQ(actions.new_render_mode, 2); // 2 == MarchingCubes; see MenuActions::new_render_mode
    EXPECT_EQ(state.panel_layer, PanelLayer::Main);
}

// ---------------------------------------------------------------------------
// Test 7 -- Live/Freeze toggle: confirming item 8 in MC mode toggles Live->Freeze
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_LiveFreezeToggle_TogglesFromLiveToFreeze)
{
    MenuState state = makeSubPanelState();
    state.current_render_mode = kRenderModeMC;
    state.live_freeze = LiveFreezeMode::Live;
    state.selected_panel_item = 8;
    state.confirm_panel_item = true;

    MenuActions actions = renderControllerPanel(state);

    EXPECT_EQ(state.live_freeze, LiveFreezeMode::Freeze);
    EXPECT_TRUE(actions.mc_params_changed);
}

// ---------------------------------------------------------------------------
// Test 8 -- Live/Freeze toggle: confirming item 8 in Freeze mode toggles back to Live
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_LiveFreezeToggle_TogglesFromFreezeToLive)
{
    MenuState state = makeSubPanelState();
    state.current_render_mode = kRenderModeMC;
    state.live_freeze = LiveFreezeMode::Freeze;
    state.selected_panel_item = 8;
    state.confirm_panel_item = true;

    MenuActions actions = renderControllerPanel(state);

    EXPECT_EQ(state.live_freeze, LiveFreezeMode::Live);
    EXPECT_TRUE(actions.mc_params_changed);
}

// ---------------------------------------------------------------------------
// Test 9 -- item count in MC Live mode = kMarchingCubesMode (9)
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_ItemCount_IsMarchingCubesMode_LiveMode)
{
    MenuState state = makeSubPanelState();
    state.current_render_mode = kRenderModeMC;
    state.live_freeze = LiveFreezeMode::Live;

    renderControllerPanel(state);

    EXPECT_EQ(state.panel_item_count, static_cast<int>(SubPanelItemCount::kMarchingCubesMode));
}

// ---------------------------------------------------------------------------
// Test 10 -- item count in MC Freeze mode = kMarchingCubesMode + 1 (10)
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_ItemCount_IsMarchingCubesModeWithRefresh_FreezeMode)
{
    MenuState state = makeSubPanelState();
    state.current_render_mode = kRenderModeMC;
    state.live_freeze = LiveFreezeMode::Freeze;

    renderControllerPanel(state);

    EXPECT_EQ(state.panel_item_count, static_cast<int>(SubPanelItemCount::kMarchingCubesMode) + 1);
}

// ---------------------------------------------------------------------------
// Test 11 -- Refresh Mesh button sets mc_refresh_requested when in Freeze mode
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_RefreshMeshButton_SetsMcRefreshRequested)
{
    MenuState state = makeSubPanelState();
    state.current_render_mode = kRenderModeMC;
    state.live_freeze = LiveFreezeMode::Freeze;
    state.selected_panel_item = 9; // index 9 = Refresh Mesh (only in Freeze mode)
    state.confirm_panel_item = true;

    renderControllerPanel(state);

    EXPECT_TRUE(state.mc_refresh_requested);
}

// ---------------------------------------------------------------------------
// Test 12 -- getMarchingCubesLabel returns standard label when compute available
// ---------------------------------------------------------------------------

TEST(MarchingCubesLabelTest, GetMarchingCubesLabel_ComputeAvailable_ReturnsStandardLabel)
{
    EXPECT_STREQ(getMarchingCubesLabel(true), "Marching Cubes");
}

// ---------------------------------------------------------------------------
// Test 13 -- getMarchingCubesLabel returns unavailable label when compute absent
// ---------------------------------------------------------------------------

TEST(MarchingCubesLabelTest, GetMarchingCubesLabel_ComputeUnavailable_ShowsUnavailable)
{
    EXPECT_STREQ(getMarchingCubesLabel(false), "Marching Cubes (unavailable)");
}
