/*
 * FramebufferCapture.hpp
 *
 * Utility for capturing OpenGL framebuffer contents to Image objects.
 * Provides off-screen rendering setup for testing and screenshot functionality.
 */

#ifndef PARTICLE_VIEWER_FRAMEBUFFER_CAPTURE_H
#define PARTICLE_VIEWER_FRAMEBUFFER_CAPTURE_H

#include <vector>

#include "Image.hpp"
#include "glad/glad.h"

/*
 * FramebufferCapture - Manages OpenGL framebuffer for off-screen rendering.
 *
 * Provides functionality to:
 * - Create and manage FBO with color and depth attachments
 * - Capture framebuffer contents to Image objects
 * - Handle proper cleanup of OpenGL resources
 *
 * Usage:
 *   FramebufferCapture capture(800, 600);
 *   if (!capture.initialize()) { handle error }
 *   capture.bind();
 *   // ... render scene ...
 *   Image screenshot = capture.capture();
 */
class FramebufferCapture
{
  private:
    GLuint fbo_;
    GLuint colorTexture_;
    GLuint depthRenderbuffer_;
    uint32_t width_;
    uint32_t height_;
    bool initialized_;

  public:
    /*
     * Construct framebuffer capture for specified dimensions.
     * Call initialize() to create OpenGL resources.
     */
    FramebufferCapture(uint32_t width, uint32_t height)
        : fbo_(0), colorTexture_(0), depthRenderbuffer_(0), width_(width), height_(height), initialized_(false)
    {
    }

    ~FramebufferCapture()
    {
        cleanup();
    }

    /*
     * Initialize OpenGL framebuffer and attachments.
     * Must be called with valid OpenGL context.
     * @return true if initialization succeeds
     */
    bool initialize()
    {
        // Create framebuffer
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        // Create color attachment texture
        glGenTextures(1, &colorTexture_);
        glBindTexture(GL_TEXTURE_2D, colorTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture_, 0);

        // Create depth attachment renderbuffer
        glGenRenderbuffers(1, &depthRenderbuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width_, height_);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer_);

        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            cleanup();
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        initialized_ = true;
        return true;
    }

    /*
     * Bind the framebuffer for rendering.
     * Call before drawing to render off-screen.
     */
    void bind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, width_, height_);
    }

    /*
     * Unbind the framebuffer (return to default framebuffer).
     */
    void unbind()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    /*
     * Capture the current framebuffer content to an Image.
     * @return Image containing RGBA pixel data, vertically flipped to match image coordinates
     */
    Image capture()
    {
        Image image(width_, height_);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());

        // Normalize alpha channel to 255 (framebuffer alpha values may be inconsistent)
        for (size_t i = 3; i < image.pixels.size(); i += 4) {
            image.pixels[i] = 255;
        }

        // Flip image vertically (OpenGL origin is bottom-left, image origin is top-left)
        flipImageVertically(image);

        return image;
    }

    /*
     * Clean up OpenGL resources.
     */
    void cleanup()
    {
        if (depthRenderbuffer_ != 0) {
            glDeleteRenderbuffers(1, &depthRenderbuffer_);
            depthRenderbuffer_ = 0;
        }
        if (colorTexture_ != 0) {
            glDeleteTextures(1, &colorTexture_);
            colorTexture_ = 0;
        }
        if (fbo_ != 0) {
            glDeleteFramebuffers(1, &fbo_);
            fbo_ = 0;
        }
        initialized_ = false;
    }

    bool isInitialized() const
    {
        return initialized_;
    }

    uint32_t getWidth() const
    {
        return width_;
    }

    uint32_t getHeight() const
    {
        return height_;
    }

  private:
    /*
     * Flip image vertically (OpenGL Y axis is bottom-up, Image Y axis is top-down).
     */
    void flipImageVertically(Image& image)
    {
        uint32_t rowSize = image.width * 4; // 4 bytes per pixel (RGBA)
        std::vector<uint8_t> rowBuffer(rowSize);

        for (uint32_t y = 0; y < image.height / 2; ++y) {
            uint32_t topRow = y * rowSize;
            uint32_t bottomRow = (image.height - 1 - y) * rowSize;

            // Swap rows
            std::copy(image.pixels.begin() + topRow, image.pixels.begin() + topRow + rowSize, rowBuffer.begin());
            std::copy(image.pixels.begin() + bottomRow, image.pixels.begin() + bottomRow + rowSize,
                      image.pixels.begin() + topRow);
            std::copy(rowBuffer.begin(), rowBuffer.end(), image.pixels.begin() + bottomRow);
        }
    }
};

#endif // PARTICLE_VIEWER_FRAMEBUFFER_CAPTURE_H
