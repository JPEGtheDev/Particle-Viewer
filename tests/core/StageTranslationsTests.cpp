/*
 * StageTranslationsTests.cpp
 *
 * Unit tests for Particle::stageTranslations — CPU-only staging method
 * that copies position data without making any GL calls.
 */

// Include glad first to avoid OpenGL header conflicts
#include <vector>

#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "MockOpenGL.hpp"
#include "particle.hpp"

// ──────────────────────────────────────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────────────────────────────────────

static void expectEqVec4(const glm::vec4& actual, const glm::vec4& expected)
{
    EXPECT_FLOAT_EQ(actual.x, expected.x);
    EXPECT_FLOAT_EQ(actual.y, expected.y);
    EXPECT_FLOAT_EQ(actual.z, expected.z);
    EXPECT_FLOAT_EQ(actual.w, expected.w);
}

// ──────────────────────────────────────────────────────────────────────────────
// Test fixture
// ──────────────────────────────────────────────────────────────────────────────

class StageTranslationsTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }
};

// ──────────────────────────────────────────────────────────────────────────────
// Valid data: copies positions and sets n
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(StageTranslationsTest, StageTranslations_ValidData_CopiesIntoTranslations)
{
    // Arrange
    constexpr long N = 3;
    std::vector<glm::vec4> initial(1, glm::vec4(0.f));
    Particle p(1, initial.data());

    std::vector<glm::vec4> data = {
        {1.f, 2.f, 3.f, 4.f},
        {5.f, 6.f, 7.f, 8.f},
        {9.f, 10.f, 11.f, 12.f},
    };

    // Act
    p.stageTranslations(data.data(), N);

    // Assert
    ASSERT_EQ(static_cast<long>(p.translations.size()), N);
    EXPECT_EQ(p.n, N);
    expectEqVec4(p.translations[0], glm::vec4(1.f, 2.f, 3.f, 4.f));
    expectEqVec4(p.translations[1], glm::vec4(5.f, 6.f, 7.f, 8.f));
    expectEqVec4(p.translations[2], glm::vec4(9.f, 10.f, 11.f, 12.f));
}

// ──────────────────────────────────────────────────────────────────────────────
// Null data: no-op
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(StageTranslationsTest, StageTranslations_NullData_DoesNothing)
{
    // Arrange
    constexpr long N = 2;
    std::vector<glm::vec4> initial = {{1.f, 1.f, 1.f, 1.f}, {2.f, 2.f, 2.f, 2.f}};
    Particle p(N, initial.data());

    // Act
    p.stageTranslations(nullptr, N);

    // Assert
    EXPECT_EQ(p.n, N);
    ASSERT_EQ(static_cast<long>(p.translations.size()), N);
    expectEqVec4(p.translations[0], glm::vec4(1.f, 1.f, 1.f, 1.f));
    expectEqVec4(p.translations[1], glm::vec4(2.f, 2.f, 2.f, 2.f));
}

// ──────────────────────────────────────────────────────────────────────────────
// N <= 0: no-op
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(StageTranslationsTest, StageTranslations_ZeroN_DoesNothing)
{
    // Arrange
    constexpr long N = 2;
    std::vector<glm::vec4> initial = {{3.f, 3.f, 3.f, 3.f}, {4.f, 4.f, 4.f, 4.f}};
    Particle p(N, initial.data());

    std::vector<glm::vec4> newData = {{9.f, 9.f, 9.f, 9.f}, {8.f, 8.f, 8.f, 8.f}};

    // Act
    p.stageTranslations(newData.data(), 0);

    // Assert
    EXPECT_EQ(p.n, N);
    ASSERT_EQ(static_cast<long>(p.translations.size()), N);
    expectEqVec4(p.translations[0], glm::vec4(3.f, 3.f, 3.f, 3.f));
}

// ──────────────────────────────────────────────────────────────────────────────
// Updates n to match staged count
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(StageTranslationsTest, StageTranslations_UpdatesN)
{
    // Arrange
    constexpr long initial_n = 1;
    constexpr long staged_n = 7;
    std::vector<glm::vec4> initial(initial_n, glm::vec4(0.f));
    Particle p(initial_n, initial.data());

    std::vector<glm::vec4> staged(staged_n, glm::vec4(1.f));

    // Act
    p.stageTranslations(staged.data(), staged_n);

    // Assert
    EXPECT_EQ(p.n, staged_n);
}

// ──────────────────────────────────────────────────────────────────────────────
// No GL buffer calls after stageTranslations (pushVBO not called)
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(StageTranslationsTest, StageTranslations_NoGLCalls_AfterStage)
{
    // Arrange
    constexpr long N = 3;
    std::vector<glm::vec4> initial(N, glm::vec4(0.f));
    Particle p(N, initial.data());

    // Record the VBO id before staging (set by constructor's setUpInstanceBuffer)
    GLuint vbo_before = p.instanceVBO;

    // Reset mock counters after construction (we only care about staging calls)
    MockOpenGL::reset();
    MockOpenGL::initGLAD();

    std::vector<glm::vec4> staged(N, glm::vec4(2.f));

    // Act — stageTranslations must not invoke any GL calls
    p.stageTranslations(staged.data(), N);

    // Assert — the VBO handle must be unchanged: no glGenBuffers or glDeleteBuffers ran.
    // (MockOpenGL does not track buffer call counts; instanceVBO identity is the proxy.)
    EXPECT_EQ(p.instanceVBO, vbo_before);
}

// ──────────────────────────────────────────────────────────────────────────────
// Negative N: no-op (guard covers N <= 0)
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(StageTranslationsTest, StageTranslations_NegativeN_DoesNothing)
{
    // Arrange
    constexpr long N = 2;
    std::vector<glm::vec4> initial = {{7.f, 7.f, 7.f, 7.f}, {8.f, 8.f, 8.f, 8.f}};
    Particle p(N, initial.data());

    std::vector<glm::vec4> newData = {{0.f, 0.f, 0.f, 0.f}};

    // Act — negative N must be treated as a no-op by the N <= 0 guard
    p.stageTranslations(newData.data(), -1);

    // Assert
    EXPECT_EQ(p.n, N);
    ASSERT_EQ(static_cast<long>(p.translations.size()), N);
    expectEqVec4(p.translations[0], glm::vec4(7.f, 7.f, 7.f, 7.f));
}
