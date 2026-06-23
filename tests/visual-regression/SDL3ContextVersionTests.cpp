/*
 * SDL3ContextVersionTests.cpp
 *
 * Verifies that SDL3Context requests GL 4.3 (not 4.1) on non-Apple platforms so
 * that compute shaders and GLAD_GL_VERSION_4_3 are available for MC rendering.
 *
 * A real GL context is required. The test skips if no context is obtainable or if
 * the renderer is software-only (llvmpipe), since llvmpipe caps at GL 3.3.
 *
 * On macOS, GL Core Profile is capped at 4.1 by the OS -- the test is excluded
 * at compile time on that platform.
 */

#include <cstring>
#include <string>

#include <gtest/gtest.h>

// clang-format off
#include "glad/glad.h"
#include <SDL3/SDL.h>
// clang-format on

#include "graphics/SDL3Context.hpp"

#ifndef __APPLE__

TEST(SDL3ContextVersionTest, GL43IsAvailable_WhenContextCreated)
{
    SDL3Context ctx(320, 240, "GL Version Test", /*visible=*/false);
    if (!ctx.isValid()) {
        GTEST_SKIP() << "No GL context available (no display or hardware)";
    }
    ctx.makeCurrent();

    // Skip on software renderers -- llvmpipe / softpipe do not guarantee GL 4.3.
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    if (renderer != nullptr && (strstr(renderer, "llvmpipe") != nullptr || strstr(renderer, "softpipe") != nullptr ||
                                strstr(renderer, "LLVM") != nullptr)) {
        GTEST_SKIP() << "Software renderer (" << renderer << ") -- GL 4.3 not guaranteed";
    }

    // SDL3Context must request GL 4.3 on non-Apple platforms so that
    // GLAD_GL_VERSION_4_3 is set and compute_shaders_available_ is true.
    // If this fails, check SDL_GL_CONTEXT_MINOR_VERSION in SDL3Context.cpp:setGLAttributes().
    EXPECT_NE(GLAD_GL_VERSION_4_3, 0) << "GLAD_GL_VERSION_4_3 is 0 after SDL3Context creation.\n"
                                      << "  Renderer: " << (renderer != nullptr ? renderer : "(null)") << "\n"
                                      << "  Root cause: setGLAttributes() in SDL3Context.cpp is requesting GL < 4.3.\n"
                                      << "  Fix: request GL 4.3 (minor version 3) on non-Apple platforms.";
}

#endif // !__APPLE__
