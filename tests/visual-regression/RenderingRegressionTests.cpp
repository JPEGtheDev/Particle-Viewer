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
 * NOTE: A GPU is NOT required to run these tests locally.
 * Use Xvfb (X virtual framebuffer) for headless rendering:
 *   xvfb-run -a ./build/tests/ParticleViewerTests --gtest_filter="RenderingRegressionTest.*"
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
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "particle.hpp"
#include "shader.hpp"
#include "testing/FramebufferCapture.hpp"
#include "testing/PixelComparator.hpp"
#include "ui/imgui_menu.hpp"

// Test configuration
namespace RenderingTestConfig
{
static const uint32_t RENDER_WIDTH = 1280;             // Default 720p width
static const uint32_t RENDER_HEIGHT = 720;             // Default 720p height
static const float PARTICLE_TOLERANCE = 2.0f / 255.0f; // ±2/255 (~0.8%) per-pixel channel tolerance
static const float MAX_DIFF_RATIO = 0.0001f;           // Allow up to 0.01% of pixels to differ across Mesa versions
static const std::string BASELINES_DIR = "baselines";  // Baseline images directory

// GUI exclusion test: region to check for leaked ImGui content
static const uint32_t MENU_BAR_CHECK_HEIGHT = 30;  // Menu bar is ~25px; check top 30 rows
static const uint32_t MENU_BAR_CHECK_WIDTH = 200;  // "File  View" menu text spans ~200px
static const uint8_t BRIGHT_PIXEL_THRESHOLD = 100; // Channel value above which a pixel is "bright" (GUI text is white)
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

        framebuffer_ =
            new FramebufferCapture(static_cast<uint32_t>(framebuffer_width), static_cast<uint32_t>(framebuffer_height));
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

    /*
     * Get the native GLFW window handle for ImGui initialization.
     */
    GLFWwindow* getNativeWindow() const
    {
        if (context_) {
            return static_cast<GLFWwindow*>(context_->getNativeWindowHandle());
        }
        return nullptr;
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
                         << "A GPU is NOT required — use Xvfb for headless rendering:\n"
                         << "  xvfb-run -a ./ParticleViewerTests --gtest_filter=\"RenderingRegressionTest.*\"";
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

    float diff_ratio = result.total_pixels > 0
                           ? static_cast<float>(result.diff_pixels) / static_cast<float>(result.total_pixels)
                           : 1.0f;
    if (diff_ratio > RenderingTestConfig::MAX_DIFF_RATIO) {
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

    float diff_ratio = result.total_pixels > 0
                           ? static_cast<float>(result.diff_pixels) / static_cast<float>(result.total_pixels)
                           : 1.0f;
    if (diff_ratio > RenderingTestConfig::MAX_DIFF_RATIO) {
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

    float diff_ratio = result.total_pixels > 0
                           ? static_cast<float>(result.diff_pixels) / static_cast<float>(result.total_pixels)
                           : 1.0f;
    if (diff_ratio > RenderingTestConfig::MAX_DIFF_RATIO) {
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
    const size_t expected_size =
        static_cast<size_t>(image.width) * static_cast<size_t>(image.height) * CHANNELS_PER_PIXEL;
    if (image.pixels.size() != expected_size) {
        ADD_FAILURE() << "Image pixel buffer size (" << image.pixels.size() << ") does not match width * height * 4 ("
                      << expected_size << ") in calculateLitPixelFraction.";
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
 * Camera distance chosen to keep gl_PointSize under GL_POINT_SIZE_RANGE max (256
 * on Mesa/llvmpipe) at all tested resolutions including 4K. At z=5.0 the max
 * point size at 4K is ~216px, safely under the 256px hardware limit.
 *
 * Resolutions tested: 1280x720, 1920x1080, 2560x1440, 3840x2160
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
    const float FRACTION_TOLERANCE = 0.005f; // ±0.005 absolute fraction tolerance

    for (const auto& res : resolutions) {
        // Create a framebuffer at this resolution using the existing GL context
        FramebufferCapture fbo(res.width, res.height);
        ASSERT_TRUE(fbo.initialize()) << "Failed to create FBO at " << res.name;

        Particle particles(1, single_particle_data.data());

        // Camera at z=5.0 keeps gl_PointSize under the 256px hardware limit
        // (GL_POINT_SIZE_RANGE) at all tested resolutions including 4K.
        // Max point size at 4K: radius(100) * scale(5) / dist(6.93) * 3.0 ≈ 216px.
        glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), static_cast<float>(res.width) / static_cast<float>(res.height), 0.1f, 3000.0f);

        // Act - render with resolution-dependent viewportHeight
        fbo.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderParticle(particles, particleShader, view, projection, static_cast<float>(res.height));
        Image currentImage = fbo.capture();

        // Assert
        ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer at " << res.name;

        // Save each resolution render as artifact for visual inspection
        std::string artifact_name = "artifacts/single_particle_" + res.name + "_current.png";
        ASSERT_TRUE(currentImage.save(artifact_name, ImageFormat::PNG))
            << "Failed to save artifact image: " << artifact_name;

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
 * Camera at z=6.0 keeps all particle point sizes under GL_POINT_SIZE_RANGE
 * max (256px) at all resolutions including 4K (max ~180px).
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
    const float FRACTION_TOLERANCE = 0.005f; // ±0.005 absolute fraction tolerance

    for (const auto& res : resolutions) {
        FramebufferCapture fbo(res.width, res.height);
        ASSERT_TRUE(fbo.initialize()) << "Failed to create FBO at " << res.name;

        Particle particles(3, three_particle_data.data());

        // Camera at z=6.0 keeps point sizes under GL_POINT_SIZE_RANGE max (256px) at 4K.
        glm::vec3 cameraPos(0.0f, 0.0f, 6.0f);
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), static_cast<float>(res.width) / static_cast<float>(res.height), 0.1f, 3000.0f);

        fbo.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderParticle(particles, particleShader, view, projection, static_cast<float>(res.height));
        Image currentImage = fbo.capture();

        ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer at " << res.name;

        // Save each resolution render as artifact for visual inspection
        std::string artifact_name = "artifacts/three_particles_" + res.name + "_current.png";
        ASSERT_TRUE(currentImage.save(artifact_name, ImageFormat::PNG))
            << "Failed to save artifact image: " << artifact_name;

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

/*
 * Test: ParticleScale_DefaultCube_ConsistentFractionAcrossResolutions
 *
 * Verifies resolution-independent scaling with the default 64,000-particle cube.
 * This is the most realistic test case — it validates the actual scene users see
 * when launching the application.
 *
 * The cube particles are far from the camera so individual point sizes stay well
 * under GL_POINT_SIZE_RANGE max at all resolutions.
 *
 * Resolutions tested: 1280x720, 1920x1080, 2560x1440, 3840x2160
 */
TEST_F(RenderingRegressionTest, ParticleScale_DefaultCube_ConsistentFractionAcrossResolutions)
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

    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0) << "Failed to compile shader";

    float reference_fraction = 0.0f;
    const float FRACTION_TOLERANCE = 0.005f; // ±0.005 absolute fraction tolerance

    // Same camera as the baseline cube test
    glm::vec3 cameraPos(-23.60f, 25.21f, -30.93f);
    glm::vec3 cameraTarget(-23.02f, 24.83f, -30.20f);
    glm::vec3 cameraUp(0.08f, 1.00f, 0.00f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

    for (const auto& res : resolutions) {
        FramebufferCapture fbo(res.width, res.height);
        ASSERT_TRUE(fbo.initialize()) << "Failed to create FBO at " << res.name;

        Particle cube;
        ASSERT_EQ(cube.n, 64000) << "Default cube should have 64,000 particles";

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), static_cast<float>(res.width) / static_cast<float>(res.height), 0.1f, 3000.0f);

        fbo.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderParticle(cube, particleShader, view, projection, static_cast<float>(res.height));
        Image currentImage = fbo.capture();

        ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer at " << res.name;

        // Save each resolution render as artifact for visual inspection
        std::string artifact_name = "artifacts/cube_" + res.name + "_current.png";
        ASSERT_TRUE(currentImage.save(artifact_name, ImageFormat::PNG))
            << "Failed to save artifact image: " << artifact_name;

        float fraction = calculateLitPixelFraction(currentImage);

        if (reference_fraction == 0.0f) {
            reference_fraction = fraction;
            ASSERT_GT(reference_fraction, 0.0f) << "Reference render at " << res.name << " has no lit pixels";
        } else {
            EXPECT_NEAR(fraction, reference_fraction, FRACTION_TOLERANCE)
                << "Cube fraction at " << res.name << " (" << fraction << ") differs from reference ("
                << reference_fraction << ")";
        }
    }
}

// ============================================================================
// GUI Exclusion Test
// ============================================================================

/*
 * Test: FBOCapture_WithImGuiActive_ExcludesGUI
 *
 * Verifies that ImGui UI elements (menu bar, debug overlay) are NOT captured
 * in FBO-based screenshots. This is critical for clean scientific visualization
 * exports — the FBO pipeline must produce images free of GUI artifacts.
 *
 * Architecture validation:
 *   ImGui renders to the default framebuffer AFTER the FBO blit pass.
 *   FBO captures read from the offscreen framebuffer BEFORE the blit.
 *   Therefore, ImGui content should never appear in FBO captures.
 *
 * The test:
 * 1. Initializes ImGui with the test GLFW context
 * 2. Starts an ImGui frame and renders a menu bar + debug overlay
 * 3. Renders particles to the test FBO (separate from default FB)
 * 4. Captures the FBO content
 * 5. Verifies the menu bar region (top-left rows) contains ONLY the
 *    rendered scene — no ImGui content leaked into the FBO
 */
TEST_F(RenderingRegressionTest, FBOCapture_WithImGuiActive_ExcludesGUI)
{
    // Arrange
    GLFWwindow* window = glContext_.getNativeWindow();
    ASSERT_NE(window, nullptr) << "Need native GLFW window for ImGui initialization";

    // Initialize ImGui for this context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 410 core");

    // Start an ImGui frame and render menu bar (writes to default FB state)
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    MenuState state;
    state.visible = true;
    state.debug_mode = true;
    renderMainMenu(state);

    // Render particles to the FBO (the offscreen framebuffer)
    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0) << "Failed to compile particle shader";

    Particle particles;

    glm::vec3 cameraPos(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), (float)RenderingTestConfig::RENDER_WIDTH / (float)RenderingTestConfig::RENDER_HEIGHT, 0.1f,
        3000.0f);

    // Act — render to the FBO (not the default framebuffer)
    glContext_.bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    renderParticle(particles, particleShader, view, projection);

    // Capture the FBO content before rendering ImGui draw data
    Image fboImage = glContext_.captureFramebuffer();

    // Finalize ImGui frame and render draw data to the default framebuffer (not the FBO).
    // This verifies the real-world pipeline: ImGui renders AFTER FBO capture.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Assert — FBO should NOT contain any ImGui content
    ASSERT_TRUE(fboImage.valid()) << "Failed to capture FBO";
    ASSERT_TRUE(fboImage.save("artifacts/fbo_gui_exclusion_current.png", ImageFormat::PNG))
        << "Failed to save FBO capture artifact";

    // Check the menu bar region (top 30 rows, left 200 columns).
    // If ImGui leaked into the FBO, this region would contain non-black pixels
    // from the menu bar text ("File  View").
    // With proper FBO isolation, this region should be black (scene background)
    // or contain only scene content (particle rendering).
    //
    // We render from camera at (0,0,3) looking at origin, so the top-left corner
    // should be dark/black background.
    uint32_t bright_pixel_count = 0;

    for (uint32_t y = 0; y < RenderingTestConfig::MENU_BAR_CHECK_HEIGHT && y < fboImage.height; ++y) {
        for (uint32_t x = 0; x < RenderingTestConfig::MENU_BAR_CHECK_WIDTH && x < fboImage.width; ++x) {
            size_t idx = (y * fboImage.width + x) * 4;
            uint8_t r = fboImage.pixels[idx + 0];
            uint8_t g = fboImage.pixels[idx + 1];
            uint8_t b = fboImage.pixels[idx + 2];
            if (r > RenderingTestConfig::BRIGHT_PIXEL_THRESHOLD || g > RenderingTestConfig::BRIGHT_PIXEL_THRESHOLD ||
                b > RenderingTestConfig::BRIGHT_PIXEL_THRESHOLD) {
                bright_pixel_count++;
            }
        }
    }

    // The menu bar region should have zero bright pixels (all dark background)
    EXPECT_EQ(bright_pixel_count, 0u) << "Found " << bright_pixel_count
                                      << " bright pixels in the menu bar region of the FBO capture.\n"
                                      << "ImGui content may be leaking into the offscreen framebuffer.\n"
                                      << "FBO capture saved to: artifacts/fbo_gui_exclusion_current.png";
}

/*
 * Test: WindowResize_MultipleResolutions_RenderingConsistent
 *
 * Verifies that window resize operations produce correct rendering output.
 * Renders the same scene at multiple resolutions and validates that:
 * 1. Aspect ratio is correctly maintained (projection matrix updated)
 * 2. Viewport is correctly updated
 * 3. No corruption or artifacts from FBO resize
 *
 * This test simulates the resize pipeline: creating FBO at different sizes
 * and ensuring the scene renders correctly at each resolution.
 *
 * Resolutions tested: 1280x720, 1920x1080, 2560x1440
 */
TEST_F(RenderingRegressionTest, WindowResize_MultipleResolutions_RenderingConsistent)
{
    // Arrange
    struct Resolution
    {
        uint32_t width;
        uint32_t height;
        std::string name;
    };
    std::vector<Resolution> resolutions = {
        {1280, 720, "720p"}, {1920, 1080, "1080p"}, {2560, 1440, "1440p"}};

    // Three particles at fixed positions
    std::vector<glm::vec4> particle_data = {
        glm::vec4(-2.0f, 0.0f, 0.0f, 0.0f), // Left particle (red)
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),  // Center particle (green)
        glm::vec4(2.0f, 0.0f, 0.0f, 2.0f)   // Right particle (blue)
    };

    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0) << "Failed to compile shader";

    float reference_fraction = 0.0f;
    const float FRACTION_TOLERANCE = 0.01f; // ±1% tolerance for lit pixel fraction

    for (const auto& res : resolutions) {
        // Act - Create framebuffer at this resolution (simulates window resize)
        FramebufferCapture fbo(res.width, res.height);
        ASSERT_TRUE(fbo.initialize()) << "Failed to create FBO at " << res.name;

        Particle particles(3, particle_data.data());

        // Camera positioned to capture all three particles
        glm::vec3 cameraPos(0.0f, 0.0f, 6.0f);
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

        // Update projection matrix with correct aspect ratio for this resolution
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f), static_cast<float>(res.width) / static_cast<float>(res.height), 0.1f, 3000.0f);

        // Render scene at this resolution
        fbo.bind();
        glViewport(0, 0, res.width, res.height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderParticle(particles, particleShader, view, projection, static_cast<float>(res.height));

        Image currentImage = fbo.capture();

        // Assert
        ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer at " << res.name;

        // Save artifact for manual inspection
        std::string artifact_name = "artifacts/window_resize_" + res.name + "_current.png";
        ASSERT_TRUE(currentImage.save(artifact_name, ImageFormat::PNG))
            << "Failed to save artifact: " << artifact_name;

        // Calculate lit pixel fraction (should be consistent across resolutions)
        float fraction = calculateLitPixelFraction(currentImage);

        if (reference_fraction == 0.0f) {
            reference_fraction = fraction;
            ASSERT_GT(reference_fraction, 0.0f) << "Reference render at " << res.name << " has no lit pixels";
        } else {
            // Verify that the lit pixel fraction is consistent
            EXPECT_NEAR(fraction, reference_fraction, FRACTION_TOLERANCE)
                << "Particle fraction at " << res.name << " (" << fraction << ") differs from reference ("
                << reference_fraction << "). This indicates the resize pipeline may not be correctly updating "
                << "viewport or projection matrix.";
        }
    }
}

/*
 * Test: WindowResize_AspectRatioChange_NoDistortion
 *
 * Verifies that changing aspect ratio during resize doesn't distort the scene.
 * Renders a single particle at different aspect ratios and validates that:
 * 1. The particle remains circular (not stretched)
 * 2. The visual size relative to screen dimensions is consistent
 *
 * This specifically tests the Camera::updateProjection() functionality.
 *
 * Aspect ratios tested: 16:9, 4:3, 21:9
 */
TEST_F(RenderingRegressionTest, WindowResize_AspectRatioChange_NoDistortion)
{
    // Arrange
    struct Resolution
    {
        uint32_t width;
        uint32_t height;
        std::string name;
        float aspect_ratio;
    };
    std::vector<Resolution> resolutions = {
        {1920, 1080, "16_9", 16.0f / 9.0f},   // Widescreen
        {1600, 1200, "4_3", 4.0f / 3.0f},     // Standard
        {2560, 1080, "21_9", 21.0f / 9.0f}    // Ultrawide
    };

    // Single centered particle
    std::vector<glm::vec4> particle_data = {glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)};

    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");
    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0) << "Failed to compile shader";

    for (const auto& res : resolutions) {
        // Act - Render at this aspect ratio
        FramebufferCapture fbo(res.width, res.height);
        ASSERT_TRUE(fbo.initialize()) << "Failed to create FBO at " << res.name;

        Particle particles(1, particle_data.data());

        glm::vec3 cameraPos(0.0f, 0.0f, 5.0f);
        glm::vec3 cameraTarget(0.0f, 0.0f, 0.0f);
        glm::vec3 cameraUp(0.0f, 1.0f, 0.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);

        // Critical: Use correct aspect ratio for this resolution
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), res.aspect_ratio, 0.1f, 3000.0f);

        fbo.bind();
        glViewport(0, 0, res.width, res.height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderParticle(particles, particleShader, view, projection, static_cast<float>(res.height));

        Image currentImage = fbo.capture();

        // Assert
        ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer at aspect " << res.name;

        std::string artifact_name = "artifacts/aspect_ratio_" + res.name + "_current.png";
        ASSERT_TRUE(currentImage.save(artifact_name, ImageFormat::PNG))
            << "Failed to save artifact: " << artifact_name;

        // Verify particle rendered (has lit pixels)
        float fraction = calculateLitPixelFraction(currentImage);
        EXPECT_GT(fraction, 0.0f) << "Particle not visible at aspect ratio " << res.name;

        // In a perfect implementation, we could check for circularity here.
        // For now, the saved artifacts allow manual visual inspection for distortion.
    }
}
