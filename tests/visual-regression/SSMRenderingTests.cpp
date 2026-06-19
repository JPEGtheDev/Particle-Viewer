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

    // Create intermediate FBO (GL_RGBA32F) — holds H-blur pass output before V pass
    GLuint intermediate_fbo = 0;
    GLuint intermediate_tex = 0;
    glGenFramebuffers(1, &intermediate_fbo);
    glGenTextures(1, &intermediate_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediate_fbo);
    glBindTexture(GL_TEXTURE_2D, intermediate_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebuffer_width_, framebuffer_height_, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediate_tex, 0);
    ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE))
        << "Intermediate FBO incomplete";
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

    // Provide a 1×1 depth texture at value 1.0 (= far plane) for u_prepass_depth.
    // prepass_d=1.0 → the depth-cull branch is skipped → same output as the
    // pre-prepass shader, so the existing baseline remains valid.
    GLuint noop_depth_tex = 0;
    {
        GLfloat far_depth = 1.0f;
        glGenTextures(1, &noop_depth_tex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, noop_depth_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &far_depth);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glActiveTexture(GL_TEXTURE0);
    }

    // Splat pass
    glBindFramebuffer(GL_FRAMEBUFFER, density_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    splatShader.Use();
    glBindVertexArray(circle_vao);
    particles.setUpInstanceArray();
    glUniformMatrix4fv(glGetUniformLocation(splatShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(splatShader.Program, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform1f(glGetUniformLocation(splatShader.Program, "blobRadius"), 100.0f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "scale"), 5.0f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "transScale"), 0.25f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "viewportHeight"), static_cast<float>(framebuffer_height_));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, noop_depth_tex);
    glUniform1i(glGetUniformLocation(splatShader.Program, "u_prepass_depth"), 1);
    glUniform2f(glGetUniformLocation(splatShader.Program, "u_viewport_inv"), 1.0f, 1.0f); // irrelevant: prepass_d=1.0
    glUniform1f(glGetUniformLocation(splatShader.Program, "u_near"), 0.1f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "u_far"), 3000.0f);
    glActiveTexture(GL_TEXTURE0);
    glDrawArraysInstanced(GL_POINTS, 0, 1, particles.n);
    glBindVertexArray(0);
    glDisable(GL_BLEND);

    // Blur pass — separable H+V matching production drawSSMScene()
    blurShader.Use();
    glBindVertexArray(quad_vao);
    glUniform1i(glGetUniformLocation(blurShader.Program, "densityTexture"), 0);
    glUniform1f(glGetUniformLocation(blurShader.Program, "blurAmount"), 3.0f);
    glUniform2f(glGetUniformLocation(blurShader.Program, "texelSize"), 1.0f / static_cast<float>(framebuffer_width_),
                1.0f / static_cast<float>(framebuffer_height_));

    // H pass: density → intermediate
    glBindFramebuffer(GL_FRAMEBUFFER, intermediate_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, density_tex);
    glUniform2f(glGetUniformLocation(blurShader.Program, "blurDir"), 1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // V pass: intermediate → blur_fbo
    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, intermediate_tex);
    glUniform2f(glGetUniformLocation(blurShader.Program, "blurDir"), 0.0f, 1.0f);
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
    glDeleteTextures(1, &noop_depth_tex);
    glDeleteTextures(1, &density_tex);
    glDeleteFramebuffers(1, &density_fbo);
    glDeleteTextures(1, &intermediate_tex);
    glDeleteFramebuffers(1, &intermediate_fbo);
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

// ============================================================================
// SSM Visual Assessment — Merge Shape Check
// ============================================================================

/*
 * Test: SSMVisualAssessment_ThreeBlueBlobs_SavesArtifact
 *
 * Renders three adjacent blue (cat=1) particles through the full SSM pipeline
 * and saves the result to artifacts/ssm_merge_assessment.png for visual inspection.
 *
 * No baseline comparison — this is a visual assessment artifact only.
 *
 * Expected visual result:
 *   - Three roughly circular blue blobs arranged horizontally
 *   - Adjacent blobs that are close enough merge into one connected shape
 *     (no visible seam / boundary between them)
 *   - No black rectangular outlines (corners of GL_POINTS quad must be zero)
 *   - Isolated blobs are circles, not squares
 *
 * Particle layout (sim space, transScale=0.25 → display space):
 *   sim (0,0,0) → display (0,    0, 0), camera depth 10
 *   sim (3,0,0) → display (0.75, 0, 0), camera depth ~10
 *   sim (6,0,0) → display (1.5,  0, 0), camera depth ~10
 *
 * blobRadius=200 produces sprite diameter ≈ 70 px at depth 10.
 * Pixel separation between adjacent blobs ≈ 65 px → edges overlap → merging.
 */
TEST_F(SSMRenderingTest, SSMVisualAssessment_ThreeBlueBlobs_SavesArtifact)
{
    // ---- GL_RGBA32F support gate -----------------------------------------------
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
            GTEST_SKIP() << "GL_RGBA32F framebuffer not supported — skipping visual assessment";
        }
    }

    // ---- Compile shaders -------------------------------------------------------
    std::string splatVertPath = getShaderPath("metaball_splat.vert");
    std::string splatFragPath = getShaderPath("metaball_splat.frag");
    Shader splatShader(splatVertPath.c_str(), splatFragPath.c_str());
    ASSERT_NE(splatShader.Program, 0u) << "Failed to compile splat shader";

    std::string blurVertPath = getShaderPath("metaball_blur.vert");
    std::string blurFragPath = getShaderPath("metaball_blur.frag");
    Shader blurShader(blurVertPath.c_str(), blurFragPath.c_str());
    ASSERT_NE(blurShader.Program, 0u) << "Failed to compile blur shader";

    std::string compositeFragPath = getShaderPath("metaball_composite.frag");
    std::string screenVertPath = getShaderPath("screenshader.vs");
    Shader compositeShader(screenVertPath.c_str(), compositeFragPath.c_str());
    ASSERT_NE(compositeShader.Program, 0u) << "Failed to compile composite shader";

    // ---- Build particles -------------------------------------------------------
    // Three blue (cat=1) particles in sim space; transScale=0.25 → display space.
    std::vector<glm::vec4> positions = {
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), // display (0,   0, 0)
        glm::vec4(2.0f, 0.0f, 0.0f, 1.0f), // display (0.5, 0, 0)
        glm::vec4(4.0f, 0.0f, 0.0f, 1.0f), // display (1.0, 0, 0)
    };
    Particle particles(static_cast<int>(positions.size()), positions.data());

    // ---- Camera ----------------------------------------------------------------
    glm::vec3 camPos(0.0f, 0.0f, 10.0f);
    glm::mat4 view = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    float aspect = static_cast<float>(framebuffer_width_) / static_cast<float>(framebuffer_height_);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 3000.0f);

    // ---- Allocate FBOs --------------------------------------------------------
    auto makeFBO_RGBA32F = [&](GLuint& fbo, GLuint& tex) {
        glGenFramebuffers(1, &fbo);
        glGenTextures(1, &tex);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebuffer_width_, framebuffer_height_, 0, GL_RGBA, GL_FLOAT,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));
    };

    GLuint density_fbo = 0, density_tex = 0;
    makeFBO_RGBA32F(density_fbo, density_tex);
    GLuint intermediate_fbo = 0, intermediate_tex = 0;
    makeFBO_RGBA32F(intermediate_fbo, intermediate_tex);
    GLuint blur_fbo = 0, blur_tex = 0;
    makeFBO_RGBA32F(blur_fbo, blur_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---- Fullscreen quad VAO/VBO -----------------------------------------------
    static constexpr float QUAD_VERTS[] = {
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f,
    };
    GLuint quad_vao = 0, quad_vbo = 0;
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

    // ---- Particle VAO + noop depth texture ------------------------------------
    GLuint circle_vao = 0;
    glGenVertexArrays(1, &circle_vao);
    particles.pushVBO();

    // 1×1 depth texture at 1.0 (far plane) → depth cull branch skipped.
    GLuint noop_depth_tex = 0;
    {
        GLfloat far_val = 1.0f;
        glGenTextures(1, &noop_depth_tex);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, noop_depth_tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, &far_val);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glActiveTexture(GL_TEXTURE0);
    }

    // ---- Splat pass -----------------------------------------------------------
    glBindFramebuffer(GL_FRAMEBUFFER, density_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    splatShader.Use();
    glBindVertexArray(circle_vao);
    particles.setUpInstanceArray();
    glUniformMatrix4fv(glGetUniformLocation(splatShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(splatShader.Program, "projection"), 1, GL_FALSE,
                       glm::value_ptr(projection));
    glUniform1f(glGetUniformLocation(splatShader.Program, "blobRadius"), 200.0f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "scale"), 5.0f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "transScale"), 0.25f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "viewportHeight"), static_cast<float>(framebuffer_height_));
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, noop_depth_tex);
    glUniform1i(glGetUniformLocation(splatShader.Program, "u_prepass_depth"), 1);
    glUniform2f(glGetUniformLocation(splatShader.Program, "u_viewport_inv"), 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "u_near"), 0.1f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "u_far"), 3000.0f);
    glActiveTexture(GL_TEXTURE0);
    glDrawArraysInstanced(GL_POINTS, 0, 1, particles.n);
    glBindVertexArray(0);
    glDisable(GL_BLEND);

    // ---- Separable blur: H pass (density → intermediate), V pass (→ blur) ----
    blurShader.Use();
    glBindVertexArray(quad_vao);
    glUniform1i(glGetUniformLocation(blurShader.Program, "densityTexture"), 0);
    glUniform1f(glGetUniformLocation(blurShader.Program, "blurAmount"), 3.0f);
    glUniform2f(glGetUniformLocation(blurShader.Program, "texelSize"), 1.0f / static_cast<float>(framebuffer_width_),
                1.0f / static_cast<float>(framebuffer_height_));

    glBindFramebuffer(GL_FRAMEBUFFER, intermediate_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, density_tex);
    glUniform2f(glGetUniformLocation(blurShader.Program, "blurDir"), 1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, blur_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, intermediate_tex);
    glUniform2f(glGetUniformLocation(blurShader.Program, "blurDir"), 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // ---- Composite pass → framebuffer -----------------------------------------
    framebuffer_->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    compositeShader.Use();
    glBindVertexArray(quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blur_tex);
    glUniform1i(glGetUniformLocation(compositeShader.Program, "blurredDensity"), 0);
    glUniform1f(glGetUniformLocation(compositeShader.Program, "threshold"), 0.5f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // ---- Cleanup ---------------------------------------------------------------
    glDeleteVertexArrays(1, &circle_vao);
    glDeleteVertexArrays(1, &quad_vao);
    glDeleteBuffers(1, &quad_vbo);
    glDeleteTextures(1, &noop_depth_tex);
    glDeleteTextures(1, &density_tex);
    glDeleteFramebuffers(1, &density_fbo);
    glDeleteTextures(1, &intermediate_tex);
    glDeleteFramebuffers(1, &intermediate_fbo);
    glDeleteTextures(1, &blur_tex);
    glDeleteFramebuffers(1, &blur_fbo);

    // ---- Capture and save artifact (no baseline comparison) -------------------
    Image img = framebuffer_->capture();
    ASSERT_TRUE(img.valid()) << "Failed to capture framebuffer";
    ASSERT_TRUE(img.save("artifacts/ssm_merge_assessment.png", ImageFormat::PNG))
        << "Failed to save visual assessment artifact";

    // Minimal sanity: at least some non-black pixels rendered.
    int non_black = 0;
    for (size_t i = 0; i < img.pixels.size(); i += 4) {
        if (img.pixels[i] > 10 || img.pixels[i + 1] > 10 || img.pixels[i + 2] > 10) {
            ++non_black;
        }
    }
    EXPECT_GT(non_black, 50) << "Expected at least 50 non-black pixels — nothing may have rendered";
}

// ============================================================================
// SSM Depth Occlusion Test
// ============================================================================

/*
 * Test: SSMDepthOcclusion_BlueClusterFront_OrangeClusterCulled
 *
 * Verifies that the depth prepass prevents background particles from bleeding
 * through foreground SSM blobs.
 *
 * Particle layout (in display space, after transScale=0.25):
 *   Foreground (blue, cat=1): sim (0,0,0)   → display (0,0,0)   → linear depth 10
 *   Background (orange, cat=3): sim (0,0,-165) → display (0,0,-41.25) → linear depth 51
 *
 * Depth cull logic (metaball_splat.frag):
 *   threshold = front_linear + 30.0 = 10 + 30 = 40
 *   orange depth (51) > 40 → discarded ✓
 *   blue   depth (10) ≤ 40 → kept       ✓
 *
 * Assertion:
 *   In the center 100×100 px region (where the blob lands), B > R for every
 *   non-black pixel.  A passing test means the blob is blue.  If the depth
 *   prepass fails, orange contributes density → R > B → test fails.
 */
TEST_F(SSMRenderingTest, SSMDepthOcclusion_BlueClusterFront_OrangeClusterCulled)
{
    // ---- GL_RGBA32F support gate -----------------------------------------------
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
            GTEST_SKIP() << "GL_RGBA32F framebuffer not supported — skipping SSM occlusion test";
        }
    }

    // ---- Compile shaders -------------------------------------------------------
    std::string depthVertPath = getShaderPath("metaball_splat.vert");
    std::string depthFragPath = getShaderPath("metaball_depth.frag");
    Shader depthPrepassShader(depthVertPath.c_str(), depthFragPath.c_str());
    ASSERT_NE(depthPrepassShader.Program, 0u) << "Failed to compile depth prepass shader";

    std::string splatFragPath = getShaderPath("metaball_splat.frag");
    Shader splatShader(depthVertPath.c_str(), splatFragPath.c_str());
    ASSERT_NE(splatShader.Program, 0u) << "Failed to compile SSM splat shader";

    std::string blurVertPath = getShaderPath("metaball_blur.vert");
    std::string blurFragPath = getShaderPath("metaball_blur.frag");
    Shader blurShader(blurVertPath.c_str(), blurFragPath.c_str());
    ASSERT_NE(blurShader.Program, 0u) << "Failed to compile SSM blur shader";

    std::string screenVertPath = getShaderPath("screenshader.vs");
    std::string compositeFragPath = getShaderPath("metaball_composite.frag");
    Shader compositeShader(screenVertPath.c_str(), compositeFragPath.c_str());
    ASSERT_NE(compositeShader.Program, 0u) << "Failed to compile SSM composite shader";

    // ---- Build particles -------------------------------------------------------
    // transScale = 0.25: display_pos = sim_pos * 0.25
    //
    // Both particles are at world x=y=0, so they project to the same screen pixel.
    // The blue prepass writes depth at the center; the orange fragment samples that
    // same depth → the cull fires correctly.
    //
    //   Blue   (cat=1): sim (0,0,    0) → display (0,0,     0) → camera depth 10
    //   Orange (cat=3): sim (0,0, -165) → display (0,0,-41.25) → camera depth 51
    //
    //   Shader threshold: front_linear + 30.0 = 10 + 30 = 40
    //     orange 51 > 40 → discarded  ✓
    //     blue   10 ≤ 40 → kept       ✓
    std::vector<glm::vec4> positions = {
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),    // blue foreground
        glm::vec4(0.0f, 0.0f, -165.0f, 3.0f), // orange background (depth 51 > threshold 40)
    };
    int n = static_cast<int>(positions.size());
    Particle particles(n, positions.data());
    ASSERT_EQ(particles.n, n);

    // ---- Camera ----------------------------------------------------------------
    glm::vec3 camPos(0.0f, 0.0f, 10.0f);
    glm::vec3 camTarget(0.0f, 0.0f, 0.0f);
    glm::mat4 view = glm::lookAt(camPos, camTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    float aspect = static_cast<float>(framebuffer_width_) / static_cast<float>(framebuffer_height_);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 3000.0f);

    // ---- Allocate FBOs --------------------------------------------------------
    auto makeFBO_RGBA32F = [&](GLuint& fbo, GLuint& tex) {
        glGenFramebuffers(1, &fbo);
        glGenTextures(1, &tex);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, framebuffer_width_, framebuffer_height_, 0, GL_RGBA, GL_FLOAT,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
        ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE));
    };

    GLuint density_fbo = 0, density_tex = 0;
    makeFBO_RGBA32F(density_fbo, density_tex);
    GLuint intermediate_fbo = 0, intermediate_tex = 0;
    makeFBO_RGBA32F(intermediate_fbo, intermediate_tex);
    GLuint blurred_fbo = 0, blurred_tex = 0;
    makeFBO_RGBA32F(blurred_fbo, blurred_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Depth prepass FBO (depth-only)
    GLuint depth_fbo = 0, depth_tex = 0;
    glGenFramebuffers(1, &depth_fbo);
    glGenTextures(1, &depth_tex);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, framebuffer_width_, framebuffer_height_, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_tex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), static_cast<GLenum>(GL_FRAMEBUFFER_COMPLETE))
        << "Depth prepass FBO incomplete";
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ---- Fullscreen quad VAO/VBO -----------------------------------------------
    static constexpr float QUAD_VERTS[] = {
        -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f,
    };
    GLuint quad_vao = 0, quad_vbo = 0;
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

    // ---- Particle VAO ----------------------------------------------------------
    GLuint circle_vao = 0;
    glGenVertexArrays(1, &circle_vao);
    particles.pushVBO();

    // ---- Render pipeline -------------------------------------------------------
    // Replicates drawSSMScene() with depth prepass, separable blur, and composite.

    auto setPointUniforms = [&](GLuint prog) {
        glUniformMatrix4fv(glGetUniformLocation(prog, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1f(glGetUniformLocation(prog, "blobRadius"), 100.0f);
        glUniform1f(glGetUniformLocation(prog, "scale"), 5.0f);
        glUniform1f(glGetUniformLocation(prog, "transScale"), 0.25f);
        glUniform1f(glGetUniformLocation(prog, "viewportHeight"), static_cast<float>(framebuffer_height_));
    };

    // Depth prepass
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
    glClear(GL_DEPTH_BUFFER_BIT);
    depthPrepassShader.Use();
    glBindVertexArray(circle_vao);
    particles.setUpInstanceArray();
    setPointUniforms(depthPrepassShader.Program);
    glDrawArraysInstanced(GL_POINTS, 0, 1, particles.n);
    glBindVertexArray(0);
    glDisable(GL_DEPTH_TEST);

    // Splat pass (additive blend, depth cull via prepass)
    glBindFramebuffer(GL_FRAMEBUFFER, density_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    splatShader.Use();
    glBindVertexArray(circle_vao);
    particles.setUpInstanceArray();
    setPointUniforms(splatShader.Program);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depth_tex);
    glUniform1i(glGetUniformLocation(splatShader.Program, "u_prepass_depth"), 1);
    glUniform2f(glGetUniformLocation(splatShader.Program, "u_viewport_inv"),
                1.0f / static_cast<float>(framebuffer_width_), 1.0f / static_cast<float>(framebuffer_height_));
    glUniform1f(glGetUniformLocation(splatShader.Program, "u_near"), 0.1f);
    glUniform1f(glGetUniformLocation(splatShader.Program, "u_far"), 3000.0f);
    glDrawArraysInstanced(GL_POINTS, 0, 1, particles.n);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);

    // Separable blur: H pass (density → intermediate), V pass (intermediate → blurred)
    blurShader.Use();
    glBindVertexArray(quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(blurShader.Program, "densityTexture"), 0);
    glUniform1f(glGetUniformLocation(blurShader.Program, "blurAmount"), 3.0f);
    glUniform2f(glGetUniformLocation(blurShader.Program, "texelSize"), 1.0f / static_cast<float>(framebuffer_width_),
                1.0f / static_cast<float>(framebuffer_height_));

    glBindFramebuffer(GL_FRAMEBUFFER, intermediate_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, density_tex);
    glUniform2f(glGetUniformLocation(blurShader.Program, "blurDir"), 1.0f, 0.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindFramebuffer(GL_FRAMEBUFFER, blurred_fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, intermediate_tex);
    glUniform2f(glGetUniformLocation(blurShader.Program, "blurDir"), 0.0f, 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // Composite pass → output framebuffer
    framebuffer_->bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    compositeShader.Use();
    glBindVertexArray(quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, blurred_tex);
    glUniform1i(glGetUniformLocation(compositeShader.Program, "blurredDensity"), 0);
    glUniform1f(glGetUniformLocation(compositeShader.Program, "threshold"), 0.18f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    // ---- Cleanup ---------------------------------------------------------------
    glDeleteVertexArrays(1, &circle_vao);
    glDeleteVertexArrays(1, &quad_vao);
    glDeleteBuffers(1, &quad_vbo);
    glDeleteTextures(1, &depth_tex);
    glDeleteFramebuffers(1, &depth_fbo);
    glDeleteTextures(1, &density_tex);
    glDeleteFramebuffers(1, &density_fbo);
    glDeleteTextures(1, &intermediate_tex);
    glDeleteFramebuffers(1, &intermediate_fbo);
    glDeleteTextures(1, &blurred_tex);
    glDeleteFramebuffers(1, &blurred_fbo);

    // ---- Capture and assert ----------------------------------------------------
    Image img = framebuffer_->capture();
    ASSERT_TRUE(img.valid()) << "Failed to capture framebuffer";

    img.save("artifacts/ssm_occlusion_test.png", ImageFormat::PNG);

    // Inspect the center 100×100 px region — the blue foreground blob should land here.
    // Blue (cat=1) = vec3(0.2, 0.6, 1.0): B is always > R.
    // If orange (cat=3 = vec3(0.89, 0.59, 0.0)) bleeds through: R rises, B drops.
    // Assertion: for every non-black pixel in the center, B > R.
    const int cx = static_cast<int>(img.width / 2);
    const int cy = static_cast<int>(img.height / 2);
    const int half = 50;

    int non_black = 0;
    int bleed_violations = 0;

    for (int py = cy - half; py <= cy + half; ++py) {
        for (int px = cx - half; px <= cx + half; ++px) {
            if (px < 0 || py < 0 || px >= static_cast<int>(img.width) || py >= static_cast<int>(img.height)) {
                continue;
            }
            size_t idx = (static_cast<size_t>(py) * img.width + static_cast<size_t>(px)) * 4;
            uint8_t r = img.pixels[idx + 0];
            uint8_t g = img.pixels[idx + 1];
            uint8_t b = img.pixels[idx + 2];
            if (r < 10 && g < 10 && b < 10) {
                continue; // skip background black
            }
            ++non_black;
            if (r > b) {
                ++bleed_violations;
            }
        }
    }

    EXPECT_GT(non_black, 100) << "Expected at least 100 non-black pixels in blob center region — "
                              << "blob may not have rendered; check threshold/blobRadius settings";
    EXPECT_EQ(bleed_violations, 0)
        << bleed_violations << " pixels in the blob center have R > B.\n"
        << "This means orange background particles are bleeding through the blue foreground blob.\n"
        << "Depth prepass is not correctly culling background particles.\n"
        << "  Center region: [" << (cx - half) << "," << (cx + half) << "] x [" << (cy - half) << "," << (cy + half)
        << "]\n"
        << "  Non-black pixels found: " << non_black << "\n"
        << "  Artifact saved to: artifacts/ssm_occlusion_test.png";
}
