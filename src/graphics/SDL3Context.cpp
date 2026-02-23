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
 *   via GLX.  If x11 also fails (e.g. pure Wayland session without XWayland),
 *   a third attempt with "wayland" forced is made.
 *
 * Flatpak + NVIDIA GL extension version mismatch:
 *   When the installed nvidia-<ver> Flatpak GL extension does not exactly
 *   match the host driver (e.g. driver 580.126.18 but only the 580.126.16
 *   extension exists on flathub), Mesa's libGLX_mesa.so is the only vendor
 *   library present in the sandbox.  Mesa's GLX client cannot negotiate with
 *   NVIDIA's X server GLX extension, so window creation fails with
 *   "Invalid window driver data".  SDL3Context detects this before the first
 *   SDL_Init (NVIDIA device node present + LD_LIBRARY_PATH has no "nvidia"
 *   path) and forces LIBGL_ALWAYS_SOFTWARE=1 so that Mesa software rendering
 *   (llvmpipe) is used.  See forceGlSoftwareIfNvidiaExtensionAbsent() below.
 *
 * If the app still fails after all four attempts:
 *   - Ensure NVIDIA drivers >= 560 are installed (Wayland DRM support).
 *   - Run with SDL_VIDEODRIVER=x11 to force X11/XWayland mode manually.
 *   - Run with SDL_VIDEODRIVER=wayland to force the Wayland path explicitly.
 *   - On Flatpak, the --device=all finish-arg is required (already set).
 *   - Check that org.freedesktop.Platform.GL.nvidia-<ver>//1.4 is installed.
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
 * Pre-flight: detect an NVIDIA GPU present in the sandbox without a matching
 * Flatpak GL extension active.
 *
 * In a Flatpak sandbox on an NVIDIA system, the GL extension
 * (org.freedesktop.Platform.GL.nvidia-<ver>//1.4) must be installed with an
 * exact driver-version match.  When it is absent or mismatched, the Flatpak
 * runtime only mounts Mesa's GL.default extension.  Mesa's libGLX_mesa.so
 * cannot negotiate with NVIDIA's X server GLX extension, so SDL3 window
 * creation fails with "Invalid window driver data".
 *
 * Detection heuristic (Flatpak-specific, requires --device=all finish-arg):
 *   - /dev/nvidia0 present  → NVIDIA GPU is present in the sandbox
 *   - LD_LIBRARY_PATH has no "nvidia" path  → GL extension is not active
 *
 * When both conditions hold, three env vars are set before the first
 * SDL_Init so that the correct Mesa software paths are chosen when GL
 * vendor libraries are first loaded by GLVND:
 *   LIBGL_ALWAYS_SOFTWARE=1     — force Mesa software rendering
 *   GALLIUM_DRIVER=llvmpipe    — explicit Mesa llvmpipe backend
 *   __GLX_VENDOR_LIBRARY_NAME=mesa — GLVND uses Mesa's libGLX_mesa vendor
 *       library instead of querying the X server extension (which would
 *       request the absent libGLX_nvidia.so and fail).
 *
 * The 'overwrite=1' flag is used because Flatpak pre-sets env vars it
 * manages to "" (empty string), which means overwrite=0 would be silently
 * ignored.  The forced values can always be overridden by re-running the app
 * with an explicit --env=<VAR>=value flag.
 *
 * Returns true if software rendering was forced.
 */
static bool forceGlSoftwareIfNvidiaExtensionAbsent()
{
    const char* ld_path = getenv("LD_LIBRARY_PATH"); // NOLINT(concurrency-mt-unsafe)
    const bool nvidia_ext_active = (ld_path != nullptr) && (strstr(ld_path, "nvidia") != nullptr);
    const bool nvidia_dev_present = (access("/dev/nvidia0", F_OK) == 0);

    if (nvidia_dev_present && !nvidia_ext_active) {
        // Use overwrite=1: Flatpak initialises env vars it controls to ""
        // (empty string) which is treated as "already set" by overwrite=0,
        // so a plain setenv with overwrite=0 would be silently ignored.
        //
        // Three env vars are required together:
        //   LIBGL_ALWAYS_SOFTWARE=1    — force Mesa software rendering
        //   GALLIUM_DRIVER=llvmpipe   — explicit Mesa llvmpipe backend
        //   __GLX_VENDOR_LIBRARY_NAME=mesa — force GLVND to use Mesa's
        //       libGLX_mesa vendor library.  Without this, GLVND queries the
        //       X server extension (which advertises NVIDIA) and tries to
        //       dlopen libGLX_nvidia.so, which is absent when the GL
        //       extension does not match; requesting Mesa here bypasses
        //       the vendor negotiation and uses software rendering instead.
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
 * Try to create an SDL3 window after calling SDL_Init with an optional video
 * driver override.  The function:
 *   1. Optionally sets SDL_HINT_VIDEO_DRIVER (must precede SDL_Init).
 *   2. Calls SDL_Init(SDL_INIT_VIDEO).
 *   3. Sets OpenGL context attributes.
 *   4. Attempts window creation with MSAA=4, then MSAA=0 as fallback.
 *
 * Returns the created window on success.  On failure, calls SDL_Quit() to
 * balance the SDL_Init() and stores the SDL error in out_error BEFORE
 * SDL_Quit() can clear it, then returns nullptr so the caller can retry.
 *
 * driver: nullptr means auto-detect (SDL3 default); "x11" forces X11/XWayland;
 *         "wayland" forces the Wayland backend explicitly.
 */
static SDL_Window* tryInitWithDriver(const char* title, int width, int height, SDL_WindowFlags flags,
                                     const char* driver, std::string& out_error)
{
    out_error.clear();
    if (driver) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, driver);
    } else {
        // Reset any previously-set driver hint so SDL auto-detects cleanly.
        // SDL hints survive SDL_Quit(), so without this reset the last explicit
        // driver hint from a failed attempt (e.g. "wayland") would be reused.
        SDL_ResetHint(SDL_HINT_VIDEO_DRIVER);
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        out_error = SDL_GetError();
        std::cerr << "SDL_Init failed: " << out_error << std::endl;
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

    // Pre-flight: if an NVIDIA GPU is present but no matching Flatpak GL
    // extension is active, force Mesa software rendering before the first
    // SDL_Init.  This must happen before any SDL video initialisation so that
    // LIBGL_ALWAYS_SOFTWARE is in effect when GLVND first loads GL vendor libs.
    if (forceGlSoftwareIfNvidiaExtensionAbsent()) {
        std::cerr << "SDL3Context: NVIDIA GPU detected without a matching Flatpak GL extension.\n"
                  << "  Enabling software rendering (Mesa llvmpipe) — performance may be reduced.\n"
                  << "  To restore hardware rendering, install the matching GL extension:\n"
                  << "  flatpak install flathub org.freedesktop.Platform.GL.nvidia-<YOUR_VER>//1.4\n";
    }

    std::string last_error;

    // Attempt A: Auto-detect video driver.
    // SDL3 prefers Wayland on Wayland sessions and X11 on X11 sessions.
    window_ = tryInitWithDriver(title, width, height, flags, nullptr, last_error);

    // Attempt B: Force the x11 driver.
    // Covers NVIDIA + Wayland where the Wayland/EGL path fails (EGL Streams
    // not available inside the Flatpak sandbox or older driver versions).
    // On a Wayland desktop this uses XWayland, which NVIDIA supports via GLX.
    if (!window_) {
        std::cerr << "SDL3Context: auto-detect driver failed (" << last_error
                  << "), retrying with x11 driver (XWayland fallback)..." << std::endl;
        window_ = tryInitWithDriver(title, width, height, flags, "x11", last_error);
    }

    // Attempt C: Force the wayland driver.
    // Covers pure Wayland sessions where XWayland is not installed or available
    // (e.g. some minimal Flatpak environments) and the auto-detect chosen path
    // failed for a transient reason.
    if (!window_) {
        std::cerr << "SDL3Context: x11 driver failed (" << last_error << "), retrying with wayland driver..."
                  << std::endl;
        window_ = tryInitWithDriver(title, width, height, flags, "wayland", last_error);
    }

    // Attempt D: Best-effort software rendering retry.
    // The pre-flight above sets LIBGL_ALWAYS_SOFTWARE=1 before the first
    // SDL_Init when a Flatpak/NVIDIA mismatch is detected, so Attempts A–C
    // should have succeeded and this branch is not normally reached.
    // For non-Flatpak scenarios where the pre-flight heuristic could not
    // detect the mismatch, this attempt sets LIBGL_ALWAYS_SOFTWARE and
    // retries.  Note: if NVIDIA's GL libraries were already loaded by a prior
    // SDL_Init call the setenv will have no effect on GLVND dispatch.
    if (!window_) {
        std::cerr << "SDL3Context: wayland driver failed (" << last_error
                  << "), retrying with software rendering hint (best-effort)..." << std::endl;
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);        // NOLINT(concurrency-mt-unsafe) overwrite=1: see pre-flight
        setenv("GALLIUM_DRIVER", "llvmpipe", 1);        // NOLINT(concurrency-mt-unsafe)
        setenv("__GLX_VENDOR_LIBRARY_NAME", "mesa", 1); // NOLINT(concurrency-mt-unsafe)
        window_ = tryInitWithDriver(title, width, height, flags, nullptr, last_error);
        if (window_) {
            std::cerr << "SDL3Context: WARNING — using software rendering. Performance may be reduced." << std::endl;
        }
    }

    if (!window_) {
        std::cerr << "SDL_CreateWindow failed: " << last_error << std::endl;
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
