/*
 * SDL3Context.cpp
 *
 * Production implementation of IOpenGLContext using SDL3.
 * Replaces GLFWContext to support runtime X11/Wayland backend selection.
 *
 * === Troubleshooting window creation failures ===
 *
 * "SDL_CreateWindow failed: Invalid window driver data" on NVIDIA:
 *   Root cause: SDL3 selects the Wayland backend on Wayland sessions, but
 *   NVIDIA's EGL/Wayland implementation (EGL Streams) may not be available
 *   inside a Flatpak sandbox or with certain driver versions.  SDL3 does not
 *   automatically fall back to X11 when Wayland window creation fails.
 *   Fix applied here: if the auto-detected driver fails, SDL3Context retries
 *   with "x11" forced (XWayland on Wayland desktops), which NVIDIA supports
 *   via GLX.
 *
 * If the app still fails after both attempts:
 *   - Ensure NVIDIA drivers >= 560 are installed (Wayland DRM support).
 *   - Run with SDL_VIDEODRIVER=x11 to force X11/XWayland mode manually.
 *   - Run with SDL_VIDEODRIVER=wayland to force the Wayland path explicitly.
 *   - On Flatpak, the --device=dri finish-arg is required (already set).
 */

#include "SDL3Context.hpp"

// clang-format off
#include <glad/glad.h>   // NOLINT(llvm-include-order)
#include <SDL3/SDL.h>    // NOLINT(llvm-include-order)
// clang-format on

#include <iostream>

/*
 * Apply the required OpenGL context attributes before window creation.
 * Must be called after each SDL_Init() attempt.
 */
static void setGLAttributes()
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
}

/*
 * Try to create an SDL3 window after calling SDL_Init with an optional video
 * driver override.  The function:
 *   1. Optionally sets SDL_HINT_VIDEO_DRIVER (must precede SDL_Init).
 *   2. Calls SDL_Init(SDL_INIT_VIDEO).
 *   3. Sets OpenGL context attributes.
 *   4. Attempts window creation with MSAA=4, then MSAA=0 as fallback.
 *
 * Returns the created window on success.  On failure, calls SDL_Quit() to
 * balance the SDL_Init() and returns nullptr so the caller can retry with a
 * different driver.
 *
 * driver: nullptr means auto-detect (SDL3 default); "x11" forces X11/XWayland.
 */
static SDL_Window* tryInitWithDriver(const char* title, int width, int height, SDL_WindowFlags flags,
                                     const char* driver)
{
    if (driver) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, driver);
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return nullptr; // Init failed — no SDL_Quit needed
    }

    setGLAttributes();

    // Attempt 1: MSAA 4x (preferred for anti-aliasing on desktop GPUs)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_Window* window = SDL_CreateWindow(title, width, height, flags);

    // Attempt 2: No MSAA — Mesa/llvmpipe and some NVIDIA configurations do
    // not expose a multisampled visual through GLX/EGL.
    if (!window) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        window = SDL_CreateWindow(title, width, height, flags);
    }

    if (!window) {
        SDL_Quit(); // Balance the SDL_Init above so the caller can retry
    }

    return window;
}

SDL3Context::SDL3Context(int width, int height, const char* title, bool visible)
    : window_(nullptr), gl_context_(nullptr), should_close_(false), width_(width), height_(height)
{
    SDL_WindowFlags flags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (!visible) {
        flags |= SDL_WINDOW_HIDDEN;
    }

    // Attempt A: Auto-detect video driver.
    // SDL3 prefers Wayland on Wayland sessions and X11 on X11 sessions.
    window_ = tryInitWithDriver(title, width, height, flags, nullptr);

    // Attempt B: Force the x11 driver.
    // Covers NVIDIA + Wayland where the Wayland/EGL path fails (EGL Streams
    // not available inside the Flatpak sandbox or older driver versions).
    // On a Wayland desktop this uses XWayland, which NVIDIA supports via GLX.
    if (!window_) {
        std::cerr << "SDL3Context: auto-detect driver failed (" << SDL_GetError()
                  << "), retrying with x11 driver (XWayland fallback)..." << std::endl;
        window_ = tryInitWithDriver(title, width, height, flags, "x11");
    }

    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        // SDL_Quit was already called by the last tryInitWithDriver on failure.
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
