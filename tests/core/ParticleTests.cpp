/*
 * ParticleTests.cpp
 *
 * Unit tests for the Particle class following AAA pattern
 * and single-assertion principle.
 */

// Include glad first to avoid OpenGL header conflicts
#define GLFW_INCLUDE_NONE
#include <glad/glad.h>

#include <gtest/gtest.h>
#include <glm/glm.hpp>

#include "particle.hpp"
#include "MockOpenGL.hpp"

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
};

// ============================================
// Constructor Tests
// ============================================

TEST_F(ParticleTest, DefaultConstructor_Creates64000Particles)
{
    // Arrange & Act
    Particle p;

    // Assert
    EXPECT_EQ(p.n, 64000);
}

TEST_F(ParticleTest, DefaultConstructor_AllocatesTranslations)
{
    // Arrange & Act
    Particle p;

    // Assert
    EXPECT_NE(p.translations, nullptr);
}

TEST_F(ParticleTest, DefaultConstructor_AllocatesVelocities)
{
    // Arrange & Act
    Particle p;

    // Assert
    EXPECT_NE(p.velocities, nullptr);
}

TEST_F(ParticleTest, DefaultConstructor_InitializesFirstParticlePosition)
{
    // Arrange & Act
    Particle p;

    // Assert - First particle should be at (0, 0, 0, 500)
    EXPECT_EQ(p.translations[0], glm::vec4(0.0f, 0.0f, 0.0f, 500.0f));
}

TEST_F(ParticleTest, DefaultConstructor_InitializesLastParticlePosition)
{
    // Arrange & Act
    Particle p;

    // Assert - Last particle position calculation
    // i = 63999: x = (63999 % 40) * 1.25 = 39 * 1.25 = 48.75
    // y = (63999 % 1600) / 40.0 * 1.25 = 1599 / 40.0 * 1.25 = 49.96875
    // z = (63999 % 64000) / 1600.0 * 1.25 = 63999 / 1600.0 * 1.25 = 49.99921875
    EXPECT_FLOAT_EQ(p.translations[63999].x, 48.75f);
    EXPECT_FLOAT_EQ(p.translations[63999].y, 49.96875f);
    EXPECT_FLOAT_EQ(p.translations[63999].z, 49.99921875f);
    EXPECT_FLOAT_EQ(p.translations[63999].w, 500.0f);
}

TEST_F(ParticleTest, CustomConstructor_SetsParticleCount)
{
    // Arrange
    long N = 10;
    glm::vec4* trans = new glm::vec4[N];

    // Act
    Particle p(N, trans);

    // Assert
    EXPECT_EQ(p.n, 10);
}

TEST_F(ParticleTest, CustomConstructor_StoresTranslationPointer)
{
    // Arrange
    long N = 5;
    glm::vec4* trans = new glm::vec4[N];
    trans[0] = glm::vec4(1.0f, 2.0f, 3.0f, 4.0f);

    // Act
    Particle p(N, trans);

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
    glm::vec4* newTrans = new glm::vec4[5]{
        {1.0f, 0.0f, 0.0f, 1.0f},
        {0.0f, 1.0f, 0.0f, 1.0f},
        {0.0f, 0.0f, 1.0f, 1.0f},
        {1.0f, 1.0f, 0.0f, 1.0f},
        {1.0f, 1.0f, 1.0f, 1.0f}
    };

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
    glm::vec4* newTrans = new glm::vec4[3]{
        {5.0f, 6.0f, 7.0f, 8.0f},
        {9.0f, 10.0f, 11.0f, 12.0f},
        {13.0f, 14.0f, 15.0f, 16.0f}
    };

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
    glm::vec4* newTrans = new glm::vec4[3]{
        {1.0f, 2.0f, 3.0f, 4.0f},
        {5.0f, 6.0f, 7.0f, 8.0f},
        {9.0f, 10.0f, 11.0f, 12.0f}
    };

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

    // Act & Assert - Should not crash
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
    EXPECT_NE(p.velocities, nullptr);

    // Cleanup
    delete[] newVel;
}

TEST_F(ParticleTest, ChangeVelocities_WithNullPointer_DoesNotCrash)
{
    // Arrange
    Particle p;

    // Act & Assert - Should not crash
    EXPECT_NO_THROW(p.changeVelocities(nullptr));
}

TEST_F(ParticleTest, ChangeVelocities_WithNullPointer_PrintsError)
{
    // Arrange
    Particle p;

    // Act
    p.changeVelocities(nullptr);

    // Assert - Can't directly test console output, but ensures no crash
    EXPECT_NE(p.velocities, nullptr); // Original velocities should still exist
}

// ============================================
// Memory Management Tests
// ============================================

TEST_F(ParticleTest, Destructor_FreesMemoryWithoutCrash)
{
    // Arrange & Act & Assert - Should not crash
    EXPECT_NO_THROW({
        Particle* p = new Particle();
        delete p;
    });
}

TEST_F(ParticleTest, Destructor_AfterChangeTranslations_FreesMemoryWithoutCrash)
{
    // Arrange
    Particle* p = new Particle();
    long newN = 5;
    glm::vec4* newTrans = new glm::vec4[5];
    p->changeTranslations(newN, newTrans);
    delete[] newTrans;

    // Act & Assert - Should not crash or leak
    EXPECT_NO_THROW(delete p);
}

TEST_F(ParticleTest, Destructor_AfterChangeVelocities_FreesMemoryWithoutCrash)
{
    // Arrange
    Particle* p = new Particle();
    glm::vec4* newVel = new glm::vec4[p->n];
    p->changeVelocities(newVel);
    delete[] newVel;

    // Act & Assert - Should not crash or leak
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

TEST_F(ParticleTest, ChangeTranslations_WithZeroParticles_HandlesGracefully)
{
    // Arrange
    Particle p;
    long newN = 0;
    glm::vec4* newTrans = new glm::vec4[1]; // Allocate at least 1 to avoid null

    // Act
    p.changeTranslations(newN, newTrans);

    // Assert
    EXPECT_EQ(p.n, 0);

    // Cleanup
    delete[] newTrans;
}

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
