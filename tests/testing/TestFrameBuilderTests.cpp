#include <cstring>
#include <fstream>
#include <vector>

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "testing/TestFrameBuilder.hpp"

TEST(TestFrameBuilderTests, TestFrameBuilder_AddFrame_BufferSizeMatchesFormat)
{
    const long n = 4;
    TestFrameBuilder builder(n);
    std::vector<glm::vec4> positions(n, glm::vec4(1.f, 2.f, 3.f, 0.f));
    builder.addFrame(positions);
    const std::size_t expected = static_cast<std::size_t>(n) * 2 * sizeof(glm::vec4);
    ASSERT_EQ(builder.buffer().size(), expected);
}

TEST(TestFrameBuilderTests, TestFrameBuilder_AddFrame_PositionsStoredAtCorrectOffset)
{
    const long n = 3;
    TestFrameBuilder builder(n);
    glm::vec4 pos0(1.1f, 2.2f, 3.3f, 0.f);
    std::vector<glm::vec4> positions = {pos0, glm::vec4(0.f), glm::vec4(0.f)};
    builder.addFrame(positions);

    glm::vec4 readBack;
    std::memcpy(&readBack, builder.buffer().data(), sizeof(glm::vec4));
    ASSERT_FLOAT_EQ(readBack.x, pos0.x);
    ASSERT_FLOAT_EQ(readBack.y, pos0.y);
    ASSERT_FLOAT_EQ(readBack.z, pos0.z);
}

TEST(TestFrameBuilderTests, TestFrameBuilder_AddFrame_VelocitiesAreZero)
{
    const long n = 2;
    TestFrameBuilder builder(n);
    std::vector<glm::vec4> positions(n, glm::vec4(5.f, 6.f, 7.f, 0.f));
    builder.addFrame(positions);

    // Velocity[0] starts at offset n * sizeof(glm::vec4)
    const std::size_t velOffset = static_cast<std::size_t>(n) * sizeof(glm::vec4);
    glm::vec4 vel0;
    std::memcpy(&vel0, builder.buffer().data() + velOffset, sizeof(glm::vec4));
    ASSERT_FLOAT_EQ(vel0.x, 0.f);
    ASSERT_FLOAT_EQ(vel0.y, 0.f);
    ASSERT_FLOAT_EQ(vel0.z, 0.f);
    ASSERT_FLOAT_EQ(vel0.w, 0.f);
}

TEST(TestFrameBuilderTests, TestFrameBuilder_AddFrame_TypeEncodedInW)
{
    const long n = 1;
    TestFrameBuilder builder(n);
    glm::vec4 pos(1.f, 0.f, 0.f, 1.f); // .w = 1 => Si body1
    builder.addFrame({pos});

    glm::vec4 readBack;
    std::memcpy(&readBack, builder.buffer().data(), sizeof(glm::vec4));
    ASSERT_FLOAT_EQ(readBack.w, 1.f);
}

TEST(TestFrameBuilderTests, TestFrameBuilder_WriteToTempFile_FileReadableWithSameContent)
{
    const long n = 1;
    TestFrameBuilder builder(n);
    glm::vec4 pos(3.f, 4.f, 5.f, 0.f);
    builder.addFrame({pos});

    std::string path = builder.writeToTempFile(".bin");
    std::ifstream f(path, std::ios::binary);
    ASSERT_TRUE(f.is_open());

    glm::vec4 readBack;
    f.read(reinterpret_cast<char*>(&readBack), sizeof(glm::vec4));
    ASSERT_FLOAT_EQ(readBack.x, pos.x);
    ASSERT_FLOAT_EQ(readBack.y, pos.y);
    ASSERT_FLOAT_EQ(readBack.z, pos.z);

    // Cleanup
    ASSERT_EQ(std::remove(path.c_str()), 0) << "Failed to remove temp file: " << path;
}

TEST(TestFrameBuilderTests, TestFrameBuilder_MultipleFrames_FrameCountCorrect)
{
    const long n = 3;
    TestFrameBuilder builder(n);
    std::vector<glm::vec4> positions(n, glm::vec4(0.f));
    builder.addFrame(positions);
    builder.addFrame(positions);
    builder.addFrame(positions);

    ASSERT_EQ(builder.frameCount(), 3L);
    const std::size_t expected = 3u * static_cast<std::size_t>(n) * 2 * sizeof(glm::vec4);
    ASSERT_EQ(builder.buffer().size(), expected);
}

TEST(TestFrameBuilderTests, TestFrameBuilder_AddFrame_WrongSize_Asserts)
{
    TestFrameBuilder builder(3);
    ASSERT_DEATH(builder.addFrame({glm::vec4(0.f)}), "");
}
