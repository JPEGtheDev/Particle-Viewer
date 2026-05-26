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
