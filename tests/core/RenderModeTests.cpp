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

TEST(RenderModeTest, CycleRenderMode_FromSpheres_ComputeAvailable_ReturnsMarchingCubes)
{
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::Spheres, true);
    EXPECT_EQ(next, RenderMode::MarchingCubes);
}

TEST(RenderModeTest, CycleRenderMode_FromMC_ComputeAvailable_ReturnsSpheres)
{
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::MarchingCubes, true);
    EXPECT_EQ(next, RenderMode::Spheres);
}

TEST(RenderModeTest, CycleRenderMode_FromSpheres_ComputeUnavailable_ReturnsSpheres)
{
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::Spheres, false);
    EXPECT_EQ(next, RenderMode::Spheres);
}

TEST(RenderModeTest, CycleRenderMode_FromScreenSpaceMetaballs_ComputeAvailable_ReturnsSpheres)
{
    EXPECT_EQ(ViewerApp::cycleRenderMode(RenderMode::ScreenSpaceMetaballs, true), RenderMode::Spheres);
}
