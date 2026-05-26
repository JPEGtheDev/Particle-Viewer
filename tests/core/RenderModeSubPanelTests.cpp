// RenderModeSubPanelTests.cpp
//
// RED gate tests for Render Mode sub-panel navigation (Wave 3 of Issue #113).
// These tests MUST FAIL until Wave 4 adds sub-panel branching to
// renderControllerPanel(). They describe the contracted behaviour of the
// sub-panel that Wave 4 will implement:
//
//   - When panel_layer == RenderMode, renderControllerPanel() renders 4 items
//     (Spheres / Screen-Space Metaballs / Marching Cubes / Back), not the
//     8-item main list.
//   - Confirming "Back" (item 3) returns panel_layer to PanelLayer::Main.
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
    MenuState makeSubPanelState() const
    {
        MenuState state;
        state.controller_panel_open = true;
        state.panel_layer = PanelLayer::RenderMode;
        state.selected_panel_item = -1;
        return state;
    }
};

// ---------------------------------------------------------------------------
// Test 1 — item count in sub-panel is 4, not 8
// ---------------------------------------------------------------------------
// RED: renderControllerPanel() currently renders the 8-item main list
// regardless of panel_layer.  Once Wave 4 adds branching, this will pass.

TEST_F(RenderModeSubPanelTest, SubPanel_ItemCount_IsFour)
{
    MenuState state = makeSubPanelState();

    renderControllerPanel(state);

    EXPECT_EQ(state.panel_item_count, 4);
}

// ---------------------------------------------------------------------------
// Test 2 — confirming "Back" (item 3) transitions to PanelLayer::Main
// ---------------------------------------------------------------------------
// RED: no sub-panel branching exists yet; item 3 is PanelItem::QUIT in the
// current 8-item main list, so panel_layer stays RenderMode.

TEST_F(RenderModeSubPanelTest, SubPanel_ConfirmBack_ReturnsToPanelLayerMain)
{
    MenuState state = makeSubPanelState();
    state.selected_panel_item = 3; // index 3 == "Back" in the sub-panel
    state.confirm_panel_item = true;

    renderControllerPanel(state);

    EXPECT_EQ(state.panel_layer, PanelLayer::Main);
}

// ---------------------------------------------------------------------------
// Test 3 — confirming "Spheres" (item 0) transitions to PanelLayer::Main
// ---------------------------------------------------------------------------
// RED: no sub-panel branching exists yet; item 0 is PanelItem::FULLSCREEN in
// the current main list, so panel_layer stays RenderMode.

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
//
// RED: currently panel_layer stays RenderMode because there is no sub-panel
// branching, so the panel_layer assertion already fails.

TEST_F(RenderModeSubPanelTest, SubPanel_BButton_ReturnsToPanelLayerMain_WithoutClosingPanel)
{
    MenuState state = makeSubPanelState();
    state.selected_panel_item = 3; // "Back" item — same path as B-button back
    state.confirm_panel_item = true;

    MenuActions actions = renderControllerPanel(state);

    EXPECT_EQ(state.panel_layer, PanelLayer::Main);
    EXPECT_FALSE(actions.close_panel);
}
