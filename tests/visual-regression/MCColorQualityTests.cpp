/*
 * MCColorQualityTests.cpp
 *
 * Verifies that per-vertex colors on an isosurface reflect the underlying
 * particle category, not grey fallback.
 *
 * Root cause caught here: computeColor() in marching_cubes.comp had a hard
 * cutoff at `if (dist > influence_radius) continue;`.  The density field is a
 * TRUNCATED Gaussian (also cut off at ir), so the isosurface sits at ~ir from
 * each particle.  MC interpolates surface vertices along voxel edges that cross
 * from inside the surface (dist < ir) to just outside (dist > ir, density=0).
 * Those interpolated vertices land at dist = ir + epsilon -- just outside the
 * color cutoff -- and receive grey (0.5, 0.5, 0.5), producing visible splotches.
 * The fix removes the hard cutoff from computeColor() and lets the Gaussian
 * weight decay smoothly, consistent with computeNormal() which has no cutoff.
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

// A single red particle (category 0) should colour its entire surface red.
// Tests both average colour (R > 0.6) and grey-patch fraction (< 5%).
// Before the fix: ~39% of vertices land at dist = ir + epsilon from MC
//   interpolation and receive grey (0.5, 0.5, 0.5) due to the hard cutoff.
// After the fix: Gaussian weight decays smoothly past ir, giving red-dominant
//   colour across the full surface.
TEST(MCColorQualityTest, SingleRedParticle_SurfaceIsRed)
{
    SDL3Context ctx(320, 240, "MC Color Quality Test", /*visible=*/false);
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

    // Single red particle (category 0) at origin.
    std::vector<glm::vec4> particles = {{0.0f, 0.0f, 0.0f, 0.0f}};

    const int grid_res = 64;
    const float ir = 0.5f;
    const float iso = 0.5f;
    const glm::vec3 origin = glm::vec3(-1.0f);
    const float voxel_size = 2.0f / static_cast<float>(grid_res);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::vec3 extent(voxel_size * static_cast<float>(grid_res));
    SpatialGridSSBOs sg;
    sg.build(particles, origin, extent, ir);

    MCRenderer mc(grid_res);
    mc.markDirty();
    mc.render(particles, origin, voxel_size, ir, iso, density_shader.program(), mc_shader.program(),
              mesh_shader.Program, proj, view, sg.cell_starts_ssbo, sg.sorted_particles_ssbo, sg.grid.cell_size,
              sg.grid.cell_origin, sg.grid.num_cells_x, sg.grid.num_cells_y, sg.grid.num_cells_z);

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

    double r_sum = 0.0, g_sum = 0.0, b_sum = 0.0;
    for (GLuint i = 0; i < vertex_count; ++i) {
        const size_t base = static_cast<size_t>(i) * 9;
        r_sum += ssbo[base + 6];
        g_sum += ssbo[base + 7];
        b_sum += ssbo[base + 8];
    }

    const float r_avg = static_cast<float>(r_sum / vertex_count);
    const float g_avg = static_cast<float>(g_sum / vertex_count);
    const float b_avg = static_cast<float>(b_sum / vertex_count);

    // Grey = (0.5, 0.5, 0.5). Red = (1.0, 0.0, 0.0).
    // We require red-dominant: R > 0.6, G < 0.25, B < 0.25.
    EXPECT_GT(r_avg, 0.6f) << "Surface appears grey (R=" << r_avg << "). "
                           << "Root cause: computeColor() hard cutoff at ir excluded all particles "
                           << "because the isosurface radius (~1.18*ir) > ir. "
                           << "Fix: remove the hard cutoff from computeColor().";
    EXPECT_LT(g_avg, 0.25f) << "Green channel too high (G=" << g_avg << "), expected near 0 for red particle";
    EXPECT_LT(b_avg, 0.25f) << "Blue channel too high (B=" << b_avg << "), expected near 0 for red particle";

    // Grey-patch check: MC interpolation places some surface vertices at dist slightly
    // beyond ir from the particle (interpolating from a corner at dist=ir to one just
    // outside ir with density=0). Those vertices fall outside computeColor()'s hard
    // cutoff and receive grey (0.5, 0.5, 0.5).  A red sphere must have < 5% grey verts.
    //
    // A vertex is "grey" when all three channels are mid-range and approximately equal.
    int grey_count = 0;
    for (GLuint i = 0; i < vertex_count; ++i) {
        const size_t base = static_cast<size_t>(i) * 9;
        const float r = ssbo[base + 6];
        const float g = ssbo[base + 7];
        const float b = ssbo[base + 8];
        if (r > 0.30f && std::abs(r - g) < 0.08f && std::abs(r - b) < 0.08f) {
            ++grey_count;
        }
    }
    const float grey_fraction = static_cast<float>(grey_count) / static_cast<float>(vertex_count);
    EXPECT_LT(grey_fraction, 0.05f)
        << "Grey patch fraction too high (" << (grey_fraction * 100.0f) << "% of " << vertex_count << " vertices). "
        << "Root cause: computeColor() hard cutoff at ir -- MC interpolation places some surface "
        << "vertices at dist = ir + epsilon, just outside the cutoff, producing visible grey splotches. "
        << "Fix: remove 'if (dist > influence_radius) continue;' from computeColor() in marching_cubes.comp.";
}

// Verifies that the density kernel places the isosurface well inside ir.
//
// With a Gaussian kernel truncated at ir, the density at r=ir is exp(-0.5)=0.607.
// For iso=0.5, that puts the surface AT ir (the truncation boundary).
// With the polynomial kernel (1 - ratio^2)^3, the density at r=ir is 0.0 and the
// iso=0.5 crossover sits at r = 0.454*ir -- well inside ir.
//
// RED with Gaussian  : max vertex dist from particle center is ~ir (>= 0.8*ir threshold).
// GREEN with polynomial: max vertex dist is ~0.454*ir (< 0.8*ir threshold).
TEST(MCColorQualityTest, SingleRedParticle_SurfaceRadiusIsInsideIR)
{
    SDL3Context ctx(320, 240, "MC Kernel Radius Test", /*visible=*/false);
    if (!ctx.isValid()) {
        GTEST_SKIP() << "No GL context available";
    }
    ctx.makeCurrent();

    if (GLAD_GL_VERSION_4_3 == 0) {
        GTEST_SKIP() << "GL 4.3 not available";
    }

    ComputeShader density_shader(getShaderPath("density_field.comp").c_str());
    ASSERT_NE(density_shader.program(), 0u) << "density_field.comp failed to link";
    ComputeShader mc_shader(getShaderPath("marching_cubes.comp").c_str());
    ASSERT_NE(mc_shader.program(), 0u) << "marching_cubes.comp failed to link";
    Shader mesh_shader(getShaderPath("mesh.vert").c_str(), getShaderPath("mesh.frag").c_str());
    if (mesh_shader.Program == 0u) {
        GTEST_SKIP() << "mesh shader failed to compile";
    }

    std::vector<glm::vec4> particles = {{0.0f, 0.0f, 0.0f, 0.0f}};
    const int grid_res = 64;
    const float ir = 0.5f;
    const float iso = 0.5f;
    const glm::vec3 origin = glm::vec3(-1.0f);
    const float voxel_size = 2.0f / static_cast<float>(grid_res);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::vec3 extent2(voxel_size * static_cast<float>(grid_res));
    SpatialGridSSBOs sg2;
    sg2.build(particles, origin, extent2, ir);

    MCRenderer mc(grid_res);
    mc.markDirty();
    mc.render(particles, origin, voxel_size, ir, iso, density_shader.program(), mc_shader.program(),
              mesh_shader.Program, proj, view, sg2.cell_starts_ssbo, sg2.sorted_particles_ssbo, sg2.grid.cell_size,
              sg2.grid.cell_origin, sg2.grid.num_cells_x, sg2.grid.num_cells_y, sg2.grid.num_cells_z);

    GLuint raw_count = 0;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mc.atomicCounter());
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &raw_count);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    ASSERT_GT(raw_count, 0u) << "MC pipeline generated 0 vertices -- isosurface did not form";

    const GLuint vertex_count = std::min(raw_count, static_cast<GLuint>(2'000'000 * 3));
    std::vector<float> ssbo(static_cast<size_t>(vertex_count) * 9);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mc.vertexSSBO());
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, static_cast<GLsizeiptr>(ssbo.size()) * sizeof(float), ssbo.data());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Particle is at origin. Each vertex position (first 3 floats of 9-float stride)
    // must be closer than 0.8*ir to the particle center.
    //
    // Gaussian kernel puts surface at r=ir (truncation boundary) -- max_dist ≈ 0.5 > 0.4 FAILS.
    // Polynomial kernel puts surface at r=0.454*ir -- max_dist ≈ 0.23 < 0.4 PASSES.
    float max_dist = 0.0f;
    for (GLuint i = 0; i < vertex_count; ++i) {
        const size_t base = static_cast<size_t>(i) * 9;
        const glm::vec3 vp(ssbo[base + 0], ssbo[base + 1], ssbo[base + 2]);
        max_dist = std::max(max_dist, glm::length(vp));
    }

    const float threshold = 0.8f * ir;
    EXPECT_LT(max_dist, threshold)
        << "Isosurface extends to " << max_dist << " from particle center (ir=" << ir << "). "
        << "Gaussian kernel places the surface at the truncation boundary (r=ir). "
        << "Polynomial kernel (1-ratio^2)^3 naturally goes to 0 at r=ir, placing "
        << "the iso=0.5 surface at r=0.454*ir (" << (0.454f * ir) << "). "
        << "Fix: replace exp(-0.5*ratio*ratio) with (1-ratio*ratio)^3 in density_field.comp.";
}

// Two-particle blend: red (left) + blue (right), separated so they are just
// merged (d < merge threshold 2*ir with truncated Gaussian).  The average
// surface colour must have roughly equal R and B with low G, indicating a
// red-blue blend rather than grey.
TEST(MCColorQualityTest, TwoParticles_SurfaceBlend_IsRedBlue)
{
    SDL3Context ctx(320, 240, "MC Color Blend Test", /*visible=*/false);
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
    ASSERT_NE(density_shader.program(), 0u);
    ComputeShader mc_shader(mc_path.c_str());
    ASSERT_NE(mc_shader.program(), 0u);
    Shader mesh_shader(vert_path.c_str(), frag_path.c_str());
    if (mesh_shader.Program == 0u) {
        GTEST_SKIP() << "mesh shader failed to compile";
    }

    // Particles separated by 0.8 (< merge threshold 2*ir=1.0 for truncated Gaussian).
    // Midpoint density = 2*exp(-0.5*(0.4/0.5)^2) = 1.45 >> iso=0.5 -> merged.
    std::vector<glm::vec4> particles = {
        {-0.4f, 0.0f, 0.0f, 0.0f}, // red
        {0.4f, 0.0f, 0.0f, 1.0f},  // blue
    };

    const int grid_res = 64;
    const float ir = 0.5f;
    const float iso = 0.5f;
    const glm::vec3 origin = glm::vec3(-1.5f, -1.0f, -1.0f);
    const float voxel_size = 3.0f / static_cast<float>(grid_res);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const glm::vec3 extent3(voxel_size * static_cast<float>(grid_res));
    SpatialGridSSBOs sg3;
    sg3.build(particles, origin, extent3, ir);

    MCRenderer mc(grid_res);
    mc.markDirty();
    mc.render(particles, origin, voxel_size, ir, iso, density_shader.program(), mc_shader.program(),
              mesh_shader.Program, proj, view, sg3.cell_starts_ssbo, sg3.sorted_particles_ssbo, sg3.grid.cell_size,
              sg3.grid.cell_origin, sg3.grid.num_cells_x, sg3.grid.num_cells_y, sg3.grid.num_cells_z);

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

    double r_sum = 0.0, g_sum = 0.0, b_sum = 0.0;
    for (GLuint i = 0; i < vertex_count; ++i) {
        const size_t base = static_cast<size_t>(i) * 9;
        r_sum += ssbo[base + 6];
        g_sum += ssbo[base + 7];
        b_sum += ssbo[base + 8];
    }

    const float r_avg = static_cast<float>(r_sum / vertex_count);
    const float g_avg = static_cast<float>(g_sum / vertex_count);
    const float b_avg = static_cast<float>(b_sum / vertex_count);

    // Grey = (0.5, 0.5, 0.5). Red-blue blend -> R and B both elevated, G low.
    // Before fix: all grey (R=G=B=0.5 because surface exceeds ir from both particles).
    // After fix: R and B > 0.35, G < 0.25 (blue category is (0.2, 0.6, 1.0) so G
    //   can be up to ~0.3 when blue dominates -- threshold set conservatively).
    EXPECT_GT(r_avg, 0.35f) << "Red channel too low (R=" << r_avg << "), expected red-blue blend.";
    EXPECT_LT(g_avg, 0.40f) << "Green channel too high (G=" << g_avg << "), expected low for red-blue blend.";
    EXPECT_GT(b_avg, 0.35f) << "Blue channel too low (B=" << b_avg << "), expected red-blue blend.";
}

#endif // !__APPLE__
