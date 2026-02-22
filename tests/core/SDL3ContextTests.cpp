/*
 * SDL3ContextTests.cpp
 *
 * Tests for the SDL3Context production OpenGL context implementation.
 * These tests exercise the real SDL3 + OpenGL stack and therefore require
 * a display (real or virtual).
 *
 * Run headless with Xvfb:
 *   xvfb-run -a ./tests/ParticleViewerTests --gtest_filter="SDL3ContextTest.*"
 *
 * Each test uses SetUp/TearDown to create and destroy the context so that
 * SDL3's init/quit reference count is always balanced between tests.
 */

#include <gtest/gtest.h>

// clang-format off
#include "glad/glad.h"  // NOLINT(llvm-include-order)
// clang-format on

#include "graphics/SDL3Context.hpp"

// ============================================================================
// Test fixture
// ============================================================================

class SDL3ContextTest : public ::testing::Test
{
  protected:
    SDL3Context* context_ = nullptr;

    void SetUp() override
    {
        context_ = new SDL3Context(640, 480, "SDL3ContextTest", false);
        if (!context_->isValid()) {
            delete context_;
            context_ = nullptr;
            GTEST_SKIP() << "SDL3Context initialization failed (no display available). "
                         << "Run headless: xvfb-run -a ./tests/ParticleViewerTests "
                         << "--gtest_filter=\"SDL3ContextTest.*\"";
        }
    }

    void TearDown() override
    {
        delete context_;
        context_ = nullptr;
    }
};

// ============================================================================
// Initialization tests
// ============================================================================

/*
 * Test: IsValid_AfterConstruction_ReturnsTrue
 *
 * A newly created context backed by a real display should report itself valid.
 */
TEST_F(SDL3ContextTest, IsValid_AfterConstruction_ReturnsTrue)
{
    // Assert
    EXPECT_TRUE(context_->isValid());
}

/*
 * Test: GetNativeWindowHandle_AfterConstruction_ReturnsNonNull
 *
 * getNativeWindowHandle() must return a non-null SDL_Window* so callers
 * (e.g. ImGui_ImplSDL3_InitForOpenGL) can use it.
 */
TEST_F(SDL3ContextTest, GetNativeWindowHandle_AfterConstruction_ReturnsNonNull)
{
    // Assert
    EXPECT_NE(context_->getNativeWindowHandle(), nullptr);
}

// ============================================================================
// Framebuffer size tests
// ============================================================================

/*
 * Test: GetFramebufferSize_AfterConstruction_ReturnsPositiveDimensions
 *
 * The framebuffer must have non-zero width and height.
 */
TEST_F(SDL3ContextTest, GetFramebufferSize_AfterConstruction_ReturnsPositiveDimensions)
{
    // Act
    auto [width, height] = context_->getFramebufferSize();

    // Assert
    EXPECT_GT(width, 0);
    EXPECT_GT(height, 0);
}

/*
 * Test: GetFramebufferSize_AfterConstruction_ReturnsAtLeastRequestedSize
 *
 * On non-HiDPI displays the framebuffer size equals the requested window
 * size.  On HiDPI it may be larger.  Either way it must be >= requested.
 */
TEST_F(SDL3ContextTest, GetFramebufferSize_AfterConstruction_ReturnsAtLeastRequestedSize)
{
    // Arrange
    const int requested_width = 640;
    const int requested_height = 480;

    // Act
    auto [width, height] = context_->getFramebufferSize();

    // Assert
    EXPECT_GE(width, requested_width);
    EXPECT_GE(height, requested_height);
}

// ============================================================================
// shouldClose / setShouldClose tests
// ============================================================================

/*
 * Test: ShouldClose_AfterConstruction_ReturnsFalse
 *
 * The window must not request closure immediately after creation.
 */
TEST_F(SDL3ContextTest, ShouldClose_AfterConstruction_ReturnsFalse)
{
    // Assert
    EXPECT_FALSE(context_->shouldClose());
}

/*
 * Test: SetShouldClose_True_ReturnsTrueFromShouldClose
 */
TEST_F(SDL3ContextTest, SetShouldClose_True_ReturnsTrueFromShouldClose)
{
    // Act
    context_->setShouldClose(true);

    // Assert
    EXPECT_TRUE(context_->shouldClose());
}

/*
 * Test: SetShouldClose_False_AfterTrue_ReturnsFalse
 */
TEST_F(SDL3ContextTest, SetShouldClose_False_AfterTrue_ReturnsFalse)
{
    // Arrange
    context_->setShouldClose(true);

    // Act
    context_->setShouldClose(false);

    // Assert
    EXPECT_FALSE(context_->shouldClose());
}

// ============================================================================
// Time tests
// ============================================================================

/*
 * Test: GetTime_AfterConstruction_ReturnsNonNegativeValue
 *
 * SDL_GetTicks() is non-negative; the returned seconds value must be >= 0.
 */
TEST_F(SDL3ContextTest, GetTime_AfterConstruction_ReturnsNonNegativeValue)
{
    // Act
    double time = context_->getTime();

    // Assert
    EXPECT_GE(time, 0.0);
}

// ============================================================================
// No-throw / smoke tests for interface methods
// ============================================================================

TEST_F(SDL3ContextTest, MakeCurrent_DoesNotThrow)
{
    // Act & Assert
    EXPECT_NO_THROW(context_->makeCurrent());
}

TEST_F(SDL3ContextTest, SwapBuffers_DoesNotThrow)
{
    // Act & Assert
    EXPECT_NO_THROW(context_->swapBuffers());
}

TEST_F(SDL3ContextTest, PollEvents_DoesNotThrow)
{
    // Act & Assert
    EXPECT_NO_THROW(context_->pollEvents());
}

TEST_F(SDL3ContextTest, SetSwapInterval_DoesNotThrow)
{
    // Act & Assert
    EXPECT_NO_THROW(context_->setSwapInterval(1));
}
