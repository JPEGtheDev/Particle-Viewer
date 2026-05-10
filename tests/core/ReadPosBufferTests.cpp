/*
 * ReadPosBufferTests.cpp
 *
 * Unit tests for SettingsIO::readPosBuffer — GL-free disk reader.
 */

#include <cstdio>
#include <vector>

// Include glad first to avoid OpenGL header conflicts
#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "MockOpenGL.hpp"
#include "settingsIO.hpp"
#include "testing/TestFrameBuilder.hpp"

// ──────────────────────────────────────────────────────────────────────────────
// Helpers
// ──────────────────────────────────────────────────────────────────────────────

class ReadPosBufferTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        MockOpenGL::reset();
        MockOpenGL::initGLAD();
    }
};

// ──────────────────────────────────────────────────────────────────────────────
// Single-frame: returns correct positions at frame 0
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_ValidFrame0_ReturnsCorrectPositions)
{
    constexpr long N = 3;
    std::vector<glm::vec4> pos0 = {
        {1.f, 2.f, 3.f, 0.f},
        {4.f, 5.f, 6.f, 1.f},
        {7.f, 8.f, 9.f, 2.f},
    };

    TestFrameBuilder builder(N);
    builder.addFrame(pos0);
    std::string path = builder.writeToTempFile("_read_pos_frame0.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 1;

    auto result = sio.readPosBuffer(0);
    ASSERT_EQ(static_cast<long>(result.size()), N);
    for (long i = 0; i < N; ++i) {
        EXPECT_FLOAT_EQ(result[i].x, pos0[i].x);
        EXPECT_FLOAT_EQ(result[i].y, pos0[i].y);
        EXPECT_FLOAT_EQ(result[i].z, pos0[i].z);
        EXPECT_FLOAT_EQ(result[i].w, pos0[i].w);
    }

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// Multi-frame: frame 0 and frame 1 return independent correct data
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_MultiFrame_IndependentFrames)
{
    constexpr long N = 2;
    std::vector<glm::vec4> pos0 = {{10.f, 20.f, 30.f, 0.f}, {40.f, 50.f, 60.f, 1.f}};
    std::vector<glm::vec4> pos1 = {{-1.f, -2.f, -3.f, 0.f}, {-4.f, -5.f, -6.f, 1.f}};

    TestFrameBuilder builder(N);
    builder.addFrame(pos0);
    builder.addFrame(pos1);
    std::string path = builder.writeToTempFile("_read_pos_multiframe.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 2;

    auto result0 = sio.readPosBuffer(0);
    ASSERT_EQ(static_cast<long>(result0.size()), N);
    for (long i = 0; i < N; ++i) {
        EXPECT_FLOAT_EQ(result0[i].x, pos0[i].x);
        EXPECT_FLOAT_EQ(result0[i].y, pos0[i].y);
        EXPECT_FLOAT_EQ(result0[i].z, pos0[i].z);
        EXPECT_FLOAT_EQ(result0[i].w, pos0[i].w);
    }

    auto result1 = sio.readPosBuffer(1);
    ASSERT_EQ(static_cast<long>(result1.size()), N);
    for (long i = 0; i < N; ++i) {
        EXPECT_FLOAT_EQ(result1[i].x, pos1[i].x);
        EXPECT_FLOAT_EQ(result1[i].y, pos1[i].y);
        EXPECT_FLOAT_EQ(result1[i].z, pos1[i].z);
        EXPECT_FLOAT_EQ(result1[i].w, pos1[i].w);
    }

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// Error handling: invalid path returns empty vector
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_InvalidPath_ReturnsEmptyVector)
{
    SettingsIO sio;
    sio.posName = "/nonexistent/path/read_pos_buffer_invalid.bin";
    sio.N = 3;
    sio.frames = 1;

    auto result = sio.readPosBuffer(0);
    EXPECT_TRUE(result.empty());
}

// ──────────────────────────────────────────────────────────────────────────────
// N=0 returns empty vector without crashing
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_NEqualsZero_ReturnsEmptyVector)
{
    SettingsIO sio;
    sio.posName = "/nonexistent/path/that_cannot_exist.bin";
    sio.N = 0;
    sio.frames = 1;

    auto result = sio.readPosBuffer(0);
    EXPECT_TRUE(result.empty());
}

// ──────────────────────────────────────────────────────────────────────────────
// Frame clamping: readPosBuffer(-1) returns frame 0 data
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_NegativeFrame_ClampedToFrame0)
{
    constexpr long N = 2;
    std::vector<glm::vec4> pos0 = {{1.f, 2.f, 3.f, 4.f}, {5.f, 6.f, 7.f, 8.f}};

    TestFrameBuilder builder(N);
    builder.addFrame(pos0);
    std::string path = builder.writeToTempFile("_read_pos_clamp_neg.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 1;

    auto result = sio.readPosBuffer(-1);
    ASSERT_EQ(static_cast<long>(result.size()), N);
    EXPECT_FLOAT_EQ(result[0].x, pos0[0].x);
    EXPECT_FLOAT_EQ(result[0].y, pos0[0].y);
    EXPECT_FLOAT_EQ(result[0].z, pos0[0].z);
    EXPECT_FLOAT_EQ(result[0].w, pos0[0].w);
    EXPECT_FLOAT_EQ(result[1].x, pos0[1].x);
    EXPECT_FLOAT_EQ(result[1].y, pos0[1].y);
    EXPECT_FLOAT_EQ(result[1].z, pos0[1].z);
    EXPECT_FLOAT_EQ(result[1].w, pos0[1].w);

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// Frame clamping: readPosBuffer(999) on a 1-frame file returns frame 0 data
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_OOBFrame_ClampedToLastFrame)
{
    constexpr long N = 2;
    std::vector<glm::vec4> pos0 = {{5.f, 6.f, 7.f, 0.f}, {8.f, 9.f, 10.f, 1.f}};

    TestFrameBuilder builder(N);
    builder.addFrame(pos0);
    std::string path = builder.writeToTempFile("_read_pos_clamp_over.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 1;

    auto result = sio.readPosBuffer(999);
    ASSERT_EQ(static_cast<long>(result.size()), N);
    EXPECT_FLOAT_EQ(result[0].x, pos0[0].x);
    EXPECT_FLOAT_EQ(result[0].y, pos0[0].y);
    EXPECT_FLOAT_EQ(result[0].z, pos0[0].z);
    EXPECT_FLOAT_EQ(result[0].w, pos0[0].w);
    EXPECT_FLOAT_EQ(result[1].x, pos0[1].x);
    EXPECT_FLOAT_EQ(result[1].y, pos0[1].y);
    EXPECT_FLOAT_EQ(result[1].z, pos0[1].z);
    EXPECT_FLOAT_EQ(result[1].w, pos0[1].w);

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// frames=0 returns empty vector without crashing
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_FramesEqualsZero_ReturnsEmptyVector)
{
    SettingsIO sio;
    sio.posName = "/nonexistent/path/that_cannot_exist.bin";
    sio.N = 3;
    sio.frames = 0;

    auto result = sio.readPosBuffer(0);
    EXPECT_TRUE(result.empty());
}

// ──────────────────────────────────────────────────────────────────────────────
// Does NOT mutate isPlaying on negative out-of-range frame
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_NegativeFrame_DoesNotMutateIsPlaying)
{
    constexpr long N = 1;
    std::vector<glm::vec4> pos0 = {{1.f, 2.f, 3.f, 0.f}};

    TestFrameBuilder builder(N);
    builder.addFrame(pos0);
    std::string path = builder.writeToTempFile("_read_pos_isplaying_neg.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 1;
    sio.isPlaying = false;
    sio.errorCount = 0;

    sio.readPosBuffer(-1);
    EXPECT_FALSE(sio.isPlaying) << "readPosBuffer must not mutate isPlaying";
    EXPECT_EQ(sio.errorCount, 0) << "readPosBuffer must not mutate errorCount";

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// Does NOT mutate isPlaying on positive out-of-bounds frame
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_PositiveOOBFrame_DoesNotMutateIsPlaying)
{
    constexpr long N = 1;
    std::vector<glm::vec4> pos0 = {{1.f, 2.f, 3.f, 0.f}};

    TestFrameBuilder builder(N);
    builder.addFrame(pos0);
    std::string path = builder.writeToTempFile("_read_pos_isplaying_oob.bin");

    SettingsIO sio;
    sio.posName = path;
    sio.N = N;
    sio.frames = 1;
    sio.isPlaying = true;
    sio.errorCount = 0;

    sio.readPosBuffer(999);
    EXPECT_TRUE(sio.isPlaying) << "readPosBuffer must not mutate isPlaying";
    EXPECT_EQ(sio.errorCount, 0) << "readPosBuffer must not mutate errorCount";

    std::remove(path.c_str());
}

// ──────────────────────────────────────────────────────────────────────────────
// Does NOT mutate errorCount on error
// ──────────────────────────────────────────────────────────────────────────────

TEST_F(ReadPosBufferTest, ReadPosBuffer_InvalidPath_DoesNotMutateErrorCount)
{
    SettingsIO sio;
    sio.posName = "/nonexistent/path/read_pos_buffer_errorcount.bin";
    sio.N = 3;
    sio.frames = 1;
    sio.errorCount = 0;

    sio.readPosBuffer(0);
    EXPECT_EQ(sio.errorCount, 0) << "readPosBuffer must not mutate errorCount";
}
