/*
 * ImGuiMCParamNavTests.cpp
 *
 * Unit tests for D-pad left/right MC parameter adjustment.
 * The adjustMcParam() function should change iso_value or influence_radius
 * based on which item is currently selected in the MC panel.
 *
 * Selected item indices when MC mode is active:
 *   6 = Iso value   (step: 0.01, range: [0.0, 2.0])
 *   7 = Radius       (step: 0.1,  range: [0.1, 10.0])
 */

#include <gtest/gtest.h>

#include "ui/imgui_menu.hpp"

// Helper: MC-mode panel state with a given selected item.
static MenuState mcState(int selected_item, float iso = 0.5f, float radius = 2.0f)
{
    MenuState s;
    s.panel_layer = PanelLayer::RenderMode;
    s.current_render_mode = kRenderModeMC;
    s.selected_panel_item = selected_item;
    s.iso_value = iso;
    s.influence_radius = radius;
    return s;
}

// ---------------------------------------------------------------------------
// Iso (item 6)
// ---------------------------------------------------------------------------

TEST(MCParamNav, IsoRight_IncrementsBy_0_01)
{
    MenuState s = mcState(6, /*iso=*/0.50f);
    bool changed = adjustMcParam(s, /*left=*/false);
    EXPECT_TRUE(changed);
    EXPECT_NEAR(s.iso_value, 0.51f, 1e-4f);
}

TEST(MCParamNav, IsoLeft_DecrementsByStep)
{
    MenuState s = mcState(6, /*iso=*/0.50f);
    bool changed = adjustMcParam(s, /*left=*/true);
    EXPECT_TRUE(changed);
    EXPECT_NEAR(s.iso_value, 0.49f, 1e-4f);
}

TEST(MCParamNav, Iso_DoesNotGoAboveMax)
{
    MenuState s = mcState(6, /*iso=*/2.0f);
    adjustMcParam(s, /*left=*/false);
    EXPECT_LE(s.iso_value, 2.0f);
}

TEST(MCParamNav, Iso_DoesNotGoBelowZero)
{
    MenuState s = mcState(6, /*iso=*/0.0f);
    adjustMcParam(s, /*left=*/true);
    EXPECT_GE(s.iso_value, 0.0f);
}

// ---------------------------------------------------------------------------
// Radius (item 7)
// ---------------------------------------------------------------------------

TEST(MCParamNav, RadiusRight_IncrementsByStep)
{
    MenuState s = mcState(7, 0.5f, /*radius=*/2.0f);
    bool changed = adjustMcParam(s, /*left=*/false);
    EXPECT_TRUE(changed);
    EXPECT_NEAR(s.influence_radius, 2.1f, 1e-3f);
}

TEST(MCParamNav, RadiusLeft_DecrementsByStep)
{
    MenuState s = mcState(7, 0.5f, /*radius=*/2.0f);
    bool changed = adjustMcParam(s, /*left=*/true);
    EXPECT_TRUE(changed);
    EXPECT_NEAR(s.influence_radius, 1.9f, 1e-3f);
}

TEST(MCParamNav, Radius_DoesNotGoAboveMax)
{
    MenuState s = mcState(7, 0.5f, /*radius=*/10.0f);
    adjustMcParam(s, /*left=*/false);
    EXPECT_LE(s.influence_radius, 10.0f);
}

TEST(MCParamNav, Radius_DoesNotGoBelowMin)
{
    MenuState s = mcState(7, 0.5f, /*radius=*/0.1f);
    adjustMcParam(s, /*left=*/true);
    EXPECT_GE(s.influence_radius, 0.1f);
}

// ---------------------------------------------------------------------------
// Non-adjustable items
// ---------------------------------------------------------------------------

TEST(MCParamNav, GridItem_ReturnsFalse)
{
    MenuState s = mcState(3); // Grid: 64 -- not adjustable by L/R
    bool changed = adjustMcParam(s, false);
    EXPECT_FALSE(changed);
}

TEST(MCParamNav, LiveItem_ReturnsFalse)
{
    MenuState s = mcState(8); // Sync/Freeze toggle -- not adjustable by D-pad L/R
    bool changed = adjustMcParam(s, false);
    EXPECT_FALSE(changed);
}

// ---------------------------------------------------------------------------
// Return value signals whether caller should mark mc_params_changed
// ---------------------------------------------------------------------------

TEST(MCParamNav, IsoAdjust_ReturnsTrue)
{
    MenuState s = mcState(6, 0.5f);
    EXPECT_TRUE(adjustMcParam(s, false));
}

TEST(MCParamNav, NonAdjustableItem_ReturnsFalse)
{
    MenuState s = mcState(3);
    EXPECT_FALSE(adjustMcParam(s, false));
}

// ---------------------------------------------------------------------------
// Default state
// ---------------------------------------------------------------------------

TEST(MCParamNav, DefaultIsoValue_IsGradual)
{
    MenuState s;
    EXPECT_NEAR(s.iso_value, 0.3f, 1e-4f);
}
