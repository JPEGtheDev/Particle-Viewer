/*
 * COMFileProviderTests.cpp
 *
 * Unit tests for COMFileProvider — wraps SettingsIO::checkCOM() and
 * SettingsIO::getCOM() behind the ICOMProvider interface.
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
