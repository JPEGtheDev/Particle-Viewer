/*
 * PixelComparator.cpp
 *
 * Implementation of pixel-by-pixel image comparison.
 */

#include "PixelComparator.hpp"

#include <algorithm>
#include <cmath>
#include <limits>

PixelComparator::PixelComparator() : mode_(ComparisonMode::TOLERANT)
{
}

ComparisonResult PixelComparator::compare(const Image& baseline, const Image& current, float tolerance,
                                          bool generate_diff) const
{
    ComparisonResult result;

    // Validate inputs
    if (baseline.empty() || current.empty()) {
        result.error = "One or both images are empty";
        return result;
    }

    if (!baseline.valid() || !current.valid()) {
        result.error = "One or both images have invalid pixel data size";
        return result;
    }

    if (baseline.width != current.width || baseline.height != current.height) {
        result.error = "Image dimensions do not match";
        return result;
    }

    // Select comparison mode
    if (tolerance == 0.0f || mode_ == ComparisonMode::EXACT) {
        return compareExact(baseline, current, generate_diff);
    }

    return compareTolerant(baseline, current, tolerance, generate_diff);
}

void PixelComparator::setMode(ComparisonMode mode)
{
    mode_ = mode;
}

ComparisonMode PixelComparator::getMode() const
{
    return mode_;
}

ComparisonResult PixelComparator::compareExact(const Image& baseline, const Image& current, bool generate_diff) const
{
    ComparisonResult result;
    result.total_pixels = baseline.width * baseline.height;

    uint32_t min_x = baseline.width;
    uint32_t min_y = baseline.height;
    uint32_t max_x = 0;
    uint32_t max_y = 0;

    Image diff;
    if (generate_diff) {
        diff = Image(baseline.width, baseline.height);
    }

    for (uint32_t y = 0; y < baseline.height; ++y) {
        for (uint32_t x = 0; x < baseline.width; ++x) {
            size_t idx = (static_cast<size_t>(y) * baseline.width + x) * 4;

            bool pixel_matches =
                (baseline.pixels[idx] == current.pixels[idx] && baseline.pixels[idx + 1] == current.pixels[idx + 1] &&
                 baseline.pixels[idx + 2] == current.pixels[idx + 2] &&
                 baseline.pixels[idx + 3] == current.pixels[idx + 3]);

            if (!pixel_matches) {
                result.diff_pixels++;

                if (x < min_x)
                    min_x = x;
                if (y < min_y)
                    min_y = y;
                if (x > max_x)
                    max_x = x;
                if (y > max_y)
                    max_y = y;

                if (generate_diff) {
                    diff.pixels[idx] = 255;     // R
                    diff.pixels[idx + 1] = 0;   // G
                    diff.pixels[idx + 2] = 0;   // B
                    diff.pixels[idx + 3] = 255; // A
                }
            } else if (generate_diff) {
                // Copy baseline pixel with reduced opacity
                diff.pixels[idx] = baseline.pixels[idx] / 2;
                diff.pixels[idx + 1] = baseline.pixels[idx + 1] / 2;
                diff.pixels[idx + 2] = baseline.pixels[idx + 2] / 2;
                diff.pixels[idx + 3] = 128;
            }
        }
    }

    if (result.diff_pixels > 0) {
        result.diff_bounds.min_x = min_x;
        result.diff_bounds.min_y = min_y;
        result.diff_bounds.max_x = max_x;
        result.diff_bounds.max_y = max_y;
    }

    result.matches = (result.diff_pixels == 0);
    result.similarity =
        result.total_pixels > 0 ? 1.0f - (static_cast<float>(result.diff_pixels) / result.total_pixels) : 0.0f;

    if (generate_diff) {
        result.diff_image = diff;
    }

    return result;
}

ComparisonResult PixelComparator::compareTolerant(const Image& baseline, const Image& current, float tolerance,
                                                  bool generate_diff) const
{
    ComparisonResult result;
    result.total_pixels = baseline.width * baseline.height;

    // Convert tolerance from 0.0-1.0 range to 0-255 range
    int tol = static_cast<int>(tolerance * 255.0f + 0.5f);
    if (tol < 0)
        tol = 0;
    if (tol > 255)
        tol = 255;

    uint32_t min_x = baseline.width;
    uint32_t min_y = baseline.height;
    uint32_t max_x = 0;
    uint32_t max_y = 0;

    Image diff;
    if (generate_diff) {
        diff = Image(baseline.width, baseline.height);
    }

    for (uint32_t y = 0; y < baseline.height; ++y) {
        for (uint32_t x = 0; x < baseline.width; ++x) {
            size_t idx = (static_cast<size_t>(y) * baseline.width + x) * 4;

            int dr = std::abs(static_cast<int>(baseline.pixels[idx]) - static_cast<int>(current.pixels[idx]));
            int dg = std::abs(static_cast<int>(baseline.pixels[idx + 1]) - static_cast<int>(current.pixels[idx + 1]));
            int db = std::abs(static_cast<int>(baseline.pixels[idx + 2]) - static_cast<int>(current.pixels[idx + 2]));
            int da = std::abs(static_cast<int>(baseline.pixels[idx + 3]) - static_cast<int>(current.pixels[idx + 3]));

            bool pixel_matches = (dr <= tol && dg <= tol && db <= tol && da <= tol);

            if (!pixel_matches) {
                result.diff_pixels++;

                if (x < min_x)
                    min_x = x;
                if (y < min_y)
                    min_y = y;
                if (x > max_x)
                    max_x = x;
                if (y > max_y)
                    max_y = y;

                if (generate_diff) {
                    diff.pixels[idx] = 255;     // R
                    diff.pixels[idx + 1] = 0;   // G
                    diff.pixels[idx + 2] = 0;   // B
                    diff.pixels[idx + 3] = 255; // A
                }
            } else if (generate_diff) {
                diff.pixels[idx] = baseline.pixels[idx] / 2;
                diff.pixels[idx + 1] = baseline.pixels[idx + 1] / 2;
                diff.pixels[idx + 2] = baseline.pixels[idx + 2] / 2;
                diff.pixels[idx + 3] = 128;
            }
        }
    }

    if (result.diff_pixels > 0) {
        result.diff_bounds.min_x = min_x;
        result.diff_bounds.min_y = min_y;
        result.diff_bounds.max_x = max_x;
        result.diff_bounds.max_y = max_y;
    }

    result.matches = (result.diff_pixels == 0);
    result.similarity =
        result.total_pixels > 0 ? 1.0f - (static_cast<float>(result.diff_pixels) / result.total_pixels) : 0.0f;

    if (generate_diff) {
        result.diff_image = diff;
    }

    return result;
}
