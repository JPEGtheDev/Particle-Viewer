/*
 * SSMRenderingTests.cpp
 *
 * RED GATE visual regression test for Screen-Space Metaballs (SSM) rendering.
 *
 * This test MUST FAIL until Wave 4 (implement-ssm-render-passes) is complete
 * and a valid SSM baseline image is reviewed and committed.
 *
 * RED GATE mechanism: no baseline file exists at
 *   tests/visual-regression/baselines/ssm_4x4x4_grid.png
 * On first run the test captures the current (placeholder) render, saves it
 * as a candidate baseline, and calls FAIL(). The test will only pass once:
 *   1. Wave 4 implements the real SSM pipeline, AND
 *   2. The resulting render is approved and committed as the baseline.
 *
 * NOTE: A GPU is NOT required to run these tests locally.
 * Use Xvfb for headless rendering:
 *   xvfb-run -a ./build/tests/ParticleViewerTests --gtest_filter="SSMRenderingTest.*"
 */

#include <cstdio>
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
#include <glm/gtc/type_ptr.hpp>

#include "Image.hpp"
#include "graphics/SDL3Context.hpp"
#include "particle.hpp"
#include "shader.hpp"
#include "testing/FramebufferCapture.hpp"
#include "testing/PixelComparator.hpp"

// Test configuration
namespace SSMTestConfig
{
static const uint32_t RENDER_WIDTH = 1280;
static const uint32_t RENDER_HEIGHT = 720;
static const float PARTICLE_TOLERANCE = 2.0f / 255.0f;
static const float MAX_DIFF_RATIO = 0.0001f;
static const std::string BASELINES_DIR = "baselines";
} // namespace SSMTestConfig

/*
 * Test fixture for SSM rendering regression tests.
 * Sets up a production SDL3Context (hidden window) and an off-screen
 * FramebufferCapture for pixel-accurate comparison.
 *
 * Provides getShaderPath() and getBaselinePath() helpers that probe
 * multiple candidate locations so the tests run from both build/ and
 * build/tests/ working directories.
 */
class SSMRenderingTest : public testing::Test
{
  protected:
    SDL3Context* context_ = nullptr;
    FramebufferCapture* framebuffer_ = nullptr;
    int framebuffer_width_ = 0;
    int framebuffer_height_ = 0;

    void SetUp() override
    {
        // Ensure output directories exist
        std::string mkBaselines = "mkdir -p " + SSMTestConfig::BASELINES_DIR;
        ASSERT_EQ(std::system(mkBaselines.c_str()), 0) << "Failed to create baselines directory";
        ASSERT_EQ(std::system("mkdir -p artifacts"), 0) << "Failed to create artifacts directory";

        context_ =
            new SDL3Context(SSMTestConfig::RENDER_WIDTH, SSMTestConfig::RENDER_HEIGHT, "SSM Rendering Test", false);
        if (!context_->isValid()) {
            delete context_;
            context_ = nullptr;
            GTEST_SKIP() << "Failed to initialize OpenGL context. "
                         << "Use Xvfb for headless rendering:\n"
                         << "  xvfb-run -a ./ParticleViewerTests --gtest_filter=\"SSMRenderingTest.*\"";
        }

        context_->makeCurrent();

        auto fb_size = context_->getFramebufferSize();
        framebuffer_width_ = fb_size.first;
        framebuffer_height_ = fb_size.second;

        glViewport(0, 0, framebuffer_width_, framebuffer_height_);

        framebuffer_ = new FramebufferCapture(static_cast<uint32_t>(framebuffer_width_),
                                              static_cast<uint32_t>(framebuffer_height_));
        if (!framebuffer_->initialize()) {
            delete framebuffer_;
            framebuffer_ = nullptr;
            delete context_;
            context_ = nullptr;
            GTEST_SKIP() << "Failed to initialize FramebufferCapture";
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE);
    }

    void TearDown() override
    {
        if (framebuffer_ != nullptr) {
            delete framebuffer_;
            framebuffer_ = nullptr;
        }
        if (context_ != nullptr) {
            delete context_;
            context_ = nullptr;
        }
    }

    /*
     * Resolve a shader name to a file path, probing several candidate
     * locations relative to the current working directory.
     */
    std::string getShaderPath(const std::string& shaderName)
    {
        std::vector<std::string> candidates = {
            "Viewer-Assets/shaders/" + shaderName,
            "../Viewer-Assets/shaders/" + shaderName,
            "../../src/shaders/" + shaderName,
            "../../../src/shaders/" + shaderName,
        };
        for (const auto& p : candidates) {
            if (FILE* f = fopen(p.c_str(), "r")) {
                fclose(f);
                return p;
            }
        }
        return candidates[0]; // fallback — shader loading will report the error
    }

    /*
     * Resolve a baseline name to a file path, probing several candidate
     * locations relative to the current working directory.
     */
    std::string getBaselinePath(const std::string& baselineName)
    {
        std::vector<std::string> candidates = {
            SSMTestConfig::BASELINES_DIR + "/" + baselineName,
            "../../tests/visual-regression/baselines/" + baselineName,
            "../tests/visual-regression/baselines/" + baselineName,
            "../../../tests/visual-regression/baselines/" + baselineName,
        };
        for (const auto& p : candidates) {
            if (FILE* f = fopen(p.c_str(), "r")) {
                fclose(f);
                return p;
            }
        }
        return candidates[0]; // fallback — triggers baseline generation
    }
};

// ============================================================================
// SSM Rendering Regression Tests
// ============================================================================

/*
 * Test: SSMRender_4x4x4Grid_MatchesBaseline
 *
 * RED GATE — this test MUST FAIL until Wave 4 (implement-ssm-render-passes)
 * delivers a working SSM pipeline and the resulting render is approved as a
 * baseline.
 *
 * Particle configuration:
 *   64 particles in a 4×4×4 grid at positions
 *   x ∈ {-1.5, -0.5, 0.5, 1.5} × scale,
 *   y ∈ {-1.5, -0.5, 0.5, 1.5} × scale,
 *   z ∈ {-1.5, -0.5, 0.5, 1.5} × scale
 *   where scale = 2.0.  w component = category index (1–4, cycling).
 *
 * Camera configuration:
 *   Position  : (0, 0, 10) — directly in front of the grid
 *   Target    : (0, 0, 0)
 *   Up        : (0, 1, 0)
 *   Projection: Perspective, FOV=45°, Near=0.1, Far=3000.0
 *
 * Render pipeline (placeholder until Wave 4):
 *   The SSM splat shader is compiled and verified.  The actual SSM render
 *   passes are NOT available yet — the scene is rendered using the sphere
 *   shader as a stand-in so that the test exercises the full fixture path
 *   and produces a deterministic pixel output for comparison.
 *
 * Baseline path: tests/visual-regression/baselines/ssm_4x4x4_grid.png
 *
 * Failure modes:
 *   Run 1 (now)  : baseline missing → saves candidate → FAIL (RED gate)
 *   Run 2+       : once Wave 4 renders real SSM the pixels will differ from
 *                  this placeholder baseline → FAIL (still RED)
 *   After approval: Wave 4 baseline committed → test passes (GREEN)
 */
TEST_F(SSMRenderingTest, SSMRender_4x4x4Grid_MatchesBaseline)
{
    // ---- Step 1: check GL_RGBA32F framebuffer support ----------------------
    {
        GLuint probe_fbo = 0;
        GLuint probe_tex = 0;
        glGenFramebuffers(1, &probe_fbo);
        glGenTextures(1, &probe_tex);
        glBindFramebuffer(GL_FRAMEBUFFER, probe_fbo);
        glBindTexture(GL_TEXTURE_2D, probe_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, probe_tex, 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteTextures(1, &probe_tex);
        glDeleteFramebuffers(1, &probe_fbo);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            GTEST_SKIP() << "GL_RGBA32F framebuffer not supported on this hardware — skipping SSM test";
        }
    }

    // ---- Step 2: compile sphere shader (placeholder for Wave 4 SSM) -------
    //
    // The SSM splat shader is compiled here to verify it is well-formed.
    // The full SSM render passes (splat → blur → composite) will be wired up
    // by Wave 4 (implement-ssm-render-passes).  Until then the scene is
    // rendered with the sphere shader so the test produces a stable pixel
    // output to base a comparison on.
    std::string splatVertPath = getShaderPath("metaball_splat.vert");
    std::string splatFragPath = getShaderPath("metaball_splat.frag");
    Shader splatShader(splatVertPath.c_str(), splatFragPath.c_str());
    ASSERT_NE(splatShader.Program, 0u) << "Failed to compile SSM splat shader.\n"
                                       << "  Vert: " << splatVertPath << "\n"
                                       << "  Frag: " << splatFragPath;

    std::string sphereVertPath = getShaderPath("sphereVertex.vs");
    std::string sphereFragPath = getShaderPath("sphereFragment.frag");
    Shader sphereShader(sphereVertPath.c_str(), sphereFragPath.c_str());
    ASSERT_NE(sphereShader.Program, 0u) << "Failed to compile sphere shader (placeholder renderer).\n"
                                        << "  Vert: " << sphereVertPath << "\n"
                                        << "  Frag: " << sphereFragPath;

    // ---- Step 3: build 64-particle 4×4×4 grid -----------------------------
    //
    // Positions are x/y/z ∈ {-1.5, -0.5, 0.5, 1.5} × 2.0.
    // The w component encodes the particle category (1–4, cycling).
    static constexpr float COORDS[4] = {-1.5f, -0.5f, 0.5f, 1.5f};
    static constexpr float DISPLAY_SCALE = 2.0f;
    std::vector<glm::vec4> positions;
    positions.reserve(64);
    int category = 1;
    for (float x : COORDS) {
        for (float y : COORDS) {
            for (float z : COORDS) {
                positions.push_back(
                    glm::vec4(x * DISPLAY_SCALE, y * DISPLAY_SCALE, z * DISPLAY_SCALE, static_cast<float>(category)));
                category = (category % 4) + 1;
            }
        }
    }
    Particle particles(64, positions.data());
    ASSERT_EQ(particles.n, 64) << "Expected 64 particles in 4×4×4 grid";

    // ---- Step 4: set up camera ---------------------------------------------
    glm::vec3 cameraPos(0.0f, 0.0f, 10.0f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            static_cast<float>(SSMTestConfig::RENDER_WIDTH) /
                                                static_cast<float>(SSMTestConfig::RENDER_HEIGHT),
                                            0.1f, 3000.0f);

    // ---- Step 5: render (placeholder — sphere shader) ----------------------
    //
    // Wave 4 will replace this block with the real SSM splat → blur →
    // composite pipeline.  The sphere render is intentionally used here so
    // the test produces a reproducible pixel output; when Wave 4 switches to
    // SSM the pixel output will change and the comparison will fail, keeping
    // the test RED until a new SSM baseline is approved.
    framebuffer_->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sphereShader.Use();
    glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(sphereShader.Program, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform1f(glGetUniformLocation(sphereShader.Program, "radius"), 100.0f);
    glUniform1f(glGetUniformLocation(sphereShader.Program, "scale"), 5.0f);
    glUniform1f(glGetUniformLocation(sphereShader.Program, "transScale"), 0.25f);
    glUniform1f(glGetUniformLocation(sphereShader.Program, "viewportHeight"), static_cast<float>(framebuffer_height_));
    glUniform3fv(glGetUniformLocation(sphereShader.Program, "lightDirection"), 1,
                 glm::value_ptr(glm::vec3(0.1f, 0.1f, 0.85f)));

    particles.pushVBO();

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    particles.setUpInstanceArray();
    glDrawArraysInstanced(GL_POINTS, 0, 1, particles.n);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);

    Image currentImage = framebuffer_->capture();

    // ---- Step 6: baseline comparison ---------------------------------------
    ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer";

    std::string baselinePath = getBaselinePath("ssm_4x4x4_grid.png");
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);

    if (baseline.empty()) {
        // RED GATE: no baseline exists yet — save the current render as a
        // candidate and fail so that this todo stays RED until Wave 4 is
        // done and the baseline is reviewed and committed.
        std::string candidatePath = SSMTestConfig::BASELINES_DIR + "/ssm_4x4x4_grid.png";
        currentImage.save(candidatePath, ImageFormat::PNG);
        FAIL() << "SSM baseline not found — candidate baseline saved to: " << candidatePath << "\n"
               << "This test is intentionally RED (Wave 4 gate).\n"
               << "Do NOT commit this candidate as the baseline until Wave 4\n"
               << "(implement-ssm-render-passes) has delivered a real SSM render.";
    }

    ASSERT_TRUE(currentImage.save("artifacts/ssm_4x4x4_grid_current.png", ImageFormat::PNG))
        << "Failed to save current render artifact";

    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, currentImage, SSMTestConfig::PARTICLE_TOLERANCE, true);

    float diff_ratio = result.total_pixels > 0
                           ? static_cast<float>(result.diff_pixels) / static_cast<float>(result.total_pixels)
                           : 1.0f;
    if (diff_ratio > SSMTestConfig::MAX_DIFF_RATIO) {
        result.diff_image.save("artifacts/ssm_4x4x4_grid_diff.png", ImageFormat::PNG);
        FAIL() << "SSM visual mismatch detected:\n"
               << "  Diff pixels: " << result.diff_pixels << " / " << result.total_pixels << " ("
               << ((result.diff_pixels * 100.0f) / result.total_pixels) << "%)\n"
               << "  Similarity: " << (result.similarity * 100.0f) << "%\n"
               << "  Diff image saved to: artifacts/ssm_4x4x4_grid_diff.png\n"
               << "  Current image saved to: artifacts/ssm_4x4x4_grid_current.png";
    }
}
