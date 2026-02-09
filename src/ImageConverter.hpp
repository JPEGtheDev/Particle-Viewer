/*
 * ImageConverter.hpp
 *
 * Image format conversion utility.
 * Supports conversion between image formats (PPM, PNG) using stb_image_write.
 * Can be used for visual regression testing and general-purpose image conversion.
 */

#ifndef PARTICLE_VIEWER_IMAGE_CONVERTER_H
#define PARTICLE_VIEWER_IMAGE_CONVERTER_H

#include <cstdint>
#include <string>
#include <vector>

/*
 * Supported image formats for conversion.
 */
enum class ImageFormat
{
    PPM, // PPM P6 binary format
    PNG, // PNG format (via stb_image_write)
    // TGA,  // Future: TGA format
    // BMP,  // Future: BMP format
};

/*
 * Result of an image conversion operation.
 */
struct ConversionResult
{
    bool success;
    std::string error;

    ConversionResult() : success(false)
    {
    }
    ConversionResult(bool s, const std::string& err = "") : success(s), error(err)
    {
    }
};

/*
 * Raw image data parsed from a PPM file.
 */
struct PpmData
{
    uint32_t width;
    uint32_t height;
    uint32_t max_val;
    std::vector<uint8_t> pixels; // RGB, 3 bytes per pixel

    PpmData() : width(0), height(0), max_val(0)
    {
    }

    bool valid() const
    {
        return width > 0 && height > 0 && max_val > 0 && pixels.size() == static_cast<size_t>(width) * height * 3;
    }
};

/*
 * ImageConverter provides format conversion for images.
 * Currently supports PPM (P6 binary) to PNG conversion via stb_image_write.
 *
 * Usage:
 *   ImageConverter converter;
 *   ConversionResult result = converter.convert("capture.ppm", "capture.png",
 *                                               ImageFormat::PPM, ImageFormat::PNG);
 *   if (!result.success) { std::cerr << result.error << std::endl; }
 */
class ImageConverter
{
  public:
    /*
     * Set PNG compression level (0 = no compression, 9 = max).
     * Default: 6 (reasonable balance of size vs speed).
     */
    ImageConverter(int compression_level = 6);

    /*
     * Convert an image file from one format to another.
     *
     * @param input_path Path to the input image file
     * @param output_path Path for the output image file
     * @param from Source image format
     * @param to Target image format
     * @return ConversionResult with success status and error message
     */
    ConversionResult convert(const std::string& input_path, const std::string& output_path, ImageFormat from,
                             ImageFormat to) const;

    /*
     * Parse a PPM P6 binary file into raw pixel data.
     *
     * @param ppm_path Path to the PPM file
     * @return PpmData with parsed image data (empty on failure)
     */
    static PpmData parsePPM(const std::string& ppm_path);

    /*
     * Write RGB pixel data to a PNG file.
     *
     * @param png_path Path for the output PNG file
     * @param pixels RGB pixel data (3 bytes per pixel)
     * @param width Image width
     * @param height Image height
     * @return ConversionResult with success status
     */
    static ConversionResult writePNG(const std::string& png_path, const uint8_t* pixels, uint32_t width,
                                     uint32_t height);

    /*
     * Get the current compression level.
     */
    int getCompressionLevel() const;

  private:
    int compression_level_;

    /*
     * Read a PPM header token, skipping comments and whitespace.
     * Comments start with '#' and extend to end of line.
     */
    static bool readToken(std::ifstream& file, std::string& token);
};

#endif // PARTICLE_VIEWER_IMAGE_CONVERTER_H
