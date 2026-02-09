/*
 * Image.hpp
 *
 * Core image class for Particle-Viewer.
 * Represents an RGBA image with read/write support for multiple formats (PPM, PNG).
 * Format-specific implementations are handled internally — the public API uses
 * ImageFormat enums for read/write operations.
 */

#ifndef PARTICLE_VIEWER_IMAGE_H
#define PARTICLE_VIEWER_IMAGE_H

#include <cstdint>
#include <string>
#include <vector>

/*
 * Supported image formats for read/write operations.
 */
enum class ImageFormat
{
    PPM, // PPM P6 binary format (RGB, 3 bytes/pixel)
    PNG, // PNG format via stb_image_write (RGB, 3 bytes/pixel)
};

/*
 * Represents an RGBA image stored as a flat pixel buffer.
 * Pixel data is stored in row-major order, 4 bytes per pixel (R, G, B, A).
 *
 * Supports saving to and loading from PPM and PNG formats.
 * Format conversions (RGBA ↔ RGB) are handled internally:
 *   - Save: drops alpha channel (writes RGB only)
 *   - Load: sets alpha to 255 for all pixels
 *
 * Usage:
 *   Image img(64, 64);
 *   img.save("output.png", ImageFormat::PNG);
 *   Image loaded = Image::load("input.ppm", ImageFormat::PPM);
 */
class Image
{
  public:
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

    /*
     * Save this image to a file in the specified format.
     * Converts RGBA to RGB internally (alpha is dropped).
     *
     * @param path Output file path
     * @param format Target format (PPM or PNG)
     * @return true on success, false on failure
     */
    bool save(const std::string& path, ImageFormat format) const;

    /*
     * Load an image from a file in the specified format.
     * Converts RGB to RGBA internally (alpha set to 255).
     *
     * @param path Input file path
     * @param format Source format (PPM or PNG)
     * @return Image with RGBA data, or empty Image on failure
     */
    static Image load(const std::string& path, ImageFormat format);
};

#endif // PARTICLE_VIEWER_IMAGE_H
