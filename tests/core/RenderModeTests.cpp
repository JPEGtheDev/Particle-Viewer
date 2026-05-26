// tests/core/RenderModeTests.cpp
// Tests for RenderMode enum (Issue #123, Todo: add-render-mode-enum).
// This file grows across future todos to cover M-key cycling and recording guards.

#include <gtest/gtest.h>

#include "viewer_app.hpp"

// These tests fail to compile until RenderMode enum is defined in viewer_app.hpp.
// That is the RED gate.

TEST(RenderMode, SpheresEnumeratorExists)
{
    [[maybe_unused]] RenderMode mode = RenderMode::Spheres;
    SUCCEED();
}

TEST(RenderMode, ScreenSpaceMetaballsEnumeratorExists)
{
    [[maybe_unused]] RenderMode mode = RenderMode::ScreenSpaceMetaballs;
    SUCCEED();
}

TEST(RenderMode, MarchingCubesEnumeratorExists)
{
    [[maybe_unused]] RenderMode mode = RenderMode::MarchingCubes;
    SUCCEED();
}

TEST(RenderMode, AllValuesAreDistinct)
{
    EXPECT_NE(RenderMode::Spheres, RenderMode::ScreenSpaceMetaballs);
    EXPECT_NE(RenderMode::ScreenSpaceMetaballs, RenderMode::MarchingCubes);
    EXPECT_NE(RenderMode::Spheres, RenderMode::MarchingCubes);
}
