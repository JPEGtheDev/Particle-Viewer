/*
 * MockFBOTests.cpp
 *
 * Unit tests for MockOpenGL FBO (Framebuffer Object) mock functions.
 * Verifies that glGenFramebuffers, glBindFramebuffer, glFramebufferTexture2D,
 * glCheckFramebufferStatus, and glDeleteFramebuffers can be called and tracked
 * without a real GPU or OpenGL context.
 */

#include <gtest/gtest.h>

#include "MockOpenGL.hpp"

class MockFBOTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }
};

// glGenFramebuffers assigns a non-zero ID
TEST_F(MockFBOTest, GenFramebuffersAssignsNonZeroId)
{
    // Arrange
    GLuint fbo = 0;

    // Act
    glGenFramebuffers(1, &fbo);

    // Assert
    EXPECT_NE(fbo, 0u);
}

// glGenFramebuffers increments genFramebuffersCalls counter
TEST_F(MockFBOTest, GenFramebuffersIncrementsCallCounter)
{
    // Arrange
    GLuint fbo = 0;

    // Act
    glGenFramebuffers(1, &fbo);

    // Assert
    EXPECT_EQ(MockOpenGL::genFramebuffersCalls, 1);
}

// glCheckFramebufferStatus returns GL_FRAMEBUFFER_COMPLETE by default
TEST_F(MockFBOTest, CheckFramebufferStatusReturnsCompleteByDefault)
{
    // Act
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    // Assert
    EXPECT_EQ(status, static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));
}

// setFramebufferStatus causes glCheckFramebufferStatus to return that status
TEST_F(MockFBOTest, SetFramebufferStatusChangesReturnValue)
{
    // Arrange
    MockOpenGL::setFramebufferStatus(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);

    // Act
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    // Assert
    EXPECT_EQ(status, static_cast<GLenum>(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT));
}

// reset() clears genFramebuffersCalls back to 0
TEST_F(MockFBOTest, ResetClearsGenFramebuffersCallCounter)
{
    // Arrange
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);

    // Act
    MockOpenGL::reset();

    // Assert
    EXPECT_EQ(MockOpenGL::genFramebuffersCalls, 0);
}

// glBindFramebuffer increments bindFramebufferCalls
TEST_F(MockFBOTest, BindFramebufferIncrementsCallCounter)
{
    // Arrange
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);

    // Act
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Assert
    EXPECT_EQ(MockOpenGL::bindFramebufferCalls, 1);
}

// glDeleteFramebuffers increments deleteFramebuffersCalls
TEST_F(MockFBOTest, DeleteFramebuffersIncrementsCallCounter)
{
    // Arrange
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);

    // Act
    glDeleteFramebuffers(1, &fbo);

    // Assert
    EXPECT_EQ(MockOpenGL::deleteFramebuffersCalls, 1);
}

// glFramebufferTexture2D increments framebufferTexture2DCalls
TEST_F(MockFBOTest, FramebufferTexture2DIncrementsCallCounter)
{
    // Act
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 1, 0);

    // Assert
    EXPECT_EQ(MockOpenGL::framebufferTexture2DCalls, 1);
}

// glCheckFramebufferStatus increments checkFramebufferStatusCalls
TEST_F(MockFBOTest, CheckFramebufferStatusIncrementsCallCounter)
{
    // Act
    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    // Assert
    EXPECT_EQ(MockOpenGL::checkFramebufferStatusCalls, 1);
}

// reset() also restores mockFramebufferStatus to GL_FRAMEBUFFER_COMPLETE
TEST_F(MockFBOTest, ResetRestoresDefaultFramebufferStatus)
{
    // Arrange
    MockOpenGL::setFramebufferStatus(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);

    // Act
    MockOpenGL::reset();

    // Assert
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    EXPECT_EQ(status, static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));
}
