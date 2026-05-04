#include <span>

// Include glad before other OpenGL-related headers to avoid conflicts
#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "COMCalculator.hpp"
#include "MassParams.hpp"
#include "constants.hpp"

/// Sentinel MassParams for tests where exact mass values don't matter,
/// only that all fractions are nonzero so a COM is computable.
constexpr MassParams kSimpleMassParams{
    1.0, // fraction_earth_mass_of_body1
    1.0, // fraction_earth_mass_of_body2
    1.0, // fraction_fe_body1
    1.0, // fraction_si_body1
    1.0, // fraction_fe_body2
    1.0, // fraction_si_body2
    1.0  // mass_of_earth
};

/// Checks that two vec3 values are component-wise within tolerance.
static void expectNearVec3(const glm::vec3& actual, const glm::vec3& expected, double tol = 1e-5)
{
    EXPECT_NEAR(static_cast<double>(actual.x), static_cast<double>(expected.x), tol);
    EXPECT_NEAR(static_cast<double>(actual.y), static_cast<double>(expected.y), tol);
    EXPECT_NEAR(static_cast<double>(actual.z), static_cast<double>(expected.z), tol);
}

class COMCalculatorTest : public ::testing::Test
{
};

// ─── Test 1 ──────────────────────────────────────────────────────────────────

TEST_F(COMCalculatorTest, COMCalculator_EmptySpan_ReturnsZero)
{
    // Arrange
    std::span<const glm::vec4> empty{};

    // Act
    glm::vec3 result = COMCalculator::computeMassWeightedCOM(empty, kSimpleMassParams);

    // Assert
    expectNearVec3(result, glm::vec3(0.0f));
}

// ─── Test 2 ──────────────────────────────────────────────────────────────────

TEST_F(COMCalculatorTest, COMCalculator_SingleFe1Particle_ReturnsPosScaled)
{
    // Arrange — one Fe1 particle at sim position (10, 20, 30)
    // With a single particle the COM equals that position; scale by kSimToDisplayScale.
    const glm::vec4 particle{10.0f, 20.0f, 30.0f, 0.0f};
    std::span<const glm::vec4> positions{&particle, 1};

    const glm::vec3 expected = glm::vec3(10.0f, 20.0f, 30.0f) * kSimToDisplayScale;

    // Act
    glm::vec3 result = COMCalculator::computeMassWeightedCOM(positions, kSimpleMassParams);

    // Assert
    expectNearVec3(result, expected);
}

// ─── Test 3 ──────────────────────────────────────────────────────────────────

TEST_F(COMCalculatorTest, COMCalculator_TwoParticlesEqualMass_ReturnsMidpoint)
{
    // Arrange — two Fe1 particles; equal per-particle mass → midpoint × scale
    const glm::vec4 particles[2] = {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {10.0f, 20.0f, 30.0f, 0.0f},
    };
    std::span<const glm::vec4> positions{particles};

    const glm::vec3 midpointSim{5.0f, 10.0f, 15.0f};
    const glm::vec3 expected = midpointSim * kSimToDisplayScale;

    // Act
    glm::vec3 result = COMCalculator::computeMassWeightedCOM(positions, kSimpleMassParams);

    // Assert
    expectNearVec3(result, expected);
}

// ─── Test 4 ──────────────────────────────────────────────────────────────────

TEST_F(COMCalculatorTest, COMCalculator_UnknownTypeIgnored_UsesOnlyKnownTypes)
{
    // Arrange — one type-0 particle at (10,20,30) and one unknown type (500) at (100,200,300).
    // Only the type-0 particle should contribute.
    const glm::vec4 particles[2] = {
        {10.0f, 20.0f, 30.0f, 0.0f},
        {100.0f, 200.0f, 300.0f, 500.0f},
    };
    std::span<const glm::vec4> positions{particles};

    const glm::vec3 expected = glm::vec3(10.0f, 20.0f, 30.0f) * kSimToDisplayScale;

    // Act
    glm::vec3 result = COMCalculator::computeMassWeightedCOM(positions, kSimpleMassParams);

    // Assert
    expectNearVec3(result, expected);
}

// ─── Test 5 ──────────────────────────────────────────────────────────────────

TEST_F(COMCalculatorTest, COMCalculator_ZeroBodyCount_ReturnsZero)
{
    // Arrange — particles are type-2 (Fe body 2) but fraction_fe_body2 = 0.
    // m_Fe2 = (0.0 * 1.0 * 1.0) / N_Fe2 = 0, so total_mass = 0 → (0,0,0).
    // (Zero comes from fraction_fe_body2 = 0, not from a missing-particle N=0 guard.)
    // The N=0 guards fire for Fe1/Si1/Si2 (no such particles in the span).
    const glm::vec4 particles[2] = {
        {5.0f, 5.0f, 5.0f, 2.0f},
        {-5.0f, -5.0f, -5.0f, 2.0f},
    };
    std::span<const glm::vec4> positions{particles};

    MassParams mp{
        1.0, // fraction_earth_mass_of_body1
        1.0, // fraction_earth_mass_of_body2
        1.0, // fraction_fe_body1
        1.0, // fraction_si_body1
        0.0, // fraction_fe_body2 — zero → m_Fe2 = 0, total_mass = 0
        0.0, // fraction_si_body2
        1.0  // mass_of_earth
    };

    // Act
    glm::vec3 result = COMCalculator::computeMassWeightedCOM(positions, mp);

    // Assert
    expectNearVec3(result, glm::vec3(0.0f));
}

// ─── Test 6 ──────────────────────────────────────────────────────────────────

TEST_F(COMCalculatorTest, COMCalculator_TwoBodyMix_WeightedCorrectly)
{
    // Arrange
    // Body 1: fraction_earth_mass_of_body1=2.0, fraction_fe_body1=1.0, mass_of_earth=1.0
    //         → m_Fe1 = 1.0 * 2.0 * 1.0 / 1 = 2.0
    // Body 2: fraction_earth_mass_of_body2=1.0, fraction_fe_body2=1.0
    //         → m_Fe2 = 1.0 * 1.0 * 1.0 / 1 = 1.0
    //
    // Particle type-0 at (0, 0, 0): contributes 2.0 * (0, 0, 0) = (0, 0, 0)
    // Particle type-2 at (6, 0, 0): contributes 1.0 * (6, 0, 0) = (6, 0, 0)
    // total_mass = 3.0
    // COM_sim = (6/3, 0, 0) = (2, 0, 0)
    // COM_display = (2, 0, 0) * kSimToDisplayScale = (0.5, 0, 0)
    const glm::vec4 particles[2] = {
        {0.0f, 0.0f, 0.0f, 0.0f},
        {6.0f, 0.0f, 0.0f, 2.0f},
    };
    std::span<const glm::vec4> positions{particles};

    MassParams mp{
        2.0, // fraction_earth_mass_of_body1
        1.0, // fraction_earth_mass_of_body2
        1.0, // fraction_fe_body1
        0.0, // fraction_si_body1 (unused)
        1.0, // fraction_fe_body2
        0.0, // fraction_si_body2 (unused)
        1.0  // mass_of_earth
    };

    const glm::vec3 expected{0.5f, 0.0f, 0.0f};

    // Act
    glm::vec3 result = COMCalculator::computeMassWeightedCOM(positions, mp);

    // Assert
    expectNearVec3(result, expected);
}

// ─── Test 7 ──────────────────────────────────────────────────────────────────

TEST_F(COMCalculatorTest, COMCalculator_SingleSi1Particle_ReturnsPosScaled)
{
    // Arrange — one Si1 particle (type 1) at sim position (4, 8, 12).
    // Verifies the case-1 branch is exercised independently.
    const glm::vec4 particle{4.0f, 8.0f, 12.0f, 1.0f};
    std::span<const glm::vec4> positions{&particle, 1};

    const glm::vec3 expected = glm::vec3(4.0f, 8.0f, 12.0f) * kSimToDisplayScale;

    // Act
    glm::vec3 result = COMCalculator::computeMassWeightedCOM(positions, kSimpleMassParams);

    // Assert
    expectNearVec3(result, expected);
}

// ─── Test 8 ──────────────────────────────────────────────────────────────────

TEST_F(COMCalculatorTest, COMCalculator_NegativePositions_WeightedCorrectly)
{
    // Arrange — two Fe1 particles with negative coordinates; COM is their midpoint.
    const glm::vec4 particles[2] = {
        {-8.0f, -4.0f, -2.0f, 0.0f},
        {-2.0f, 0.0f, 2.0f, 0.0f},
    };
    std::span<const glm::vec4> positions{particles};

    // Equal mass → midpoint × scale
    const glm::vec3 midpointSim{-5.0f, -2.0f, 0.0f};
    const glm::vec3 expected = midpointSim * kSimToDisplayScale;

    // Act
    glm::vec3 result = COMCalculator::computeMassWeightedCOM(positions, kSimpleMassParams);

    // Assert
    expectNearVec3(result, expected);
}
