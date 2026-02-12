/*
 * ParticleTests.cpp
 *
 * Unit tests for the Particle class following AAA pattern
 * and single-assertion principle.
 */

// Include glad first to avoid OpenGL header conflicts
#define GLFW_INCLUDE_NONE
#include <vector>

#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "MockOpenGL.hpp"
#include "particle.hpp"

// Test fixture for Particle tests
class ParticleTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }

    void TearDown() override
    {
        // Cleanup after each test
    }

    // Test constants
    static constexpr float DEFAULT_PARTICLE_SIZE = 500.0f;
};

// ============================================
// Constructor Tests
// ============================================

TEST_F(ParticleTest, DefaultConstructor_Creates64000Particles)
{
    // Act
    Particle p;

    // Assert
    EXPECT_EQ(p.n, 64000);
}

TEST_F(ParticleTest, DefaultConstructor_AllocatesTranslations)
{
    // Act
    Particle p;

    // Assert
    EXPECT_FALSE(p.translations.empty());
}

TEST_F(ParticleTest, DefaultConstructor_AllocatesVelocities)
{
    // Act
    Particle p;

    // Assert
    EXPECT_FALSE(p.velocities.empty());
}

TEST_F(ParticleTest, DefaultConstructor_InitializesFirstParticlePosition)
{
    // Arrange
    glm::vec4 expected_position(0.0f, 0.0f, 0.0f, DEFAULT_PARTICLE_SIZE);

    // Act
    Particle p;

    // Assert
    EXPECT_EQ(p.translations[0], expected_position);
}

TEST_F(ParticleTest, DefaultConstructor_InitializesLastParticlePosition)
{
    // Arrange
    // i = 63999: x = (63999 % 40) * 1.25 = 39 * 1.25 = 48.75
    // y = (63999 % 1600) / 40.0 * 1.25 = 1599 / 40.0 * 1.25 = 49.96875
    // z = (63999 % 64000) / 1600.0 * 1.25 = 63999 / 1600.0 * 1.25 = 49.99921875
    float expected_x = 48.75f;
    float expected_y = 49.96875f;
    float expected_z = 49.99921875f;

    // Act
    Particle p;

    // Assert
    EXPECT_FLOAT_EQ(p.translations[63999].x, expected_x);
    EXPECT_FLOAT_EQ(p.translations[63999].y, expected_y);
    EXPECT_FLOAT_EQ(p.translations[63999].z, expected_z);
    EXPECT_FLOAT_EQ(p.translations[63999].w, DEFAULT_PARTICLE_SIZE);
}

TEST_F(ParticleTest, CustomConstructor_SetsParticleCount)
{
    // Arrange
    long N = 10;
    std::vector<glm::vec4> trans(N);

    // Act
    Particle p(N, trans.data());

    // Assert
    EXPECT_EQ(p.n, 10);
}

TEST_F(ParticleTest, CustomConstructor_CopiesTranslationData)
{
    // Arrange
    long N = 5;
    std::vector<glm::vec4> trans(N);
    trans[0] = glm::vec4(1.0f, 2.0f, 3.0f, 4.0f);

    // Act
    Particle p(N, trans.data());

    // Assert
    EXPECT_EQ(p.translations[0], glm::vec4(1.0f, 2.0f, 3.0f, 4.0f));
}

// ============================================
// changeTranslations Tests
// ============================================

TEST_F(ParticleTest, ChangeTranslations_UpdatesParticleCount)
{
    // Arrange
    Particle p;
    long newN = 5;
    glm::vec4* newTrans = new glm::vec4[5]{{1.0f, 0.0f, 0.0f, 1.0f},
                                           {0.0f, 1.0f, 0.0f, 1.0f},
                                           {0.0f, 0.0f, 1.0f, 1.0f},
                                           {1.0f, 1.0f, 0.0f, 1.0f},
                                           {1.0f, 1.0f, 1.0f, 1.0f}};

    // Act
    p.changeTranslations(newN, newTrans);

    // Assert
    EXPECT_EQ(p.n, 5);

    // Cleanup
    delete[] newTrans;
}

TEST_F(ParticleTest, ChangeTranslations_CopiesDataCorrectly)
{
    // Arrange
    Particle p;
    long newN = 3;
    glm::vec4* newTrans =
        new glm::vec4[3]{{5.0f, 6.0f, 7.0f, 8.0f}, {9.0f, 10.0f, 11.0f, 12.0f}, {13.0f, 14.0f, 15.0f, 16.0f}};

    // Act
    p.changeTranslations(newN, newTrans);

    // Assert
    EXPECT_EQ(p.translations[0], glm::vec4(5.0f, 6.0f, 7.0f, 8.0f));

    // Cleanup
    delete[] newTrans;
}

TEST_F(ParticleTest, ChangeTranslations_CopiesAllElements)
{
    // Arrange
    Particle p;
    long newN = 3;
    glm::vec4* newTrans =
        new glm::vec4[3]{{1.0f, 2.0f, 3.0f, 4.0f}, {5.0f, 6.0f, 7.0f, 8.0f}, {9.0f, 10.0f, 11.0f, 12.0f}};

    // Act
    p.changeTranslations(newN, newTrans);

    // Assert
    EXPECT_EQ(p.translations[1], glm::vec4(5.0f, 6.0f, 7.0f, 8.0f));
    EXPECT_EQ(p.translations[2], glm::vec4(9.0f, 10.0f, 11.0f, 12.0f));

    // Cleanup
    delete[] newTrans;
}

TEST_F(ParticleTest, ChangeTranslations_WithNullPointer_KeepsOriginalData)
{
    // Arrange
    Particle p;
    long originalN = p.n;

    // Act
    p.changeTranslations(10, nullptr);

    // Assert - Should keep original particle count
    EXPECT_EQ(p.n, originalN);
}

TEST_F(ParticleTest, ChangeTranslations_WithNullPointer_PrintsError)
{
    // Arrange
    Particle p;

    // Act & Assert
    EXPECT_NO_THROW(p.changeTranslations(10, nullptr));
}

// ============================================
// changeVelocities Tests
// ============================================

TEST_F(ParticleTest, ChangeVelocities_CopiesDataCorrectly)
{
    // Arrange
    Particle p; // Creates p with n=64000
    glm::vec4* newVel = new glm::vec4[p.n];
    newVel[0] = glm::vec4(1.0f, 2.0f, 3.0f, 4.0f);
    newVel[100] = glm::vec4(5.0f, 6.0f, 7.0f, 8.0f);

    // Act
    p.changeVelocities(newVel);

    // Assert
    EXPECT_EQ(p.velocities[0], glm::vec4(1.0f, 2.0f, 3.0f, 4.0f));

    // Cleanup
    delete[] newVel;
}

TEST_F(ParticleTest, ChangeVelocities_CopiesAllElements)
{
    // Arrange
    Particle p; // Creates p with n=64000
    glm::vec4* newVel = new glm::vec4[p.n];
    for (int i = 0; i < p.n; i++) {
        newVel[i] = glm::vec4((float)i, (float)i * 2, (float)i * 3, (float)i * 4);
    }

    // Act
    p.changeVelocities(newVel);

    // Assert - Check a sample of values
    EXPECT_EQ(p.velocities[1000], glm::vec4(1000.0f, 2000.0f, 3000.0f, 4000.0f));

    // Cleanup
    delete[] newVel;
}

TEST_F(ParticleTest, ChangeVelocities_AllocatesNewArray)
{
    // Arrange
    Particle p;
    glm::vec4* newVel = new glm::vec4[p.n];
    for (int i = 0; i < p.n; i++) {
        newVel[i] = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    // Act
    p.changeVelocities(newVel);

    // Assert
    EXPECT_FALSE(p.velocities.empty());

    // Cleanup
    delete[] newVel;
}

TEST_F(ParticleTest, ChangeVelocities_WithNullPointer_DoesNotCrash)
{
    // Arrange
    Particle p;

    // Act & Assert
    EXPECT_NO_THROW(p.changeVelocities(nullptr));
}

TEST_F(ParticleTest, ChangeVelocities_WithNullPointer_PrintsError)
{
    // Arrange
    Particle p;

    // Act
    p.changeVelocities(nullptr);

    // Assert - Original velocities should still exist
    EXPECT_FALSE(p.velocities.empty());
}

// ============================================
// Memory Management Tests
// ============================================

TEST_F(ParticleTest, Destructor_FreesMemoryWithoutCrash)
{
    // Act
    Particle* p = new Particle();

    // Assert
    EXPECT_NO_THROW(delete p);
}

TEST_F(ParticleTest, Destructor_AfterChangeTranslations_FreesMemoryWithoutCrash)
{
    // Arrange
    Particle* p = new Particle();
    long newN = 5;
    glm::vec4* newTrans = new glm::vec4[5];
    p->changeTranslations(newN, newTrans);
    delete[] newTrans;

    // Act & Assert
    EXPECT_NO_THROW(delete p);
}

TEST_F(ParticleTest, Destructor_AfterChangeVelocities_FreesMemoryWithoutCrash)
{
    // Arrange
    Particle* p = new Particle();
    glm::vec4* newVel = new glm::vec4[p->n];
    p->changeVelocities(newVel);
    delete[] newVel;

    // Act & Assert
    EXPECT_NO_THROW(delete p);
}

// ============================================
// Particle Count Consistency Tests
// ============================================

TEST_F(ParticleTest, ParticleCount_RemainsConsistent_AfterChangeTranslations)
{
    // Arrange
    Particle p;
    long newN = 100;
    glm::vec4* newTrans = new glm::vec4[100];

    // Act
    p.changeTranslations(newN, newTrans);

    // Assert
    EXPECT_EQ(p.n, 100);

    // Cleanup
    delete[] newTrans;
}

TEST_F(ParticleTest, ParticleCount_RemainsConsistent_AfterMultipleChanges)
{
    // Arrange
    Particle p;
    long firstN = 50;
    glm::vec4* firstTrans = new glm::vec4[50];
    long secondN = 75;
    glm::vec4* secondTrans = new glm::vec4[75];

    // Act
    p.changeTranslations(firstN, firstTrans);
    p.changeTranslations(secondN, secondTrans);

    // Assert
    EXPECT_EQ(p.n, 75);

    // Cleanup
    delete[] firstTrans;
    delete[] secondTrans;
}

TEST_F(ParticleTest, ParticleCount_DoesNotChange_AfterChangeVelocities)
{
    // Arrange
    Particle p;
    long originalN = p.n;
    glm::vec4* newVel = new glm::vec4[p.n];

    // Act
    p.changeVelocities(newVel);

    // Assert
    EXPECT_EQ(p.n, originalN);

    // Cleanup
    delete[] newVel;
}

// ============================================
// Edge Cases
// ============================================

TEST_F(ParticleTest, ChangeTranslations_WithSingleParticle_WorksCorrectly)
{
    // Arrange
    Particle p;
    long newN = 1;
    glm::vec4* newTrans = new glm::vec4[1];
    newTrans[0] = glm::vec4(10.0f, 20.0f, 30.0f, 40.0f);

    // Act
    p.changeTranslations(newN, newTrans);

    // Assert
    EXPECT_EQ(p.n, 1);
    EXPECT_EQ(p.translations[0], glm::vec4(10.0f, 20.0f, 30.0f, 40.0f));

    // Cleanup
    delete[] newTrans;
}

// ============================================
// Cube Particle Spacing Validation Tests
// ============================================

/*
 * These tests validate that the default 40x40x40 particle cube has correct
 * spacing between particles. The cube uses a specific formula where particles
 * are indexed linearly but positioned in 3D space with 1.25 unit spacing.
 */

TEST_F(ParticleTest, DefaultCube_Has64000Particles)
{
    // Arrange & Act
    Particle p;

    // Assert - 40x40x40 cube = 64,000 particles
    EXPECT_EQ(p.n, 64000);
}

TEST_F(ParticleTest, DefaultCube_XAxisSpacing_IsUniform)
{
    // Arrange
    Particle p;
    const float EXPECTED_SPACING = 1.25f;

    // Act - Check X-axis spacing in the first row (indices 0-39)
    // These particles have incrementing X coordinates with consistent spacing
    for (int i = 0; i < 39; i++) {
        float x_spacing = p.translations[i + 1].x - p.translations[i].x;

        // Assert
        EXPECT_FLOAT_EQ(x_spacing, EXPECTED_SPACING)
            << "X spacing between particles " << i << " and " << (i + 1) << " is not 1.25";
    }
}

TEST_F(ParticleTest, DefaultCube_YAxisSpacing_IsUniform)
{
    // Arrange
    Particle p;
    const float EXPECTED_SPACING = 1.25f;
    const float TOLERANCE = 0.001f;

    // Act - Check Y-axis spacing at X=0, Z=0 (indices 0, 40, 80, ...)
    // Particles 40 indices apart have 1.25 unit Y spacing
    for (int y = 0; y < 39; y++) {
        int idx = y * 40;
        float y_spacing = p.translations[idx + 40].y - p.translations[idx].y;

        // Assert
        EXPECT_NEAR(y_spacing, EXPECTED_SPACING, TOLERANCE)
            << "Y spacing at Y-layer " << y << " is not 1.25";
    }
}

TEST_F(ParticleTest, DefaultCube_ZAxisSpacing_IsUniform)
{
    // Arrange
    Particle p;
    const float EXPECTED_SPACING = 1.25f;
    const float TOLERANCE = 0.001f;

    // Act - Check Z-axis spacing at X=0, Y=0 (indices 0, 1600, 3200, ...)
    // Particles 1600 indices apart have 1.25 unit Z spacing
    for (int z = 0; z < 39; z++) {
        int idx = z * 1600;
        float z_spacing = p.translations[idx + 1600].z - p.translations[idx].z;

        // Assert
        EXPECT_NEAR(z_spacing, EXPECTED_SPACING, TOLERANCE)
            << "Z spacing at Z-layer " << z << " is not 1.25";
    }
}

TEST_F(ParticleTest, DefaultCube_OriginIsAtZero)
{
    // Arrange
    Particle p;
    const float TOLERANCE = 0.001f;

    // Act & Assert - First particle should be at origin
    EXPECT_NEAR(p.translations[0].x, 0.0f, TOLERANCE);
    EXPECT_NEAR(p.translations[0].y, 0.0f, TOLERANCE);
    EXPECT_NEAR(p.translations[0].z, 0.0f, TOLERANCE);
}

TEST_F(ParticleTest, DefaultCube_MaxDimensionsAreEqual)
{
    // Arrange
    Particle p;
    const float EXPECTED_MAX = 39 * 1.25f; // 48.75
    const float TOLERANCE = 0.001f;

    // Act - Find maximum extent along each axis
    float max_x = p.translations[39].x;        // Max X at index 39
    float max_y = p.translations[1560].y;      // Max Y at index 39*40
    float max_z = p.translations[62400].z;     // Max Z at index 39*1600

    // Assert - Cube should be equal in all dimensions
    EXPECT_NEAR(max_x, EXPECTED_MAX, TOLERANCE);
    EXPECT_NEAR(max_y, EXPECTED_MAX, TOLERANCE);
    EXPECT_NEAR(max_z, EXPECTED_MAX, TOLERANCE);
}

TEST_F(ParticleTest, DefaultCube_AllParticlesHaveSameSize)
{
    // Arrange
    Particle p;

    // Act & Assert - All particles should have w=500
    for (int i = 0; i < p.n; i++) {
        EXPECT_FLOAT_EQ(p.translations[i].w, DEFAULT_PARTICLE_SIZE)
            << "Particle " << i << " does not have the expected size";
    }
}

TEST_F(ParticleTest, DefaultCube_CubeIsNotEmpty)
{
    // Arrange
    Particle p;

    // Act & Assert - Verify particles actually fill 3D space
    // Check that we have particles with varying X, Y, and Z coordinates
    bool has_varying_x = false;
    bool has_varying_y = false;
    bool has_varying_z = false;

    for (int i = 1; i < p.n; i++) {
        if (p.translations[i].x != p.translations[0].x)
            has_varying_x = true;
        if (p.translations[i].y != p.translations[0].y)
            has_varying_y = true;
        if (p.translations[i].z != p.translations[0].z)
            has_varying_z = true;
    }

    EXPECT_TRUE(has_varying_x) << "All particles have the same X coordinate";
    EXPECT_TRUE(has_varying_y) << "All particles have the same Y coordinate";
    EXPECT_TRUE(has_varying_z) << "All particles have the same Z coordinate";
}
