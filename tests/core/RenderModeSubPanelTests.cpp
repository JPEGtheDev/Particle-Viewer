// RenderModeSubPanelTests.cpp
//
// Tests for Render Mode sub-panel navigation.
// The sub-panel renders 3 items: Spheres / Marching Cubes / Back.
// SSM was removed; Marching Cubes remains as a greyed placeholder.
//
//   - When panel_layer == RenderMode, renderControllerPanel() renders 3 items.
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
        // Provide a minimal font atlas (1×1 white pixel) so NewFrame doesn't assert.
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
// Test 1 — item count in sub-panel is 3 (Spheres / Marching Cubes / Back)
// ---------------------------------------------------------------------------

TEST_F(RenderModeSubPanelTest, SubPanel_ItemCount_IsThree)
{
    MenuState state = makeSubPanelState();

    renderControllerPanel(state);

    EXPECT_EQ(state.panel_item_count, 3);
}

// ---------------------------------------------------------------------------
// Test 2 — confirming "Back" (item 2) transitions to PanelLayer::Main
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
// Test 3 — confirming "Spheres" (item 0) transitions to PanelLayer::Main
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
// Test 4 — B-button / back navigation does NOT close the panel
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
