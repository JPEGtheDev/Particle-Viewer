/*
 * SDL3Context.cpp
 *
 * Production implementation of IOpenGLContext using SDL3.
 * Replaces GLFWContext to support runtime X11/Wayland backend selection.
 */

#include "SDL3Context.hpp"

// clang-format off
#include <glad/glad.h>   // NOLINT(llvm-include-order)
#include <SDL3/SDL.h>    // NOLINT(llvm-include-order)
// clang-format on

#include <iostream>

SDL3Context::SDL3Context(int width, int height, const char* title, bool visible)
    : window_(nullptr), gl_context_(nullptr), should_close_(false), width_(width), height_(height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif

    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (!visible) {
        flags |= SDL_WINDOW_HIDDEN;
    }

    window_ = SDL_CreateWindow(title, width, height, flags);
    if (!window_) {
        // MSAA may not be supported (e.g. Mesa/Xvfb software renderer).
        // Retry without multisampling.
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        window_ = SDL_CreateWindow(title, width, height, flags);
    }
    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return;
    }

    SDL_GL_MakeCurrent(window_, gl_context_);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        std::cerr << "Failed to initialize GLAD: unable to load OpenGL function pointers." << std::endl;
        SDL_GL_DestroyContext(gl_context_);
        gl_context_ = nullptr;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
    }
}

SDL3Context::~SDL3Context()
{
    if (gl_context_) {
        SDL_GL_DestroyContext(gl_context_);
        gl_context_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

bool SDL3Context::isValid() const
{
    return window_ != nullptr && gl_context_ != nullptr;
}

void SDL3Context::makeCurrent()
{
    if (window_ && gl_context_) {
        SDL_GL_MakeCurrent(window_, gl_context_);
    }
}

void SDL3Context::swapBuffers()
{
    if (window_) {
        SDL_GL_SwapWindow(window_);
    }
}

std::pair<int, int> SDL3Context::getFramebufferSize() const
{
    if (window_) {
        int framebuffer_width = 0;
        int framebuffer_height = 0;
        SDL_GetWindowSizeInPixels(window_, &framebuffer_width, &framebuffer_height);
        if (framebuffer_width > 0 && framebuffer_height > 0) {
            return std::make_pair(framebuffer_width, framebuffer_height);
        }
    }
    return std::make_pair(width_, height_);
}

bool SDL3Context::shouldClose() const
{
    return should_close_;
}

void SDL3Context::setShouldClose(bool value)
{
    should_close_ = value;
}

void SDL3Context::pollEvents()
{
    // Events are processed by the application's main loop via SDL_PollEvent.
    // This no-op exists for IOpenGLContext interface compatibility.
}

double SDL3Context::getTime() const
{
    return static_cast<double>(SDL_GetTicks()) / 1000.0;
}

void SDL3Context::setSwapInterval(int interval)
{
    SDL_GL_SetSwapInterval(interval);
}

void* SDL3Context::getNativeWindowHandle() const
{
    return static_cast<void*>(window_);
}
