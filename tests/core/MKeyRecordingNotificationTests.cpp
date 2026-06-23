// MKeyRecordingNotificationTests.cpp
//
// Tests for M-key-during-recording notification in the controller panel Main layer.
// When m_key_recording_notification is true, an additional dismissible item
// ("M: mode locked (recording)") appears at the end of the Main panel item list.
// Confirming it clears the flag (dismisses the notification).
//
// Spec: Todo 16 -- M-key-during-recording notification rendering.

#include <gtest/gtest.h>

#include "imgui.h"
#include "ui/imgui_menu.hpp"

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

class MKeyRecordingNotificationTest : public ::testing::Test
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
// Test 1 -- Default value: m_key_recording_notification is false in MenuState
// ---------------------------------------------------------------------------

TEST_F(MKeyRecordingNotificationTest, MenuState_DefaultsToFalse)
{
    EXPECT_FALSE(MenuState{}.m_key_recording_notification);
}

// ---------------------------------------------------------------------------
// Test 2 -- Notification flag true: item count increases by exactly 1
// ---------------------------------------------------------------------------
// Derives the base count dynamically to avoid hardcoding the Main panel size.
// Frame 1: render without notification -> capture base_count.
// Frame 2: render with notification -> assert base_count + 1.

TEST_F(MKeyRecordingNotificationTest, MainPanel_MKeyNotification_IncreasesItemCountByOne)
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
    state.m_key_recording_notification = true;
    renderControllerPanel(state);

    EXPECT_EQ(state.panel_item_count, base_count + 1);
}

// ---------------------------------------------------------------------------
// Test 3 -- Confirming the M-key notification item clears the flag
// ---------------------------------------------------------------------------
// The notification item is the last item (index == panel_item_count - 1).

TEST_F(MKeyRecordingNotificationTest, MainPanel_MKeyNotification_ConfirmDismiss_ClearsFlag)
{
    MenuState state = makeMainPanelState();
    state.m_key_recording_notification = true;

    // First call to obtain item count (notification item index).
    renderControllerPanel(state);
    const int notification_index = state.panel_item_count - 1;

    // Second call: confirm the notification item.
    ImGui::EndFrame();
    ImGui::NewFrame();
    state.selected_panel_item = notification_index;
    state.confirm_panel_item = true;

    renderControllerPanel(state);

    EXPECT_FALSE(state.m_key_recording_notification);
}

// ---------------------------------------------------------------------------
// Test 4 -- Both notifications active: item count increases by 2
// ---------------------------------------------------------------------------
// Frame 1: base count (no notifications).
// Frame 2: both flags active -> assert base_count + 2.

TEST_F(MKeyRecordingNotificationTest, MainPanel_BothNotificationsActive_IncreasesItemCountByTwo)
{
    // Frame 1: base count (no notifications)
    MenuState state = makeMainPanelState();
    renderControllerPanel(state);
    const int base_count = state.panel_item_count;

    // Frame 2: both flags active
    ImGui::EndFrame();
    ImGui::NewFrame();
    state = makeMainPanelState();
    state.mc_vram_downgrade_notification = true;
    state.m_key_recording_notification = true;
    renderControllerPanel(state);

    EXPECT_EQ(state.panel_item_count, base_count + 2);
}

// ---------------------------------------------------------------------------
// Test 5 -- Both active: confirming VRAM slot clears VRAM flag only
// ---------------------------------------------------------------------------
// When both notifications are active, VRAM is second-to-last (panel_item_count - 2).
// Confirming it must clear mc_vram_downgrade_notification and leave m_key_recording_notification.

TEST_F(MKeyRecordingNotificationTest, MainPanel_BothNotificationsActive_ConfirmVramSlot_ClearsVramFlagOnly)
{
    MenuState state = makeMainPanelState();
    state.mc_vram_downgrade_notification = true;
    state.m_key_recording_notification = true;

    // Render to establish item count
    renderControllerPanel(state);
    const int vram_index = state.panel_item_count - 2; // VRAM is second-to-last

    ImGui::EndFrame();
    ImGui::NewFrame();
    state.selected_panel_item = vram_index;
    state.confirm_panel_item = true;
    renderControllerPanel(state);

    EXPECT_FALSE(state.mc_vram_downgrade_notification);
    EXPECT_TRUE(state.m_key_recording_notification); // M_KEY flag must remain
}

// ---------------------------------------------------------------------------
// Test 6 -- Both active: confirming M_KEY slot clears M_KEY flag only
// ---------------------------------------------------------------------------
// When both notifications are active, M_KEY is last (panel_item_count - 1).
// Confirming it must clear m_key_recording_notification and leave mc_vram_downgrade_notification.

TEST_F(MKeyRecordingNotificationTest, MainPanel_BothNotificationsActive_ConfirmMKeySlot_ClearsMKeyFlagOnly)
{
    MenuState state = makeMainPanelState();
    state.mc_vram_downgrade_notification = true;
    state.m_key_recording_notification = true;

    // Render to establish item count
    renderControllerPanel(state);
    const int mkey_index = state.panel_item_count - 1; // M_KEY is last

    ImGui::EndFrame();
    ImGui::NewFrame();
    state.selected_panel_item = mkey_index;
    state.confirm_panel_item = true;
    renderControllerPanel(state);

    EXPECT_FALSE(state.m_key_recording_notification);
    EXPECT_TRUE(state.mc_vram_downgrade_notification); // VRAM flag must remain
}
