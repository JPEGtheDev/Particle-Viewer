/*
 * PixelComparatorTests.cpp
 *
 * Unit tests for the PixelComparator class following AAA pattern
 * and single-assertion principle.
 */

#include <gtest/gtest.h>

#include "testing/PixelComparator.hpp"

// ============================================
// Helper Functions
// ============================================

static Image createSolidImage(uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    Image img(width, height);
    for (size_t i = 0; i < img.pixels.size(); i += 4) {
        img.pixels[i] = r;
        img.pixels[i + 1] = g;
        img.pixels[i + 2] = b;
        img.pixels[i + 3] = a;
    }
    return img;
}

// ============================================
// Image Struct Tests
// ============================================

TEST(ImageTest, DefaultConstructor_CreatesEmptyImage)
{
    // Act
    Image img;

    // Assert
    EXPECT_TRUE(img.empty());
}

TEST(ImageTest, SizedConstructor_CreatesValidImage)
{
    // Act
    Image img(4, 4);

    // Assert
    EXPECT_TRUE(img.valid());
}

TEST(ImageTest, SizedConstructor_AllocatesCorrectPixelCount)
{
    // Arrange
    size_t expected_size = 10 * 20 * 4;

    // Act
    Image img(10, 20);

    // Assert
    EXPECT_EQ(img.pixels.size(), expected_size);
}

TEST(ImageTest, DataConstructor_StoresProvidedPixels)
{
    // Arrange
    std::vector<uint8_t> data(16, 128);

    // Act
    Image img(2, 2, data);

    // Assert
    EXPECT_EQ(img.pixels[0], 128);
}

TEST(ImageTest, Valid_WithMismatchedSize_ReturnsFalse)
{
    // Arrange
    Image img;
    img.width = 10;
    img.height = 10;
    img.pixels.resize(8); // Too small

    // Act & Assert
    EXPECT_FALSE(img.valid());
}

// ============================================
// Exact Match Tests
// ============================================

TEST(PixelComparatorTest, Compare_IdenticalImages_Matches)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 255, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_TRUE(result.matches);
}

TEST(PixelComparatorTest, Compare_IdenticalImages_SimilarityIsOne)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 255, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_FLOAT_EQ(result.similarity, 1.0f);
}

TEST(PixelComparatorTest, Compare_IdenticalImages_ZeroDiffPixels)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 255, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_EQ(result.diff_pixels, 0u);
}

// ============================================
// Mismatch Tests
// ============================================

TEST(PixelComparatorTest, Compare_DifferentImages_DoesNotMatch)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 0, 255, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_FALSE(result.matches);
}

TEST(PixelComparatorTest, Compare_DifferentImages_AllPixelsDiffer)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 0, 255, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_EQ(result.diff_pixels, 16u);
}

TEST(PixelComparatorTest, Compare_DifferentImages_SimilarityIsZero)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 0, 255, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_FLOAT_EQ(result.similarity, 0.0f);
}

TEST(PixelComparatorTest, Compare_SinglePixelDifference_ReportsCorrectCount)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 100, 100, 100, 255);
    Image img2 = createSolidImage(4, 4, 100, 100, 100, 255);
    img2.pixels[0] = 200; // Change first pixel's red channel

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_EQ(result.diff_pixels, 1u);
}

// ============================================
// Tolerance Tests
// ============================================

TEST(PixelComparatorTest, Compare_SmallDiffWithinTolerance_Matches)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 100, 100, 100, 255);
    Image img2 = createSolidImage(4, 4, 101, 100, 100, 255);

    // Act - tolerance of 1/255 allows ±1 difference
    ComparisonResult result = comparator.compare(img1, img2, 1.0f / 255.0f);

    // Assert
    EXPECT_TRUE(result.matches);
}

TEST(PixelComparatorTest, Compare_DiffExceedsTolerance_DoesNotMatch)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 100, 100, 100, 255);
    Image img2 = createSolidImage(4, 4, 103, 100, 100, 255);

    // Act - tolerance of 1/255 should not cover a difference of 3
    ComparisonResult result = comparator.compare(img1, img2, 1.0f / 255.0f);

    // Assert
    EXPECT_FALSE(result.matches);
}

TEST(PixelComparatorTest, Compare_HighTolerance_MatchesDifferentImages)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 0, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 255, 255, 255, 255);

    // Act - tolerance of 1.0 allows any difference
    ComparisonResult result = comparator.compare(img1, img2, 1.0f);

    // Assert
    EXPECT_TRUE(result.matches);
}

// ============================================
// Edge Cases - Size Mismatch
// ============================================

TEST(PixelComparatorTest, Compare_DifferentWidths_ReturnsError)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 0, 0, 0, 255);
    Image img2 = createSolidImage(8, 4, 0, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_FALSE(result.error.empty());
}

TEST(PixelComparatorTest, Compare_DifferentHeights_ReturnsError)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 0, 0, 0, 255);
    Image img2 = createSolidImage(4, 8, 0, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_FALSE(result.error.empty());
}

// ============================================
// Edge Cases - Empty Images
// ============================================

TEST(PixelComparatorTest, Compare_EmptyBaseline_ReturnsError)
{
    // Arrange
    PixelComparator comparator;
    Image empty;
    Image img = createSolidImage(4, 4, 0, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(empty, img, 0.0f);

    // Assert
    EXPECT_FALSE(result.error.empty());
}

TEST(PixelComparatorTest, Compare_EmptyCurrent_ReturnsError)
{
    // Arrange
    PixelComparator comparator;
    Image img = createSolidImage(4, 4, 0, 0, 0, 255);
    Image empty;

    // Act
    ComparisonResult result = comparator.compare(img, empty, 0.0f);

    // Assert
    EXPECT_FALSE(result.error.empty());
}

TEST(PixelComparatorTest, Compare_BothEmpty_ReturnsError)
{
    // Arrange
    PixelComparator comparator;
    Image empty1;
    Image empty2;

    // Act
    ComparisonResult result = comparator.compare(empty1, empty2, 0.0f);

    // Assert
    EXPECT_FALSE(result.error.empty());
}

// ============================================
// Edge Cases - Invalid Images
// ============================================

TEST(PixelComparatorTest, Compare_InvalidPixelData_ReturnsError)
{
    // Arrange
    PixelComparator comparator;
    Image img1;
    img1.width = 4;
    img1.height = 4;
    img1.pixels.resize(8); // Wrong size
    Image img2 = createSolidImage(4, 4, 0, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_FALSE(result.error.empty());
}

// ============================================
// Bounding Box Tests
// ============================================

TEST(PixelComparatorTest, Compare_SingleDiffPixel_BoundingBoxCoversPixel)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 100, 100, 100, 255);
    Image img2 = createSolidImage(4, 4, 100, 100, 100, 255);
    // Change pixel at (2, 1) -> index = (1*4+2)*4 = 24
    img2.pixels[24] = 200;

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_EQ(result.diff_bounds.min_x, 2u);
    EXPECT_EQ(result.diff_bounds.min_y, 1u);
}

TEST(PixelComparatorTest, Compare_NoDiffs_BoundingBoxIsZero)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 100, 100, 100, 255);
    Image img2 = createSolidImage(4, 4, 100, 100, 100, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_EQ(result.diff_bounds.min_x, 0u);
    EXPECT_EQ(result.diff_bounds.max_x, 0u);
}

// ============================================
// Diff Image Tests
// ============================================

TEST(PixelComparatorTest, Compare_WithDiffGeneration_CreatesDiffImage)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 0, 255, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f, true);

    // Assert
    EXPECT_TRUE(result.diff_image.valid());
}

TEST(PixelComparatorTest, Compare_WithDiffGeneration_DiffPixelsAreRed)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 0, 255, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f, true);

    // Assert - First pixel in diff should be red
    EXPECT_EQ(result.diff_image.pixels[0], 255); // R
    EXPECT_EQ(result.diff_image.pixels[1], 0);   // G
    EXPECT_EQ(result.diff_image.pixels[2], 0);   // B
}

TEST(PixelComparatorTest, Compare_WithoutDiffGeneration_DiffImageIsEmpty)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 255, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 0, 255, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f, false);

    // Assert
    EXPECT_TRUE(result.diff_image.empty());
}

// ============================================
// Comparison Mode Tests
// ============================================

TEST(PixelComparatorTest, SetMode_Exact_SetsExactMode)
{
    // Arrange
    PixelComparator comparator;

    // Act
    comparator.setMode(ComparisonMode::EXACT);

    // Assert
    EXPECT_EQ(comparator.getMode(), ComparisonMode::EXACT);
}

TEST(PixelComparatorTest, SetMode_Tolerant_SetsTolerantMode)
{
    // Arrange
    PixelComparator comparator;

    // Act
    comparator.setMode(ComparisonMode::TOLERANT);

    // Assert
    EXPECT_EQ(comparator.getMode(), ComparisonMode::TOLERANT);
}

TEST(PixelComparatorTest, ExactMode_WithTolerance_StillUsesExactComparison)
{
    // Arrange
    PixelComparator comparator;
    comparator.setMode(ComparisonMode::EXACT);
    Image img1 = createSolidImage(4, 4, 100, 100, 100, 255);
    Image img2 = createSolidImage(4, 4, 101, 100, 100, 255);

    // Act - Even with tolerance, exact mode should fail on ±1 diff
    ComparisonResult result = comparator.compare(img1, img2, 1.0f / 255.0f);

    // Assert
    EXPECT_FALSE(result.matches);
}

// ============================================
// Total Pixels Tests
// ============================================

TEST(PixelComparatorTest, Compare_4x4Image_TotalPixelsIs16)
{
    // Arrange
    PixelComparator comparator;
    Image img1 = createSolidImage(4, 4, 0, 0, 0, 255);
    Image img2 = createSolidImage(4, 4, 0, 0, 0, 255);

    // Act
    ComparisonResult result = comparator.compare(img1, img2, 0.0f);

    // Assert
    EXPECT_EQ(result.total_pixels, 16u);
}

TEST(PixelComparatorTest, DefaultMode_IsTolerant)
{
    // Act
    PixelComparator comparator;

    // Assert
    EXPECT_EQ(comparator.getMode(), ComparisonMode::TOLERANT);
}
