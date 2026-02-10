/*
 * RenderingRegressionTests.cpp
 *
 * Visual regression tests for real-world particle rendering scenarios.
 * These tests verify that the OpenGL rendering pipeline produces consistent output
 * for various camera angles and particle configurations.
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

// Include GLAD before GLFW to properly load OpenGL functions
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Image.hpp"
#include "glad/glad.h"
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
 * Manages GLFW window and OpenGL context for off-screen rendering.
 * Uses FramebufferCapture utility for framebuffer management.
 */
class OpenGLTestContext
{
  private:
    GLFWwindow* window_;
    FramebufferCapture* framebuffer_;
    bool initialized_;

  public:
    OpenGLTestContext() : window_(nullptr), framebuffer_(nullptr), initialized_(false)
    {
    }

    ~OpenGLTestContext()
    {
        cleanup();
    }

    /*
     * Initialize GLFW, create window, and set up framebuffer for off-screen rendering.
     * @return true if initialization succeeds
     */
    bool initialize()
    {
        // Initialize GLFW
        if (!glfwInit()) {
            return false;
        }

        // Track that GLFW was initialized so cleanup() will terminate it on failure
        initialized_ = true;

        // Set OpenGL version and profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Headless

#ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

        // Create window
        window_ = glfwCreateWindow(RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT,
                                   "Rendering Test", NULL, NULL);
        if (!window_) {
            cleanup();
            return false;
        }

        glfwMakeContextCurrent(window_);

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            cleanup();
            return false;
        }

        // Set up viewport
        glViewport(0, 0, RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT);

        // Create framebuffer using FramebufferCapture utility
        framebuffer_ = new FramebufferCapture(RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT);
        if (!framebuffer_->initialize()) {
            cleanup();
            return false;
        }

        // Enable OpenGL features
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE); // Required for gl_PointSize in shader

        return true;
    }

    /*
     * Bind the framebuffer for rendering.
     */
    void bindFramebuffer()
    {
        if (framebuffer_) {
            framebuffer_->bind();
        }
    }

    /*
     * Capture the current framebuffer content to an Image.
     * @return Image containing RGBA pixel data
     */
    Image captureFramebuffer()
    {
        if (framebuffer_) {
            return framebuffer_->capture();
        }
        return Image();
    }

    /*
     * Clean up OpenGL resources and GLFW.
     */
    void cleanup()
    {
        if (framebuffer_ != nullptr) {
            delete framebuffer_;
            framebuffer_ = nullptr;
        }
        if (window_ != nullptr) {
            glfwDestroyWindow(window_);
            window_ = nullptr;
        }
        if (initialized_) {
            glfwTerminate();
            initialized_ = false;
        }
    }

    bool isInitialized() const
    {
        return initialized_;
    }
};

/*
 * Helper class for creating and rendering particle data.
 * Manages VBO for instanced particle rendering.
 */
class ParticleRenderer
{
  private:
    GLuint vao_;
    GLuint vbo_;
    std::vector<glm::vec4> particleData_;

  public:
    ParticleRenderer() : vao_(0), vbo_(0)
    {
    }

    ~ParticleRenderer()
    {
        cleanup();
    }

    /*
     * Create the default particle cube (40x40x40 grid, 64,000 particles).
     * This matches the default cube generated in Particle() constructor.
     */
    void createDefaultCube()
    {
        const int numParticles = 64000;
        const float spacing = 1.25f;
        const float colorVal = 500.0f; // Trigger rainbow coloring in shader

        particleData_.resize(numParticles);

        for (int i = 0; i < numParticles; i++) {
            float x = (i % 40) * spacing;
            float y = (i % 1600) / 40.0f * spacing;
            float z = (i % 64000) / 1600.0f * spacing;
            particleData_[i] = glm::vec4(x, y, z, colorVal);
        }

        setupBuffers();
    }

    /*
     * Set up OpenGL buffers for instanced rendering.
     */
    void setupBuffers()
    {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);

        glBindVertexArray(vao_);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);
        glBufferData(GL_ARRAY_BUFFER, particleData_.size() * sizeof(glm::vec4), particleData_.data(), GL_STATIC_DRAW);

        // Set up vertex attribute (layout location 0 = offset vec4)
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribDivisor(0, 1); // Instanced

        glBindVertexArray(0);
    }

    /*
     * Render particles using instanced drawing.
     * @param shader Shader program to use
     * @param view View matrix
     * @param projection Projection matrix
     */
    void render(Shader& shader, const glm::mat4& view, const glm::mat4& projection)
    {
        shader.Use();

        // Set uniforms
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Render parameters (matching main.cpp defaults)
        glUniform1f(glGetUniformLocation(shader.Program, "radius"), 100.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "scale"), 5.0f);
        glUniform1f(glGetUniformLocation(shader.Program, "transScale"), 0.25f);
        glUniform3fv(glGetUniformLocation(shader.Program, "lightDirection"), 1,
                     glm::value_ptr(glm::vec3(0.1f, 0.1f, 0.85f)));

        // Draw
        glBindVertexArray(vao_);
        glDrawArraysInstanced(GL_POINTS, 0, 1, particleData_.size());
        glBindVertexArray(0);
    }

    size_t getParticleCount() const
    {
        return particleData_.size();
    }

  private:
    void cleanup()
    {
        if (vbo_ != 0) {
            glDeleteBuffers(1, &vbo_);
            vbo_ = 0;
        }
        if (vao_ != 0) {
            glDeleteVertexArrays(1, &vao_);
            vao_ = 0;
        }
    }
};

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

        // Create directory if it doesn't exist
        std::string command = "mkdir -p " + baselines_dir;
        int result = std::system(command.c_str());
        ASSERT_EQ(result, 0) << "Failed to create baselines directory: " << baselines_dir;

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
    // Arrange - Set up shaders
    std::string vertexShaderPath = getShaderPath("sphereVertex.vs");
    std::string fragmentShaderPath = getShaderPath("sphereFragment.frag");

    Shader particleShader(vertexShaderPath.c_str(), fragmentShaderPath.c_str());
    ASSERT_TRUE(particleShader.Program != 0) << "Failed to compile particle shader. "
                                             << "Vertex: " << vertexShaderPath << ", Fragment: " << fragmentShaderPath;

    // Arrange - Create default particle cube
    ParticleRenderer particles;
    particles.createDefaultCube();
    ASSERT_EQ(particles.getParticleCount(), 64000u) << "Default cube should have 64,000 particles";

    // Arrange - Set up camera at angled view to match reference image composition
    // Reference image shows cube smaller with more blank space around it.
    // Adjusted camera position to be further back for proper framing:
    // - Maintains same viewing direction as reference
    // - Increased distance from cube center for more blank space
    // - Camera moved from ~29 units to ~51 units from cube center
    glm::vec3 cameraPos(-23.60f, 25.21f, -30.93f);
    glm::vec3 cameraTarget(-23.02f, 24.83f, -30.20f);
    glm::vec3 cameraUp(0.08f, 1.00f, 0.00f);

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, cameraUp);
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f), (float)RenderingTestConfig::RENDER_WIDTH / (float)RenderingTestConfig::RENDER_HEIGHT, 0.1f,
        3000.0f); // Near=0.1, Far=3000 from reference

    // Act - Render scene
    glContext_.bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    particles.render(particleShader, view, projection);

    // Act - Capture framebuffer
    Image currentImage = glContext_.captureFramebuffer();
    ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer";

    // Assert - Compare with baseline
    std::string baselinePath = getBaselinePath("particle_cube_angle_baseline.png");
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);

    if (baseline.empty()) {
        // Baseline doesn't exist yet - save current as baseline for review
        std::string localBaselinePath = RenderingTestConfig::BASELINES_DIR + "/particle_cube_angle_baseline.png";
        currentImage.save(localBaselinePath, ImageFormat::PNG);
        GTEST_SKIP() << "Baseline image not found. Current render saved to: " << localBaselinePath
                     << "\nPlease review and commit this baseline if correct.";
    }

    // Use tolerant comparison for Mesa software rendering compatibility
    // Save current image as artifact for inspection (always, even on pass)
    currentImage.save("artifacts/particle_cube_angle_current.png", ImageFormat::PNG);

    // Compare images
    PixelComparator comparator;
    ComparisonResult result = comparator.compare(baseline, currentImage, RenderingTestConfig::PARTICLE_TOLERANCE, true);

    // Save diff image if there's a mismatch
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
