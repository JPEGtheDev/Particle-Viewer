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
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "visual-regression/VisualTestHelpers.hpp"
#include "Image.hpp"
#include "shader.hpp"

// Test configuration
namespace RenderingTestConfig
{
static const uint32_t RENDER_WIDTH = 800;
static const uint32_t RENDER_HEIGHT = 600;
static const float PARTICLE_TOLERANCE = VisualTestConfig::TOLERANT_THRESHOLD; // 2% for Mesa compatibility
} // namespace RenderingTestConfig

/*
 * Helper class for OpenGL rendering test setup.
 * Manages GLFW window, OpenGL context, and framebuffer for off-screen rendering.
 */
class OpenGLTestContext
{
  private:
    GLFWwindow* window_;
    GLuint fbo_;
    GLuint colorTexture_;
    GLuint depthRenderbuffer_;
    bool initialized_;

  public:
    OpenGLTestContext() : window_(nullptr), fbo_(0), colorTexture_(0), depthRenderbuffer_(0), initialized_(false)
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
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(window_);

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            glfwDestroyWindow(window_);
            glfwTerminate();
            return false;
        }

        // Set up framebuffer
        glViewport(0, 0, RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT);

        // Create framebuffer
        glGenFramebuffers(1, &fbo_);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

        // Create color attachment texture
        glGenTextures(1, &colorTexture_);
        glBindTexture(GL_TEXTURE_2D, colorTexture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture_, 0);

        // Create depth attachment renderbuffer
        glGenRenderbuffers(1, &depthRenderbuffer_);
        glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer_);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, RenderingTestConfig::RENDER_WIDTH,
                              RenderingTestConfig::RENDER_HEIGHT);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer_);

        // Check framebuffer completeness
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            cleanup();
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Enable OpenGL features
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_PROGRAM_POINT_SIZE); // Required for gl_PointSize in shader

        initialized_ = true;
        return true;
    }

    /*
     * Bind the framebuffer for rendering.
     */
    void bindFramebuffer()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glViewport(0, 0, RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT);
    }

    /*
     * Capture the current framebuffer content to an Image.
     * @return Image containing RGBA pixel data
     */
    Image captureFramebuffer()
    {
        Image image(RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
        glReadPixels(0, 0, RenderingTestConfig::RENDER_WIDTH, RenderingTestConfig::RENDER_HEIGHT, GL_RGBA,
                     GL_UNSIGNED_BYTE, image.pixels.data());

        // Normalize alpha channel to 255 (framebuffer alpha values may be inconsistent)
        for (size_t i = 3; i < image.pixels.size(); i += 4) {
            image.pixels[i] = 255;
        }

        // Flip image vertically (OpenGL origin is bottom-left, image origin is top-left)
        flipImageVertically(image);

        return image;
    }

    /*
     * Clean up OpenGL resources and GLFW.
     */
    void cleanup()
    {
        if (depthRenderbuffer_ != 0) {
            glDeleteRenderbuffers(1, &depthRenderbuffer_);
            depthRenderbuffer_ = 0;
        }
        if (colorTexture_ != 0) {
            glDeleteTextures(1, &colorTexture_);
            colorTexture_ = 0;
        }
        if (fbo_ != 0) {
            glDeleteFramebuffers(1, &fbo_);
            fbo_ = 0;
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

  private:
    /*
     * Flip image vertically (OpenGL Y axis is bottom-up, Image Y axis is top-down).
     */
    void flipImageVertically(Image& image)
    {
        uint32_t rowSize = image.width * 4; // 4 bytes per pixel (RGBA)
        std::vector<uint8_t> rowBuffer(rowSize);

        for (uint32_t y = 0; y < image.height / 2; ++y) {
            uint32_t topRow = y * rowSize;
            uint32_t bottomRow = (image.height - 1 - y) * rowSize;

            // Swap rows
            std::copy(image.pixels.begin() + topRow, image.pixels.begin() + topRow + rowSize, rowBuffer.begin());
            std::copy(image.pixels.begin() + bottomRow, image.pixels.begin() + bottomRow + rowSize,
                      image.pixels.begin() + topRow);
            std::copy(rowBuffer.begin(), rowBuffer.end(), image.pixels.begin() + bottomRow);
        }
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
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE,
                           glm::value_ptr(projection));

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
 * Extends VisualRegressionTest with OpenGL rendering capabilities.
 */
class RenderingRegressionTest : public VisualRegressionTest
{
  protected:
    OpenGLTestContext glContext_;

    void SetUp() override
    {
        VisualRegressionTest::SetUp();

        // Ensure baselines directory exists
        std::string baselines_dir = VisualTestConfig::BASELINES_DIR;
        ASSERT_TRUE(ensureDirectory(baselines_dir)) << "Failed to create baselines directory: " << baselines_dir;

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
        VisualRegressionTest::TearDown();
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
};

/*
 * Test: RenderDefaultCube_AngledView_MatchesBaseline
 *
 * Renders the default particle cube (64,000 particles in 40×40×40 grid)
 * from an angled camera position to verify 3D perspective and depth rendering.
 *
 * Camera Configuration (from debug output):
 * - Position: (-16.72, 16.49, -8.95)
 * - Target: (-15.99, 16.02, -8.04)
 * - Up: (0.08, 1.00, 0.00)
 * - Projection: Perspective, FOV=45°, Near=0.1, Far=5.0
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
                                             << "Vertex: " << vertexShaderPath
                                             << ", Fragment: " << fragmentShaderPath;

    // Arrange - Create default particle cube
    ParticleRenderer particles;
    particles.createDefaultCube();
    ASSERT_EQ(particles.getParticleCount(), 64000u) << "Default cube should have 64,000 particles";

    // Arrange - Set up camera at angled view (from debug image)
    glm::vec3 cameraPos(-16.72f, 16.49f, -8.95f);
    glm::vec3 cameraTarget(-15.99f, 16.02f, -8.04f);
    glm::vec3 cameraUp(0.08f, 1.00f, 0.00f);

    glm::mat4 view = glm::lookAt(cameraPos, cameraTarget, glm::normalize(cameraUp));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                            (float)RenderingTestConfig::RENDER_WIDTH /
                                                (float)RenderingTestConfig::RENDER_HEIGHT,
                                            0.1f, 5000.0f); // Increased far plane to 5000 for full cube visibility

    // Act - Render scene
    glContext_.bindFramebuffer();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    particles.render(particleShader, view, projection);

    // Act - Capture framebuffer
    Image currentImage = glContext_.captureFramebuffer();
    ASSERT_TRUE(currentImage.valid()) << "Failed to capture framebuffer";

    // Assert - Compare with baseline
    std::string baselinePath = VisualTestConfig::BASELINES_DIR + "/particle_cube_angle_baseline.png";
    Image baseline = Image::load(baselinePath, ImageFormat::PNG);

    if (baseline.empty()) {
        // Baseline doesn't exist yet - save current as baseline for review
        currentImage.save(baselinePath, ImageFormat::PNG);
        GTEST_SKIP() << "Baseline image not found. Current render saved to: " << baselinePath
                     << "\nPlease review and commit this baseline if correct.";
    }

    // Use tolerant comparison for Mesa software rendering compatibility
    assertVisualMatch(baseline, currentImage, "particle_cube_angle", RenderingTestConfig::PARTICLE_TOLERANCE);
}

