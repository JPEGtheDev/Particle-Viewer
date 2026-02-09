/*
 * ImageConverterTests.cpp
 *
 * Unit tests for the ImageConverter class following AAA pattern
 * and single-assertion principle.
 */

#include <cstdio>
#include <fstream>

#include <dirent.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include "testing/ImageConverter.hpp"

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

class ImageConverterTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        test_dir_ = "/tmp/image_converter_tests";
        mkdir(test_dir_.c_str(), 0755);
    }

    void TearDown() override
    {
        removeDirectoryRecursive(test_dir_);
    }

    /*
     * Create a valid P6 PPM file with solid color.
     */
    std::string createPPM(const std::string& name, uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b)
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

    std::string test_dir_;
};

// ============================================
// Constructor Tests
// ============================================

TEST_F(ImageConverterTest, DefaultConstructor_SetsCompressionToSix)
{
    // Arrange & Act
    ImageConverter converter;

    // Assert
    EXPECT_EQ(converter.getCompressionLevel(), 6);
}

TEST_F(ImageConverterTest, Constructor_WithCustomCompression_SetsLevel)
{
    // Arrange & Act
    ImageConverter converter(3);

    // Assert
    EXPECT_EQ(converter.getCompressionLevel(), 3);
}

TEST_F(ImageConverterTest, Constructor_NegativeCompression_ClampsToZero)
{
    // Arrange & Act
    ImageConverter converter(-5);

    // Assert
    EXPECT_EQ(converter.getCompressionLevel(), 0);
}

TEST_F(ImageConverterTest, Constructor_ExcessiveCompression_ClampsToNine)
{
    // Arrange & Act
    ImageConverter converter(99);

    // Assert
    EXPECT_EQ(converter.getCompressionLevel(), 9);
}

// ============================================
// PPM Parsing Tests
// ============================================

TEST_F(ImageConverterTest, ParsePPM_ValidFile_ReturnsValidData)
{
    // Arrange
    std::string path = createPPM("test.ppm", 4, 4, 255, 0, 0);

    // Act
    PpmData data = ImageConverter::parsePPM(path);

    // Assert
    EXPECT_TRUE(data.valid());
}

TEST_F(ImageConverterTest, ParsePPM_ValidFile_CorrectWidth)
{
    // Arrange
    std::string path = createPPM("test.ppm", 10, 20, 0, 0, 0);

    // Act
    PpmData data = ImageConverter::parsePPM(path);

    // Assert
    EXPECT_EQ(data.width, 10u);
}

TEST_F(ImageConverterTest, ParsePPM_ValidFile_CorrectHeight)
{
    // Arrange
    std::string path = createPPM("test.ppm", 10, 20, 0, 0, 0);

    // Act
    PpmData data = ImageConverter::parsePPM(path);

    // Assert
    EXPECT_EQ(data.height, 20u);
}

TEST_F(ImageConverterTest, ParsePPM_ValidFile_CorrectPixelData)
{
    // Arrange
    std::string path = createPPM("test.ppm", 2, 2, 255, 128, 64);

    // Act
    PpmData data = ImageConverter::parsePPM(path);

    // Assert
    EXPECT_EQ(data.pixels[0], 255);
    EXPECT_EQ(data.pixels[1], 128);
    EXPECT_EQ(data.pixels[2], 64);
}

TEST_F(ImageConverterTest, ParsePPM_WithComment_ParsesCorrectly)
{
    // Arrange
    std::string path = createPPMWithComment("comment.ppm", 4, 4);

    // Act
    PpmData data = ImageConverter::parsePPM(path);

    // Assert
    EXPECT_TRUE(data.valid());
}

TEST_F(ImageConverterTest, ParsePPM_MissingFile_ReturnsInvalidData)
{
    // Arrange & Act
    PpmData data = ImageConverter::parsePPM("/tmp/nonexistent_file.ppm");

    // Assert
    EXPECT_FALSE(data.valid());
}

TEST_F(ImageConverterTest, ParsePPM_InvalidMagic_ReturnsInvalidData)
{
    // Arrange
    std::string path = test_dir_ + "/bad.ppm";
    std::ofstream file(path.c_str());
    file << "P5\n4 4\n255\n";
    for (int i = 0; i < 16; ++i)
        file.put(0);
    file.close();

    // Act
    PpmData data = ImageConverter::parsePPM(path);

    // Assert
    EXPECT_FALSE(data.valid());
}

TEST_F(ImageConverterTest, ParsePPM_TruncatedData_ReturnsInvalidData)
{
    // Arrange
    std::string path = test_dir_ + "/truncated.ppm";
    std::ofstream file(path.c_str(), std::ios::binary);
    file << "P6\n4 4\n255\n";
    // Only write 5 bytes instead of 4*4*3=48
    file.write("hello", 5);
    file.close();

    // Act
    PpmData data = ImageConverter::parsePPM(path);

    // Assert
    EXPECT_FALSE(data.valid());
}

// ============================================
// PPM to PNG Conversion Tests
// ============================================

TEST_F(ImageConverterTest, ConvertPPMtoPNG_ValidFile_Succeeds)
{
    // Arrange
    ImageConverter converter;
    std::string ppm_path = createPPM("test.ppm", 4, 4, 255, 0, 0);
    std::string png_path = test_dir_ + "/test.png";

    // Act
    ConversionResult result = converter.convertPPMtoPNG(ppm_path, png_path);

    // Assert
    EXPECT_TRUE(result.success);
}

TEST_F(ImageConverterTest, ConvertPPMtoPNG_ValidFile_CreatesPNGFile)
{
    // Arrange
    ImageConverter converter;
    std::string ppm_path = createPPM("test.ppm", 4, 4, 255, 0, 0);
    std::string png_path = test_dir_ + "/test.png";

    // Act
    converter.convertPPMtoPNG(ppm_path, png_path);

    // Assert - PNG file should exist
    std::ifstream check(png_path.c_str());
    EXPECT_TRUE(check.good());
}

TEST_F(ImageConverterTest, ConvertPPMtoPNG_ValidFile_PNGHasContent)
{
    // Arrange
    ImageConverter converter;
    std::string ppm_path = createPPM("test.ppm", 8, 8, 128, 128, 128);
    std::string png_path = test_dir_ + "/test.png";

    // Act
    converter.convertPPMtoPNG(ppm_path, png_path);

    // Assert - PNG file should have nonzero size
    std::ifstream check(png_path.c_str(), std::ios::binary | std::ios::ate);
    EXPECT_GT(check.tellg(), 0);
}

TEST_F(ImageConverterTest, ConvertPPMtoPNG_MissingPPM_Fails)
{
    // Arrange
    ImageConverter converter;
    std::string png_path = test_dir_ + "/test.png";

    // Act
    ConversionResult result = converter.convertPPMtoPNG("/tmp/nonexistent.ppm", png_path);

    // Assert
    EXPECT_FALSE(result.success);
}

TEST_F(ImageConverterTest, ConvertPPMtoPNG_EmptyPPMPath_Fails)
{
    // Arrange
    ImageConverter converter;

    // Act
    ConversionResult result = converter.convertPPMtoPNG("", test_dir_ + "/test.png");

    // Assert
    EXPECT_FALSE(result.success);
}

TEST_F(ImageConverterTest, ConvertPPMtoPNG_EmptyPNGPath_Fails)
{
    // Arrange
    ImageConverter converter;
    std::string ppm_path = createPPM("test.ppm", 4, 4, 0, 0, 0);

    // Act
    ConversionResult result = converter.convertPPMtoPNG(ppm_path, "");

    // Assert
    EXPECT_FALSE(result.success);
}

TEST_F(ImageConverterTest, ConvertPPMtoPNG_MissingPPM_HasErrorMessage)
{
    // Arrange
    ImageConverter converter;

    // Act
    ConversionResult result = converter.convertPPMtoPNG("/tmp/nonexistent.ppm", test_dir_ + "/test.png");

    // Assert
    EXPECT_FALSE(result.error.empty());
}

// ============================================
// WritePNG Edge Case Tests
// ============================================

TEST_F(ImageConverterTest, WritePNG_NullPixels_Fails)
{
    // Arrange & Act
    ConversionResult result = ImageConverter::writePNG(test_dir_ + "/null.png", NULL, 4, 4);

    // Assert
    EXPECT_FALSE(result.success);
}

TEST_F(ImageConverterTest, WritePNG_ZeroWidth_Fails)
{
    // Arrange
    uint8_t data[12] = {0};

    // Act
    ConversionResult result = ImageConverter::writePNG(test_dir_ + "/zero.png", data, 0, 4);

    // Assert
    EXPECT_FALSE(result.success);
}

TEST_F(ImageConverterTest, WritePNG_ZeroHeight_Fails)
{
    // Arrange
    uint8_t data[12] = {0};

    // Act
    ConversionResult result = ImageConverter::writePNG(test_dir_ + "/zero.png", data, 4, 0);

    // Assert
    EXPECT_FALSE(result.success);
}

// ============================================
// PpmData Struct Tests
// ============================================

TEST(PpmDataTest, DefaultConstructor_IsInvalid)
{
    // Arrange & Act
    PpmData data;

    // Assert
    EXPECT_FALSE(data.valid());
}

TEST(PpmDataTest, Valid_WithCorrectSize_ReturnsTrue)
{
    // Arrange
    PpmData data;
    data.width = 2;
    data.height = 2;
    data.max_val = 255;
    data.pixels.resize(12, 0);

    // Act & Assert
    EXPECT_TRUE(data.valid());
}

TEST(PpmDataTest, Valid_WithWrongSize_ReturnsFalse)
{
    // Arrange
    PpmData data;
    data.width = 2;
    data.height = 2;
    data.max_val = 255;
    data.pixels.resize(5, 0); // Wrong size

    // Act & Assert
    EXPECT_FALSE(data.valid());
}
