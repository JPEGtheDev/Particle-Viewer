#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <vector>

// clang-format off
#include <glad/glad.h>  // NOLINT(llvm-include-order) -- must precede GL headers
// clang-format on

#include <glm/glm.hpp>

#include "SpatialGrid.hpp"

// ============================================================================
// SpatialGridSSBOs -- RAII wrapper for temporary SSBOs used in VR tests.
//
// Visual-regression tests call MCRenderer::render() which now requires
// pre-built SpatialGrid SSBOs.  This helper builds the grid from the same
// particle/origin/ir arguments used in the test, uploads the data, and
// deletes the SSBOs on destruction (RAII).
// ============================================================================
struct SpatialGridSSBOs
{
    SpatialGrid grid;
    GLuint cell_starts_ssbo = 0;
    GLuint sorted_particles_ssbo = 0;

    /// Build and upload.  Call this once before MCRenderer::render().
    void build(const std::vector<glm::vec4>& particles, glm::vec3 origin, glm::vec3 extent, float ir)
    {
        const auto* data = particles.empty() ? nullptr : particles.data();
        grid.build(data, static_cast<int>(particles.size()), ir, origin, extent, /*max_cells_per_axis=*/256);

        glGenBuffers(1, &cell_starts_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, cell_starts_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(grid.cell_starts.size() * sizeof(uint32_t)),
                     grid.cell_starts.data(), GL_STREAM_DRAW);

        glGenBuffers(1, &sorted_particles_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, sorted_particles_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     static_cast<GLsizeiptr>(grid.sorted_particles.size() * sizeof(glm::vec4)),
                     grid.sorted_particles.data(), GL_STREAM_DRAW);

        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }

    ~SpatialGridSSBOs()
    {
        if (cell_starts_ssbo != 0) {
            glDeleteBuffers(1, &cell_starts_ssbo);
        }
        if (sorted_particles_ssbo != 0) {
            glDeleteBuffers(1, &sorted_particles_ssbo);
        }
    }

    // Non-copyable (owns GL resources)
    SpatialGridSSBOs() = default;
    SpatialGridSSBOs(const SpatialGridSSBOs&) = delete;
    SpatialGridSSBOs& operator=(const SpatialGridSSBOs&) = delete;
};

// Shared constants and helpers for visual-regression tests.
namespace VRTestConfig
{
static const uint32_t RENDER_WIDTH = 1280;
static const uint32_t RENDER_HEIGHT = 720;
static const float PARTICLE_TOLERANCE = 2.0f / 255.0f;
static const float MAX_DIFF_RATIO = 0.0001f;
static const std::string BASELINES_DIR = "baselines";
} // namespace VRTestConfig

inline std::string getShaderPath(const std::string& shaderName)
{
    std::vector<std::string> possiblePaths = {"Viewer-Assets/shaders/" + shaderName,
                                              "../Viewer-Assets/shaders/" + shaderName,
                                              "../../src/shaders/" + shaderName, "../../../src/shaders/" + shaderName};

    for (const auto& path : possiblePaths) {
        FILE* file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            return path;
        }
    }

    return possiblePaths[0];
}

inline std::string getBaselinePath(const std::string& baselineName)
{
    std::vector<std::string> possiblePaths = {VRTestConfig::BASELINES_DIR + "/" + baselineName,
                                              "../../tests/visual-regression/baselines/" + baselineName,
                                              "../tests/visual-regression/baselines/" + baselineName,
                                              "../../../tests/visual-regression/baselines/" + baselineName};

    for (const auto& path : possiblePaths) {
        FILE* file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            return path;
        }
    }

    return possiblePaths[0];
}
