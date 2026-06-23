/*
 * mc_render_gallery_tests.cpp
 *
 * Quick-render tests for visual inspection of MC rendering modes.
 * No baseline comparison -- saves images to artifacts/ on every run and always passes.
 *
 * Run with:
 *   cd build && ./tests/ParticleViewerTests --gtest_filter="MCRenderGallery.*"
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

struct GalleryContext
{
    SDL3Context* ctx = nullptr;
    FramebufferCapture* fb = nullptr;
    Shader mesh_shader;

    bool init(const char* title)
    {
        namespace fs = std::filesystem;
        fs::create_directories("artifacts");

        ctx = new SDL3Context(VRTestConfig::RENDER_WIDTH, VRTestConfig::RENDER_HEIGHT, title, false);
        if (!ctx->isValid()) {
            return false;
        }
        ctx->makeCurrent();

        if (GLAD_GL_VERSION_4_3 == 0) {
            return false;
        }

        auto [w, h] = ctx->getFramebufferSize();
        glViewport(0, 0, w, h);

        fb = new FramebufferCapture(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
        if (!fb->initialize()) {
            return false;
        }

        glEnable(GL_DEPTH_TEST);

        mesh_shader = Shader(getShaderPath("mesh.vert").c_str(), getShaderPath("mesh.frag").c_str());
        return mesh_shader.Program != 0u;
    }

    ~GalleryContext()
    {
        if (mesh_shader.Program != 0u) {
            glDeleteProgram(mesh_shader.Program);
        }
        delete fb;
        delete ctx;
    }
};

bool renderAndSave(GalleryContext& gc, const std::vector<glm::vec4>& particles, const glm::vec3& grid_origin,
                   float voxel_size, float ir, float iso, int grid_res, const glm::mat4& proj, const glm::mat4& view,
                   const std::string& out_path)
{
    const std::string density_path = getShaderPath("density_field.comp");
    const std::string mc_path = getShaderPath("marching_cubes.comp");

    ComputeShader density_sh(density_path.c_str());
    ComputeShader mc_sh(mc_path.c_str());
    if (density_sh.program() == 0u || mc_sh.program() == 0u) {
        return false;
    }

    MCRenderer renderer(grid_res);
    renderer.markDirty();

    gc.fb->bind();
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // dark grey background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer.render(particles, grid_origin, voxel_size, ir, iso, density_sh.program(), mc_sh.program(),
                    gc.mesh_shader.Program, proj, view);

    Image img = gc.fb->capture();
    if (!img.valid()) {
        return false;
    }
    return img.save(out_path, ImageFormat::PNG);
}

glm::vec3 bboxMin(const std::vector<glm::vec4>& pts)
{
    glm::vec3 mn(FLT_MAX);
    for (const auto& p : pts) {
        mn = glm::min(mn, glm::vec3(p));
    }
    return mn;
}

glm::vec3 bboxMax(const std::vector<glm::vec4>& pts)
{
    glm::vec3 mx(-FLT_MAX);
    for (const auto& p : pts) {
        mx = glm::max(mx, glm::vec3(p));
    }
    return mx;
}

} // namespace

// ---------------------------------------------------------------------------
// Rainbow cube -- 40x40x40 = 64000-particle lattice, category 500 (debug coloring).
// Parameters match live app behavior: radius=2.0 in UI -> ir=0.5 in shader space
// (kSimToDisplayScale=0.25), iso=0.58 (user-confirmed working value).
// Bounding box padded by ir*2 to ensure the isosurface (at ~1.18*ir from particle
// centers) fits inside the grid.
// ---------------------------------------------------------------------------
TEST(MCRenderGallery, RainbowCube_64kParticles)
{
    GalleryContext gc;
    if (!gc.init("Gallery: Rainbow Cube 64k")) {
        GTEST_SKIP() << "No GL 4.3 context";
    }

    // 40x40x40 lattice; spacing=0.075, ir=0.5.  All blobs merge into one
    // solid cloud; the iso surface forms a smooth rounded cube.
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

    const float ir = 0.5f;   // matches live app radius=2.0 * kSimToDisplayScale=0.25
    const float iso = 0.58f; // user-confirmed working iso value
    const int grid_res = 64;

    // Pad by ir*2 so the isosurface (at ~1.18*ir from outermost particles) is
    // fully inside the grid even with density contributions from multiple layers.
    glm::vec3 bmin = bboxMin(particles) - glm::vec3(ir * 2.0f);
    glm::vec3 bmax = bboxMax(particles) + glm::vec3(ir * 2.0f);
    glm::vec3 ext = bmax - bmin;
    float voxel_size = glm::max(ext.x, glm::max(ext.y, ext.z)) / static_cast<float>(grid_res);

    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(VRTestConfig::RENDER_WIDTH) / static_cast<float>(VRTestConfig::RENDER_HEIGHT), 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(4.7f, 6.1f, 5.4f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const std::string out = "artifacts/gallery_rainbow_cube.png";
    ASSERT_TRUE(renderAndSave(gc, particles, bmin, voxel_size, ir, iso, grid_res, proj, view, out))
        << "Failed to render/save rainbow cube 64k";

    std::cout << "Rainbow cube (64k particles) saved: " << out << "\n";
}

// ---------------------------------------------------------------------------
// Two particles merging -- red (category 0) left, blue (category 1) right.
// influence_radius chosen so their density fields overlap at the midpoint,
// producing a merged isosurface with a color gradient from red to blue.
// ---------------------------------------------------------------------------
TEST(MCRenderGallery, TwoParticlesMerging)
{
    GalleryContext gc;
    if (!gc.init("Gallery: Two Particles Merging")) {
        GTEST_SKIP() << "No GL 4.3 context";
    }

    // Two particles 0.8 apart; ir=0.5 -> midpoint density = 2*exp(-0.5*(0.4/0.5)^2) ~= 1.45
    // Well above iso=0.5 -> clearly merged with visible bridge
    std::vector<glm::vec4> particles = {
        {-0.4f, 0.0f, 0.0f, 0.0f}, // red
        {0.4f, 0.0f, 0.0f, 1.0f},  // blue
    };

    const float ir = 0.5f;
    const float iso = 0.5f;
    const int grid_res = 32;

    glm::vec3 bmin = bboxMin(particles) - glm::vec3(ir);
    glm::vec3 bmax = bboxMax(particles) + glm::vec3(ir);
    glm::vec3 ext = bmax - bmin;
    float voxel_size = glm::max(ext.x, glm::max(ext.y, ext.z)) / static_cast<float>(grid_res);

    // Camera: angled view showing the merge bridge
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(VRTestConfig::RENDER_WIDTH) / static_cast<float>(VRTestConfig::RENDER_HEIGHT), 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(3.0f, 1.5f, 3.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    const std::string out = "artifacts/gallery_two_particles_merging.png";
    ASSERT_TRUE(renderAndSave(gc, particles, bmin, voxel_size, ir, iso, grid_res, proj, view, out))
        << "Failed to render/save two-particle merge";

    std::cout << "Two particles merging saved: " << out << "\n";

    // Side view: camera along +Z looking at origin.
    // Particles are separated in X, so this angle shows them side-by-side with the
    // red-to-blue gradient and merge bridge clearly visible in the centre.
    glm::mat4 view_side = glm::lookAt(glm::vec3(0.0f, 0.5f, 4.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    const std::string out_side = "artifacts/gallery_two_particles_side.png";
    ASSERT_TRUE(renderAndSave(gc, particles, bmin, voxel_size, ir, iso, grid_res, proj, view_side, out_side))
        << "Failed to render/save two-particle merge side view";

    std::cout << "Two particles side view saved: " << out_side << "\n";
}

// ---------------------------------------------------------------------------
// Bridging effect sweep -- red (category 0) + blue (category 1), 9 separation
// steps from clearly separate down to fully merged.  Matches the reference
// montage showing: separate spheres -> bridge formation -> color mix.
//
// With a truncated-Gaussian density field (hard cutoff at ir), the merge
// threshold is d = 2*ir = 1.0.  Steps below 1.0 show the bridge forming.
// Camera is a side view (along Z) so left=red and right=blue are clear.
// ---------------------------------------------------------------------------
TEST(MCRenderGallery, BridgingEffectSweep)
{
    GalleryContext gc;
    if (!gc.init("Gallery: Bridging Effect Sweep")) {
        GTEST_SKIP() << "No GL 4.3 context";
    }

    const float ir = 0.5f;
    const float iso = 0.5f;
    const int grid_res = 64;

    // Side view: camera along +Z, particles separated in X.
    // Framed for the widest case (d=2.0, surface extends to ~+-1.5 in X).
    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        static_cast<float>(VRTestConfig::RENDER_WIDTH) / static_cast<float>(VRTestConfig::RENDER_HEIGHT), 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.3f, 4.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Merge threshold with truncated Gaussian = 2*ir = 1.0.
    // Steps straddle the threshold to show: separate -> touching -> bridge -> merged.
    struct Step
    {
        float d;
        const char* label;
    };
    const Step steps[] = {
        {2.0f, "start"}, // separate, large gap
        {1.5f, "step1"}, // separate, narrowing gap
        {1.1f, "step2"}, // just barely separate
        {1.0f, "step3"}, // surfaces touching (threshold)
        {0.8f, "step4"}, // bridge just formed
        {0.6f, "step5"}, // clear bridge
        {0.4f, "step6"}, // wide bridge, colour mixing
        {0.2f, "step7"}, // nearly round
        {0.0f, "end"},   // fully merged
    };

    for (const auto& s : steps) {
        std::vector<glm::vec4> particles = {
            {-s.d * 0.5f, 0.0f, 0.0f, 0.0f}, // red
            {s.d * 0.5f, 0.0f, 0.0f, 1.0f},  // blue
        };

        // Pad by ir to ensure the surface fits inside the grid.
        glm::vec3 bmin = bboxMin(particles) - glm::vec3(ir);
        glm::vec3 bmax = bboxMax(particles) + glm::vec3(ir);
        glm::vec3 ext = bmax - bmin;
        float voxel_size = glm::max(ext.x, glm::max(ext.y, ext.z)) / static_cast<float>(grid_res);

        const std::string out = std::string("artifacts/gallery_bridge_") + s.label + ".png";
        ASSERT_TRUE(renderAndSave(gc, particles, bmin, voxel_size, ir, iso, grid_res, proj, view, out))
            << "Failed to render bridge step: " << s.label;

        std::cout << "Bridge sweep [" << s.label << "] sep=" << s.d << " saved: " << out << "\n";
    }
}

#endif // !__APPLE__
