/*
 * WindowConfigTests.cpp
 *
 * Unit tests for window configuration save/load functionality.
 */

#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "windowConfig.hpp"

// Test fixture for WindowConfig tests
class WindowConfigTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Use a temporary test file
        test_config_path = "/tmp/test_particle_viewer_config.cfg";
        // Clean up any existing test file
        std::remove(test_config_path.c_str());
    }

    void TearDown() override
    {
        // Clean up test file after each test
        std::remove(test_config_path.c_str());
    }

    std::string test_config_path;
};

// ============================================
// Save Configuration Tests
// ============================================

TEST_F(WindowConfigTest, SaveWindowConfig_ValidPath_ReturnsTrue)
{
    // Arrange
    int width = 1920;
    int height = 1080;
    bool fullscreen = false;

    // Act
    bool result = saveWindowConfig(test_config_path, width, height, fullscreen);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(WindowConfigTest, SaveWindowConfig_CreatesFile_FileExists)
{
    // Arrange
    int width = 1920;
    int height = 1080;
    bool fullscreen = false;

    // Act
    saveWindowConfig(test_config_path, width, height, fullscreen);

    // Assert
    std::ifstream file(test_config_path);
    EXPECT_TRUE(file.good());
}

TEST_F(WindowConfigTest, SaveWindowConfig_WritesWidth_CorrectValue)
{
    // Arrange
    int width = 1920;
    int height = 1080;
    bool fullscreen = false;
    saveWindowConfig(test_config_path, width, height, fullscreen);

    // Act
    int loaded_width = 0;
    int loaded_height = 0;
    bool loaded_fullscreen = false;
    loadWindowConfig(test_config_path, loaded_width, loaded_height, loaded_fullscreen);

    // Assert
    EXPECT_EQ(loaded_width, width);
}

TEST_F(WindowConfigTest, SaveWindowConfig_WritesHeight_CorrectValue)
{
    // Arrange
    int width = 1920;
    int height = 1080;
    bool fullscreen = false;
    saveWindowConfig(test_config_path, width, height, fullscreen);

    // Act
    int loaded_width = 0;
    int loaded_height = 0;
    bool loaded_fullscreen = false;
    loadWindowConfig(test_config_path, loaded_width, loaded_height, loaded_fullscreen);

    // Assert
    EXPECT_EQ(loaded_height, height);
}

TEST_F(WindowConfigTest, SaveWindowConfig_WritesFullscreenTrue_CorrectValue)
{
    // Arrange
    int width = 1920;
    int height = 1080;
    bool fullscreen = true;
    saveWindowConfig(test_config_path, width, height, fullscreen);

    // Act
    int loaded_width = 0;
    int loaded_height = 0;
    bool loaded_fullscreen = false;
    loadWindowConfig(test_config_path, loaded_width, loaded_height, loaded_fullscreen);

    // Assert
    EXPECT_TRUE(loaded_fullscreen);
}

TEST_F(WindowConfigTest, SaveWindowConfig_WritesFullscreenFalse_CorrectValue)
{
    // Arrange
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    saveWindowConfig(test_config_path, width, height, fullscreen);

    // Act
    int loaded_width = 0;
    int loaded_height = 0;
    bool loaded_fullscreen = true; // Start with opposite value
    loadWindowConfig(test_config_path, loaded_width, loaded_height, loaded_fullscreen);

    // Assert
    EXPECT_FALSE(loaded_fullscreen);
}

// ============================================
// Load Configuration Tests
// ============================================

TEST_F(WindowConfigTest, LoadWindowConfig_NonExistentFile_ReturnsFalse)
{
    // Arrange
    int width = 0;
    int height = 0;
    bool fullscreen = false;

    // Act
    bool result = loadWindowConfig("/tmp/nonexistent_config_file.cfg", width, height, fullscreen);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(WindowConfigTest, LoadWindowConfig_EmptyFile_ReturnsFalse)
{
    // Arrange
    std::ofstream file(test_config_path);
    file.close();
    int width = 0;
    int height = 0;
    bool fullscreen = false;

    // Act
    bool result = loadWindowConfig(test_config_path, width, height, fullscreen);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(WindowConfigTest, LoadWindowConfig_MissingWidth_ReturnsFalse)
{
    // Arrange
    std::ofstream file(test_config_path);
    file << "height=1080\n";
    file << "fullscreen=0\n";
    file.close();
    int width = 0;
    int height = 0;
    bool fullscreen = false;

    // Act
    bool result = loadWindowConfig(test_config_path, width, height, fullscreen);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(WindowConfigTest, LoadWindowConfig_MissingHeight_ReturnsFalse)
{
    // Arrange
    std::ofstream file(test_config_path);
    file << "width=1920\n";
    file << "fullscreen=0\n";
    file.close();
    int width = 0;
    int height = 0;
    bool fullscreen = false;

    // Act
    bool result = loadWindowConfig(test_config_path, width, height, fullscreen);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(WindowConfigTest, LoadWindowConfig_MissingFullscreen_ReturnsFalse)
{
    // Arrange
    std::ofstream file(test_config_path);
    file << "width=1920\n";
    file << "height=1080\n";
    file.close();
    int width = 0;
    int height = 0;
    bool fullscreen = false;

    // Act
    bool result = loadWindowConfig(test_config_path, width, height, fullscreen);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(WindowConfigTest, LoadWindowConfig_IgnoresComments_LoadsCorrectly)
{
    // Arrange
    std::ofstream file(test_config_path);
    file << "# This is a comment\n";
    file << "width=1920\n";
    file << "; Another comment\n";
    file << "height=1080\n";
    file << "fullscreen=0\n";
    file.close();
    int width = 0;
    int height = 0;
    bool fullscreen = false;

    // Act
    bool result = loadWindowConfig(test_config_path, width, height, fullscreen);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(width, 1920);
    EXPECT_EQ(height, 1080);
}

TEST_F(WindowConfigTest, LoadWindowConfig_IgnoresWhitespace_LoadsCorrectly)
{
    // Arrange
    std::ofstream file(test_config_path);
    file << "  width  =  1920  \n";
    file << "  height  =  1080  \n";
    file << "  fullscreen  =  1  \n";
    file.close();
    int width = 0;
    int height = 0;
    bool fullscreen = false;

    // Act
    bool result = loadWindowConfig(test_config_path, width, height, fullscreen);

    // Assert
    EXPECT_TRUE(result);
    EXPECT_EQ(width, 1920);
    EXPECT_EQ(height, 1080);
    EXPECT_TRUE(fullscreen);
}

// ============================================
// Round-trip Tests
// ============================================

TEST_F(WindowConfigTest, SaveAndLoad_720p_PreservesValues)
{
    // Arrange
    int save_width = 1280;
    int save_height = 720;
    bool save_fullscreen = false;
    saveWindowConfig(test_config_path, save_width, save_height, save_fullscreen);

    // Act
    int load_width = 0;
    int load_height = 0;
    bool load_fullscreen = true;
    loadWindowConfig(test_config_path, load_width, load_height, load_fullscreen);

    // Assert
    EXPECT_EQ(load_width, save_width);
    EXPECT_EQ(load_height, save_height);
    EXPECT_EQ(load_fullscreen, save_fullscreen);
}

TEST_F(WindowConfigTest, SaveAndLoad_4K_PreservesValues)
{
    // Arrange
    int save_width = 3840;
    int save_height = 2160;
    bool save_fullscreen = true;
    saveWindowConfig(test_config_path, save_width, save_height, save_fullscreen);

    // Act
    int load_width = 0;
    int load_height = 0;
    bool load_fullscreen = false;
    loadWindowConfig(test_config_path, load_width, load_height, load_fullscreen);

    // Assert
    EXPECT_EQ(load_width, save_width);
    EXPECT_EQ(load_height, save_height);
    EXPECT_EQ(load_fullscreen, save_fullscreen);
}
