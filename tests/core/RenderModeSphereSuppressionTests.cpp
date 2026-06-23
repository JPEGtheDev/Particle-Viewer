// tests/core/RenderModeSphereSuppressionTests.cpp
//
// Tests for shouldRenderSpheres() helper.
// Verifies that sphere rendering is suppressed in non-Spheres render modes.
//
// Spec: Three Amigos Signoff Fix 1 -- suppress sphere rendering in MC mode.

#include <gtest/gtest.h>

#include "viewer_app.hpp"

TEST(RenderModeSphereSuppressionTest, ShouldRenderSpheres_Spheres_ReturnsTrue)
{
    EXPECT_TRUE(ViewerApp::shouldRenderSpheres(RenderMode::Spheres));
}

TEST(RenderModeSphereSuppressionTest, ShouldRenderSpheres_MarchingCubes_ReturnsFalse)
{
    EXPECT_FALSE(ViewerApp::shouldRenderSpheres(RenderMode::MarchingCubes));
}

TEST(RenderModeSphereSuppressionTest, ShouldRenderSpheres_ScreenSpaceMetaballs_ReturnsFalse)
{
    EXPECT_FALSE(ViewerApp::shouldRenderSpheres(RenderMode::ScreenSpaceMetaballs));
}
