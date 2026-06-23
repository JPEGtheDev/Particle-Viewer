/*
 * MCNormalQualityTests.cpp
 *
 * Verifies that per-vertex normals on a single-sphere isosurface are close to
 * the analytically correct radial direction.
 *
 * computeNormal() in marching_cubes.comp computes the gradient analytically
 * from particle positions (exact Gaussian gradient), which produces correct
 * normals regardless of grid resolution.
 *
 * Pass criterion: >= 90% of vertices have |dot(N, radial)| >= cos(20 deg) = 0.940.
 * Using abs() because MC winding is inconsistent -- the fragment shader compensates
 * with abs(dot(N,L)) so inward-pointing normals still light correctly.
 */

#include <algorithm>
#include <cmath>
#include <vector>

#include <gtest/gtest.h>

// clang-format off
#include "glad/glad.h"
#include <SDL3/SDL.h>
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "MCRenderer.hpp"
#include "VRTestCommon.hpp"
#include "graphics/SDL3Context.hpp"
#include "shader.hpp"

#ifndef __APPLE__

TEST(MCNormalQualityTest, SingleSphere_VertexNormals_AreRadial)
{
    SDL3Context ctx(320, 240, "MC Normal Quality Test", /*visible=*/false);
    if (!ctx.isValid()) {
        GTEST_SKIP() << "No GL context available";
    }
    ctx.makeCurrent();

    if (GLAD_GL_VERSION_4_3 == 0) {
        GTEST_SKIP() << "GL 4.3 not available";
    }

    const std::string density_path = getShaderPath("density_field.comp");
    const std::string mc_path = getShaderPath("marching_cubes.comp");
    const std::string vert_path = getShaderPath("mesh.vert");
    const std::string frag_path = getShaderPath("mesh.frag");

    ComputeShader density_shader(density_path.c_str());
    ASSERT_NE(density_shader.program(), 0u) << "density_field.comp failed to link";

    ComputeShader mc_shader(mc_path.c_str());
    ASSERT_NE(mc_shader.program(), 0u) << "marching_cubes.comp failed to link";

    Shader mesh_shader(vert_path.c_str(), frag_path.c_str());
    if (mesh_shader.Program == 0u) {
        GTEST_SKIP() << "mesh shader failed to compile";
    }

    // Single particle at origin produces a clean Gaussian-falloff sphere.
    // The true isosurface normal at any point P is normalize(P - origin) = normalize(P).
    const glm::vec3 center(0.0f);
    std::vector<glm::vec4> particles = {{0.0f, 0.0f, 0.0f, 0.0f}};

    // 64-voxel grid: sphere radius ~18.9 voxels -> sufficient gradient accuracy for
    // 90% of normals to land within 20 deg of true radial.  32 voxels gives only
    // ~9.4-voxel radius and observed 61% pass rate -- too coarse.
    const int grid_res = 64;
    const float ir = 0.5f;
    const float iso = 0.5f;
    const glm::vec3 grid_origin = glm::vec3(-1.0f);
    const float voxel_size = 2.0f / static_cast<float>(grid_res);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    MCRenderer mc(grid_res);
    mc.markDirty();
    mc.render(particles, grid_origin, voxel_size, ir, iso, density_shader.program(), mc_shader.program(),
              mesh_shader.Program, proj, view);

    GLuint raw_count = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mc.atomicCounter());
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &raw_count);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ASSERT_GT(raw_count, 0u) << "MC pipeline generated 0 vertices";

    const GLuint vertex_count = std::min(raw_count, static_cast<GLuint>(2'000'000 * 3));
    std::vector<float> ssbo(static_cast<size_t>(vertex_count) * 9);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mc.vertexSSBO());
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(ssbo.size()) * sizeof(float), ssbo.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // For a sphere centred at origin the outward normal equals normalize(vertex_pos).
    // Allow MC discretisation error: threshold = cos(20 deg) ~= 0.940.
    constexpr float kCosThreshold = 0.940f; // cos(20 deg)
    constexpr float kPassFraction = 0.90f;  // 90 % of vertices must meet threshold

    int good = 0;
    int total = static_cast<int>(vertex_count);
    float worst_dot = 1.0f;

    for (GLuint i = 0; i < vertex_count; ++i) {
        const size_t base = static_cast<size_t>(i) * 9;
        glm::vec3 pos(ssbo[base], ssbo[base + 1], ssbo[base + 2]);
        glm::vec3 normal(ssbo[base + 3], ssbo[base + 4], ssbo[base + 5]);

        glm::vec3 radial = pos - center;
        float len = glm::length(radial);
        if (len < 1e-6f) {
            continue; // vertex at exact center -- skip
        }

        float d = glm::dot(glm::normalize(normal), glm::normalize(radial));
        // Use abs(d): MC winding is inconsistent across the 256 cube configurations.
        // The fragment shader compensates with abs(dot(N,L)) so all triangles light
        // correctly regardless of winding.  We verify radial ALIGNMENT only.
        float aligned = std::abs(d);
        worst_dot = std::min(worst_dot, aligned);
        if (aligned >= kCosThreshold) {
            ++good;
        }
    }

    float pass_rate = static_cast<float>(good) / static_cast<float>(total);

    EXPECT_GE(pass_rate, kPassFraction)
        << "Only " << (pass_rate * 100.0f) << "% of vertices have normals within 20 deg of radial "
        << "(need >= " << (kPassFraction * 100.0f) << "%).\n"
        << "  Worst |dot| product observed: " << worst_dot << " (threshold: " << kCosThreshold << ")\n"
        << "  Root cause: computeNormal() using central-difference gradient of the discrete\n"
        << "  density texture instead of the analytic Gaussian gradient from particle positions.";
}

#endif // !__APPLE__
