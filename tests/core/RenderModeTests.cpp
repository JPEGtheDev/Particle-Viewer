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

// SSMResources struct tests (add-ssm-resources-and-paths)

TEST(SSMResourcesTest, DefaultConstruct_FloatFboSupported_IsFalse)
{
    // Assert
    RenderResources r;
    EXPECT_FALSE(r.ssm.float_fbo_supported);
}

TEST(SSMResourcesTest, DefaultConstruct_DensityFbo_IsZero)
{
    // Assert
    RenderResources r;
    EXPECT_EQ(r.ssm.density_fbo, 0u);
}

TEST(SSMResourcesTest, DefaultConstruct_BlurredFbo_IsZero)
{
    // Assert
    RenderResources r;
    EXPECT_EQ(r.ssm.blurred_fbo, 0u);
}

// ShaderPaths SSM path defaults

TEST(ShaderPathsTest, SsmSplatVertex_DefaultPath_ContainsMetaballSplatVert)
{
    // Assert
    ShaderPaths paths;
    EXPECT_NE(paths.ssm_splat_vertex.find("metaball_splat.vert"), std::string::npos);
}

TEST(ShaderPathsTest, SsmSplatFragment_DefaultPath_ContainsMetaballSplatFrag)
{
    // Assert
    ShaderPaths paths;
    EXPECT_NE(paths.ssm_splat_fragment.find("metaball_splat.frag"), std::string::npos);
}

TEST(ShaderPathsTest, SsmBlurVertex_DefaultPath_ContainsMetaballBlurVert)
{
    // Assert
    ShaderPaths paths;
    EXPECT_NE(paths.ssm_blur_vertex.find("metaball_blur.vert"), std::string::npos);
}

TEST(ShaderPathsTest, SsmBlurFragment_DefaultPath_ContainsMetaballBlurFrag)
{
    // Assert
    ShaderPaths paths;
    EXPECT_NE(paths.ssm_blur_fragment.find("metaball_blur.frag"), std::string::npos);
}

TEST(ShaderPathsTest, SsmCompositeFragment_DefaultPath_ContainsMetaballCompositeFrag)
{
    // Assert
    ShaderPaths paths;
    EXPECT_NE(paths.ssm_composite_fragment.find("metaball_composite.frag"), std::string::npos);
}
