/*
 * MCCubeIntegrityTests.cpp
 *
 * Verifies that all six faces of the MC isosurface of a dense particle cube
 * are actually rendered -- i.e. no face is missing due to grid boundary
 * clipping.
 *
 * Root cause: viewer_app.cpp originally padded the MC bounding box by ir*1.
 * The isosurface of outer particles sits at dist = ir from the particle
 * center.  With ir*1 padding the surface is at exactly the grid boundary;
 * the density-field voxel there has one corner inside the surface and the
 * adjacent corner outside the grid (density=0 by default).  The MC either
 * generates no triangles or generates flat-cap triangles that are culled,
 * producing the visible "open box" defect seen from the camera.
 *
 * Fix: pad by ir*2 so the surface is at least ir inside the grid boundary.
 * This matches the gallery test fix and the two-particle padding fix.
 *
 * Test replicated from live-app parameters reported in PR #132:
 *   radius=2.0 in UI -> mc_scaled_ir = 2.0 * kSimToDisplayScale(0.25) = 0.5
 *   40x40x40 lattice, spacing=0.075, iso=0.58, grid_res=64
 */

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// clang-format off
#include "glad/glad.h"
#include <SDL3/SDL.h>
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Image.hpp"
#include "MCRenderer.hpp"
#include "VRTestCommon.hpp"
#include "graphics/SDL3Context.hpp"
#include "shader.hpp"
#include "testing/FramebufferCapture.hpp"

#ifndef __APPLE__

namespace
{

// Returns the fraction of pixels in the centre region (middle third of
// the image in both axes) that are above the background black.  A face
// is "present" when this value is > 0.  A missing face leaves the
// centre completely black (fraction = 0).
// Image stores RGBA, 4 bytes per pixel, row-major in img.pixels.
float centreBrightnessFraction(const Image& img, float brightness_threshold = 0.05f)
{
    const int w = static_cast<int>(img.width);
    const int h = static_cast<int>(img.height);
    const int x0 = w / 3, x1 = 2 * w / 3;
    const int y0 = h / 3, y1 = 2 * h / 3;

    int total = 0, bright = 0;
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            const size_t idx = (static_cast<size_t>(y) * static_cast<size_t>(w) + static_cast<size_t>(x)) * 4;
            const float r = img.pixels[idx + 0] / 255.0f;
            const float g = img.pixels[idx + 1] / 255.0f;
            const float b = img.pixels[idx + 2] / 255.0f;
            if (std::max({r, g, b}) > brightness_threshold) {
                ++bright;
            }
            ++total;
        }
    }
    return total > 0 ? static_cast<float>(bright) / static_cast<float>(total) : 0.0f;
}

} // namespace

// ---------------------------------------------------------------------------
// Six-face coverage test.
//
// Renders the 40x40x40 rainbow cube from 6 orthogonal camera positions and
// verifies that the centre of each face view contains non-black pixels
// (i.e. the isosurface triangles were generated for that face).
//
// PADDING = ir * 1 (live-app bug): back, left, top faces are missing.
// PADDING = ir * 2 (fix): all 6 faces are present.
// ---------------------------------------------------------------------------
TEST(MCCubeIntegrityTest, AllSixFacesHaveCoverage)
{
    namespace fs = std::filesystem;
    fs::create_directories("artifacts");

    SDL3Context ctx(VRTestConfig::RENDER_WIDTH, VRTestConfig::RENDER_HEIGHT, "MC Cube Integrity", false);
    if (!ctx.isValid()) {
        GTEST_SKIP() << "No GL context available";
    }
    ctx.makeCurrent();

    if (GLAD_GL_VERSION_4_3 == 0) {
        GTEST_SKIP() << "GL 4.3 not available";
    }

    auto [fb_w, fb_h] = ctx.getFramebufferSize();
    glViewport(0, 0, fb_w, fb_h);
    glEnable(GL_DEPTH_TEST);

    FramebufferCapture fb(static_cast<uint32_t>(fb_w), static_cast<uint32_t>(fb_h));
    ASSERT_TRUE(fb.initialize());

    ComputeShader density_sh(getShaderPath("density_field.comp").c_str());
    ASSERT_NE(density_sh.program(), 0u) << "density_field.comp failed";
    ComputeShader mc_sh(getShaderPath("marching_cubes.comp").c_str());
    ASSERT_NE(mc_sh.program(), 0u) << "marching_cubes.comp failed";
    Shader mesh_sh(getShaderPath("mesh.vert").c_str(), getShaderPath("mesh.frag").c_str());
    if (mesh_sh.Program == 0u) {
        GTEST_SKIP() << "mesh shader failed";
    }

    // 40x40x40 particle lattice matching live-app defaults
    std::vector<glm::vec4> particles;
    particles.reserve(64000);
    constexpr int DIM = 40;
    constexpr float SCALE = 0.075f;
    constexpr float HALF = (DIM - 1) / 2.0f;
    for (int i = 0; i < DIM; ++i) {
        for (int j = 0; j < DIM; ++j) {
            for (int k = 0; k < DIM; ++k) {
                float x = (static_cast<float>(i) - HALF) * SCALE;
                float y = (static_cast<float>(j) - HALF) * SCALE;
                float z = (static_cast<float>(k) - HALF) * SCALE;
                particles.emplace_back(x, y, z, 500.0f);
            }
        }
    }

    const float ir = 0.5f;
    const float iso = 0.58f;
    const int grid_res = 64;

    // Compute bounding box with ir*2 padding -- the fix applied to viewer_app.cpp.
    // With ir*1 (the original bug), back/left/top face centres read black.
    glm::vec3 bmin(FLT_MAX), bmax(-FLT_MAX);
    for (const auto& p : particles) {
        bmin = glm::min(bmin, glm::vec3(p));
        bmax = glm::max(bmax, glm::vec3(p));
    }
    bmin -= glm::vec3(ir * 2.0f);
    bmax += glm::vec3(ir * 2.0f);
    const glm::vec3 ext = bmax - bmin;
    const float voxel_sz = glm::max(ext.x, glm::max(ext.y, ext.z)) / static_cast<float>(grid_res);

    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(VRTestConfig::RENDER_WIDTH) / static_cast<float>(VRTestConfig::RENDER_HEIGHT), 0.1f, 100.0f);

    // Camera at distance 5.5: coverage = 5.5 * tan(22.5 deg) = 2.28 > cube half-extent 1.96
    constexpr float D = 5.5f;
    struct CamView
    {
        const char* label;
        glm::vec3 eye;
        glm::vec3 up;
    };
    const CamView views[] = {
        {"front", {0, 0, D}, {0, 1, 0}}, {"back", {0, 0, -D}, {0, 1, 0}}, {"left", {-D, 0, 0}, {0, 1, 0}},
        {"right", {D, 0, 0}, {0, 1, 0}}, {"top", {0, D, 0}, {0, 0, -1}},  {"bottom", {0, -D, 0}, {0, 0, 1}},
    };

    // Each face is expected to cover >= 30% of the centre third.
    // Background (black) pixels indicate missing surface triangles.
    constexpr float MIN_COVERAGE = 0.30f;

    for (const auto& v : views) {
        MCRenderer mc(grid_res);
        mc.markDirty();

        glm::mat4 view = glm::lookAt(v.eye, glm::vec3(0.0f), v.up);

        fb.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mc.render(particles, bmin, voxel_sz, ir, iso, density_sh.program(), mc_sh.program(), mesh_sh.Program, proj,
                  view);

        Image img = fb.capture();
        ASSERT_TRUE(img.valid()) << "Framebuffer capture failed for face: " << v.label;

        const std::string out = std::string("artifacts/integrity_cube_") + v.label + ".png";
        img.save(out, ImageFormat::PNG);

        const float coverage = centreBrightnessFraction(img);
        EXPECT_GE(coverage, MIN_COVERAGE) << "Face '" << v.label << "' has only " << (coverage * 100.0f)
                                          << "% centre coverage -- back/left/top faces are empty when the MC "
                                          << "bounding box is padded by ir*1 instead of ir*2. "
                                          << "Fix: change viewer_app.cpp mc_bbox_min_ -= glm::vec3(mc_scaled_ir_) "
                                          << "to mc_bbox_min_ -= glm::vec3(mc_scaled_ir_ * 2.0f).";
    }
}

#endif // !__APPLE__
