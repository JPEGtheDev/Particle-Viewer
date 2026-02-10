/*
 * ImageTests.cpp
 *
 * Unit tests for the Image class following AAA pattern
 * and single-assertion principle. Tests save/load for PPM and PNG formats,
 * PPM comment handling, and edge cases.
 */

#include <cstdio>
#include <fstream>

#include <dirent.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Image.hpp"

// ============================================
// Helper: Recursively remove a directory
// ============================================

static void removeDirectoryRecursive(const std::string& path)
{
    DIR* dir = opendir(path.c_str());
    if (dir == nullptr)
        return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name(entry->d_name);
        if (name == "." || name == "..")
            continue;

        std::string full_path = path + "/" + name;
        struct stat st;
        if (stat(full_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            removeDirectoryRecursive(full_path);
        } else {
            unlink(full_path.c_str());
        }
    }
    closedir(dir);
    rmdir(path.c_str());
}

// ============================================
// Test Fixture
// ============================================

class ImageIOTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        test_dir_ = "/tmp/image_tests";
        mkdir(test_dir_.c_str(), 0755);
    }

    void TearDown() override
    {
        removeDirectoryRecursive(test_dir_);
    }

    /*
     * Create a valid P6 PPM file with solid color.
     */
    std::string createPPMFile(const std::string& name, uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b)
    {
        std::string path = test_dir_ + "/" + name;
        std::ofstream file(path.c_str(), std::ios::binary);
        file << "P6\n" << width << " " << height << "\n255\n";
        for (uint32_t i = 0; i < width * height; ++i) {
            file.put(static_cast<char>(r));
            file.put(static_cast<char>(g));
            file.put(static_cast<char>(b));
        }
        file.close();
        return path;
    }

    /*
     * Create a PPM file with a comment in the header.
     */
    std::string createPPMWithComment(const std::string& name, uint32_t width, uint32_t height)
    {
        std::string path = test_dir_ + "/" + name;
        std::ofstream file(path.c_str(), std::ios::binary);
        file << "P6\n# This is a comment\n" << width << " " << height << "\n255\n";
        for (uint32_t i = 0; i < width * height; ++i) {
            file.put(static_cast<char>(128));
            file.put(static_cast<char>(128));
            file.put(static_cast<char>(128));
        }
        file.close();
        return path;
    }

    /*
     * Create a PPM file with comments between all header tokens.
     */
    std::string createPPMWithInterleavedComments(const std::string& name, uint32_t width, uint32_t height)
    {
        std::string path = test_dir_ + "/" + name;
        std::ofstream file(path.c_str(), std::ios::binary);
        file << "P6\n# comment before width\n"
             << width << "\n# comment between width and height\n"
             << height << "\n# comment before max_val\n255\n";
        for (uint32_t i = 0; i < width * height; ++i) {
            file.put(static_cast<char>(64));
            file.put(static_cast<char>(128));
            file.put(static_cast<char>(192));
        }
        file.close();
        return path;
    }

    std::string test_dir_;
};

// ============================================
// Constructor Tests
// ============================================

TEST(ImageStructTest, DefaultConstructor_CreatesEmptyImage)
{
    // Act
    Image image;

    // Assert
    EXPECT_TRUE(image.empty());
}

TEST(ImageStructTest, DefaultConstructor_HasZeroDimensions)
{
    // Act
    Image image;

    // Assert
    EXPECT_EQ(image.width, 0u);
    EXPECT_EQ(image.height, 0u);
}

TEST(ImageStructTest, Constructor_WithDimensions_SetsWidthAndHeight)
{
    // Arrange
    uint32_t expected_width = 16;
    uint32_t expected_height = 32;

    // Act
    Image image(expected_width, expected_height);

    // Assert
    EXPECT_EQ(image.width, expected_width);
    EXPECT_EQ(image.height, expected_height);
}

TEST(ImageStructTest, Constructor_WithDimensions_AllocatesCorrectSize)
{
    // Arrange
    uint32_t width = 4;
    uint32_t height = 4;
    size_t expected_size = width * height * 4;

    // Act
    Image image(width, height);

    // Assert
    EXPECT_EQ(image.pixels.size(), expected_size);
}

TEST(ImageStructTest, Constructor_WithDimensions_IsValid)
{
    // Act
    Image image(8, 8);

    // Assert
    EXPECT_TRUE(image.valid());
}

TEST(ImageStructTest, Valid_WithWrongPixelSize_ReturnsFalse)
{
    // Arrange
    std::vector<uint8_t> wrong_size(5, 0);

    // Act
    Image image(2, 2, wrong_size);

    // Assert
    EXPECT_FALSE(image.valid());
}

// ============================================
// PPM Load Tests
// ============================================

TEST_F(ImageIOTest, Load_PPM_ValidFile_ReturnsValidImage)
{
    // Arrange
    std::string path = createPPMFile("test.ppm", 4, 4, 255, 0, 0);

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(image.valid());
}

TEST_F(ImageIOTest, Load_PPM_ValidFile_CorrectWidth)
{
    // Arrange
    uint32_t expected_width = 10;
    std::string path = createPPMFile("test.ppm", expected_width, 20, 0, 0, 0);

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_EQ(image.width, expected_width);
}

TEST_F(ImageIOTest, Load_PPM_ValidFile_CorrectHeight)
{
    // Arrange
    uint32_t expected_height = 20;
    std::string path = createPPMFile("test.ppm", 10, expected_height, 0, 0, 0);

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_EQ(image.height, expected_height);
}

TEST_F(ImageIOTest, Load_PPM_ValidFile_CorrectPixelData)
{
    // Arrange
    std::string path = createPPMFile("test.ppm", 2, 2, 255, 128, 64);

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_EQ(image.pixels[0], 255u);
    EXPECT_EQ(image.pixels[1], 128u);
    EXPECT_EQ(image.pixels[2], 64u);
    EXPECT_EQ(image.pixels[3], 255u); // Alpha set to 255
}

TEST_F(ImageIOTest, Load_PPM_WithComment_LoadsCorrectly)
{
    // Arrange
    std::string path = createPPMWithComment("comment.ppm", 4, 4);

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(image.valid());
}

TEST_F(ImageIOTest, Load_PPM_WithInterleavedComments_LoadsCorrectly)
{
    // Arrange
    std::string path = createPPMWithInterleavedComments("interleaved.ppm", 4, 4);

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(image.valid());
}

TEST_F(ImageIOTest, Load_PPM_WithInterleavedComments_CorrectDimensions)
{
    // Arrange
    uint32_t expected_width = 8;
    uint32_t expected_height = 6;
    std::string path = createPPMWithInterleavedComments("interleaved.ppm", expected_width, expected_height);

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_EQ(image.width, expected_width);
    EXPECT_EQ(image.height, expected_height);
}

TEST_F(ImageIOTest, Load_PPM_MissingFile_ReturnsEmptyImage)
{
    // Act
    Image image = Image::load("/tmp/nonexistent_file.ppm", ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(image.empty());
}

TEST_F(ImageIOTest, Load_PPM_InvalidMagic_ReturnsEmptyImage)
{
    // Arrange
    std::string path = test_dir_ + "/bad.ppm";
    std::ofstream file(path.c_str());
    file << "P5\n4 4\n255\n";
    for (int i = 0; i < 16; ++i)
        file.put(0);
    file.close();

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(image.empty());
}

TEST_F(ImageIOTest, Load_PPM_TruncatedData_ReturnsEmptyImage)
{
    // Arrange
    std::string path = test_dir_ + "/truncated.ppm";
    std::ofstream file(path.c_str(), std::ios::binary);
    file << "P6\n4 4\n255\n";
    file.write("hello", 5); // Only 5 bytes instead of 4*4*3=48
    file.close();

    // Act
    Image image = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(image.empty());
}

// ============================================
// PPM Save Tests
// ============================================

TEST_F(ImageIOTest, Save_PPM_ValidImage_Succeeds)
{
    // Arrange
    Image image(4, 4);
    std::string path = test_dir_ + "/save.ppm";

    // Act
    bool result = image.save(path, ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(ImageIOTest, Save_PPM_EmptyImage_ReturnsFalse)
{
    // Arrange
    Image empty_image;
    std::string path = test_dir_ + "/empty.ppm";

    // Act
    bool result = empty_image.save(path, ImageFormat::PPM);

    // Assert
    EXPECT_FALSE(result);
}

TEST_F(ImageIOTest, Save_PPM_RoundTrip_PreservesPixelData)
{
    // Arrange
    Image original(2, 2);
    original.pixels[0] = 255;
    original.pixels[1] = 128;
    original.pixels[2] = 64;
    original.pixels[3] = 255;
    std::string path = test_dir_ + "/roundtrip.ppm";
    original.save(path, ImageFormat::PPM);

    // Act
    Image loaded = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_EQ(loaded.pixels[0], 255u);
    EXPECT_EQ(loaded.pixels[1], 128u);
    EXPECT_EQ(loaded.pixels[2], 64u);
    EXPECT_EQ(loaded.pixels[3], 255u); // Alpha restored to 255
}

// ============================================
// PNG Save Tests
// ============================================

TEST_F(ImageIOTest, Save_PNG_ValidImage_Succeeds)
{
    // Arrange
    Image image(4, 4);
    std::string path = test_dir_ + "/save.png";

    // Act
    bool result = image.save(path, ImageFormat::PNG);

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(ImageIOTest, Save_PNG_ValidImage_CreatesPNGFile)
{
    // Arrange
    Image image(4, 4);
    std::string path = test_dir_ + "/check.png";
    image.save(path, ImageFormat::PNG);

    // Act
    std::ifstream check(path.c_str());

    // Assert
    EXPECT_TRUE(check.good());
}

TEST_F(ImageIOTest, Save_PNG_ValidImage_PNGHasContent)
{
    // Arrange
    Image image(8, 8);
    std::string path = test_dir_ + "/content.png";
    image.save(path, ImageFormat::PNG);

    // Act
    std::ifstream check(path.c_str(), std::ios::binary | std::ios::ate);

    // Assert
    EXPECT_GT(check.tellg(), 0);
}

TEST_F(ImageIOTest, Save_PNG_EmptyImage_ReturnsFalse)
{
    // Arrange
    Image empty_image;
    std::string path = test_dir_ + "/empty.png";

    // Act
    bool result = empty_image.save(path, ImageFormat::PNG);

    // Assert
    EXPECT_FALSE(result);
}

// ============================================
// PNG Load Tests
// ============================================

TEST_F(ImageIOTest, Load_PNG_ValidFile_ReturnsValidImage)
{
    // Arrange
    Image original(8, 8);
    for (uint32_t i = 0; i < 8 * 8 * 4; i += 4) {
        original.pixels[i] = 128;
        original.pixels[i + 1] = 64;
        original.pixels[i + 2] = 32;
        original.pixels[i + 3] = 255;
    }
    std::string path = test_dir_ + "/test_load.png";
    original.save(path, ImageFormat::PNG);

    // Act
    Image loaded = Image::load(path, ImageFormat::PNG);

    // Assert
    EXPECT_TRUE(loaded.valid());
}

TEST_F(ImageIOTest, Load_PNG_ValidFile_CorrectDimensions)
{
    // Arrange
    uint32_t expected_width = 16;
    uint32_t expected_height = 12;
    Image original(expected_width, expected_height);
    std::string path = test_dir_ + "/test_dimensions.png";
    original.save(path, ImageFormat::PNG);

    // Act
    Image loaded = Image::load(path, ImageFormat::PNG);

    // Assert
    EXPECT_EQ(loaded.width, expected_width);
    EXPECT_EQ(loaded.height, expected_height);
}

TEST_F(ImageIOTest, Load_PNG_ValidFile_CorrectPixelData)
{
    // Arrange
    Image original(4, 4);
    for (uint32_t i = 0; i < 4 * 4 * 4; i += 4) {
        original.pixels[i] = 200;
        original.pixels[i + 1] = 100;
        original.pixels[i + 2] = 50;
        original.pixels[i + 3] = 255;
    }
    std::string path = test_dir_ + "/test_pixels.png";
    original.save(path, ImageFormat::PNG);

    // Act
    Image loaded = Image::load(path, ImageFormat::PNG);

    // Assert - PNG only stores RGB, alpha is set to 255
    EXPECT_EQ(loaded.pixels[0], 200u);  // R preserved
    EXPECT_EQ(loaded.pixels[1], 100u);  // G preserved
    EXPECT_EQ(loaded.pixels[2], 50u);   // B preserved
    EXPECT_EQ(loaded.pixels[3], 255u);  // A set to 255
}

TEST_F(ImageIOTest, Load_PNG_RoundTrip_PreservesRGBData)
{
    // Arrange
    Image original(8, 8);
    for (uint32_t i = 0; i < 8 * 8 * 4; i += 4) {
        original.pixels[i] = 150;
        original.pixels[i + 1] = 200;
        original.pixels[i + 2] = 250;
        original.pixels[i + 3] = 255;
    }
    std::string path = test_dir_ + "/test_roundtrip.png";
    original.save(path, ImageFormat::PNG);

    // Act
    Image loaded = Image::load(path, ImageFormat::PNG);

    // Assert - RGB should match exactly after round-trip
    EXPECT_EQ(loaded.width, original.width);
    EXPECT_EQ(loaded.height, original.height);
    for (uint32_t i = 0; i < loaded.width * loaded.height; ++i) {
        EXPECT_EQ(loaded.pixels[i * 4 + 0], original.pixels[i * 4 + 0]) << "R mismatch at pixel " << i;
        EXPECT_EQ(loaded.pixels[i * 4 + 1], original.pixels[i * 4 + 1]) << "G mismatch at pixel " << i;
        EXPECT_EQ(loaded.pixels[i * 4 + 2], original.pixels[i * 4 + 2]) << "B mismatch at pixel " << i;
    }
}

TEST_F(ImageIOTest, Load_PNG_NonexistentFile_ReturnsEmptyImage)
{
    // Act
    Image loaded = Image::load(test_dir_ + "/nonexistent.png", ImageFormat::PNG);

    // Assert
    EXPECT_TRUE(loaded.empty());
}
