#pragma once
// GL-dependent concrete VRAM query implementation.
// Do NOT include this header in unit test files -- it requires GLAD and a live GL context.

// clang-format off
#include "glad/glad.h"  // NOLINT(llvm-include-order) -- must precede project headers
#include "IVramQuery.hpp"
// clang-format on

/// Queries available VRAM via vendor-specific OpenGL extensions.
///
/// Supported hardware:
///   - NVIDIA: GL_NVX_gpu_memory_info (GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, KB)
///   - AMD:    GL_ATI_meminfo (GL_TEXTURE_FREE_MEMORY_ATI, first element in KB)
///   - Other:  conservative fallback of 128 MB (causes 256^3 to grey out by default)
///
/// Must only be instantiated after a GL context has been created and GLAD loaded.
class GladVramQuery : public IVramQuery
{
  public:
    int availableMB() const override
    {
        GLint kb = 0;
        if (GLAD_GL_NVX_gpu_memory_info) {
            glGetIntegerv(GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &kb);
            return kb / 1024;
        }
        if (GLAD_GL_ATI_meminfo) {
            GLint params[4] = {};
            glGetIntegerv(GL_TEXTURE_FREE_MEMORY_ATI, params);
            return params[0] / 1024;
        }
        return 128; // conservative fallback for unknown hardware
    }
};
