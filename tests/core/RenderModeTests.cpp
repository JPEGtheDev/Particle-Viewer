// tests/core/RenderModeTests.cpp
// Tests for RenderMode enum and cycleRenderMode helper.

#include <gtest/gtest.h>

#include "viewer_app.hpp"

TEST(RenderModeTest, Spheres_WhenEnumDefined_IsAccessible)
{
    [[maybe_unused]] RenderMode mode = RenderMode::Spheres;
    SUCCEED();
}

TEST(RenderModeTest, ScreenSpaceMetaballs_WhenEnumDefined_IsAccessible)
{
    [[maybe_unused]] RenderMode mode = RenderMode::ScreenSpaceMetaballs;
    SUCCEED();
}

TEST(RenderModeTest, MarchingCubes_WhenEnumDefined_IsAccessible)
{
    [[maybe_unused]] RenderMode mode = RenderMode::MarchingCubes;
    SUCCEED();
}

TEST(RenderModeTest, AllEnumerators_WhenCompared_AreDistinct)
{
    EXPECT_NE(RenderMode::Spheres, RenderMode::ScreenSpaceMetaballs);
    EXPECT_NE(RenderMode::ScreenSpaceMetaballs, RenderMode::MarchingCubes);
    EXPECT_NE(RenderMode::Spheres, RenderMode::MarchingCubes);
}

// cycleRenderMode always returns Spheres; SSM and MarchingCubes are not yet implemented.

TEST(RenderModeTest, CycleRenderMode_FromSpheres_ReturnsSpheres)
{
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::Spheres);
    EXPECT_EQ(next, RenderMode::Spheres);
}

TEST(RenderModeTest, CycleRenderMode_FromSSM_ReturnsSpheres)
{
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::ScreenSpaceMetaballs);
    EXPECT_EQ(next, RenderMode::Spheres);
}

TEST(RenderModeTest, CycleRenderMode_FromMarchingCubes_ReturnsSpheres)
{
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::MarchingCubes);
    EXPECT_EQ(next, RenderMode::Spheres);
}
