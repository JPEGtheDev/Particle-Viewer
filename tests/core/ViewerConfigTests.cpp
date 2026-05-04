/*
 * ViewerConfigTests.cpp
 *
 * Unit tests for viewer configuration save/load functionality.
 */

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "viewerConfig.hpp"

// Test fixture for ViewerConfig tests
class ViewerConfigTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        test_folder = "/tmp/test_particle_viewer_config";
        // Linux-only: project does not support Windows.
        std::system(("mkdir -p " + test_folder).c_str());
        // Clean up any existing viewer.cfg
        std::remove((test_folder + "/viewer.cfg").c_str());
    }

    void TearDown() override
    {
        // Remove the test directory and its contents
        std::system(("rm -rf " + test_folder).c_str());
    }

    std::string test_folder;
};

// ============================================
// Save Configuration Tests
// ============================================

TEST_F(ViewerConfigTest, ViewerConfig_Save_ReturnsTrue)
{
    // Arrange
    bool auto_com_compute = true;

    // Act
    bool result = saveViewerConfig(test_folder, auto_com_compute);

    // Assert
    EXPECT_TRUE(result);
}

// ============================================
// Load Configuration Tests
// ============================================

TEST_F(ViewerConfigTest, ViewerConfig_Load_TrueValue_RoundsTrip)
{
    // Arrange
    ASSERT_TRUE(saveViewerConfig(test_folder, true));

    // Act
    bool loaded = false;
    bool found = loadViewerConfig(test_folder, loaded);

    // Assert
    EXPECT_TRUE(found);
    EXPECT_TRUE(loaded);
}

TEST_F(ViewerConfigTest, ViewerConfig_Load_FalseValue_RoundsTrip)
{
    // Arrange
    ASSERT_TRUE(saveViewerConfig(test_folder, false));

    // Act
    bool loaded = true; // start with opposite to confirm it's overwritten
    bool found = loadViewerConfig(test_folder, loaded);

    // Assert
    EXPECT_TRUE(found);
    EXPECT_FALSE(loaded);
}

TEST_F(ViewerConfigTest, ViewerConfig_Load_MissingFile_ReturnsFalse)
{
    // Arrange: folder exists but viewer.cfg does not
    bool auto_com_compute = true;

    // Act
    bool result = loadViewerConfig(test_folder, auto_com_compute);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(ViewerConfigTest, ViewerConfig_Load_UnknownKeys_AreIgnored)
{
    // Arrange: manually write a file with unknown keys plus the known key
    std::ofstream file(test_folder + "/viewer.cfg");
    file << "# Particle-Viewer Simulation Configuration\n";
    file << "unknown_key=42\n";
    file << "auto_com_compute=0\n";
    file << "another_unknown=hello\n";
    file.close();

    // Act
    bool auto_com_compute = true; // default opposite
    bool result = loadViewerConfig(test_folder, auto_com_compute);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_FALSE(auto_com_compute);
}

TEST_F(ViewerConfigTest, ViewerConfig_Save_ReadOnly_ReturnsFalse)
{
    // Arrange: directory does not exist — save should fail silently
    std::string nonexistent_folder = "/tmp/nonexistent_dir_xyz";

    // Act
    bool result = saveViewerConfig(nonexistent_folder, true);

    // Assert
    EXPECT_FALSE(result);
}
