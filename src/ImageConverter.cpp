/*
 * ImageConverter.cpp
 *
 * Implementation of image format conversion.
 */

#include "ImageConverter.hpp"

#include <cstdio>
#include <cstdlib>
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

ConversionResult ImageConverter::convert(const std::string& input_path, const std::string& output_path,
                                         ImageFormat from, ImageFormat to) const
{
    if (input_path.empty()) {
        return ConversionResult(false, "Input path is empty");
    }

    if (output_path.empty()) {
        return ConversionResult(false, "Output path is empty");
    }

    // Currently only PPM → PNG is supported
    if (from == ImageFormat::PPM && to == ImageFormat::PNG) {
        PpmData ppm = parsePPM(input_path);
        if (!ppm.valid()) {
            return ConversionResult(false, "Failed to parse PPM file: " + input_path);
        }

        // Save and restore the global compression level to avoid side effects
        int prev_level = stbi_write_png_compression_level;
        stbi_write_png_compression_level = compression_level_;
        ConversionResult result = writePNG(output_path, ppm.pixels.data(), ppm.width, ppm.height);
        stbi_write_png_compression_level = prev_level;

        return result;
    }

    return ConversionResult(false, "Unsupported conversion: format combination not implemented");
}

bool ImageConverter::readToken(std::ifstream& file, std::string& token)
{
    token.clear();
    char ch = '\0';

    // Skip whitespace and comments
    while (file.get(ch)) {
        if (ch == '#') {
            // Skip comment line
            while (file.get(ch) && ch != '\n') {
            }
        } else if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            // Skip whitespace
        } else {
            // Start of a token
            token += ch;
            break;
        }
    }

    if (token.empty()) {
        return false;
    }

    // Read rest of token until whitespace
    while (file.get(ch)) {
        if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
            break;
        }
        token += ch;
    }

    return true;
}

PpmData ImageConverter::parsePPM(const std::string& ppm_path)
{
    PpmData result;

    std::ifstream file(ppm_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return result;
    }

    // Read magic number (skipping any leading comments)
    std::string magic;
    if (!readToken(file, magic) || magic != "P6") {
        return result;
    }

    // Read width, height, max_val - each can have comments before them
    std::string width_str, height_str, maxval_str;
    if (!readToken(file, width_str) || !readToken(file, height_str) || !readToken(file, maxval_str)) {
        return result;
    }

    uint32_t width = static_cast<uint32_t>(std::atoi(width_str.c_str()));
    uint32_t height = static_cast<uint32_t>(std::atoi(height_str.c_str()));
    uint32_t max_val = static_cast<uint32_t>(std::atoi(maxval_str.c_str()));

    if (width == 0 || height == 0 || max_val == 0 || max_val > 255) {
        return result;
    }

    // After readToken consumed the whitespace following max_val,
    // handle possible CRLF: if the whitespace was '\r', check for '\n'
    // readToken already consumed the single whitespace after max_val.
    // But if it was '\r', the '\n' may still be pending.
    int next = file.peek();
    if (next == '\n') {
        char skip;
        file.get(skip);
    }

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
    if (pixels == nullptr) {
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
