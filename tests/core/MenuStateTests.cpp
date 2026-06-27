// tests/core/MenuStateTests.cpp
// Tests for MenuState default values for marching cubes parameters.

#include <gtest/gtest.h>

#include "ui/imgui_menu.hpp"

TEST(MenuStateTest, DefaultConstructed_GridResolutionIsGrid128)
{
    MenuState state;
    EXPECT_EQ(state.grid_resolution, GridResolution::Grid128);
}

TEST(MenuStateTest, DefaultConstructed_IsoValueIs0Point3)
{
    MenuState state;
    EXPECT_FLOAT_EQ(state.iso_value, 0.3f);
}

TEST(MenuStateTest, DefaultConstructed_InfluenceRadiusIs2Point0)
{
    MenuState state;
    EXPECT_FLOAT_EQ(state.influence_radius, 2.0f);
}

TEST(MenuStateTest, DefaultConstructed_LiveFreezeIsLive)
{
    MenuState state;
    EXPECT_EQ(state.live_freeze, LiveFreezeMode::Live);
}

TEST(MenuStateTest, DefaultConstructed_McRefreshRequestedIsFalse)
{
    MenuState state;
    EXPECT_FALSE(state.mc_refresh_requested);
}

TEST(MenuStateTest, DefaultConstructed_McVramDowngradeNotificationIsFalse)
{
    MenuState state;
    EXPECT_FALSE(state.mc_vram_downgrade_notification);
}
