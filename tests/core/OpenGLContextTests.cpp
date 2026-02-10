/*
 * OpenGLContextTests.cpp
 *
 * Tests for the OpenGL context dependency injection pattern.
 * Validates MockOpenGLContext behavior and demonstrates how
 * the DI pattern enables headless testing.
 */

#include <gtest/gtest.h>

#include "MockOpenGLContext.hpp"

// ============================================================================
// MockOpenGLContext Unit Tests
// ============================================================================

TEST(MockOpenGLContextTest, Constructor_WithDimensions_SetsFramebufferSize)
{
    // Arrange
    int expected_width = 1280;
    int expected_height = 720;

    // Act
    MockOpenGLContext context(expected_width, expected_height);

    // Assert
    auto size = context.getFramebufferSize();
    EXPECT_EQ(size.first, expected_width);
    EXPECT_EQ(size.second, expected_height);
}

TEST(MockOpenGLContextTest, ShouldClose_Default_ReturnsFalse)
{
    // Act
    MockOpenGLContext context(800, 600);

    // Assert
    EXPECT_FALSE(context.shouldClose());
}

TEST(MockOpenGLContextTest, SetShouldClose_True_ReturnsTrueFromShouldClose)
{
    // Arrange
    MockOpenGLContext context(800, 600);

    // Act
    context.setShouldClose(true);

    // Assert
    EXPECT_TRUE(context.shouldClose());
}

TEST(MockOpenGLContextTest, SwapBuffers_CalledMultipleTimes_TracksCount)
{
    // Arrange
    MockOpenGLContext context(800, 600);
    int expected_count = 3;

    // Act
    context.swapBuffers();
    context.swapBuffers();
    context.swapBuffers();

    // Assert
    EXPECT_EQ(context.getSwapCount(), expected_count);
}

TEST(MockOpenGLContextTest, PollEvents_CalledMultipleTimes_TracksCount)
{
    // Arrange
    MockOpenGLContext context(800, 600);
    int expected_count = 2;

    // Act
    context.pollEvents();
    context.pollEvents();

    // Assert
    EXPECT_EQ(context.getPollCount(), expected_count);
}

TEST(MockOpenGLContextTest, GetTime_Default_ReturnsZero)
{
    // Act
    MockOpenGLContext context(800, 600);

    // Assert
    EXPECT_DOUBLE_EQ(context.getTime(), 0.0);
}

TEST(MockOpenGLContextTest, SetTime_CustomValue_ReturnsSetValue)
{
    // Arrange
    MockOpenGLContext context(800, 600);
    double expected_time = 1.5;

    // Act
    context.setTime(expected_time);

    // Assert
    EXPECT_DOUBLE_EQ(context.getTime(), expected_time);
}

TEST(MockOpenGLContextTest, SetSwapInterval_Value_StoresIt)
{
    // Arrange
    MockOpenGLContext context(800, 600);
    int expected_interval = 1;

    // Act
    context.setSwapInterval(expected_interval);

    // Assert
    EXPECT_EQ(context.getSwapInterval(), expected_interval);
}

TEST(MockOpenGLContextTest, GetNativeWindowHandle_ReturnsNullptr)
{
    // Act
    MockOpenGLContext context(800, 600);

    // Assert
    EXPECT_EQ(context.getNativeWindowHandle(), nullptr);
}

TEST(MockOpenGLContextTest, Reset_AfterOperations_ClearsAllState)
{
    // Arrange
    MockOpenGLContext context(800, 600);
    context.setShouldClose(true);
    context.swapBuffers();
    context.pollEvents();
    context.setTime(5.0);
    context.setSwapInterval(2);

    // Act
    context.reset();

    // Assert
    EXPECT_FALSE(context.shouldClose());
    EXPECT_EQ(context.getSwapCount(), 0);
    EXPECT_EQ(context.getPollCount(), 0);
    EXPECT_DOUBLE_EQ(context.getTime(), 0.0);
    EXPECT_EQ(context.getSwapInterval(), 0);
}

TEST(MockOpenGLContextTest, MakeCurrent_DoesNotThrow)
{
    // Arrange
    MockOpenGLContext context(800, 600);

    // Act & Assert
    EXPECT_NO_THROW(context.makeCurrent());
}

// ============================================================================
// IOpenGLContext Interface Tests (via MockOpenGLContext)
// ============================================================================

TEST(IOpenGLContextTest, PolymorphicUsage_ThroughBasePointer_Works)
{
    // Arrange
    MockOpenGLContext mock_context(1920, 1080);
    IOpenGLContext* context = &mock_context;

    // Act
    context->pollEvents();
    context->swapBuffers();
    auto size = context->getFramebufferSize();

    // Assert
    EXPECT_EQ(size.first, 1920);
    EXPECT_EQ(size.second, 1080);
    EXPECT_EQ(mock_context.getSwapCount(), 1);
    EXPECT_EQ(mock_context.getPollCount(), 1);
}

TEST(IOpenGLContextTest, ShouldCloseLoop_SimulatesMainLoop_ExitsAfterClose)
{
    // Arrange
    MockOpenGLContext mock_context(800, 600);
    IOpenGLContext* context = &mock_context;
    int frame_count = 0;
    int max_frames = 5;

    // Act — simulate a main loop
    while (!context->shouldClose()) {
        context->pollEvents();
        context->swapBuffers();
        frame_count++;
        if (frame_count >= max_frames) {
            context->setShouldClose(true);
        }
    }

    // Assert
    EXPECT_EQ(frame_count, max_frames);
    EXPECT_EQ(mock_context.getSwapCount(), max_frames);
    EXPECT_EQ(mock_context.getPollCount(), max_frames);
}
