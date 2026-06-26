/*
 * mc_render_tests.cpp
 *
 * Visual regression test for Marching Cubes rendering with a 64-particle 4x4x4 grid.
 *
 * Requires GL 4.3 compute shaders. Skips automatically if the runtime does not support them.
 *
 * Baseline management:
 *   - First run (no baseline): saves the rendered frame and skips (PASS).
 *   - Subsequent runs: compares against the saved baseline and fails if mismatch
 *     exceeds VRTestConfig::MAX_DIFF_RATIO.
 *
 * Baseline file: tests/visual-regression/baselines/mc_64_grid.png
 */

#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// clang-format off
// GLAD must come before SDL3 to properly initialize OpenGL functions
#include "glad/glad.h"       // NOLINT(llvm-include-order)
#include <SDL3/SDL.h>        // NOLINT(llvm-include-order)
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Image.hpp"
#include "MCRenderer.hpp"
#include "VRTestCommon.hpp"
#include "graphics/SDL3Context.hpp"
#include "shader.hpp"
#include "testing/FramebufferCapture.hpp"
#include "testing/PixelComparator.hpp"
#include "visual-regression/fixtures/grid4x4x4_fixture.hpp"

// ============================================================================
// MC Rendering Regression Tests
// ============================================================================

/*
 * Test fixture for Marching Cubes rendering regression tests.
 * Sets up an SDL3Context (hidden window) and a FramebufferCapture for off-screen
 * rendering, following the same pattern as RenderingRegressionTests.cpp.
 *
 * mesh_shader_ is constructed in SetUp() after the GL context is active, using
 * the Shader class (same as production code in viewer_app.cpp).
 */
class MarchingCubesVRTest : public testing::Test
{
  protected:
    static constexpr int MC_GRID_RESOLUTION = 64; // voxels per side (matches Grid64 menu option)

    SDL3Context* context_ = nullptr;
    FramebufferCapture* framebuffer_ = nullptr;
    Shader mesh_shader_;

    void SetUp() override
    {
        namespace fs = std::filesystem;

        // Ensure baselines directory exists
        const std::string baselines_dir = VRTestConfig::BASELINES_DIR;
        fs::create_directories(baselines_dir);
        fs::create_directories("artifacts");

        // Initialize production SDL3Context (hidden window)
        context_ = new SDL3Context(VRTestConfig::RENDER_WIDTH, VRTestConfig::RENDER_HEIGHT, "MC VR Test", false);
        if (!context_->isValid()) {
            delete context_;
            context_ = nullptr;
            GTEST_SKIP() << "Failed to initialize OpenGL context. "
                         << "Use Xvfb for headless rendering:\n"
                         << "  xvfb-run -a ./ParticleViewerTests --gtest_filter=\"MarchingCubesVRTest.*\"";
        }

        context_->makeCurrent();

        // Check for GL 4.3 compute shader support before allocating GPU resources
        if (GLAD_GL_VERSION_4_3 == 0) {
            GTEST_SKIP() << "Skipping: GL 4.3 compute shaders not available";
        }

        auto fb_size = context_->getFramebufferSize();
        int fb_width = fb_size.first;
        int fb_height = fb_size.second;

        glViewport(0, 0, fb_width, fb_height);

        framebuffer_ = new FramebufferCapture(static_cast<uint32_t>(fb_width), static_cast<uint32_t>(fb_height));
        if (!framebuffer_->initialize()) {
            delete framebuffer_;
            framebuffer_ = nullptr;
            GTEST_SKIP() << "Failed to initialize FramebufferCapture";
        }

        glEnable(GL_DEPTH_TEST);
        // GL_CULL_FACE is intentionally absent here: MCRenderer::render() enables and
        // restores it internally (MCRenderer.cpp:249-257) around every draw call.

        // Compile mesh vertex + fragment shaders using the Shader class
        // (mirrors how viewer_app.cpp constructs render programs)
        const std::string mesh_vert_path = getShaderPath("mesh.vert");
        const std::string mesh_frag_path = getShaderPath("mesh.frag");
        mesh_shader_ = Shader(mesh_vert_path.c_str(), mesh_frag_path.c_str());
        ASSERT_NE(mesh_shader_.Program, 0u) << "Failed to compile/link mesh shader program. "
                                            << "Vertex: " << mesh_vert_path << ", Fragment: " << mesh_frag_path;
    }

    void TearDown() override
    {
        if (mesh_shader_.Program != 0u) {
            glDeleteProgram(mesh_shader_.Program);
            mesh_shader_.Program = 0u;
        }
        if (framebuffer_ != nullptr) {
            delete framebuffer_;
            framebuffer_ = nullptr;
        }
        if (context_ != nullptr) {
            delete context_;
            context_ = nullptr;
        }
    }
};

/*
 * Test: MarchingCubesRenders_64ParticleGrid_MatchesBaseline
 *
 * Renders a 64-particle 4x4x4 grid in Marching Cubes mode and compares the
 * output against a stored baseline image.
 *
 * MC Parameters (match viewer_app.cpp defaults):
 *   - influence_radius (display space): 2.0 * 0.25 = 0.5
 *   - iso_value: 0.5
 *   - grid_res: 64
 *
 * Camera (from grid4x4x4::CAMERA):
 *   - Position: (4.7, 6.1, 5.4) -- ~9.4 units from origin
 *   - Target: (0, 0, 0)
 *   - FOV: 45 degrees
 *
 * On first run: saves the rendered frame as the baseline and SKIPs.
 * On subsequent runs: compares with tolerance and FAILs if mismatch exceeds threshold.
 */
TEST_F(MarchingCubesVRTest, MarchingCubesRenders_64ParticleGrid_MatchesBaseline)
{
    // Arrange -- load compute shaders
    const std::string density_path = getShaderPath("density_field.comp");
    const std::string mc_path = getShaderPath("marching_cubes.comp");

    ComputeShader density_shader(density_path.c_str());
    ASSERT_NE(density_shader.program(), 0u) << "Failed to compile density_field.comp: " << density_path;

    ComputeShader mc_shader(mc_path.c_str());
    ASSERT_NE(mc_shader.program(), 0u) << "Failed to compile marching_cubes.comp: " << mc_path;

    // Arrange -- particle data and MC parameters
    // Fixture particles are already in display space (SIM_TO_DISPLAY_SCALE applied internally).
    std::vector<glm::vec4> particles = grid4x4x4::makeParticles();
    ASSERT_EQ(static_cast<int>(particles.size()), grid4x4x4::GRID_DIM * grid4x4x4::GRID_DIM * grid4x4x4::GRID_DIM)
        << "Expected " << (grid4x4x4::GRID_DIM * grid4x4x4::GRID_DIM * grid4x4x4::GRID_DIM)
        << " particles from 4x4x4 grid";

    // Match viewer_app.cpp defaults:
    //   influence_radius (sim space) = 2.0, kSimToDisplayScale = 0.25
    //   mc_scaled_ir = 2.0 * 0.25 = 0.5 (display space)
    const float mc_scaled_ir = 0.5f;
    const float iso_value = 0.5f;
    const int grid_res = MC_GRID_RESOLUTION;

    // Compute grid origin and voxel size from display-space particle bbox
    // (mirrors viewer_app.cpp lines 351-366)
    glm::vec3 bbox_min(FLT_MAX);
    glm::vec3 bbox_max(-FLT_MAX);
    for (const auto& p : particles) {
        glm::vec3 pos(p.x, p.y, p.z);
        bbox_min = glm::min(bbox_min, pos);
        bbox_max = glm::max(bbox_max, pos);
    }
    bbox_min -= glm::vec3(mc_scaled_ir);
    bbox_max += glm::vec3(mc_scaled_ir);
    const glm::vec3 extent = bbox_max - bbox_min;
    const float grid_f = static_cast<float>(grid_res);
    const float voxel_size = glm::max(extent.x, glm::max(extent.y, extent.z)) / grid_f;
    const glm::vec3 grid_origin = bbox_min;

    // Arrange -- camera (from grid4x4x4::CAMERA)
    const grid4x4x4::CameraConfig& cam = grid4x4x4::CAMERA;
    glm::mat4 view = glm::lookAt(cam.position, cam.target, cam.up);
    glm::mat4 projection = glm::perspective(glm::radians(cam.fov_degrees),
                                            static_cast<float>(VRTestConfig::RENDER_WIDTH) /
                                                static_cast<float>(VRTestConfig::RENDER_HEIGHT),
                                            cam.near_clip, cam.far_clip);

    // Act -- construct MCRenderer and render one frame
    SpatialGridSSBOs sg_rt;
    sg_rt.build(particles, grid_origin, extent, mc_scaled_ir);

    MCRenderer mc_renderer(grid_res);
    mc_renderer.markDirty();

    framebuffer_->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mc_renderer.render(particles, grid_origin, voxel_size, mc_scaled_ir, iso_value, density_shader.program(),
                       mc_shader.program(), mesh_shader_.Program, projection, view, sg_rt.cell_starts_ssbo,
                       sg_rt.sorted_particles_ssbo, sg_rt.grid.cell_size, sg_rt.grid.cell_origin,
                       sg_rt.grid.num_cells_x, sg_rt.grid.num_cells_y, sg_rt.grid.num_cells_z);

    Image current_image = framebuffer_->capture();

    // Assert -- image validity
    ASSERT_TRUE(current_image.valid()) << "Failed to capture framebuffer";

    // Baseline management
    const std::string baseline_name = "mc_64_grid.png";
    const std::string local_baseline_path = VRTestConfig::BASELINES_DIR + "/" + baseline_name;
    std::string baseline_path = getBaselinePath(baseline_name);
    Image baseline = Image::load(baseline_path, ImageFormat::PNG);

    if (baseline.empty()) {
        // First run: save baseline and skip
        ASSERT_TRUE(current_image.save(local_baseline_path, ImageFormat::PNG))
            << "Failed to save baseline to: " << local_baseline_path;
        GTEST_SKIP() << "Baseline saved to: " << local_baseline_path
                     << "\nPlease review and commit this baseline if correct."
                     << "\nRun again to compare.";
    }

    // Subsequent runs: save artifact and compare
    ASSERT_TRUE(current_image.save("artifacts/mc_64_grid_current.png", ImageFormat::PNG))
        << "Failed to save current render artifact";

    // MC uses atomic-counter writes: triangle draw order is non-deterministic across
    // GPU runs. At exactly co-planar shared edges, the last-drawn triangle wins
    // the depth test, causing speckled pixel differences between runs (~2-3%).
    // This tolerance catches real regressions (blank screen, wrong shape, wrong color)
    // while accepting the inherent compute-order noise.
    constexpr float kMCMaxDiffRatio = 0.05f; // 5% -- covers ~2-3% compute non-determinism

    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, current_image, VRTestConfig::PARTICLE_TOLERANCE, true);

    float diff_ratio = result.total_pixels > 0
                           ? static_cast<float>(result.diff_pixels) / static_cast<float>(result.total_pixels)
                           : 1.0f;
    if (diff_ratio > kMCMaxDiffRatio) {
        result.diff_image.save("artifacts/mc_64_grid_diff.png", ImageFormat::PNG);
        FAIL() << "Visual mismatch detected:\n"
               << "  Diff pixels: " << result.diff_pixels << " / " << result.total_pixels << " ("
               << ((result.diff_pixels * 100.0f) / result.total_pixels) << "%)\n"
               << "  Similarity: " << (result.similarity * 100.0f) << "%\n"
               << "  Diff image saved to: artifacts/mc_64_grid_diff.png\n"
               << "  Current image saved to: artifacts/mc_64_grid_current.png";
    }
}
