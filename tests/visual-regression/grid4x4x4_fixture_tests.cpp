#include <gtest/gtest.h>

#include "fixtures/grid4x4x4_fixture.hpp"

constexpr float EXPECTED_EXTENT = 0.375f; // 1.5 * SIM_TO_DISPLAY_SCALE
constexpr float EXTENT_TOLERANCE = 0.001f;

TEST(Grid4x4x4FixtureTest, Grid4x4x4Fixture_MakeParticles_Returns64Particles)
{
    auto particles = grid4x4x4::makeParticles();
    EXPECT_EQ(particles.size(), static_cast<size_t>(grid4x4x4::GRID_DIM * grid4x4x4::GRID_DIM * grid4x4x4::GRID_DIM));
}

TEST(Grid4x4x4FixtureTest, Grid4x4x4Fixture_AllParticles_HaveCategoryZero)
{
    auto particles = grid4x4x4::makeParticles();
    for (const auto& p : particles) {
        EXPECT_EQ(static_cast<int>(p.w), 0);
    }
}

TEST(Grid4x4x4FixtureTest, Grid4x4x4Fixture_Particles_SpanExpectedDisplayRange)
{
    auto particles = grid4x4x4::makeParticles();
    // min and max should be at -0.375 and 0.375 on all axes
    for (const auto& p : particles) {
        EXPECT_GE(p.x, -(EXPECTED_EXTENT + EXTENT_TOLERANCE));
        EXPECT_LE(p.x, (EXPECTED_EXTENT + EXTENT_TOLERANCE));
        EXPECT_GE(p.y, -(EXPECTED_EXTENT + EXTENT_TOLERANCE));
        EXPECT_LE(p.y, (EXPECTED_EXTENT + EXTENT_TOLERANCE));
        EXPECT_GE(p.z, -(EXPECTED_EXTENT + EXTENT_TOLERANCE));
        EXPECT_LE(p.z, (EXPECTED_EXTENT + EXTENT_TOLERANCE));
    }
}
