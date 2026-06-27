/*
 * MCRendererComputeTests.cpp
 *
 * Verifies that the Marching Cubes compute pipeline generates a non-zero vertex count
 * for a known particle configuration. A real GL 4.3 context is required.
 *
 * Root cause documented: the MCTables block was declared as a std140 UBO which:
 *   1. Pads each int to 16 bytes -- UBO is 69,632 bytes > GL_MAX_UNIFORM_BLOCK_SIZE (65,536 on NVIDIA)
 *      causing marching_cubes.comp to fail to link.
 *   2. Even if linked, std140 stride (16) != CPU upload stride (4), so all MC table
 *      reads return wrong values and no surface is generated.
 * Fix: MCTables declared as std430 SSBO (tight packing, no size cap).
 */

#include <cstring>
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

TEST(MCRendererComputeTest, MCPipeline_GeneratesNonZeroVertexCount_With8Particles)
{
    SDL3Context ctx(320, 240, "MC Compute Test", /*visible=*/false);
    if (!ctx.isValid()) {
        GTEST_SKIP() << "No GL context available";
    }
    ctx.makeCurrent();

    if (GLAD_GL_VERSION_4_3 == 0) {
        GTEST_SKIP() << "GL 4.3 not available -- compute shaders require GL 4.3";
    }

    // Load compute shaders
    const std::string density_path = getShaderPath("density_field.comp");
    const std::string mc_path = getShaderPath("marching_cubes.comp");
    const std::string mesh_vert_path = getShaderPath("mesh.vert");
    const std::string mesh_frag_path = getShaderPath("mesh.frag");

    ComputeShader density_shader(density_path.c_str());
    ASSERT_NE(density_shader.program(), 0u) << "density_field.comp failed to link: " << density_path;

    ComputeShader mc_shader(mc_path.c_str());
    // This assertion catches the MCTables UBO link failure.
    // If marching_cubes.comp fails to link (e.g. UBO exceeds GL_MAX_UNIFORM_BLOCK_SIZE),
    // mc_shader.program() returns the unlinked program handle (non-zero but invalid).
    // The EXPECT below probes behavior rather than the handle value.

    // 8-particle 2x2x2 cube arrangement
    std::vector<glm::vec4> particles;
    for (int x = -1; x <= 1; x += 2) {
        for (int y = -1; y <= 1; y += 2) {
            for (int z = -1; z <= 1; z += 2) {
                particles.emplace_back(x * 0.1f, y * 0.1f, z * 0.1f, 0.0f);
            }
        }
    }

    // Grid covers particle extent + influence radius on all sides
    const glm::vec3 grid_origin(-0.3f, -0.3f, -0.3f);
    const float voxel_size = 0.6f / 16.0f; // 16-voxel grid
    const float influence_radius = 0.2f;
    const float iso_value = 0.5f;

    Shader mesh_shader(mesh_vert_path.c_str(), mesh_frag_path.c_str());
    if (mesh_shader.Program == 0) {
        GTEST_SKIP() << "mesh shader failed to compile/link";
    }

    const glm::vec3 extent_cr(voxel_size * 16.0f);
    SpatialGridSSBOs sg_cr;
    sg_cr.build(particles, grid_origin, extent_cr, influence_radius);

    MCRenderer mc_renderer(16);
    mc_renderer.markDirty();

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    mc_renderer.render(particles, grid_origin, voxel_size, influence_radius, iso_value, density_shader.program(),
                       mc_shader.program(), mesh_shader.Program, projection, view, sg_cr.cell_starts_ssbo,
                       sg_cr.sorted_particles_ssbo, sg_cr.grid.cell_size, sg_cr.grid.cell_origin,
                       sg_cr.grid.num_cells_x, sg_cr.grid.num_cells_y, sg_cr.grid.num_cells_z);

    // Read the atomic counter -- it holds the vertex count written by marching_cubes.comp.
    // render() reads it internally but does not reset it afterwards.
    GLuint vertex_count = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mc_renderer.atomicCounter());
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &vertex_count);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    EXPECT_GT(vertex_count, 0u)
        << "MC pipeline generated 0 vertices for an 8-particle 2x2x2 cube.\n"
        << "  Root cause (most likely): marching_cubes.comp MCTables is a std140 UBO.\n"
        << "  std140 pads each int to 16 bytes -> UBO = 69,632 bytes > GL_MAX_UNIFORM_BLOCK_SIZE (65,536).\n"
        << "  Fix: change 'layout(std140, binding = 4) uniform MCTables' to "
        << "'layout(std430, binding = 4) readonly buffer MCTables' in marching_cubes.comp,\n"
        << "  and change GL_UNIFORM_BUFFER to GL_SHADER_STORAGE_BUFFER in MCRenderer.cpp.";
}

#endif // !__APPLE__
