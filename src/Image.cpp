/*
 * Image.cpp
 *
 * Implementation of Image read/write operations.
 * Handles PPM P6 binary parsing and PNG writing via stb_image_write.
 * Format-specific details are encapsulated — the public API uses
 * ImageFormat enums for save/load.
 */

#include "Image.hpp"

#include <cstdlib>
#include <fstream>

#include "stb_image_write.h"

// ============================================================================
// PPM Read/Write (internal helpers)
// ============================================================================

/*
 * Read a PPM header token, skipping comments and whitespace.
 */
static bool readPPMToken(std::ifstream& file, std::string& token)
{
    token.clear();
    char ch = '\0';

    while (file.get(ch)) {
        if (ch == '#') {
            while (file.get(ch) && ch != '\n') {
            }
        } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
        } else {
            token += ch;
            break;
        }
    }

    if (token.empty()) {
        return false;
    }

    while (file.get(ch)) {
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            break;
        }
        token += ch;
    }

    return true;
}

static Image loadPPM(const std::string& path)
{
    std::ifstream file(path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return Image();
    }

    std::string magic;
    if (!readPPMToken(file, magic) || magic != "P6") {
        return Image();
    }

    std::string width_str, height_str, maxval_str;
    if (!readPPMToken(file, width_str) || !readPPMToken(file, height_str) || !readPPMToken(file, maxval_str)) {
        return Image();
    }

    uint32_t w = static_cast<uint32_t>(std::atoi(width_str.c_str()));
    uint32_t h = static_cast<uint32_t>(std::atoi(height_str.c_str()));
    uint32_t max_val = static_cast<uint32_t>(std::atoi(maxval_str.c_str()));

    if (w == 0 || h == 0 || max_val == 0 || max_val > 255) {
        return Image();
    }

    int next = file.peek();
    if (next == '\n') {
        char skip;
        file.get(skip);
    }

    size_t data_size = static_cast<size_t>(w) * h * 3;
    std::vector<uint8_t> rgb(data_size);
    file.read(reinterpret_cast<char*>(rgb.data()), data_size);

    if (static_cast<size_t>(file.gcount()) != data_size) {
        return Image();
    }

    // Convert RGB to RGBA
    Image image(w, h);
    for (uint32_t i = 0; i < w * h; ++i) {
        image.pixels[i * 4 + 0] = rgb[i * 3 + 0];
        image.pixels[i * 4 + 1] = rgb[i * 3 + 1];
        image.pixels[i * 4 + 2] = rgb[i * 3 + 2];
        image.pixels[i * 4 + 3] = 255;
    }

    return image;
}

static bool savePPM(const std::string& path, const Image& image)
{
    std::ofstream file(path, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    file << "P6\n" << image.width << " " << image.height << "\n255\n";

    for (uint32_t i = 0; i < image.width * image.height; ++i) {
        file.put(static_cast<char>(image.pixels[i * 4 + 0]));
        file.put(static_cast<char>(image.pixels[i * 4 + 1]));
        file.put(static_cast<char>(image.pixels[i * 4 + 2]));
    }

    return file.good();
}

// ============================================================================
// PNG Read/Write (internal helpers)
// ============================================================================

static bool savePNG(const std::string& path, const Image& image)
{
    // Convert RGBA to RGB for stb_image_write
    std::vector<uint8_t> rgb(image.width * image.height * 3);
    for (uint32_t i = 0; i < image.width * image.height; ++i) {
        rgb[i * 3 + 0] = image.pixels[i * 4 + 0];
        rgb[i * 3 + 1] = image.pixels[i * 4 + 1];
        rgb[i * 3 + 2] = image.pixels[i * 4 + 2];
    }

    int stride = image.width * 3;
    int ret = stbi_write_png(path.c_str(), image.width, image.height, 3, rgb.data(), stride);
    return ret != 0;
}

// ============================================================================
// Public API
// ============================================================================

bool Image::save(const std::string& path, ImageFormat format) const
{
    if (!valid()) {
        return false;
    }

    switch (format) {
        case ImageFormat::PPM:
            return savePPM(path, *this);
        case ImageFormat::PNG:
            return savePNG(path, *this);
        default:
            return false;
    }
}

Image Image::load(const std::string& path, ImageFormat format)
{
    switch (format) {
        case ImageFormat::PPM:
            return loadPPM(path);
        case ImageFormat::PNG:
            // PNG loading not yet implemented
            return Image();
        default:
            return Image();
    }
}
