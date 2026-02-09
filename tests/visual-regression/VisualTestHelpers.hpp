/*
 * VisualTestHelpers.hpp
 *
 * Helper macros and utilities for visual regression testing.
 * Provides EXPECT_VISUAL_MATCH and ASSERT_VISUAL_MATCH test macros,
 * plus a VisualRegressionTest fixture for reusable test setup/teardown.
 *
 * Uses the Image struct from PixelComparator.hpp as the base image type
 * and integrates PixelComparator for comparison and ImageConverter for
 * format conversion.
 */

#ifndef PARTICLE_VIEWER_VISUAL_TEST_HELPERS_H
#define PARTICLE_VIEWER_VISUAL_TEST_HELPERS_H

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <gtest/gtest.h>
#include <sys/stat.h>

#include "ImageConverter.hpp"
#include "testing/PixelComparator.hpp"

/*
 * Configuration constants for visual regression testing.
 */
namespace VisualTestConfig
{
static const float DEFAULT_TOLERANCE = 0.0f;           // Exact match by default
static const float TOLERANT_THRESHOLD = 2.0f / 255.0f; // ±2 per channel (8-bit)
static const std::string BASELINES_DIR = "baselines";  // Relative to test binary
static const std::string DIFFS_DIR = "diffs";          // Relative to test binary
static const std::string ARTIFACTS_DIR = "artifacts";  // For CI artifact upload
} // namespace VisualTestConfig

/*
 * Write an RGBA Image to a PPM file (RGB only, drops alpha).
 * Used to save baseline and diff images for inspection.
 *
 * @param path Output PPM file path
 * @param image The Image to write
 * @return true on success
 */
inline bool writeImageToPPM(const std::string& path, const Image& image)
{
    if (!image.valid()) {
        return false;
    }

    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file << "P6\n" << image.width << " " << image.height << "\n255\n";

    for (uint32_t i = 0; i < image.width * image.height; ++i) {
        file.put(static_cast<char>(image.pixels[i * 4 + 0])); // R
        file.put(static_cast<char>(image.pixels[i * 4 + 1])); // G
        file.put(static_cast<char>(image.pixels[i * 4 + 2])); // B
    }

    return file.good();
}

/*
 * Write an RGBA Image directly to a PNG file using ImageConverter.
 * Converts RGBA (4 bpp) to RGB (3 bpp) before writing.
 *
 * @param path Output PNG file path
 * @param image The Image to write
 * @return true on success
 */
inline bool writeImageToPNG(const std::string& path, const Image& image)
{
    if (!image.valid()) {
        return false;
    }

    // Convert RGBA to RGB for stb_image_write
    std::vector<uint8_t> rgb(image.width * image.height * 3);
    for (uint32_t i = 0; i < image.width * image.height; ++i) {
        rgb[i * 3 + 0] = image.pixels[i * 4 + 0];
        rgb[i * 3 + 1] = image.pixels[i * 4 + 1];
        rgb[i * 3 + 2] = image.pixels[i * 4 + 2];
    }

    ConversionResult result = ImageConverter::writePNG(path, rgb.data(), image.width, image.height);
    return result.success;
}

/*
 * Create a directory if it does not exist.
 * @param path Directory path
 * @return true if directory exists or was created
 */
inline bool ensureDirectory(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return S_ISDIR(st.st_mode);
    }
    return mkdir(path.c_str(), 0755) == 0;
}

/*
 * Load a PPM file and convert to RGBA Image for comparison.
 * Uses ImageConverter::parsePPM() then converts RGB→RGBA.
 *
 * @param path Path to PPM file
 * @return Image in RGBA format (empty Image on failure)
 */
inline Image loadImageFromPPM(const std::string& path)
{
    PpmData ppm = ImageConverter::parsePPM(path);
    if (!ppm.valid()) {
        return Image();
    }

    Image image(ppm.width, ppm.height);
    for (uint32_t i = 0; i < ppm.width * ppm.height; ++i) {
        image.pixels[i * 4 + 0] = ppm.pixels[i * 3 + 0]; // R
        image.pixels[i * 4 + 1] = ppm.pixels[i * 3 + 1]; // G
        image.pixels[i * 4 + 2] = ppm.pixels[i * 3 + 2]; // B
        image.pixels[i * 4 + 3] = 255;                   // A
    }

    return image;
}

/*
 * Helper to create a solid-color test Image (RGBA).
 */
inline Image createTestImage(uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
{
    Image image(width, height);
    for (uint32_t i = 0; i < width * height; ++i) {
        image.pixels[i * 4 + 0] = r;
        image.pixels[i * 4 + 1] = g;
        image.pixels[i * 4 + 2] = b;
        image.pixels[i * 4 + 3] = a;
    }
    return image;
}

/*
 * Helper to create a gradient test Image (RGBA) for more realistic test data.
 * Creates a horizontal gradient from (r1,g1,b1) to (r2,g2,b2).
 */
inline Image createGradientImage(uint32_t width, uint32_t height, uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2,
                                 uint8_t g2, uint8_t b2)
{
    Image image(width, height);
    for (uint32_t y = 0; y < height; ++y) {
        for (uint32_t x = 0; x < width; ++x) {
            float t = (width > 1) ? static_cast<float>(x) / (width - 1) : 0.0f;
            uint32_t idx = (y * width + x) * 4;
            float val_r = static_cast<float>(r1) + t * (static_cast<float>(r2) - static_cast<float>(r1));
            float val_g = static_cast<float>(g1) + t * (static_cast<float>(g2) - static_cast<float>(g1));
            float val_b = static_cast<float>(b1) + t * (static_cast<float>(b2) - static_cast<float>(b1));
            image.pixels[idx + 0] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, val_r)));
            image.pixels[idx + 1] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, val_g)));
            image.pixels[idx + 2] = static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, val_b)));
            image.pixels[idx + 3] = 255;
        }
    }
    return image;
}

/*
 * VisualRegressionTest fixture for Google Test.
 *
 * Provides reusable setup/teardown for visual regression tests:
 * - Creates output directories (diffs/, artifacts/)
 * - Provides comparison helpers using PixelComparator
 * - Saves diff images on failure for CI artifact upload
 * - Supports configurable tolerance per test
 *
 * Usage:
 *   TEST_F(VisualRegressionTest, RenderScene_MatchesBaseline)
 *   {
 *       Image baseline = createTestImage(64, 64, 255, 0, 0);
 *       Image current = createTestImage(64, 64, 255, 0, 0);
 *       assertVisualMatch(baseline, current, "red_square");
 *   }
 */
class VisualRegressionTest : public ::testing::Test
{
  protected:
    PixelComparator comparator_;
    std::string diffs_dir_;
    std::string artifacts_dir_;

    void SetUp() override
    {
        diffs_dir_ = VisualTestConfig::DIFFS_DIR;
        artifacts_dir_ = VisualTestConfig::ARTIFACTS_DIR;
        ASSERT_TRUE(ensureDirectory(diffs_dir_)) << "Failed to create diffs directory: " << diffs_dir_;
        ASSERT_TRUE(ensureDirectory(artifacts_dir_)) << "Failed to create artifacts directory: " << artifacts_dir_;
    }

    void TearDown() override
    {
        // Cleanup is handled by CI artifact collection, not per-test
    }

    /*
     * Compare two images and assert they match within tolerance.
     * On failure, saves diff image to diffs/ directory and logs details.
     *
     * @param baseline The expected reference image
     * @param current The actual image being tested
     * @param test_name Name for the diff image file
     * @param tolerance Per-channel tolerance (default: exact match)
     */
    void assertVisualMatch(const Image& baseline, const Image& current, const std::string& test_name,
                           float tolerance = VisualTestConfig::DEFAULT_TOLERANCE)
    {
        ComparisonResult result = comparator_.compare(baseline, current, tolerance, true);

        if (!result.error.empty()) {
            FAIL() << "Visual comparison error for '" << test_name << "': " << result.error;
            return;
        }

        if (!result.matches) {
            // Save diff image for inspection
            std::string diff_path = diffs_dir_ + "/" + test_name + "_diff.png";
            if (result.diff_image.valid()) {
                writeImageToPNG(diff_path, result.diff_image);
            }

            // Save current image for comparison
            std::string current_path = artifacts_dir_ + "/" + test_name + "_current.png";
            writeImageToPNG(current_path, current);

            // Save baseline for comparison
            std::string baseline_path = artifacts_dir_ + "/" + test_name + "_baseline.png";
            writeImageToPNG(baseline_path, baseline);

            FAIL() << "Visual regression detected for '" << test_name << "':\n"
                   << "  Similarity: " << (result.similarity * 100.0f) << "%\n"
                   << "  Diff pixels: " << result.diff_pixels << " / " << result.total_pixels << "\n"
                   << "  Diff bounds: [" << result.diff_bounds.min_x << "," << result.diff_bounds.min_y << "] to ["
                   << result.diff_bounds.max_x << "," << result.diff_bounds.max_y << "]\n"
                   << "  Diff image: " << diff_path << "\n"
                   << "  Baseline: " << baseline_path << "\n"
                   << "  Current: " << current_path;
        }
    }

    /*
     * Compare two images with tolerant mode and assert they match.
     * Uses the default tolerant threshold from VisualTestConfig.
     */
    void assertVisualMatchTolerant(const Image& baseline, const Image& current, const std::string& test_name)
    {
        assertVisualMatch(baseline, current, test_name, VisualTestConfig::TOLERANT_THRESHOLD);
    }
};

/*
 * Macro: EXPECT_VISUAL_MATCH
 *
 * Convenience macro for visual comparison within any test.
 * Compares two Image objects and fails with detailed diagnostics if they differ.
 *
 * Usage:
 *   EXPECT_VISUAL_MATCH(baseline_image, current_image, tolerance);
 */
#define EXPECT_VISUAL_MATCH(baseline, current, tolerance)                                                              \
    do {                                                                                                               \
        PixelComparator _comparator;                                                                                   \
        ComparisonResult _result = _comparator.compare(baseline, current, tolerance, false);                           \
        EXPECT_TRUE(_result.matches) << "Visual mismatch: similarity=" << (_result.similarity * 100.0f)                \
                                     << "%, diff_pixels=" << _result.diff_pixels << "/" << _result.total_pixels        \
                                     << (_result.error.empty() ? "" : (", error=" + _result.error));                   \
    } while (0)

/*
 * Macro: ASSERT_VISUAL_MATCH
 *
 * Fatal version of EXPECT_VISUAL_MATCH - stops the test on failure.
 */
#define ASSERT_VISUAL_MATCH(baseline, current, tolerance)                                                              \
    do {                                                                                                               \
        PixelComparator _comparator;                                                                                   \
        ComparisonResult _result = _comparator.compare(baseline, current, tolerance, false);                           \
        ASSERT_TRUE(_result.matches) << "Visual mismatch: similarity=" << (_result.similarity * 100.0f)                \
                                     << "%, diff_pixels=" << _result.diff_pixels << "/" << _result.total_pixels        \
                                     << (_result.error.empty() ? "" : (", error=" + _result.error));                   \
    } while (0)

#endif // PARTICLE_VIEWER_VISUAL_TEST_HELPERS_H
