/*
 * WindowConfigTests.cpp
 *
 * Unit tests for window configuration save/load functionality.
 */

// clang-format off
#include <glad/glad.h>  // NOLINT(llvm-include-order) — must precede GL headers
// clang-format on

#include <cstdio>
#include <fstream>
#include <string>

#include <gtest/gtest.h>

#include "viewer_app.hpp"
#include "windowConfig.hpp"

// Test fixture for WindowConfig tests
class WindowConfigTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Use a portable test file path
#ifdef _WIN32
        const char* temp_dir = std::getenv("TEMP");
        if (!temp_dir) {
            temp_dir = std::getenv("TMP");
        }
        if (!temp_dir) {
            temp_dir = ".";
        }
        test_config_path = std::string(temp_dir) + "\\test_particle_viewer_config.cfg";
#else
        test_config_path = "/tmp/test_particle_viewer_config.cfg";
#endif
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

// ============================================
// UI Scale Configuration Tests
// ============================================

TEST_F(WindowConfigTest, SaveAndLoad_UiScale_PreservesValue)
{
    // Arrange
    float save_scale = 1.75f;
    saveWindowConfig(test_config_path, 1920, 1080, false, save_scale);

    // Act
    int width = 0;
    int height = 0;
    bool fullscreen = false;
    float load_scale = 0.0f;
    loadWindowConfig(test_config_path, width, height, fullscreen, &load_scale);

    // Assert
    EXPECT_FLOAT_EQ(load_scale, save_scale);
}

TEST_F(WindowConfigTest, LoadWindowConfig_NoUiScaleKey_LeavesDefaultUnchanged)
{
    // Arrange — file has no ui_scale key
    std::ofstream file(test_config_path);
    file << "width=1920\nheight=1080\nfullscreen=0\n";
    file.close();
    float ui_scale = 1.5f; // caller pre-initialized default

    // Act
    int width = 0;
    int height = 0;
    bool fullscreen = false;
    loadWindowConfig(test_config_path, width, height, fullscreen, &ui_scale);

    // Assert
    EXPECT_FLOAT_EQ(ui_scale, 1.5f); // unchanged
}

TEST_F(WindowConfigTest, LoadWindowConfig_NullUiScalePtr_DoesNotCrash)
{
    // Arrange
    saveWindowConfig(test_config_path, 1920, 1080, false, 2.0f);

    // Act & Assert — passing nullptr for ui_scale should not crash
    int width = 0;
    int height = 0;
    bool fullscreen = false;
    bool result = loadWindowConfig(test_config_path, width, height, fullscreen, nullptr);
    EXPECT_TRUE(result);
}

TEST_F(WindowConfigTest, LoadWindowConfig_UnparseableUiScale_LeavesDefaultUnchanged)
{
    // Arrange — file has unparseable ui_scale
    std::ofstream file(test_config_path);
    file << "width=1920\nheight=1080\nfullscreen=0\nui_scale=notanumber\n";
    file.close();
    float ui_scale = 1.5f; // caller pre-initialized default

    // Act
    int width = 0;
    int height = 0;
    bool fullscreen = false;
    loadWindowConfig(test_config_path, width, height, fullscreen, &ui_scale);

    // Assert
    EXPECT_FLOAT_EQ(ui_scale, 1.5f); // unchanged — bad parse leaves default
}

TEST_F(WindowConfigTest, SaveWindowConfig_DefaultUiScale_KeyAbsentInFile)
{
    // Arrange — save using the 4-arg call (ui_scale defaults to 0.0f sentinel)
    saveWindowConfig(test_config_path, 1920, 1080, false);

    // Act — read the raw file content
    std::ifstream file(test_config_path);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Assert — sentinel 0.0f must NOT produce a ui_scale key in the file
    EXPECT_EQ(contents.find("ui_scale="), std::string::npos);
}

// ============================================
// Last Confirmed Folder Tests
// ============================================

TEST_F(WindowConfigTest, SaveThenLoad_LastConfirmedFolder_RoundTrips)
{
    // Arrange
    const std::string save_folder = "/home/user/simulations/run42";
    saveWindowConfig(test_config_path, 1920, 1080, false, 0.0f, &save_folder);

    // Act
    int width = 0;
    int height = 0;
    bool fullscreen = false;
    std::string load_folder = "default_value";
    loadWindowConfig(test_config_path, width, height, fullscreen, nullptr, &load_folder);

    // Assert
    EXPECT_EQ(load_folder, save_folder);
}

TEST_F(WindowConfigTest, Load_WithoutLastConfirmedFolderKey_LeavesOutParamUnchanged)
{
    // Arrange — file has no last_confirmed_folder key
    std::ofstream file(test_config_path);
    file << "width=1920\nheight=1080\nfullscreen=0\n";
    file.close();
    std::string folder = "/my/default/path"; // caller pre-initialized default

    // Act
    int width = 0;
    int height = 0;
    bool fullscreen = false;
    loadWindowConfig(test_config_path, width, height, fullscreen, nullptr, &folder);

    // Assert — key absent, default must be preserved unchanged
    EXPECT_EQ(folder, "/my/default/path");
}

TEST_F(WindowConfigTest, SaveWindowConfig_NullLastConfirmedFolder_DoesNotWriteKey)
{
    // Arrange — save with nullptr (key must be absent)
    saveWindowConfig(test_config_path, 1920, 1080, false, 0.0f, nullptr);

    // Act — read raw file content
    std::ifstream file(test_config_path);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Assert — key must not appear in the file
    EXPECT_EQ(contents.find("last_confirmed_folder="), std::string::npos);
}

TEST_F(WindowConfigTest, SaveWindowConfig_EmptyLastConfirmedFolder_DoesNotWriteKey)
{
    // Arrange
    const std::string empty_folder;
    saveWindowConfig(test_config_path, 1920, 1080, false, 0.0f, &empty_folder);

    // Act
    std::ifstream file(test_config_path);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Assert
    EXPECT_EQ(contents.find("last_confirmed_folder="), std::string::npos);
}

TEST_F(WindowConfigTest, SaveWindowConfig_FolderWithNewline_NewlineStripped)
{
    // Arrange — a path containing a newline could corrupt the INI format by
    // injecting additional keys into the file as separate lines.
    const std::string folder = "/some/path\nmalicious_key=injected";
    saveWindowConfig(test_config_path, 1920, 1080, false, 0.0f, &folder);

    // Act
    std::ifstream file(test_config_path);
    std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    // Assert — the injected text must not appear as a standalone line/key.
    // After sanitization the newline is stripped, so "malicious_key" stays
    // embedded in the value and never starts a new line.
    EXPECT_EQ(contents.find("\nmalicious_key"), std::string::npos);
    // The folder key itself must still be written (non-empty sanitized path)
    EXPECT_NE(contents.find("last_confirmed_folder="), std::string::npos);
}

// ============================================
// SSM Parameter Persistence Tests (persist-ssm-params)
// ============================================

TEST_F(WindowConfigTest, LoadWindowConfig_SsmThreshold_OutOfRangeHigh_ClampedToOne)
{
    // Arrange: write a file with ssm_threshold > 1.0
    {
        std::ofstream f(test_config_path);
        f << "width=1280\nheight=720\nfullscreen=0\nssm_threshold=5.0\n";
    }
    int w = 0, h = 0;
    bool fs = false;
    float threshold = 0.5f;
    // Act
    loadWindowConfig(test_config_path, w, h, fs, nullptr, nullptr, &threshold);
    // Assert
    EXPECT_FLOAT_EQ(threshold, 1.0f);
}

TEST_F(WindowConfigTest, LoadWindowConfig_SsmThreshold_OutOfRangeLow_ClampedToZero)
{
    // Arrange
    {
        std::ofstream f(test_config_path);
        f << "width=1280\nheight=720\nfullscreen=0\nssm_threshold=-0.5\n";
    }
    int w = 0, h = 0;
    bool fs = false;
    float threshold = 0.5f;
    // Act
    loadWindowConfig(test_config_path, w, h, fs, nullptr, nullptr, &threshold);
    // Assert
    EXPECT_FLOAT_EQ(threshold, 0.0f);
}

TEST_F(WindowConfigTest, LoadWindowConfig_SsmBlobRadius_OutOfRangeLow_ClampedToMinimum)
{
    // Arrange
    {
        std::ofstream f(test_config_path);
        f << "width=1280\nheight=720\nfullscreen=0\nssm_blob_radius=0.0\n";
    }
    int w = 0, h = 0;
    bool fs = false;
    float blob_radius = 2.0f;
    // Act
    loadWindowConfig(test_config_path, w, h, fs, nullptr, nullptr, nullptr, &blob_radius);
    // Assert
    EXPECT_FLOAT_EQ(blob_radius, 0.1f);
}

TEST_F(WindowConfigTest, LoadWindowConfig_SsmBlurAmount_OutOfRangeHigh_ClampedToTwenty)
{
    // Arrange
    {
        std::ofstream f(test_config_path);
        f << "width=1280\nheight=720\nfullscreen=0\nssm_blur_amount=99.0\n";
    }
    int w = 0, h = 0;
    bool fs = false;
    float blur_amount = 3.0f;
    // Act
    loadWindowConfig(test_config_path, w, h, fs, nullptr, nullptr, nullptr, nullptr, &blur_amount);
    // Assert
    EXPECT_FLOAT_EQ(blur_amount, 20.0f);
}

TEST_F(WindowConfigTest, LoadWindowConfig_SsmParams_Absent_DefaultPreserved)
{
    // Arrange: file has no SSM keys
    {
        std::ofstream f(test_config_path);
        f << "width=1280\nheight=720\nfullscreen=0\n";
    }
    int w = 0, h = 0;
    bool fs = false;
    float threshold = 0.5f;
    float blob_radius = 2.0f;
    float blur_amount = 3.0f;
    // Act
    loadWindowConfig(test_config_path, w, h, fs, nullptr, nullptr, &threshold, &blob_radius, &blur_amount);
    // Assert: caller defaults unchanged when keys are absent
    EXPECT_FLOAT_EQ(threshold, 0.5f);
    EXPECT_FLOAT_EQ(blob_radius, 2.0f);
    EXPECT_FLOAT_EQ(blur_amount, 3.0f);
}

TEST_F(WindowConfigTest, SaveAndLoadWindowConfig_SsmParams_RoundTrip)
{
    // Arrange
    float threshold = 0.3f;
    float blob_radius = 5.0f;
    float blur_amount = 7.0f;
    // Act: save
    saveWindowConfig(test_config_path, 1280, 720, false, 0.0f, nullptr, threshold, blob_radius, blur_amount);
    // Act: load
    int w = 0, h = 0;
    bool fs = false;
    float loaded_threshold = 0.5f;
    float loaded_blob_radius = 2.0f;
    float loaded_blur_amount = 3.0f;
    loadWindowConfig(test_config_path, w, h, fs, nullptr, nullptr, &loaded_threshold, &loaded_blob_radius,
                     &loaded_blur_amount);
    // Assert
    EXPECT_FLOAT_EQ(loaded_threshold, threshold);
    EXPECT_FLOAT_EQ(loaded_blob_radius, blob_radius);
    EXPECT_FLOAT_EQ(loaded_blur_amount, blur_amount);
}

TEST_F(WindowConfigTest, WindowConfig_SsmParams_DefaultValues)
{
    // Assert: struct defaults match spec
    WindowConfig config;
    EXPECT_FLOAT_EQ(config.ssm_threshold, 0.5f);
    EXPECT_FLOAT_EQ(config.ssm_blob_radius, 2.0f);
    EXPECT_FLOAT_EQ(config.ssm_blur_amount, 3.0f);
}

TEST_F(WindowConfigTest, SaveWindowConfig_SsmParams_DefaultSentinel_KeysAbsentInFile)
{
    // Arrange & Act: save with default sentinel args (no SSM params)
    saveWindowConfig(test_config_path, 1280, 720, false);
    // Assert: none of the SSM keys appear in the file
    std::ifstream f(test_config_path);
    std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    EXPECT_EQ(content.find("ssm_threshold"), std::string::npos);
    EXPECT_EQ(content.find("ssm_blob_radius"), std::string::npos);
    EXPECT_EQ(content.find("ssm_blur_amount"), std::string::npos);
}

TEST_F(WindowConfigTest, LoadWindowConfig_SsmParams_Unparseable_DefaultPreserved)
{
    // Arrange: file has unparseable SSM values
    {
        std::ofstream f(test_config_path);
        f << "width=1280\nheight=720\nfullscreen=0\n";
        f << "ssm_threshold=not_a_float\n";
        f << "ssm_blob_radius=also_not_a_float\n";
        f << "ssm_blur_amount=still_not_a_float\n";
    }
    int w = 0, h = 0;
    bool fs = false;
    float threshold = 0.5f;
    float blob_radius = 2.0f;
    float blur_amount = 3.0f;
    // Act
    loadWindowConfig(test_config_path, w, h, fs, nullptr, nullptr, &threshold, &blob_radius, &blur_amount);
    // Assert: caller defaults preserved when values are unparseable
    EXPECT_FLOAT_EQ(threshold, 0.5f);
    EXPECT_FLOAT_EQ(blob_radius, 2.0f);
    EXPECT_FLOAT_EQ(blur_amount, 3.0f);
}
