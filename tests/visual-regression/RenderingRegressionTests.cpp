/*
 * RenderingRegressionTests.cpp
 *
 * Visual regression tests for real-world particle rendering scenarios.
 * These tests verify that the OpenGL rendering pipeline produces consistent output
 * for various camera angles and particle configurations.
 *
 * Uses the production GLFWContext and Particle classes directly, ensuring tests
 * exercise the same code paths as the real application.
 *
 * Tests require:
 * - GLFW for window/context creation (headless mode with Xvfb in CI)
 * - OpenGL for rendering
 * - Shaders from src/shaders/
 *
 * Baseline images are stored in tests/visual-regression/baselines/
 * Current render outputs are uploaded as CI artifacts for comparison.
 */

#include <cstdio>
#include <string>
#include <vector>

#include <gtest/gtest.h>

// clang-format off
// GLAD must come before GLFW to properly initialize OpenGL functions
#include "glad/glad.h"       // NOLINT(llvm-include-order)
#include <GLFW/glfw3.h>      // NOLINT(llvm-include-order)
// clang-format on

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Image.hpp"
#include "graphics/GLFWContext.hpp"
#include "particle.hpp"
#include "shader.hpp"
#include "testing/FramebufferCapture.hpp"
#include "testing/PixelComparator.hpp"

// Test configuration
namespace RenderingTestConfig
{
static const uint32_t RENDER_WIDTH = 1280;             // Default 720p width
static const uint32_t RENDER_HEIGHT = 720;             // Default 720p height
static const float PARTICLE_TOLERANCE = 2.0f / 255.0f; // ±2/255 (~0.8%) for Mesa compatibility
static const std::string BASELINES_DIR = "baselines";  // Baseline images directory
} // namespace RenderingTestConfig

/*
 * Helper class for OpenGL rendering test setup.
 * Uses the production GLFWContext (hidden window) to ensure tests exercise
 * the same context creation path as the real application.
 * Adds FramebufferCapture for off-screen rendering.
 */
class OpenGLTestContext
{
  private:
    GLFWContext* context_;
    FramebufferCapture* framebuffer_;

  public:
    OpenGLTestContext() : context_(nullptr), framebuffer_(nullptr)
    {
    }

    ~OpenGLTestContext()
    {
        cleanup();
    }

    /*
     * Initialize production GLFWContext (hidden), then set up framebuffer for off-screen rendering.
     * @return true if initialization succeeds
     */
    bool initialize()
    {
        context_ = new GLFWContext(RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT,
                                   "Rendering Test", false);
        if (!context_->isValid()) {
            cleanup();
            return false;
        }

        context_->makeCurrent();

        // Use the actual framebuffer size from the context (handles HiDPI/platform scaling).
        // This ensures glViewport and FramebufferCapture match the real framebuffer.
        auto fb_size = context_->getFramebufferSize();
        int framebuffer_width = fb_size.first;
        int framebuffer_height = fb_size.second;

        glViewport(0, 0, framebuffer_width, framebuffer_height);

        framebuffer_ = new FramebufferCapture(static_cast<uint32_t>(framebuffer_width),
                                              static_cast<uint32_t>(framebuffer_height));
        if (!framebuffer_->initialize()) {
            cleanup();
            return false;
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE);

        return true;
    }

    void bindFramebuffer()
    {
        if (framebuffer_) {
            framebuffer_->bind();
        }
    }

    Image captureFramebuffer()
    {
        if (framebuffer_) {
            return framebuffer_->capture();
        }
        return Image();
    }

    void cleanup()
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

    bool isInitialized() const
    {
        return context_ != nullptr && context_->isValid();
    }
};

/*
 * Renders a Particle using the same instanced drawing pipeline as the application.
 * Uses the production Particle class directly instead of duplicating its logic.
 *
 * @param particle The Particle to render (must have translations populated)
 * @param shader Shader program to use
 * @param view View matrix
 * @param projection Projection matrix
 * @param viewport_height Viewport height for resolution-independent scaling (default: 720)
 */
void renderParticle(Particle& particle, Shader& shader, const glm::mat4& view, const glm::mat4& projection,
                    float viewport_height = 720.0f)
{
    shader.Use();

    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glUniform1f(glGetUniformLocation(shader.Program, "radius"), 100.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "scale"), 5.0f);
    glUniform1f(glGetUniformLocation(shader.Program, "transScale"), 0.25f);
    glUniform1f(glGetUniformLocation(shader.Program, "viewportHeight"), viewport_height);
    glUniform3fv(glGetUniformLocation(shader.Program, "lightDirection"), 1,
                 glm::value_ptr(glm::vec3(0.1f, 0.1f, 0.85f)));

    particle.pushVBO();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glEnableVertexAttribArray(0);
    particle.setUpInstanceArray();

    glDrawArraysInstanced(GL_POINTS, 0, 1, particle.n);
    glBindVertexArray(0);
    glDeleteVertexArrays(1, &vao);
}

// ============================================================================
// Rendering Regression Tests
// ============================================================================

/*
 * Test fixture for rendering regression tests.
 * Provides OpenGL rendering capabilities for visual regression testing.
 */
class RenderingRegressionTest : public testing::Test
{
  protected:
    OpenGLTestContext glContext_;

    void SetUp() override
    {
        // Ensure baselines directory exists
        std::string baselines_dir = RenderingTestConfig::BASELINES_DIR;
        std::string command = "mkdir -p " + baselines_dir;
        int result = std::system(command.c_str());
        ASSERT_EQ(result, 0) << "Failed to create baselines directory: " << baselines_dir;

        // Ensure artifacts directory exists for test output
        result = std::system("mkdir -p artifacts");
        ASSERT_EQ(result, 0) << "Failed to create artifacts directory";

        // Initialize OpenGL context
        bool initialized = glContext_.initialize();
        if (!initialized) {
            GTEST_SKIP() << "Failed to initialize OpenGL context. "
                         << "This test requires OpenGL 3.3 support (Mesa/Xvfb in CI, GPU locally).";
        }
    }

    void TearDown() override
    {
        glContext_.cleanup();
    }

    /*
     * Helper to get shader paths relative to the build directory.
     * Shaders are copied to build/Viewer-Assets/shaders/ during build.
     */
    std::string getShaderPath(const std::string& shaderName)
    {
        // Try multiple possible locations
        std::vector<std::string> possiblePaths = {
            "Viewer-Assets/shaders/" + shaderName,    // From build directory
            "../Viewer-Assets/shaders/" + shaderName, // From build/tests/
            "../../src/shaders/" + shaderName,        // From build directory to src
            "../../../src/shaders/" + shaderName      // From build/tests/ to src
        };

        for (const auto& path : possiblePaths) {
            FILE* file = fopen(path.c_str(), "r");
            if (file) {
                fclose(file);
                return path;
            }
        }

        // If not found, return first path and let shader loading fail with error
        return possiblePaths[0];
    }

    /*
     * Helper to get baseline image paths relative to the test working directory.
     * Baselines are committed in tests/visual-regression/baselines/ in the source tree.
     */
    std::string getBaselinePath(const std::string& baselineName)
    {
        // Try multiple possible locations (from build/tests/ working directory)
        std::vector<std::string> possiblePaths = {
            RenderingTestConfig::BASELINES_DIR + "/" + baselineName,     // baselines/ (local)
            "../../tests/visual-regression/baselines/" + baselineName,   // From build/tests/ to source
            "../tests/visual-regression/baselines/" + baselineName,      // From build/ to source
            "../../../tests/visual-regression/baselines/" + baselineName // Alternative path
        };

        for (const auto& path : possiblePaths) {
            FILE* file = fopen(path.c_str(), "r");
            if (file) {
                fclose(file);
                return path;
            }
        }

        // If not found, return first path (will trigger baseline generation)
        return possiblePaths[0];
    }
};

/*
 * Test: RenderDefaultCube_AngledView_MatchesBaseline
 *
 * Renders the default particle cube (64,000 particles in 40×40×40 grid)
 * from an angled camera position to verify 3D perspective and depth rendering.
 *
 * Camera Configuration (adjusted for proper framing):
 * - Position: (-23.60, 25.21, -30.93)
 * - Target: (-23.02, 24.83, -30.20)
 * - Up: (0.08, 1.00, 0.00)
 * - Projection: Perspective, FOV=45°, Near=0.1, Far=3000.0
 *
 * The camera is positioned ~51 units from cube center to match the reference
 * image composition with appropriate blank space around the cube.
 *
 * This is the most realistic rendering test - validates the default scene
 * that users see when first launching the application.
 */
TEST_F(RenderingRegressionTest, RenderDefaultCube_AngledView_MatchesBaseline)
{
    // Arrange
    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0) << "Failed to compile particle shader. " << "Vertex: " << vertexShaderPath
                                             << ", Fragment: " << fragmentShaderPath;

    Particle particles;
    ASSERT_EQ(particles.n, 64000) << "Default cube should have 64,000 particles";

    glm::vec3 cameraPos(-23.60f, 25.21f, -30.93f);
    glm::vec3 cameraTarget(-23.02f, 24.83f, -30.20f);
    glm::vec3 cameraUp(0.08f, 1.00f, 0.00f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), (float)RenderingTestConfig::RENDER_WIDTH / (float)RenderingTestConfig::RENDER_HEIGHT, 0.1f,
        3000.0f);

    // Act
    glContext_.bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderParticle(particles, particleShader, view, projection);
    Image currentImage = glContext_.captureFramebuffer();

    // Assert
    ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer";

    std::string baselinePath = getBaselinePath("particle_cube_angle_baseline.png");
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);

    if (baseline.empty()) {
        std::string localBaselinePath = RenderingTestConfig::BASELINES_DIR + "/particle_cube_angle_baseline.png";
        currentImage.save(localBaselinePath, ImageFormat::PNG);
        GTEST_SKIP() << "Baseline image not found. Current render saved to: " << localBaselinePath
                     << "\nPlease review and commit this baseline if correct.";
    }

    ASSERT_TRUE(currentImage.save("artifacts/particle_cube_angle_current.png", ImageFormat::PNG))
        << "Failed to save current render artifact";

    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, currentImage, RenderingTestConfig::PARTICLE_TOLERANCE, true);

    if (!result.matches) {
        result.diff_image.save("artifacts/particle_cube_angle_diff.png", ImageFormat::PNG);
        FAIL() << "Visual mismatch detected:\n"
               << "  Diff pixels: " << result.diff_pixels << " / " << result.total_pixels << " ("
               << ((result.diff_pixels * 100.0f) / result.total_pixels) << "%)\n"
               << "  Similarity: " << (result.similarity * 100.0f) << "%\n"
               << "  Diff image saved to: artifacts/particle_cube_angle_diff.png\n"
               << "  Current image saved to: artifacts/particle_cube_angle_current.png";
    }
}

/*
 * Test: RenderSingleParticle_CenteredView_MatchesBaseline
 *
 * Renders a single red particle centered in the viewport, filling approximately
 * 50% of the screen height. Validates the most basic rendering case: one particle
 * with correct shader coloring, sphere shading, and lighting.
 *
 * Camera Configuration:
 * - Particle at origin (0, 0, 0) with colVal=0 (red)
 * - Camera at (0, 0, 1.1) looking at origin
 * - Projection: Perspective, FOV=45°, Near=0.1, Far=3000.0
 *
 * The camera distance of 1.1 units produces a point size of ~350 pixels,
 * which is approximately 49% of the 720px viewport height.
 */
TEST_F(RenderingRegressionTest, RenderSingleParticle_CenteredView_MatchesBaseline)
{
    // Arrange
    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0)
        << "Failed to compile particle shader. Vertex: " << vertexShaderPath << ", Fragment: " << fragmentShaderPath;

    std::vector<glm::vec4> single_particle_data = {glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};
    Particle particles(1, single_particle_data.data());
    ASSERT_EQ(particles.n, 1) << "Should have exactly 1 particle";

    glm::vec3 cameraPos(0.0f, 0.0f, 1.1f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), (float)RenderingTestConfig::RENDER_WIDTH / (float)RenderingTestConfig::RENDER_HEIGHT, 0.1f,
        3000.0f);

    // Act
    glContext_.bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderParticle(particles, particleShader, view, projection);
    Image currentImage = glContext_.captureFramebuffer();

    // Assert
    ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer";

    std::string baselinePath = getBaselinePath("single_particle_baseline.png");
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);

    if (baseline.empty()) {
        std::string localBaselinePath = RenderingTestConfig::BASELINES_DIR + "/single_particle_baseline.png";
        currentImage.save(localBaselinePath, ImageFormat::PNG);
        GTEST_SKIP() << "Baseline image not found. Current render saved to: " << localBaselinePath
                     << "\nPlease review and commit this baseline if correct.";
    }

    ASSERT_TRUE(currentImage.save("artifacts/single_particle_current.png", ImageFormat::PNG))
        << "Failed to save current render artifact";

    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, currentImage, RenderingTestConfig::PARTICLE_TOLERANCE, true);

    if (!result.matches) {
        result.diff_image.save("artifacts/single_particle_diff.png", ImageFormat::PNG);
        FAIL() << "Visual mismatch detected:\n"
               << "  Diff pixels: " << result.diff_pixels << " / " << result.total_pixels << " ("
               << ((result.diff_pixels * 100.0f) / result.total_pixels) << "%)\n"
               << "  Similarity: " << (result.similarity * 100.0f) << "%\n"
               << "  Diff image saved to: artifacts/single_particle_diff.png\n"
               << "  Current image saved to: artifacts/single_particle_current.png";
    }
}

/*
 * Test: RenderParticleGroup_ThreeParticles_MatchesBaseline
 *
 * Renders three particles side by side with different colors to validate
 * multi-particle rendering and the shader's color pipeline.
 *
 * Particle Configuration:
 * - Left:   (-4, 0, 0) colVal=0 → Red
 * - Center: ( 0, 0, 0) colVal=1 → Blue
 * - Right:  ( 4, 0, 0) colVal=2 → Magenta
 *
 * With transScale=0.25, world positions are (-1, 0, 0), (0, 0, 0), (1, 0, 0).
 *
 * Camera Configuration:
 * - Position: (0, 0, 4) looking at origin
 * - All three particles visible with clear spacing
 * - Projection: Perspective, FOV=45°, Near=0.1, Far=3000.0
 */
TEST_F(RenderingRegressionTest, RenderParticleGroup_ThreeParticles_MatchesBaseline)
{
    // Arrange
    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0)
        << "Failed to compile particle shader. Vertex: " << vertexShaderPath << ", Fragment: " << fragmentShaderPath;

    std::vector<glm::vec4> three_particle_data = {
        glm::vec4(-4.0f, 0.0f, 0.0f, 0.0f), // Red (colVal=0), world pos: (-1, 0, 0)
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),  // Blue (colVal=1), world pos: (0, 0, 0)
        glm::vec4(4.0f, 0.0f, 0.0f, 2.0f)   // Magenta (colVal=2), world pos: (1, 0, 0)
    };
    Particle particles(3, three_particle_data.data());
    ASSERT_EQ(particles.n, 3) << "Should have exactly 3 particles";

    glm::vec3 cameraPos(0.0f, 0.0f, 4.0f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), (float)RenderingTestConfig::RENDER_WIDTH / (float)RenderingTestConfig::RENDER_HEIGHT, 0.1f,
        3000.0f);

    // Act
    glContext_.bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderParticle(particles, particleShader, view, projection);
    Image currentImage = glContext_.captureFramebuffer();

    // Assert
    ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer";

    std::string baselinePath = getBaselinePath("particle_group_baseline.png");
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);

    if (baseline.empty()) {
        std::string localBaselinePath = RenderingTestConfig::BASELINES_DIR + "/particle_group_baseline.png";
        currentImage.save(localBaselinePath, ImageFormat::PNG);
        GTEST_SKIP() << "Baseline image not found. Current render saved to: " << localBaselinePath
                     << "\nPlease review and commit this baseline if correct.";
    }

    ASSERT_TRUE(currentImage.save("artifacts/particle_group_current.png", ImageFormat::PNG))
        << "Failed to save current render artifact";

    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, currentImage, RenderingTestConfig::PARTICLE_TOLERANCE, true);

    if (!result.matches) {
        result.diff_image.save("artifacts/particle_group_diff.png", ImageFormat::PNG);
        FAIL() << "Visual mismatch detected:\n"
               << "  Diff pixels: " << result.diff_pixels << " / " << result.total_pixels << " ("
               << ((result.diff_pixels * 100.0f) / result.total_pixels) << "%)\n"
               << "  Similarity: " << (result.similarity * 100.0f) << "%\n"
               << "  Diff image saved to: artifacts/particle_group_diff.png\n"
               << "  Current image saved to: artifacts/particle_group_current.png";
    }
}

// ============================================================================
// Resolution Independence Tests
// ============================================================================

/*
 * Calculates the fraction of non-black (lit) pixels in an image.
 * Used to verify that particles occupy a consistent fraction of the viewport
 * regardless of rendering resolution.
 *
 * Uses a small brightness threshold (>2 in any RGB channel) to ignore
 * readback noise and driver-specific artifacts.
 *
 * @param image The image to analyze
 * @return Fraction of pixels that are lit (0.0 to 1.0)
 */
float calculateLitPixelFraction(const Image& image)
{
    const size_t CHANNELS_PER_PIXEL = 4;
    const size_t expected_size = static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * CHANNELS_PER_PIXEL;
    if (image.pixels.size() != expected_size) {
        ADD_FAILURE() << "Image pixel buffer size (" << image.pixels.size()
                      << ") does not match width * height * 4 (" << expected_size
                      << ") in calculateLitPixelFraction.";
        return 0.0f;
    }
    const size_t total_pixels = image.pixels.size() / CHANNELS_PER_PIXEL;
    const uint8_t LIT_THRESHOLD = 2; // ignore readback noise / driver artifacts
    size_t lit_count = 0;
    for (size_t i = 0; i < image.pixels.size(); i += CHANNELS_PER_PIXEL) {
        if (image.pixels[i] > LIT_THRESHOLD || image.pixels[i + 1] > LIT_THRESHOLD ||
            image.pixels[i + 2] > LIT_THRESHOLD) {
            lit_count++;
        }
    }
    return total_pixels == 0 ? 0.0f : static_cast<float>(lit_count) / static_cast<float>(total_pixels);
}

/*
 * Test: ParticleScale_SingleParticle_ConsistentFractionAcrossResolutions
 *
 * Verifies resolution-independent particle scaling by rendering a single particle
 * at multiple resolutions and checking that it occupies the same fraction of the
 * viewport at each resolution. This validates the viewportHeight shader uniform
 * correctly scales particles to maintain visual consistency.
 *
 * Uses the existing OpenGL context with different-sized framebuffers for each
 * resolution to avoid GLFW context lifecycle issues.
 *
 * Resolutions tested: 1280x720, 1920x1080, 2560x1440, 3840x2160
 * Tolerance: ±0.01 absolute fraction tolerance (accounts for rasterization differences)
 */
TEST_F(RenderingRegressionTest, ParticleScale_SingleParticle_ConsistentFractionAcrossResolutions)
{
    // Arrange
    struct Resolution
    {
        uint32_t width;
        uint32_t height;
        std::string name;
    };
    std::vector<Resolution> resolutions = {
        {1280, 720, "720p"}, {1920, 1080, "1080p"}, {2560, 1440, "1440p"}, {3840, 2160, "4K"}};

    std::vector<glm::vec4> single_particle_data = {glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};

    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0) << "Failed to compile shader";

    float reference_fraction = 0.0f;
    const float FRACTION_TOLERANCE = 0.01f; // ±0.01 absolute fraction tolerance

    for (const auto& res : resolutions) {
        // Create a framebuffer at this resolution using the existing GL context
        FramebufferCapture fbo(res.width, res.height);
        ASSERT_TRUE(fbo.initialize()) << "Failed to create FBO at " << res.name;

        Particle particles(1, single_particle_data.data());

        // Camera at z=3.0 produces point sizes under the GPU max for all tested resolutions.
        // gl_PointSize has a hardware max (GL_POINT_SIZE_RANGE); at z=3.0 with radius=100,
        // scale=5, the max point size at 1440p is ~244px, under typical limits (256+ px).
        // At 4K (2160p), the computed point size (~366px) may exceed limits on some GPUs;
        // OpenGL silently clamps to the maximum supported size. The test still validates
        // that scaling is proportional up to the clamp point.
        glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                 static_cast<float>(res.width) / static_cast<float>(res.height), 0.1f,
                                                 3000.0f);

        // Act - render with resolution-dependent viewportHeight
        fbo.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderParticle(particles, particleShader, view, projection, static_cast<float>(res.height));
        Image currentImage = fbo.capture();

        // Assert
        ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer at " << res.name;

        // Save each resolution render as artifact for visual inspection
        std::string artifact_name = "artifacts/single_particle_" + res.name + ".png";
        currentImage.save(artifact_name, ImageFormat::PNG);

        float fraction = calculateLitPixelFraction(currentImage);

        if (reference_fraction == 0.0f) {
            reference_fraction = fraction;
            ASSERT_GT(reference_fraction, 0.0f) << "Reference render at " << res.name << " has no lit pixels";
        } else {
            EXPECT_NEAR(fraction, reference_fraction, FRACTION_TOLERANCE)
                << "Particle fraction at " << res.name << " (" << fraction << ") differs from reference ("
                << reference_fraction << ")";
        }
    }
}

/*
 * Test: ParticleScale_ThreeParticles_ConsistentFractionAcrossResolutions
 *
 * Verifies resolution-independent particle scaling with multiple particles.
 * Renders three colored particles at different resolutions and validates
 * that the total lit pixel fraction remains consistent.
 *
 * Resolutions tested: 1280x720, 1920x1080, 2560x1440, 3840x2160
 */
TEST_F(RenderingRegressionTest, ParticleScale_ThreeParticles_ConsistentFractionAcrossResolutions)
{
    // Arrange
    struct Resolution
    {
        uint32_t width;
        uint32_t height;
        std::string name;
    };
    std::vector<Resolution> resolutions = {
        {1280, 720, "720p"}, {1920, 1080, "1080p"}, {2560, 1440, "1440p"}, {3840, 2160, "4K"}};

    std::vector<glm::vec4> three_particle_data = {
        glm::vec4(-4.0f, 0.0f, 0.0f, 0.0f), // Red
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),  // Blue
        glm::vec4(4.0f, 0.0f, 0.0f, 2.0f)   // Magenta
    };

    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0) << "Failed to compile shader";

    float reference_fraction = 0.0f;
    const float FRACTION_TOLERANCE = 0.01f; // ±0.01 absolute fraction tolerance

    for (const auto& res : resolutions) {
        FramebufferCapture fbo(res.width, res.height);
        ASSERT_TRUE(fbo.initialize()) << "Failed to create FBO at " << res.name;

        Particle particles(3, three_particle_data.data());

        glm::vec3 cameraPos(0.0f, 0.0f, 4.0f);
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                 static_cast<float>(res.width) / static_cast<float>(res.height), 0.1f,
                                                 3000.0f);

        fbo.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderParticle(particles, particleShader, view, projection, static_cast<float>(res.height));
        Image currentImage = fbo.capture();

        ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer at " << res.name;

        // Save each resolution render as artifact for visual inspection
        std::string artifact_name = "artifacts/three_particles_" + res.name + ".png";
        currentImage.save(artifact_name, ImageFormat::PNG);

        float fraction = calculateLitPixelFraction(currentImage);

        if (reference_fraction == 0.0f) {
            reference_fraction = fraction;
            ASSERT_GT(reference_fraction, 0.0f) << "Reference render at " << res.name << " has no lit pixels";
        } else {
            EXPECT_NEAR(fraction, reference_fraction, FRACTION_TOLERANCE)
                << "Three-particle fraction at " << res.name << " (" << fraction << ") differs from reference ("
                << reference_fraction << ")";
        }
    }
}
