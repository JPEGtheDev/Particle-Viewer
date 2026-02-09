/*
 * PixelComparator.hpp
 *
 * Pixel-by-pixel image comparison utility for visual regression testing.
 * Supports RGBA comparison with configurable per-channel tolerance.
 */

#ifndef PARTICLE_VIEWER_TESTING_PIXEL_COMPARATOR_H
#define PARTICLE_VIEWER_TESTING_PIXEL_COMPARATOR_H

#include <cstdint>
#include <string>
#include <vector>

/*
 * Represents an RGBA image stored as a flat pixel buffer.
 * Pixel data is stored in row-major order, 4 bytes per pixel (R, G, B, A).
 */
struct Image
{
    uint32_t width;
    uint32_t height;
    std::vector<uint8_t> pixels; // RGBA, 4 bytes per pixel

    Image() : width(0), height(0)
    {
    }

    Image(uint32_t w, uint32_t h) : width(w), height(h), pixels(w * h * 4, 0)
    {
    }

    Image(uint32_t w, uint32_t h, const std::vector<uint8_t>& data) : width(w), height(h), pixels(data)
    {
    }

    bool empty() const
    {
        return width == 0 || height == 0 || pixels.empty();
    }

    bool valid() const
    {
        return !empty() && pixels.size() == static_cast<size_t>(width) * height * 4;
    }
};

/*
 * Axis-aligned bounding box for the region containing pixel differences.
 */
struct BoundingBox
{
    uint32_t min_x;
    uint32_t min_y;
    uint32_t max_x;
    uint32_t max_y;

    BoundingBox() : min_x(0), min_y(0), max_x(0), max_y(0)
    {
    }
};

/*
 * Result of a pixel comparison operation.
 * Contains match status, similarity metrics, and optional diff image.
 */
struct ComparisonResult
{
    bool matches;
    float similarity;        // 0.0 to 1.0 (1.0 = identical)
    uint32_t diff_pixels;    // Number of pixels that differ
    uint32_t total_pixels;   // Total number of pixels compared
    BoundingBox diff_bounds; // Bounding box of differences
    Image diff_image;        // Optional: red overlay on differing pixels
    std::string error;       // Error message if comparison failed

    ComparisonResult()
        : matches(false), similarity(0.0f), diff_pixels(0), total_pixels(0), diff_bounds(), diff_image(), error()
    {
    }
};

/*
 * Comparison modes for pixel comparison.
 */
enum class ComparisonMode
{
    EXACT,    // Tolerance = 0, early exit on first diff
    TOLERANT, // Per-channel tolerance comparison
    // PERCEPTUAL  // Future extension point
};

/*
 * PixelComparator compares two RGBA images with configurable tolerance.
 * Designed for visual regression testing of OpenGL framebuffer output.
 *
 * Usage:
 *   PixelComparator comparator;
 *   ComparisonResult result = comparator.compare(baseline, current, 1.0f/255.0f);
 *   if (!result.matches) { ... }
 */
class PixelComparator
{
  public:
    PixelComparator();

    /*
     * Compare two images with the specified tolerance.
     *
     * @param baseline The reference image
     * @param current The image to compare against baseline
     * @param tolerance Per-channel tolerance (0.0 = exact, 1.0/255.0 = ±1 for 8-bit)
     * @param generate_diff If true, generates a diff image highlighting differences
     * @return ComparisonResult with match status and metrics
     */
    ComparisonResult compare(const Image& baseline, const Image& current, float tolerance,
                             bool generate_diff = false) const;

    /*
     * Set the comparison mode.
     *
     * @param mode The comparison mode to use
     */
    void setMode(ComparisonMode mode);

    /*
     * Get the current comparison mode.
     */
    ComparisonMode getMode() const;

  private:
    ComparisonMode mode_;

    ComparisonResult compareExact(const Image& baseline, const Image& current, bool generate_diff) const;

    ComparisonResult compareTolerant(const Image& baseline, const Image& current, float tolerance,
                                     bool generate_diff) const;
};

#endif // PARTICLE_VIEWER_TESTING_PIXEL_COMPARATOR_H
