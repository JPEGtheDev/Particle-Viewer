// VramNotificationTests.cpp
//
// Tests for VRAM downgrade notification in the controller panel Main layer.
// When mc_vram_downgrade_notification is true, an additional dismissible item
// ("VRAM: 256^3 unavailable") appears at the end of the Main panel item list.
// Confirming it clears the flag (dismisses the notification).
//
// Spec: Todo 15 -- VRAM downgrade notification rendering.

#include <gtest/gtest.h>

#include "imgui.h"
#include "ui/imgui_menu.hpp"

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

class VramNotificationTest : public ::testing::Test
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

    // Returns a MenuState with the panel open and panel_layer == Main.
    static MenuState makeMainPanelState()
    {
        MenuState state;
        state.controller_panel_open = true;
        state.panel_layer = PanelLayer::Main;
        state.selected_panel_item = -1;
        return state;
    }
};

// ---------------------------------------------------------------------------
// Test 1 -- Notification flag true: item count increases by exactly 1
// ---------------------------------------------------------------------------
// Derives the base count dynamically to avoid hardcoding the Main panel size.
// Frame 1: render without notification -> capture base_count.
// Frame 2: render with notification -> assert base_count + 1.

TEST_F(VramNotificationTest, MainPanel_VramNotification_IncreasesItemCountByOne)
{
    // Frame 1: establish base count (notification flag off)
    MenuState state = makeMainPanelState();
    renderControllerPanel(state);
    const int base_count = state.panel_item_count;
    ASSERT_GT(base_count, 0);

    // Frame 2: notification flag on
    ImGui::EndFrame();
    ImGui::NewFrame();
    state = makeMainPanelState();
    state.mc_vram_downgrade_notification = true;
    renderControllerPanel(state);

    EXPECT_EQ(state.panel_item_count, base_count + 1);
}

// ---------------------------------------------------------------------------
// Test 2 -- Confirming the VRAM notification item clears the flag
// ---------------------------------------------------------------------------
// The notification item is the last item (index == panel_item_count - 1).

TEST_F(VramNotificationTest, MainPanel_VramNotification_ConfirmDismiss_ClearsFlag)
{
    MenuState state = makeMainPanelState();
    state.mc_vram_downgrade_notification = true;

    // First call to obtain item count (notification item index).
    renderControllerPanel(state);
    const int notification_index = state.panel_item_count - 1;

    // Second call: confirm the notification item.
    ImGui::EndFrame();
    ImGui::NewFrame();
    state.selected_panel_item = notification_index;
    state.confirm_panel_item = true;

    renderControllerPanel(state);

    EXPECT_FALSE(state.mc_vram_downgrade_notification);
}

// ---------------------------------------------------------------------------
// Test 3 -- Notification flag false: confirming Close (item 7) closes panel, not VRAM item
// ---------------------------------------------------------------------------
// Regression guard: with flag off the item layout must not shift.

TEST_F(VramNotificationTest, MainPanel_NoVramNotification_ConfirmClose_ClosesPanel)
{
    MenuState state = makeMainPanelState();
    state.mc_vram_downgrade_notification = false;
    state.selected_panel_item = 7; // index 7 == "Close"
    state.confirm_panel_item = true;

    MenuActions actions = renderControllerPanel(state);

    EXPECT_TRUE(actions.close_panel);
}

// ---------------------------------------------------------------------------
// Test 4 -- MenuState::mc_256_available defaults to true
// ---------------------------------------------------------------------------
// The field must default to true so that 256^3 is available until proven otherwise.

TEST(MenuStateTest, MenuState_Mc256Available_DefaultsToTrue)
{
    MenuState state;
    EXPECT_TRUE(state.mc_256_available);
}

// ---------------------------------------------------------------------------
// Test 5 -- 256^3 radio button is non-interactive when mc_256_available is false
// ---------------------------------------------------------------------------
// When mc_256_available = false, confirming the Grid: 256 item (index 5 in MC mode)
// must NOT set mc_params_changed (the item is disabled).

class VramMc256AvailableTest : public VramNotificationTest
{
  protected:
    static MenuState makeMCSubPanelState()
    {
        MenuState state;
        state.controller_panel_open = true;
        state.panel_layer = PanelLayer::RenderMode;
        state.current_render_mode = kRenderModeMC;
        state.compute_shaders_available = true;
        state.selected_panel_item = -1;
        return state;
    }
};

TEST_F(VramMc256AvailableTest, MCSubPanel_256ResolutionGreyedOut_When256Unavailable)
{
    constexpr int kMCGrid256ItemIndex = 5; // "Grid: 256" is at index 5 in the MC sub-panel layout
    MenuState state = makeMCSubPanelState();
    state.mc_256_available = false;
    state.selected_panel_item = kMCGrid256ItemIndex;
    state.confirm_panel_item = true;

    MenuActions actions = renderControllerPanel(state);

    // When 256^3 is unavailable (disabled), confirming it must be a no-op.
    EXPECT_FALSE(actions.mc_params_changed);
    EXPECT_EQ(state.grid_resolution, GridResolution::Grid128);
}
