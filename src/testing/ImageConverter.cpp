/*
 * ImageConverter.cpp
 *
 * Implementation of PPM to PNG image conversion using stb_image_write.
 */

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "ImageConverter.hpp"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>

#include "stb_image_write.h"

ImageConverter::ImageConverter(int compression_level) : compression_level_(compression_level)
{
    if (compression_level_ < 0)
        compression_level_ = 0;
    if (compression_level_ > 9)
        compression_level_ = 9;
}

int ImageConverter::getCompressionLevel() const
{
    return compression_level_;
}

ConversionResult ImageConverter::convertPPMtoPNG(const std::string& ppm_path, const std::string& png_path) const
{
    if (ppm_path.empty()) {
        return ConversionResult(false, "PPM path is empty");
    }

    if (png_path.empty()) {
        return ConversionResult(false, "PNG path is empty");
    }

    PpmData ppm = parsePPM(ppm_path);
    if (!ppm.valid()) {
        return ConversionResult(false, "Failed to parse PPM file: " + ppm_path);
    }

    // Set stb compression level before writing
    stbi_write_png_compression_level = compression_level_;

    return writePNG(png_path, ppm.pixels.data(), ppm.width, ppm.height);
}

PpmData ImageConverter::parsePPM(const std::string& ppm_path)
{
    PpmData result;

    std::ifstream file(ppm_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return result;
    }

    // Read magic number
    std::string magic;
    file >> magic;
    if (magic != "P6") {
        return result;
    }

    // Skip comments and whitespace
    char ch = '\0';
    file.get(ch);
    while (file.good()) {
        if (ch == '#') {
            // Skip comment line
            while (file.get(ch) && ch != '\n') {
            }
        } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            // Skip whitespace, peek at next char
            file.get(ch);
        } else {
            // Put back non-whitespace char
            file.putback(ch);
            break;
        }
    }

    // Read dimensions
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t max_val = 0;
    file >> width >> height >> max_val;

    if (width == 0 || height == 0 || max_val == 0 || max_val > 255) {
        return result;
    }

    // Skip single whitespace after max_val
    file.get(ch);

    // Read pixel data
    size_t data_size = static_cast<size_t>(width) * height * 3;
    std::vector<uint8_t> pixels(data_size);
    file.read(reinterpret_cast<char*>(pixels.data()), data_size);

    if (static_cast<size_t>(file.gcount()) != data_size) {
        return result;
    }

    result.width = width;
    result.height = height;
    result.max_val = max_val;
    result.pixels = pixels;

    return result;
}

ConversionResult ImageConverter::writePNG(const std::string& png_path, const uint8_t* pixels, uint32_t width,
                                          uint32_t height)
{
    if (pixels == NULL) {
        return ConversionResult(false, "Pixel data is null");
    }

    if (width == 0 || height == 0) {
        return ConversionResult(false, "Invalid image dimensions");
    }

    int stride = width * 3;
    int ret = stbi_write_png(png_path.c_str(), width, height, 3, pixels, stride);

    if (ret == 0) {
        return ConversionResult(false, "Failed to write PNG: " + png_path);
    }

    return ConversionResult(true);
}
