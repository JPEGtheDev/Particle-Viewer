/*
 * FramebufferCapture.hpp
 *
 * Utility for capturing OpenGL framebuffer contents to Image objects.
 * Provides off-screen rendering setup for testing and screenshot functionality.
 */

#ifndef PARTICLE_VIEWER_FRAMEBUFFER_CAPTURE_H
#define PARTICLE_VIEWER_FRAMEBUFFER_CAPTURE_H

#include <algorithm>
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
    GLuint fbo_ = 0;
    GLuint colorTexture_ = 0;
    GLuint depthRenderbuffer_ = 0;
    uint32_t width_;
    uint32_t height_;
    bool initialized_ = false;

  public:
    /*
     * Construct framebuffer capture for specified dimensions.
     * Call initialize() to create OpenGL resources.
     */
    FramebufferCapture(uint32_t width, uint32_t height)
        : width_(width), height_(height)
    {
    }

    ~FramebufferCapture()
    {
        cleanup();
    }

    // Delete copy constructor and copy assignment
    FramebufferCapture(const FramebufferCapture&) = delete;
    FramebufferCapture& operator=(const FramebufferCapture&) = delete;

    // Delete move constructor and move assignment
    FramebufferCapture(FramebufferCapture&&) = delete;
    FramebufferCapture& operator=(FramebufferCapture&&) = delete;

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture_, 0);

        // Create depth attachment renderbuffer
        glGenRenderbuffers(1, &depthRenderbuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
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
    void bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_));
    }

    /*
     * Unbind the framebuffer (return to default framebuffer).
     */
    static void unbind()
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
        glReadPixels(0, 0, static_cast<GLsizei>(width_), static_cast<GLsizei>(height_), GL_RGBA, GL_UNSIGNED_BYTE, image.pixels.data());

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
    static void flipImageVertically(Image& image)
    {
        uint32_t row_size = image.width * 4; // 4 bytes per pixel (RGBA)
        std::vector<uint8_t> row_buffer(row_size);

        for (uint32_t y = 0; y < image.height / 2; ++y) {
            uint32_t top_row = y * row_size;
            uint32_t bottom_row = (image.height - 1 - y) * row_size;

            // Swap rows
            std::copy(image.pixels.begin() + top_row, image.pixels.begin() + top_row + row_size, row_buffer.begin());
            std::copy(image.pixels.begin() + bottom_row, image.pixels.begin() + bottom_row + row_size,
                      image.pixels.begin() + top_row);
            std::copy(row_buffer.begin(), row_buffer.end(), image.pixels.begin() + bottom_row);
        }
    }
};

#endif // PARTICLE_VIEWER_FRAMEBUFFER_CAPTURE_H
