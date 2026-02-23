/*
 * SDL3Context.hpp
 *
 * Production implementation of IOpenGLContext using SDL3.
 * Manages the SDL3 window, OpenGL context creation, and GLAD initialization.
 *
 * Replaces GLFWContext to support runtime X11/Wayland backend selection,
 * eliminating "GLXFBConfig" errors on Wayland desktops.
 */

#ifndef PARTICLE_VIEWER_SDL3_CONTEXT_H
#define PARTICLE_VIEWER_SDL3_CONTEXT_H

#include <SDL3/SDL.h>

#include "IOpenGLContext.hpp"

class SDL3Context : public IOpenGLContext
{
  public:
    /*
     * Create an SDL3 window and initialize the OpenGL context.
     * Calls SDL_Init(), creates a window, makes the context current,
     * and loads GLAD. Check isValid() after construction.
     *
     * If visible is false, creates a hidden window (for off-screen rendering/testing).
     */
    SDL3Context(int width, int height, const char* title, bool visible = true);

    ~SDL3Context() override;

    // Prevent copying (owns SDL3 and GL resources)
    SDL3Context(const SDL3Context&) = delete;
    SDL3Context& operator=(const SDL3Context&) = delete;

    /*
     * Returns true if the context was successfully initialized.
     */
    bool isValid() const;

    // ============================================
    // IOpenGLContext interface
    // ============================================
    void makeCurrent() override;
    void swapBuffers() override;
    std::pair<int, int> getFramebufferSize() const override;
    bool shouldClose() const override;
    void setShouldClose(bool value) override;
    void pollEvents() override;
    double getTime() const override;
    void setSwapInterval(int interval) override;
    void* getNativeWindowHandle() const override;

  private:
    SDL_Window* window_;
    SDL_GLContext gl_context_;
    bool should_close_;
    int width_;
    int height_;
};

#endif // PARTICLE_VIEWER_SDL3_CONTEXT_H
