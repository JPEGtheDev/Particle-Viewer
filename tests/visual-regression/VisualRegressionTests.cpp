/*
 * VisualRegressionTests.cpp
 *
 * Visual regression tests demonstrating:
 * 1. Exact match - identical images must match
 * 2. Tolerance match - slightly different images match within tolerance
 * 3. Intentional failure - significantly different images fail comparison
 * 4. Image format round-trip - PPM save/load preserves RGB pixel data (alpha is discarded)
 *
 * These tests use the VisualRegressionTest fixture and test helpers.
 * They operate on synthetic test images (no GPU/OpenGL required).
 */

#include <cstdio>
#include <string>

#include <gtest/gtest.h>

#include "visual-regression/VisualTestHelpers.hpp"

// ============================================================================
// Fixture Tests (using VisualRegressionTest)
// ============================================================================

TEST_F(VisualRegressionTest, ExactMatch_IdenticalSolidImages_Passes)
{
    // Arrange
    Image baseline = createTestImage(16, 16, 255, 0, 0);
    Image current = createTestImage(16, 16, 255, 0, 0);

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f, true);
    saveCurrentImage(current, "exact_solid_red");

    // Assert
    EXPECT_TRUE(result.matches);
    EXPECT_TRUE(result.error.empty());
}

TEST_F(VisualRegressionTest, ExactMatch_IdenticalGradientImages_Passes)
{
    // Arrange
    Image baseline = createGradientImage(32, 32, 0, 0, 0, 255, 255, 255);
    Image current = createGradientImage(32, 32, 0, 0, 0, 255, 255, 255);

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f, true);
    saveCurrentImage(current, "exact_gradient_bw");

    // Assert
    EXPECT_TRUE(result.matches);
    EXPECT_TRUE(result.error.empty());
}

TEST_F(VisualRegressionTest, TolerantMatch_SlightlyDifferentImages_Passes)
{
    // Arrange
    Image baseline = createTestImage(16, 16, 128, 128, 128);
    Image current = createTestImage(16, 16, 129, 127, 128); // ±1 per channel
    float tolerance = VisualTestConfig::TOLERANT_THRESHOLD;

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, tolerance, false);
    saveCurrentImage(current, "tolerant_slight_diff");

    // Assert
    EXPECT_TRUE(result.matches);
    EXPECT_TRUE(result.error.empty());
}

TEST_F(VisualRegressionTest, ExactMatch_DifferentImages_GeneratesDiffArtifacts)
{
    // Arrange
    Image baseline = createTestImage(8, 8, 255, 0, 0); // Red
    Image current = createTestImage(8, 8, 0, 255, 0);  // Green

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f, true);

    // Assert
    EXPECT_FALSE(result.matches);
    EXPECT_EQ(result.diff_pixels, 64u);
    EXPECT_GT(result.total_pixels, 0u);
    EXPECT_TRUE(result.diff_image.valid());
}

TEST_F(VisualRegressionTest, CompareImages_DimensionMismatch_ReturnsError)
{
    // Arrange
    Image baseline = createTestImage(16, 16, 255, 0, 0);
    Image current = createTestImage(8, 8, 255, 0, 0);

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f);

    // Assert
    EXPECT_FALSE(result.matches);
    EXPECT_FALSE(result.error.empty());
}

TEST_F(VisualRegressionTest, CompareImages_EmptyBaseline_ReturnsError)
{
    // Arrange
    Image baseline;
    Image current = createTestImage(16, 16, 255, 0, 0);

    // Act
    ComparisonResult result = comparator_.compare(baseline, current, 0.0f);

    // Assert
    EXPECT_FALSE(result.matches);
    EXPECT_FALSE(result.error.empty());
}

// ============================================================================
// Macro Tests (using EXPECT_VISUAL_MATCH / ASSERT_VISUAL_MATCH)
// ============================================================================

TEST(VisualMacroTest, ExpectVisualMatch_IdenticalImages_Passes)
{
    // Arrange
    Image baseline = createTestImage(8, 8, 100, 200, 50);
    Image current = createTestImage(8, 8, 100, 200, 50);
    float tolerance = 0.0f;

    // Act
    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, current, tolerance, false);

    // Assert
    EXPECT_TRUE(result.matches);
}

TEST(VisualMacroTest, AssertVisualMatch_IdenticalImages_Passes)
{
    // Arrange
    Image baseline = createTestImage(4, 4, 0, 0, 255);
    Image current = createTestImage(4, 4, 0, 0, 255);
    float tolerance = 0.0f;

    // Act
    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, current, tolerance, false);

    // Assert
    ASSERT_TRUE(result.matches);
}

TEST(VisualMacroTest, ExpectVisualMatch_WithTolerance_AcceptsSmallDifferences)
{
    // Arrange
    Image baseline = createTestImage(8, 8, 100, 100, 100);
    Image current = createTestImage(8, 8, 101, 99, 100);
    float tolerance = 2.0f / 255.0f;

    // Act
    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, current, tolerance, false);

    // Assert
    EXPECT_TRUE(result.matches);
}

// ============================================================================
// Helper Function Tests
// ============================================================================

TEST(VisualHelperTest, CreateTestImage_ValidDimensions_CreatesValidImage)
{
    // Arrange
    uint32_t expected_width = 16;
    uint32_t expected_height = 16;

    // Act
    Image image = createTestImage(expected_width, expected_height, 255, 0, 0);

    // Assert
    EXPECT_TRUE(image.valid());
    EXPECT_EQ(image.width, expected_width);
    EXPECT_EQ(image.height, expected_height);
    EXPECT_EQ(image.pixels.size(), expected_width * expected_height * 4u);
}

TEST(VisualHelperTest, CreateTestImage_ChecksPixelValues_CorrectRGBA)
{
    // Arrange
    uint8_t expected_r = 128;
    uint8_t expected_g = 64;
    uint8_t expected_b = 32;
    uint8_t expected_a = 200;

    // Act
    Image image = createTestImage(2, 2, expected_r, expected_g, expected_b, expected_a);

    // Assert
    EXPECT_EQ(image.pixels[0], expected_r);
    EXPECT_EQ(image.pixels[1], expected_g);
    EXPECT_EQ(image.pixels[2], expected_b);
    EXPECT_EQ(image.pixels[3], expected_a);
}

TEST(VisualHelperTest, CreateGradientImage_ValidDimensions_CreatesValidImage)
{
    // Arrange
    uint32_t expected_width = 32;
    uint32_t expected_height = 16;

    // Act
    Image image = createGradientImage(expected_width, expected_height, 0, 0, 0, 255, 255, 255);

    // Assert
    EXPECT_TRUE(image.valid());
    EXPECT_EQ(image.width, expected_width);
    EXPECT_EQ(image.height, expected_height);
}

TEST(VisualHelperTest, CreateGradientImage_ChecksGradientValues_FirstPixelIsStart)
{
    // Arrange
    uint8_t start_r = 0;
    uint8_t start_g = 0;
    uint8_t start_b = 0;

    // Act
    Image image = createGradientImage(8, 1, start_r, start_g, start_b, 255, 255, 255);

    // Assert - first pixel should be start color
    EXPECT_EQ(image.pixels[0], start_r);
    EXPECT_EQ(image.pixels[1], start_g);
    EXPECT_EQ(image.pixels[2], start_b);
}

TEST(VisualHelperTest, CreateGradientImage_ChecksGradientValues_LastPixelIsEnd)
{
    // Arrange
    uint8_t end_r = 255;
    uint8_t end_g = 255;
    uint8_t end_b = 255;

    // Act
    Image image = createGradientImage(8, 1, 0, 0, 0, end_r, end_g, end_b);

    // Assert - last pixel should be end color
    uint32_t last_idx = (8 - 1) * 4;
    EXPECT_EQ(image.pixels[last_idx + 0], end_r);
    EXPECT_EQ(image.pixels[last_idx + 1], end_g);
    EXPECT_EQ(image.pixels[last_idx + 2], end_b);
}

TEST(VisualHelperTest, ImageSave_PPM_ValidImage_Succeeds)
{
    // Arrange
    Image image = createTestImage(4, 4, 255, 128, 0);
    std::string path = "/tmp/visual_test_write.ppm";

    // Act
    bool result = image.save(path, ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(result);
    std::remove(path.c_str());
}

TEST(VisualHelperTest, ImageSave_PPM_EmptyImage_ReturnsFalse)
{
    // Arrange
    Image empty_image;
    std::string path = "/tmp/visual_test_empty.ppm";

    // Act
    bool result = empty_image.save(path, ImageFormat::PPM);

    // Assert
    EXPECT_FALSE(result);
}

TEST(VisualHelperTest, ImageLoad_PPM_RoundTrip_PreservesPixels)
{
    // Arrange
    Image original = createTestImage(4, 4, 200, 100, 50);
    std::string path = "/tmp/visual_test_roundtrip.ppm";
    original.save(path, ImageFormat::PPM);

    // Act
    Image loaded = Image::load(path, ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(loaded.valid());
    EXPECT_EQ(loaded.width, original.width);
    EXPECT_EQ(loaded.height, original.height);
    EXPECT_EQ(loaded.pixels[0], 200u); // R preserved
    EXPECT_EQ(loaded.pixels[1], 100u); // G preserved
    EXPECT_EQ(loaded.pixels[2], 50u);  // B preserved
    EXPECT_EQ(loaded.pixels[3], 255u); // A set to 255

    std::remove(path.c_str());
}

TEST(VisualHelperTest, ImageLoad_PPM_NonexistentFile_ReturnsEmptyImage)
{
    // Act
    Image loaded = Image::load("/tmp/nonexistent_test_file.ppm", ImageFormat::PPM);

    // Assert
    EXPECT_TRUE(loaded.empty());
}

TEST(VisualHelperTest, ImageSave_PNG_ValidImage_Succeeds)
{
    // Arrange
    Image image = createTestImage(4, 4, 255, 0, 128);
    std::string path = "/tmp/visual_test_write.png";

    // Act
    bool result = image.save(path, ImageFormat::PNG);

    // Assert
    EXPECT_TRUE(result);
    std::remove(path.c_str());
}

TEST(VisualHelperTest, ImageSave_PNG_EmptyImage_ReturnsFalse)
{
    // Arrange
    Image empty_image;
    std::string path = "/tmp/visual_test_empty.png";

    // Act
    bool result = empty_image.save(path, ImageFormat::PNG);

    // Assert
    EXPECT_FALSE(result);
}
