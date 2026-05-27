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
#include <cstdlib>
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
#include "VRTestCommon.hpp"
#include "graphics/SDL3Context.hpp"
#include "particle.hpp"
#include "shader.hpp"
#include "testing/FramebufferCapture.hpp"
#include "testing/PixelComparator.hpp"

/*
 * Test fixture for SSM rendering regression tests.
 * Sets up a production SDL3Context (hidden window) and an off-screen
 * FramebufferCapture for pixel-accurate comparison.
 *
 * Uses getShaderPath() and getBaselinePath() free functions from VRTestCommon.hpp
 * to probe multiple candidate locations so the tests run from both build/ and
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
        std::string mkBaselines = "mkdir -p " + VRTestConfig::BASELINES_DIR;
        ASSERT_EQ(std::system(mkBaselines.c_str()), 0) << "Failed to create baselines directory";
        ASSERT_EQ(std::system("mkdir -p artifacts"), 0) << "Failed to create artifacts directory";

        context_ =
            new SDL3Context(VRTestConfig::RENDER_WIDTH, VRTestConfig::RENDER_HEIGHT, "SSM Rendering Test", false);
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
 *   where scale = 2.0.  w component = category index (0–3, cycling).
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

    // ---- Step 2: compile SSM shaders ----------------------------------------
    //
    // The SSM splat shader is compiled here to verify it is well-formed.
    // The full SSM render passes (splat → blur → composite) are now wired up
    // to match the production ViewerApp pipeline.
    std::string splatVertPath = getShaderPath("metaball_splat.vert");
    std::string splatFragPath = getShaderPath("metaball_splat.frag");
    Shader splatShader(splatVertPath.c_str(), splatFragPath.c_str());
    ASSERT_NE(splatShader.Program, 0u) << "Failed to compile SSM splat shader.\n"
                                       << "  Vert: " << splatVertPath << "\n"
                                       << "  Frag: " << splatFragPath;

    // ---- Step 3: build 64-particle 4×4×4 grid -----------------------------
    //
    // Positions are x/y/z ∈ {-1.5, -0.5, 0.5, 1.5} × 2.0.
    // The w component encodes the particle category (0–3, cycling).
    static constexpr float COORDS[4] = {-1.5f, -0.5f, 0.5f, 1.5f};
    static constexpr float DISPLAY_SCALE = 2.0f;
    std::vector<glm::vec4> positions;
    positions.reserve(64);
    int category = 0;
    for (float x : COORDS) {
        for (float y : COORDS) {
            for (float z : COORDS) {
                positions.push_back(
                    glm::vec4(x * DISPLAY_SCALE, y * DISPLAY_SCALE, z * DISPLAY_SCALE, static_cast<float>(category)));
                category = (category + 1) % 4;
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
                                            static_cast<float>(VRTestConfig::RENDER_WIDTH) /
                                                static_cast<float>(VRTestConfig::RENDER_HEIGHT),
                                            0.1f, 3000.0f);

    // ---- Step 5: render — real SSM pipeline --------------------------------
    // NOTE: mirrors drawSSMScene() — keep in sync if the pipeline order or uniforms change.
    std::string blurVertPath = getShaderPath("metaball_blur.vert");
    std::string blurFragPath = getShaderPath("metaball_blur.frag");
    Shader blurShader(blurVertPath.c_str(), blurFragPath.c_str());
    ASSERT_NE(blurShader.Program, 0u) << "Failed to compile SSM blur shader.\n"
                                      << "  Vert: " << blurVertPath << "\n"
                                      << "  Frag: " << blurFragPath;

    std::string screenVertPath = getShaderPath("screenshader.vs");
    std::string compositeFragPath = getShaderPath("metaball_composite.frag");
    Shader compositeShader(screenVertPath.c_str(), compositeFragPath.c_str());
    ASSERT_NE(compositeShader.Program, 0u) << "Failed to compile SSM composite shader.\n"
                                           << "  Vert: " << screenVertPath << "\n"
                                           << "  Frag: " << compositeFragPath;

    // Create density FBO (GL_RGBA32F)
    GLuint density_fbo = 0;
    GLuint density_tex = 0;
    glGenFramebuffers(1, &density_fbo);
    glGenTextures(1, &density_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, density_fbo);
    glBindTexture(GL_TEXTURE_2D, density_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebuffer_width_, framebuffer_height_, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, density_tex, 0);
    ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE))
        << "Density FBO incomplete";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create blur FBO (GL_RGBA32F)
    GLuint blur_fbo = 0;
    GLuint blur_tex = 0;
    glGenFramebuffers(1, &blur_fbo);
    glGenTextures(1, &blur_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
    glBindTexture(GL_TEXTURE_2D, blur_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebuffer_width_, framebuffer_height_, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blur_tex, 0);
    ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE))
        << "Blur FBO incomplete";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Full-screen quad VAO/VBO (matches screenshader.vs layout)
    static constexpr float QUAD_VERTS[] = {
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f,
    };
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    glGenVertexArrays(1, &quad_vao);
    glGenBuffers(1, &quad_vbo);
    glBindVertexArray(quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTS), QUAD_VERTS, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
    glBindVertexArray(0);

    // VAO for particle point sprites (circle_vao equivalent)
    GLuint circle_vao = 0;
    glGenVertexArrays(1, &circle_vao);

    particles.pushVBO();

    // Splat pass
    glBindFramebuffer(GL_FRAMEBUFFER, density_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    splatShader.Use();
    glBindVertexArray(circle_vao);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, particles.instanceVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glUniformMatrix4fv(glGetUniformLocation(splatShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(splatShader.Program, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform1f(glGetUniformLocation(splatShader.Program, "blobRadius"), 100.0f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "scale"), 5.0f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "transScale"), 0.25f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "viewportHeight"), static_cast<float>(framebuffer_height_));
    glDrawArraysInstanced(GL_POINTS, 0, 1, particles.n);
    glBindVertexArray(0);
    glDisable(GL_BLEND);

    // Blur pass
    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    blurShader.Use();
    glBindVertexArray(quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, density_tex);
    glUniform1i(glGetUniformLocation(blurShader.Program, "densityTexture"), 0);
    glUniform1f(glGetUniformLocation(blurShader.Program, "blurAmount"), 3.0f);
    glUniform2f(glGetUniformLocation(blurShader.Program, "texelSize"), 1.0f / static_cast<float>(framebuffer_width_),
                1.0f / static_cast<float>(framebuffer_height_));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Composite pass into framebuffer_
    framebuffer_->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    compositeShader.Use();
    glBindVertexArray(quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blur_tex);
    glUniform1i(glGetUniformLocation(compositeShader.Program, "blurredDensity"), 0);
    glUniform1f(glGetUniformLocation(compositeShader.Program, "threshold"), 0.5f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glDisable(GL_BLEND);

    // Cleanup intermediate GL resources
    glDeleteVertexArrays(1, &circle_vao);
    glDeleteVertexArrays(1, &quad_vao);
    glDeleteBuffers(1, &quad_vbo);
    glDeleteTextures(1, &density_tex);
    glDeleteFramebuffers(1, &density_fbo);
    glDeleteTextures(1, &blur_tex);
    glDeleteFramebuffers(1, &blur_fbo);

    Image currentImage = framebuffer_->capture();

    // ---- Step 6: baseline comparison ---------------------------------------
    ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer";

    std::string baselinePath = getBaselinePath("ssm_4x4x4_grid.png");
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);

    if (baseline.empty()) {
        // RED GATE: no baseline exists yet — save the current render as a
        // candidate and fail so that this todo stays RED until Wave 4 is
        // done and the baseline is reviewed and committed.
        std::string candidatePath = VRTestConfig::BASELINES_DIR + "/ssm_4x4x4_grid.png";
        currentImage.save(candidatePath, ImageFormat::PNG);
        FAIL() << "SSM baseline not found — candidate baseline saved to: " << candidatePath << "\n"
               << "This test is intentionally RED (Wave 4 gate).\n"
               << "Do NOT commit this candidate as the baseline until Wave 4\n"
               << "(implement-ssm-render-passes) has delivered a real SSM render.";
    }

    ASSERT_TRUE(currentImage.save("artifacts/ssm_4x4x4_grid_current.png", ImageFormat::PNG))
        << "Failed to save current render artifact";

    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, currentImage, VRTestConfig::PARTICLE_TOLERANCE, true);

    float diff_ratio = result.total_pixels > 0
                           ? static_cast<float>(result.diff_pixels) / static_cast<float>(result.total_pixels)
                           : 1.0f;
    if (diff_ratio > VRTestConfig::MAX_DIFF_RATIO) {
        result.diff_image.save("artifacts/ssm_4x4x4_grid_diff.png", ImageFormat::PNG);
        FAIL() << "SSM visual mismatch detected:\n"
               << "  Diff pixels: " << result.diff_pixels << " / " << result.total_pixels << " ("
               << ((result.diff_pixels * 100.0f) / result.total_pixels) << "%)\n"
               << "  Similarity: " << (result.similarity * 100.0f) << "%\n"
               << "  Diff image saved to: artifacts/ssm_4x4x4_grid_diff.png\n"
               << "  Current image saved to: artifacts/ssm_4x4x4_grid_current.png";
    }
}
