#include <vector>

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "marching_cubes_color.hpp"

TEST(MCColorBlendingTest, TwoEquidistantParticles_DifferentColors_ReturnsAverage)
{
    // Both particles at same distance (0.5f) from voxel_center, influence_radius=2.0f
    // Category 0 -> red (1,0,0), category 1 -> blue (0.2,0.6,1)
    // Same distance => same gaussian weight => result = average of the two colors
    glm::vec3 voxel_center(0.0f, 0.0f, 0.0f);
    std::vector<glm::vec4> particles = {
        glm::vec4(0.5f, 0.0f, 0.0f, 0.0f),  // dist=0.5, category=0 (red)
        glm::vec4(-0.5f, 0.0f, 0.0f, 1.0f), // dist=0.5, category=1 (blue)
    };
    float influence_radius = 2.0f;

    glm::vec3 result = mc_color::blendColors(particles, voxel_center, influence_radius);

    // average of red(1,0,0) and blue(0.2,0.6,1) = (0.6, 0.3, 0.5)
    EXPECT_NEAR(result.x, 0.6f, 1e-5f);
    EXPECT_NEAR(result.y, 0.3f, 1e-5f);
    EXPECT_NEAR(result.z, 0.5f, 1e-5f);
}

TEST(MCColorBlendingTest, OneParticleDominant_ResultCloserToDominantColor)
{
    // Particle 0 very close (dist=0.1), category 0 (red) -> high weight
    // Particle 1 far (dist=5.0 > influence_radius=2.0) -> weight=0 (hard cutoff)
    glm::vec3 voxel_center(0.0f, 0.0f, 0.0f);
    std::vector<glm::vec4> particles = {
        glm::vec4(0.1f, 0.0f, 0.0f, 0.0f), // dist=0.1, category=0 (red)
        glm::vec4(5.0f, 0.0f, 0.0f, 1.0f), // dist=5.0, category=1 (blue), outside radius
    };
    float influence_radius = 2.0f;

    glm::vec3 result = mc_color::blendColors(particles, voxel_center, influence_radius);

    // Only red contributes (blue is outside radius) -> result is exactly red
    EXPECT_GT(result.x, 0.9f);
    EXPECT_LT(result.y, 0.1f);
    EXPECT_LT(result.z, 0.1f);
}

TEST(MCColorBlendingTest, Category500_Index0_ReturnsZeroVector)
{
    // Single particle with category=500 at vector index 0
    // Color formula: vec3(0%40/40.0f, 0%1600/1600.0f, 0%64000/64000.0f) = vec3(0,0,0)
    glm::vec3 voxel_center(0.0f, 0.0f, 0.0f);
    std::vector<glm::vec4> particles = {
        glm::vec4(0.0f, 0.0f, 0.0f, 500.0f), // at voxel_center, category=500, index=0
    };
    float influence_radius = 2.0f;

    glm::vec3 result = mc_color::blendColors(particles, voxel_center, influence_radius);

    EXPECT_NEAR(result.x, 0.0f, 1e-5f);
    EXPECT_NEAR(result.y, 0.0f, 1e-5f);
    EXPECT_NEAR(result.z, 0.0f, 1e-5f);
}

TEST(MCColorBlendingTest, Category500_Index40_ReturnsExpectedColor)
{
    // 41 particles total; particles[0..39] have category=0 but are at dist=100 (outside radius)
    // particles[40] has category=500 and is at voxel_center (dist=0, inside radius)
    // Color for index 40: vec3(40%40/40.0f, 40%1600/1600.0f, 40%64000/64000.0f)
    //                   = vec3(1.0f, 0.025f, 0.000625f)
    glm::vec3 voxel_center(0.0f, 0.0f, 0.0f);
    std::vector<glm::vec4> particles;
    // 40 dummy particles far outside influence radius
    for (int i = 0; i < 40; ++i) {
        particles.push_back(glm::vec4(100.0f, 0.0f, 0.0f, 0.0f));
    }
    // index 40: category=500, at voxel_center
    particles.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 500.0f));

    float influence_radius = 2.0f;

    glm::vec3 result = mc_color::blendColors(particles, voxel_center, influence_radius);

    // i=40: 40%40=0 -> x=0.0f, 40%1600=40 -> y=0.025f, 40%64000=40 -> z=0.000625f
    EXPECT_NEAR(result.x, 0.0f, 1e-5f);
    EXPECT_NEAR(result.y, 0.025f, 1e-5f);
    EXPECT_NEAR(result.z, 0.000625f, 1e-5f);
}

TEST(MCColorBlendingTest, EmptyParticleList_ReturnsZeroVector)
{
    glm::vec3 voxel_center(0.0f, 0.0f, 0.0f);
    std::vector<glm::vec4> particles;
    float influence_radius = 2.0f;

    glm::vec3 result = mc_color::blendColors(particles, voxel_center, influence_radius);

    EXPECT_NEAR(result.x, 0.0f, 1e-5f);
    EXPECT_NEAR(result.y, 0.0f, 1e-5f);
    EXPECT_NEAR(result.z, 0.0f, 1e-5f);
}

TEST(MCColorBlendingTest, AllParticlesOutsideRadius_ReturnsZeroVector)
{
    // Two particles at dist=10.0f, influence_radius=1.0f -> both outside, weight=0
    glm::vec3 voxel_center(0.0f, 0.0f, 0.0f);
    std::vector<glm::vec4> particles = {
        glm::vec4(10.0f, 0.0f, 0.0f, 0.0f),  // dist=10, outside radius
        glm::vec4(-10.0f, 0.0f, 0.0f, 1.0f), // dist=10, outside radius
    };
    float influence_radius = 1.0f;

    glm::vec3 result = mc_color::blendColors(particles, voxel_center, influence_radius);

    EXPECT_NEAR(result.x, 0.0f, 1e-5f);
    EXPECT_NEAR(result.y, 0.0f, 1e-5f);
    EXPECT_NEAR(result.z, 0.0f, 1e-5f);
}
