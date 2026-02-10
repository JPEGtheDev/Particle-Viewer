/*
 * MockOpenGLContext.hpp
 *
 * Mock implementation of IOpenGLContext for headless testing.
 * Used with MockOpenGL::initGLAD() which patches GLAD function pointers
 * for low-level GL call mocking.
 *
 * Usage in tests:
 *   MockOpenGL::initGLAD();    // Mock low-level GL calls
 *   MockOpenGLContext context(1280, 720);  // Mock window/context
 */

#ifndef MOCK_OPENGL_CONTEXT_HPP
#define MOCK_OPENGL_CONTEXT_HPP

#include "graphics/IOpenGLContext.hpp"

class MockOpenGLContext : public IOpenGLContext
{
  public:
    MockOpenGLContext(int width, int height)
        : width_(width), height_(height), should_close_(false), swap_count_(0), poll_count_(0), time_(0.0),
          swap_interval_(0)
    {
    }

    // ============================================
    // IOpenGLContext interface
    // ============================================

    void makeCurrent() override
    {
        // No-op for testing
    }

    void swapBuffers() override
    {
        swap_count_++;
    }

    std::pair<int, int> getFramebufferSize() const override
    {
        return std::make_pair(width_, height_);
    }

    bool shouldClose() const override
    {
        return should_close_;
    }

    void setShouldClose(bool value) override
    {
        should_close_ = value;
    }

    void pollEvents() override
    {
        poll_count_++;
    }

    double getTime() const override
    {
        return time_;
    }

    void setSwapInterval(int interval) override
    {
        swap_interval_ = interval;
    }

    void* getNativeWindowHandle() const override
    {
        return nullptr;
    }

    // ============================================
    // Test helpers
    // ============================================

    /*
     * Get the number of times swapBuffers() was called.
     */
    int getSwapCount() const
    {
        return swap_count_;
    }

    /*
     * Get the number of times pollEvents() was called.
     */
    int getPollCount() const
    {
        return poll_count_;
    }

    /*
     * Set the mock time returned by getTime().
     */
    void setTime(double time)
    {
        time_ = time;
    }

    /*
     * Get the swap interval that was set.
     */
    int getSwapInterval() const
    {
        return swap_interval_;
    }

    /*
     * Reset all counters and state.
     */
    void reset()
    {
        should_close_ = false;
        swap_count_ = 0;
        poll_count_ = 0;
        time_ = 0.0;
        swap_interval_ = 0;
    }

  private:
    int width_;
    int height_;
    bool should_close_;
    int swap_count_;
    int poll_count_;
    double time_;
    int swap_interval_;
};

#endif // MOCK_OPENGL_CONTEXT_HPP
