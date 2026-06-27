/*
 * MCTriangleCoherenceTests.cpp
 *
 * Verifies that the 3 vertices forming each rendered triangle are geometrically
 * adjacent -- i.e. they came from the same MC cube invocation.
 *
 * Root cause of the defect this test exposes:
 *   marching_cubes.comp calls atomicCounterIncrement(vertex_counter) once per
 *   vertex (3 separate increments per triangle). GPU invocations run in parallel,
 *   so another invocation can claim SSBO slots between vertex 0 and vertex 1 of
 *   the same triangle. glDrawArrays(GL_TRIANGLES) then forms a "triangle" from
 *   3 consecutive SSBO entries that each came from DIFFERENT cubes, producing
 *   the crumpled/spiky surface appearance.
 *
 * Fix: atomicCounterAdd(vertex_counter, 3u) claims all 3 consecutive slots for
 *   a triangle in a single atomic operation. The test passes after the fix.
 */

#include <algorithm>
#include <cmath>
#include <string>
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

TEST(MCTriangleCoherenceTest, TriangleVertices_AreFromSameMCCube)
{
    SDL3Context ctx(320, 240, "MC Coherence Test", /*visible=*/false);
    if (!ctx.isValid()) {
        GTEST_SKIP() << "No GL context available";
    }
    ctx.makeCurrent();

    if (GLAD_GL_VERSION_4_3 == 0) {
        GTEST_SKIP() << "GL 4.3 not available -- compute shaders require GL 4.3";
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

    // 8-particle 2x2x2 arrangement -- produces a compact isosurface with many
    // triangles from distinct adjacent cubes, maximising interleave probability.
    std::vector<glm::vec4> particles;
    for (int x = -1; x <= 1; x += 2) {
        for (int y = -1; y <= 1; y += 2) {
            for (int z = -1; z <= 1; z += 2) {
                particles.emplace_back(x * 0.1f, y * 0.1f, z * 0.1f, 0.0f);
            }
        }
    }

    const glm::vec3 grid_origin(-0.3f, -0.3f, -0.3f);
    const float voxel_size = 0.6f / 16.0f; // 16-voxel grid
    const float ir = 0.2f;
    const float iso = 0.5f;

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::vec3 extent_tc(voxel_size * 16.0f);
    SpatialGridSSBOs sg_tc;
    sg_tc.build(particles, grid_origin, extent_tc, ir);

    MCRenderer mc(16);
    mc.markDirty();
    mc.render(particles, grid_origin, voxel_size, ir, iso, density_shader.program(), mc_shader.program(),
              mesh_shader.Program, proj, view, sg_tc.cell_starts_ssbo, sg_tc.sorted_particles_ssbo,
              sg_tc.grid.cell_size, sg_tc.grid.cell_origin, sg_tc.grid.num_cells_x, sg_tc.grid.num_cells_y,
              sg_tc.grid.num_cells_z);

    // Read the raw vertex count from the atomic counter buffer.
    GLuint raw_count = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mc.atomicCounter());
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &raw_count);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    ASSERT_GT(raw_count, 0u) << "MC pipeline generated 0 vertices -- no surface to test";

    // Cap to what was actually written (overflow guard mirrors MCRenderer::render).
    const GLuint vertex_count = std::min(raw_count, static_cast<GLuint>(2'000'000 * 3));

    ASSERT_EQ(vertex_count % 3, 0u) << "Vertex count " << vertex_count
                                    << " is not divisible by 3 -- incomplete triangles present";

    // Read back all vertex positions from the SSBO.
    // Layout: 9 floats per vertex [px py pz nx ny nz cr cg cb].
    std::vector<float> ssbo_data(static_cast<size_t>(vertex_count) * 9);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mc.vertexSSBO());
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(ssbo_data.size()) * sizeof(float),
                       ssbo_data.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Each vertex of a triangle is an interpolated point on a voxel cube edge.
    // Two edge endpoints on the same voxel cube are at most sqrt(3)*voxel_size apart
    // (opposite corners of a unit cube). Use 1.05x margin for floating point.
    const float max_valid_dist = std::sqrt(3.0f) * voxel_size * 1.05f;

    auto vertex_pos = [&](GLuint idx) -> glm::vec3 {
        const size_t base = static_cast<size_t>(idx) * 9;
        return glm::vec3(ssbo_data[base], ssbo_data[base + 1], ssbo_data[base + 2]);
    };

    const GLuint triangle_count = vertex_count / 3;
    int bad_triangles = 0;
    float worst_dist = 0.0f;

    for (GLuint tri = 0; tri < triangle_count; ++tri) {
        const GLuint base_idx = tri * 3;
        const glm::vec3 p0 = vertex_pos(base_idx);
        const glm::vec3 p1 = vertex_pos(base_idx + 1);
        const glm::vec3 p2 = vertex_pos(base_idx + 2);

        const float d01 = glm::length(p1 - p0);
        const float d12 = glm::length(p2 - p1);
        const float d02 = glm::length(p2 - p0);
        const float worst = std::max({d01, d12, d02});

        if (worst > max_valid_dist) {
            ++bad_triangles;
            worst_dist = std::max(worst_dist, worst);
        }
    }

    EXPECT_EQ(bad_triangles, 0)
        << bad_triangles << " / " << triangle_count << " triangles have vertices from different MC cubes.\n"
        << "  Max observed inter-vertex distance: " << worst_dist << " (limit: " << max_valid_dist << ")\n"
        << "  Root cause: atomicCounterIncrement is called once per vertex (3 separate calls\n"
        << "  per triangle). Parallel GPU invocations interleave their writes between vertex 0\n"
        << "  and vertex 1 of the same triangle. glDrawArrays(GL_TRIANGLES) then draws\n"
        << "  triangles from SSBO slots that came from entirely different cubes.\n"
        << "  Fix: replace the three atomicCounterIncrement calls with a single\n"
        << "  atomicCounterAdd(vertex_counter, 3u) to claim all 3 slots atomically.";
}

#endif // !__APPLE__
