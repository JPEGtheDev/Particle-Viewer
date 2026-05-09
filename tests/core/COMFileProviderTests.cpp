/*
 * COMFileProviderTests.cpp
 *
 * Unit tests for COMFileProvider — wraps SettingsIO::getCOM() behind the
 * ICOMProvider interface. getCOM() returns false for missing files, truncated
 * reads, and frame-index mismatches.
 */

#include <cstdio>

// glad must be included before any header that transitively pulls in
// particle.hpp (which uses inline GL methods).
#include <glad/glad.h>
#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "COMFileProvider.hpp"
#include "constants.hpp"
#include "settingsIO.hpp"

// ─── Test 1: NoCOMFile ───────────────────────────────────────────────────────

TEST(COMFileProviderTest, COMFileProvider_NoCOMFile_ReturnsFalse)
{
    // Arrange — SettingsIO pointing at a path that does not exist on disk.
    SettingsIO sio;
    sio.comName = "/tmp/no_such_com_file_that_exists.bin";

    COMFileProvider provider(sio);

    glm::vec3 out{99.f, 99.f, 99.f};

    // Act
    bool result = provider.getCOM(0, out);

    // Assert — provider reports unavailable; sentinel left unchanged.
    EXPECT_FALSE(result);
    EXPECT_FLOAT_EQ(out.x, 99.f);
    EXPECT_FLOAT_EQ(out.y, 99.f);
    EXPECT_FLOAT_EQ(out.z, 99.f);
}

// ─── Test 2: WithCOMFile ─────────────────────────────────────────────────────

TEST(COMFileProviderTest, COMFileProvider_WithCOMFile_ReturnsTrue)
{
    // Arrange — write a minimal COMFile: one glm::vec4 for frame 0.
    // SettingsIO::getCOM() reads: fseek(frame * sizeof(vec4), SEEK_CUR)
    // then fread(&read_val, sizeof(vec4), 1, ...) and checks (long)read_val.w == frame.
    // For frame 0: w = 0.0f so (long)0.0f == 0 → valid.
    const std::string tmpPath = ::testing::TempDir() + "com_file_provider_test_frame0.bin";
    {
        glm::vec4 record{1.0f, 2.0f, 3.0f, 0.0f}; // x,y,z payload; w = frame index 0
        FILE* f = fopen(tmpPath.c_str(), "wb");
        ASSERT_NE(f, nullptr) << "Could not open temp file for writing";
        fwrite(&record, sizeof(glm::vec4), 1, f);
        fclose(f);
    }

    SettingsIO sio;
    sio.comName = tmpPath;

    COMFileProvider provider(sio);

    glm::vec3 out{0.f, 0.f, 0.f};

    // Act
    bool result = provider.getCOM(0, out);

    // Assert — provider returns true; out = {1,2,3} * kSimToDisplayScale.
    EXPECT_TRUE(result);
    EXPECT_FLOAT_EQ(out.x, 1.0f * kSimToDisplayScale);
    EXPECT_FLOAT_EQ(out.y, 2.0f * kSimToDisplayScale);
    EXPECT_FLOAT_EQ(out.z, 3.0f * kSimToDisplayScale);

    std::remove(tmpPath.c_str());
}

// ─── Test 3: TruncatedFile ───────────────────────────────────────────────────
// When the COMFile exists but contains no data for the requested frame,
// getCOM() must return false and leave `out` unchanged.

TEST(COMFileProviderTest, COMFileProvider_TruncatedFile_ReturnsFalse)
{
    // Arrange — create an empty COMFile (0 bytes).
    const std::string tmpPath = ::testing::TempDir() + "com_file_provider_test_truncated.bin";
    {
        FILE* f = fopen(tmpPath.c_str(), "wb");
        ASSERT_NE(f, nullptr) << "Could not create temp file";
        fclose(f);
    }

    SettingsIO sio;
    sio.comName = tmpPath;
    COMFileProvider provider(sio);

    glm::vec3 out{99.f, 99.f, 99.f};

    // Act
    bool result = provider.getCOM(0, out);

    // Assert — empty file → no valid record → false; out unchanged.
    EXPECT_FALSE(result);
    EXPECT_FLOAT_EQ(out.x, 99.f);
    EXPECT_FLOAT_EQ(out.y, 99.f);
    EXPECT_FLOAT_EQ(out.z, 99.f);

    std::remove(tmpPath.c_str());
}

// ─── Test 4: FrameIndexMismatch ──────────────────────────────────────────────
// When the record exists but its w field does not match the requested frame,
// getCOM() must return false and leave `out` unchanged.

TEST(COMFileProviderTest, COMFileProvider_FrameIndexMismatch_ReturnsFalse)
{
    // Arrange — write two records so fread succeeds at frame 1's offset,
    // but r1.w = 99 ≠ 1, exercising the (long)read_val.w == frame guard.
    const std::string tmpPath = ::testing::TempDir() + "com_file_provider_test_mismatch.bin";
    {
        glm::vec4 r0{0.f, 0.f, 0.f, 0.f};  // padding — fseek skips this for frame 1
        glm::vec4 r1{1.f, 2.f, 3.f, 99.f}; // w=99 ≠ frame=1 → mismatch
        FILE* f = fopen(tmpPath.c_str(), "wb");
        ASSERT_NE(f, nullptr) << "Could not create temp file";
        fwrite(&r0, sizeof(glm::vec4), 1, f);
        fwrite(&r1, sizeof(glm::vec4), 1, f);
        fclose(f);
    }

    SettingsIO sio;
    sio.comName = tmpPath;
    COMFileProvider provider(sio);

    glm::vec3 out{99.f, 99.f, 99.f};

    // Act — frame=1: fseek skips r0, fread reads r1 (items_read=1),
    //       but (long)99.f != 1 → returns false.
    bool result = provider.getCOM(1, out);

    // Assert — frame index mismatch → false; out unchanged.
    EXPECT_FALSE(result);
    EXPECT_FLOAT_EQ(out.x, 99.f);
    EXPECT_FLOAT_EQ(out.y, 99.f);
    EXPECT_FLOAT_EQ(out.z, 99.f);

    std::remove(tmpPath.c_str());
}
