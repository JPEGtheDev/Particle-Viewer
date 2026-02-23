/*
 * SDL3Context.cpp
 *
 * Production implementation of IOpenGLContext using SDL3.
 * Replaces GLFWContext to support runtime X11/Wayland backend selection.
 *
 * Window creation attempts A–D handle display driver and GL extension
 * fallbacks.  See .github/skills/workflow/references/FLATPAK_GL_GOTCHAS.md
 * for the full explanation of each failure mode.
 */

#include "SDL3Context.hpp"

// clang-format off
#include <glad/glad.h>   // NOLINT(llvm-include-order)
#include <SDL3/SDL.h>    // NOLINT(llvm-include-order)
// clang-format on

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

#include <unistd.h>

/*
 * Pre-flight check: if an NVIDIA GPU is present but no matching Flatpak GL
 * extension is active, set three env vars to force Mesa software rendering
 * before the first SDL_Init.  Must run before SDL_Init so GLVND picks up
 * the vars when it first loads vendor libraries.
 * See FLATPAK_GL_GOTCHAS.md — "NVIDIA GL extension version mismatch".
 */
static bool forceGlSoftwareIfNvidiaExtensionAbsent()
{
    const char* ld_path = getenv("LD_LIBRARY_PATH"); // NOLINT(concurrency-mt-unsafe)
    const bool nvidia_ext_active =                   // NOLINT(readability-identifier-naming)
        (ld_path != nullptr) && (strstr(ld_path, "nvidia") != nullptr);
    const bool nvidia_dev_present = (access("/dev/nvidia0", F_OK) == 0); // NOLINT(readability-identifier-naming)

    if (nvidia_dev_present && !nvidia_ext_active) {
        // overwrite=1: Flatpak pre-sets managed vars to "" so overwrite=0 silently does nothing.
        // All three vars are required together — see FLATPAK_GL_GOTCHAS.md.
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);        // NOLINT(concurrency-mt-unsafe)
        setenv("GALLIUM_DRIVER", "llvmpipe", 1);        // NOLINT(concurrency-mt-unsafe)
        setenv("__GLX_VENDOR_LIBRARY_NAME", "mesa", 1); // NOLINT(concurrency-mt-unsafe)
        return true;
    }
    return false;
}

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
 * Try to create an SDL3 window with an optional video driver override.
 * Sets the hint, calls SDL_Init, sets GL attributes, then attempts window
 * creation with MSAA=4 then MSAA=0 as fallback (Mesa/llvmpipe rejects MSAA).
 * On failure, captures the SDL error before SDL_Quit() clears it, then
 * returns nullptr so the caller can retry with a different driver.
 */
static SDL_Window* tryInitWithDriver(const char* title, int width, int height, SDL_WindowFlags flags,
                                     const char* driver, std::string& out_error)
{
    out_error.clear();
    if (driver != nullptr) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, driver);
    } else {
        // Reset hint so SDL auto-detects — hints survive SDL_Quit().
        SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        out_error = SDL_GetError();
        std::cerr << "SDL_Init failed: " << out_error << '\n';
        return nullptr; // Init failed — no SDL_Quit needed
    }

    setGLAttributes();

    // Attempt 1: MSAA 4x
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
    SDL_Window* window = SDL_CreateWindow(title, width, height, flags);

    // Attempt 2: No MSAA — Mesa/llvmpipe does not expose a multisampled visual.
    if (window == nullptr) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);
        window = SDL_CreateWindow(title, width, height, flags);
    }

    if (window == nullptr) {
        out_error = SDL_GetError(); // capture BEFORE SDL_Quit() clears it
        SDL_Quit();                 // balance the SDL_Init above so the caller can retry
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

    // Pre-flight: force Mesa software rendering if NVIDIA GPU is present
    // without a matching Flatpak GL extension.  Must precede any SDL_Init.
    if (forceGlSoftwareIfNvidiaExtensionAbsent()) {
        std::cerr << "SDL3Context: NVIDIA GPU detected without a matching Flatpak GL extension.\n"
                  << "  Enabling software rendering (Mesa llvmpipe) — performance may be reduced.\n"
                  << "  To restore hardware rendering, install the matching GL extension:\n"
                  << "  flatpak install flathub org.freedesktop.Platform.GL.nvidia-<YOUR_VER>//1.4\n";
    }

    std::string last_error;

    // Attempt A: auto-detect video driver.
    window_ = tryInitWithDriver(title, width, height, flags, nullptr, last_error);

    // Attempt B: force x11 — covers NVIDIA+Wayland where EGL Streams are unavailable.
    if (window_ == nullptr) {
        std::cerr << "SDL3Context: auto-detect driver failed (" << last_error
                  << "), retrying with x11 driver (XWayland fallback)...\n";
        window_ = tryInitWithDriver(title, width, height, flags, "x11", last_error);
    }

    // Attempt C: force wayland — covers pure-Wayland sessions without XWayland.
    if (window_ == nullptr) {
        std::cerr << "SDL3Context: x11 driver failed (" << last_error << "), retrying with wayland driver...\n";
        window_ = tryInitWithDriver(title, width, height, flags, "wayland", last_error);
    }

    // Attempt D: software rendering last resort.  The pre-flight above normally
    // handles Flatpak/NVIDIA mismatches before SDL_Init; this catches any
    // remaining cases on non-Flatpak platforms.
    if (window_ == nullptr) {
        std::cerr << "SDL3Context: wayland driver failed (" << last_error
                  << "), retrying with software rendering hint (best-effort)...\n";
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);        // NOLINT(concurrency-mt-unsafe)
        setenv("GALLIUM_DRIVER", "llvmpipe", 1);        // NOLINT(concurrency-mt-unsafe)
        setenv("__GLX_VENDOR_LIBRARY_NAME", "mesa", 1); // NOLINT(concurrency-mt-unsafe)
        window_ = tryInitWithDriver(title, width, height, flags, nullptr, last_error);
        if (window_ != nullptr) {
            std::cerr << "SDL3Context: WARNING — using software rendering. Performance may be reduced.\n";
        }
    }

    if (window_ == nullptr) {
        std::cerr << "SDL_CreateWindow failed: " << last_error << '\n';
        // SDL_Quit was already called by the last tryInitWithDriver on failure.
        return;
    }

    gl_context_ = SDL_GL_CreateContext(window_);
    if (gl_context_ == nullptr) {
        std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
        return;
    }

    SDL_GL_MakeCurrent(window_, gl_context_);

    if (gladLoadGLLoader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
        std::cerr << "Failed to initialize GLAD: unable to load OpenGL function pointers.\n";
        SDL_GL_DestroyContext(gl_context_);
        gl_context_ = nullptr;
        SDL_DestroyWindow(window_);
        window_ = nullptr;
        SDL_Quit();
    }
}

SDL3Context::~SDL3Context()
{
    if (gl_context_ != nullptr) {
        SDL_GL_DestroyContext(gl_context_);
        gl_context_ = nullptr;
    }
    if (window_ != nullptr) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}

bool SDL3Context::isValid() const
{
    return (window_ != nullptr) && (gl_context_ != nullptr);
}

void SDL3Context::makeCurrent()
{
    if (window_ != nullptr && gl_context_ != nullptr) {
        SDL_GL_MakeCurrent(window_, gl_context_);
    }
}

void SDL3Context::swapBuffers()
{
    if (window_ != nullptr) {
        SDL_GL_SwapWindow(window_);
    }
}

std::pair<int, int> SDL3Context::getFramebufferSize() const
{
    if (window_ != nullptr) {
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
