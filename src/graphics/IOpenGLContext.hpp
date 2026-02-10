/*
 * IOpenGLContext.hpp
 *
 * Abstract interface for OpenGL context management.
 * Enables dependency injection: production code uses GLFWContext,
 * tests use MockOpenGLContext for headless testing without a GPU.
 *
 * This interface abstracts window and context lifecycle operations.
 * Low-level GL calls (glDraw*, glUniform*, etc.) are mocked separately
 * via MockOpenGL::initGLAD() which patches GLAD function pointers.
 */

#ifndef PARTICLE_VIEWER_IOPENGL_CONTEXT_H
#define PARTICLE_VIEWER_IOPENGL_CONTEXT_H

#include <utility>

class IOpenGLContext
{
  public:
    virtual ~IOpenGLContext() = default;

    /*
     * Make this context current for OpenGL operations.
     */
    virtual void makeCurrent() = 0;

    /*
     * Swap front and back buffers (present the rendered frame).
     */
    virtual void swapBuffers() = 0;

    /*
     * Get the framebuffer size in pixels.
     * Returns (width, height).
     */
    virtual std::pair<int, int> getFramebufferSize() const = 0;

    /*
     * Check if the window/context should close.
     */
    virtual bool shouldClose() const = 0;

    /*
     * Signal that the window/context should close.
     */
    virtual void setShouldClose(bool value) = 0;

    /*
     * Poll for input events (keyboard, mouse, etc.).
     */
    virtual void pollEvents() = 0;

    /*
     * Get the time in seconds since context initialization.
     */
    virtual double getTime() const = 0;

    /*
     * Set the swap interval (vsync). 0 = off, 1 = on.
     */
    virtual void setSwapInterval(int interval) = 0;

    /*
     * Get the native window handle for platform-specific operations.
     * Returns nullptr if no native window exists (e.g., in mock/headless mode).
     * For GLFW contexts, this returns the GLFWwindow*.
     */
    virtual void* getNativeWindowHandle() const = 0;
};

#endif // PARTICLE_VIEWER_IOPENGL_CONTEXT_H
