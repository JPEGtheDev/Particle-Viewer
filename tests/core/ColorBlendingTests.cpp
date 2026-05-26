// Unit tests for SSM color blending math (Issue #123, AC8).
// These functions are extracted from shader logic so they can be tested
// without a GL context.

#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "ssm_color_math.hpp"

// ============================================================
// category500Color tests
// ============================================================

TEST(Category500Color, ParticleId0_ReturnsBlack)
{
    glm::vec3 c = category500Color(0);
    EXPECT_FLOAT_EQ(c.r, 0.0f);
    EXPECT_FLOAT_EQ(c.g, 0.0f);
    EXPECT_FLOAT_EQ(c.b, 0.0f);
}

// id=1: R=(1%40)/40.0=0.025, G=(1%1600)/1600.0=0.000625, B=(1%64000)/64000.0=0.000015625
TEST(Category500Color, ParticleId1_MatchesGLSLFormula)
{
    glm::vec3 c = category500Color(1);
    EXPECT_FLOAT_EQ(c.r, 1.0f / 40.0f);
    EXPECT_FLOAT_EQ(c.g, 1.0f / 1600.0f);
    EXPECT_FLOAT_EQ(c.b, 1.0f / 64000.0f);
}

// id=40: R=(40%40)/40.0=0/40=0, G=(40%1600)/1600.0=40/1600=0.025, B=(40%64000)/64000=0.000625
// Verifies that integer modulo wraps correctly (40%40=0, NOT 1.0)
TEST(Category500Color, ParticleId40_RedWrapsToZero)
{
    glm::vec3 c = category500Color(40);
    EXPECT_FLOAT_EQ(c.r, 0.0f);
    EXPECT_FLOAT_EQ(c.g, 40.0f / 1600.0f);
    EXPECT_FLOAT_EQ(c.b, 40.0f / 64000.0f);
}

// id=1600: R=(1600%40)/40.0=0/40=0, G=(1600%1600)/1600.0=0/1600=0, B=(1600%64000)/64000=0.025
TEST(Category500Color, ParticleId1600_BlueChannelCycles)
{
    glm::vec3 c = category500Color(1600);
    EXPECT_FLOAT_EQ(c.r, 0.0f);
    EXPECT_FLOAT_EQ(c.g, 0.0f);
    EXPECT_FLOAT_EQ(c.b, 1600.0f / 64000.0f);
}

// ============================================================
// blendColors tests
// ============================================================

TEST(BlendColors, EmptyInput_ReturnsBlack)
{
    std::vector<std::pair<glm::vec3, float>> colors;
    glm::vec3 result = blendColors(colors);
    EXPECT_FLOAT_EQ(result.r, 0.0f);
    EXPECT_FLOAT_EQ(result.g, 0.0f);
    EXPECT_FLOAT_EQ(result.b, 0.0f);
}

TEST(BlendColors, AllZeroWeights_ReturnsBlack)
{
    std::vector<std::pair<glm::vec3, float>> colors = {
        {glm::vec3(1.0f, 0.0f, 0.0f), 0.0f},
        {glm::vec3(0.0f, 1.0f, 0.0f), 0.0f},
    };
    glm::vec3 result = blendColors(colors);
    EXPECT_FLOAT_EQ(result.r, 0.0f);
    EXPECT_FLOAT_EQ(result.g, 0.0f);
    EXPECT_FLOAT_EQ(result.b, 0.0f);
}

TEST(BlendColors, SingleParticle_ReturnsItsColor)
{
    std::vector<std::pair<glm::vec3, float>> colors = {
        {glm::vec3(0.5f, 0.3f, 0.7f), 1.0f},
    };
    glm::vec3 result = blendColors(colors);
    EXPECT_FLOAT_EQ(result.r, 0.5f);
    EXPECT_FLOAT_EQ(result.g, 0.3f);
    EXPECT_FLOAT_EQ(result.b, 0.7f);
}

TEST(BlendColors, TwoEqualWeights_AveragesColors)
{
    std::vector<std::pair<glm::vec3, float>> colors = {
        {glm::vec3(1.0f, 0.0f, 0.0f), 0.5f},
        {glm::vec3(0.0f, 1.0f, 0.0f), 0.5f},
    };
    glm::vec3 result = blendColors(colors);
    EXPECT_FLOAT_EQ(result.r, 0.5f);
    EXPECT_FLOAT_EQ(result.g, 0.5f);
    EXPECT_FLOAT_EQ(result.b, 0.0f);
}

TEST(BlendColors, DominantWeight_ApproximatesItsColor)
{
    // Weight 1.0 vs 0.001 — dominant particle overwhelms the blend
    std::vector<std::pair<glm::vec3, float>> colors = {
        {glm::vec3(1.0f, 0.0f, 0.0f), 1.0f},
        {glm::vec3(0.0f, 1.0f, 0.0f), 0.001f},
    };
    glm::vec3 result = blendColors(colors);
    // Red should dominate heavily
    EXPECT_GT(result.r, 0.99f);
    EXPECT_LT(result.g, 0.01f);
}
