// tests/core/RenderModeTests.cpp
// Tests for RenderMode enum (Issue #123, Todo: add-render-mode-enum).
// This file grows across future todos to cover M-key cycling and recording guards.

#include <gtest/gtest.h>

#include "viewer_app.hpp"

// These tests fail to compile until RenderMode enum is defined in viewer_app.hpp.
// That is the RED gate.

TEST(RenderModeTest, Spheres_WhenEnumDefined_IsAccessible)
{
    // Act
    [[maybe_unused]] RenderMode mode = RenderMode::Spheres;
    // Assert
    SUCCEED();
}

TEST(RenderModeTest, ScreenSpaceMetaballs_WhenEnumDefined_IsAccessible)
{
    // Act
    [[maybe_unused]] RenderMode mode = RenderMode::ScreenSpaceMetaballs;
    // Assert
    SUCCEED();
}

TEST(RenderModeTest, MarchingCubes_WhenEnumDefined_IsAccessible)
{
    // Act
    [[maybe_unused]] RenderMode mode = RenderMode::MarchingCubes;
    // Assert
    SUCCEED();
}

TEST(RenderModeTest, AllEnumerators_WhenCompared_AreDistinct)
{
    // Assert
    EXPECT_NE(RenderMode::Spheres, RenderMode::ScreenSpaceMetaballs);
    EXPECT_NE(RenderMode::ScreenSpaceMetaballs, RenderMode::MarchingCubes);
    EXPECT_NE(RenderMode::Spheres, RenderMode::MarchingCubes);
}

// Cycling tests for cycleRenderMode static helper (add-m-key-handler)

TEST(RenderModeTest, CycleRenderMode_FromSpheres_ReturnsSSM)
{
    // Act
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::Spheres);
    // Assert
    EXPECT_EQ(next, RenderMode::ScreenSpaceMetaballs);
}

TEST(RenderModeTest, CycleRenderMode_FromSSM_ReturnsSpheres)
{
    // Act
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::ScreenSpaceMetaballs);
    // Assert
    EXPECT_EQ(next, RenderMode::Spheres);
}

TEST(RenderModeTest, CycleRenderMode_FromMarchingCubes_ReturnsSpheres)
{
    // MarchingCubes is always greyed; cycling from it still wraps to Spheres
    // Act
    RenderMode next = ViewerApp::cycleRenderMode(RenderMode::MarchingCubes);
    // Assert
    EXPECT_EQ(next, RenderMode::Spheres);
}
