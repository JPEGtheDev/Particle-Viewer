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
    GLuint fbo = 0;

    glGenFramebuffers(1, &fbo);

    EXPECT_NE(fbo, 0u);
}

// glGenFramebuffers increments genFramebuffersCalls counter
TEST_F(MockFBOTest, GenFramebuffersIncrementsCallCounter)
{
    GLuint fbo = 0;

    glGenFramebuffers(1, &fbo);

    EXPECT_EQ(MockOpenGL::genFramebuffersCalls, 1);
}

// glCheckFramebufferStatus returns GL_FRAMEBUFFER_COMPLETE by default
TEST_F(MockFBOTest, CheckFramebufferStatusReturnsCompleteByDefault)
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    EXPECT_EQ(status, static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));
}

// setFramebufferStatus causes glCheckFramebufferStatus to return that status
TEST_F(MockFBOTest, SetFramebufferStatusChangesReturnValue)
{
    MockOpenGL::setFramebufferStatus(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    EXPECT_EQ(status, static_cast<GLenum>(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT));
}

// reset() clears genFramebuffersCalls back to 0
TEST_F(MockFBOTest, ResetClearsGenFramebuffersCallCounter)
{
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);

    MockOpenGL::reset();

    EXPECT_EQ(MockOpenGL::genFramebuffersCalls, 0);
}

// glBindFramebuffer increments bindFramebufferCalls
TEST_F(MockFBOTest, BindFramebufferIncrementsCallCounter)
{
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    EXPECT_EQ(MockOpenGL::bindFramebufferCalls, 1);
}

// glDeleteFramebuffers increments deleteFramebuffersCalls
TEST_F(MockFBOTest, DeleteFramebuffersIncrementsCallCounter)
{
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);

    glDeleteFramebuffers(1, &fbo);

    EXPECT_EQ(MockOpenGL::deleteFramebuffersCalls, 1);
}

// glFramebufferTexture2D increments framebufferTexture2DCalls
TEST_F(MockFBOTest, FramebufferTexture2DIncrementsCallCounter)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 1, 0);

    EXPECT_EQ(MockOpenGL::framebufferTexture2DCalls, 1);
}

// reset() also restores mockFramebufferStatus to GL_FRAMEBUFFER_COMPLETE
TEST_F(MockFBOTest, ResetRestoresDefaultFramebufferStatus)
{
    MockOpenGL::setFramebufferStatus(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT);
    MockOpenGL::reset();
    MockOpenGL::initGLAD();

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    EXPECT_EQ(status, static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));
}
