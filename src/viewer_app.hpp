/*
 * viewer_app.hpp
 *
 * Main application class for Particle-Viewer.
 * Encapsulates all application state that was previously global in clutter.hpp.
 * Owns the main loop, window, rendering pipeline, and scene objects.
 *
 * Supports dependency injection: accepts an IOpenGLContext* for testability.
 * Production code uses GLFWContext; tests use MockOpenGLContext.
 *
 * Architecture: Input → Data Loading → Rendering
 */

#ifndef PARTICLE_VIEWER_VIEWER_APP_H
#define PARTICLE_VIEWER_VIEWER_APP_H

#include <string>

// clang-format off
// GLAD must come before GLFW to properly initialize OpenGL functions
#include <glad/glad.h>       // NOLINT(llvm-include-order)
#include <GLFW/glfw3.h>      // NOLINT(llvm-include-order)
// clang-format on

#include "camera.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "graphics/IOpenGLContext.hpp"
#include "particle.hpp"
#include "settingsIO.hpp"
#include "shader.hpp"
#include "ui/imgui_menu.hpp"

/*
 * Window configuration.
 */
struct WindowConfig
{
    GLint width = 0;
    GLint height = 0;
    GLint fullscreen = 0;
    bool debug_camera = false;
};

/*
 * GL object handles for the framebuffer-based rendering pipeline.
 * Includes the offscreen FBO, fullscreen quad, and particle circle VAO/VBO.
 */
struct RenderResources
{
    GLuint quad_vao = 0;
    GLuint quad_vbo = 0;
    GLuint framebuffer = 0;
    GLuint rbo = 0;
    GLuint texture_colorbuffer = 0;
    GLuint circle_vao = 0;
    GLuint circle_vbo = 0;
    Shader sphere_shader;
    Shader screen_shader;
};

/*
 * Sphere rendering parameters that scale with resolution.
 */
struct SphereParams
{
    GLfloat scale = 0.0f;
    GLfloat base_radius = 250.0f;
    GLfloat radius = 0.0f;
};

/*
 * State for recording frames to disk.
 */
struct RecordingState
{
    bool is_active = false;
    std::string folder;
    int error_count = 0;
    int error_max = 5;
};

/*
 * Paths to shader assets on disk.
 */
struct ShaderPaths
{
    std::string exe;
    std::string sphere_vertex = "/Viewer-Assets/shaders/sphereVertex.vs";
    std::string sphere_fragment = "/Viewer-Assets/shaders/sphereFragment.frag";
    std::string screen_vertex = "/Viewer-Assets/shaders/screenshader.vs";
    std::string screen_fragment = "/Viewer-Assets/shaders/screenshader.frag";
};

/*
 * ViewerApp owns all application state and manages the main loop.
 *
 * Replaces the global state previously in clutter.hpp with proper encapsulation.
 * Requires a non-null IOpenGLContext* passed to the constructor for testability.
 * Production code typically uses GLFWContext; tests use MockOpenGLContext.
 * ViewerApp does not create or own the OpenGL context itself.
 *
 * GLFW callbacks use the window user pointer to delegate to instance methods.
 *
 * Usage (production):
 *   GLFWContext context(1280, 720, "Particle-Viewer");
 *   ViewerApp app(&context);
 *   app.parseArgs(argc, argv);
 *   if (app.initialize()) {
 *       app.run();
 *   }
 *
 * Usage (testing):
 *   MockOpenGLContext context(1280, 720);
 *   ViewerApp app(&context);
 */
class ViewerApp
{
  public:
    /*
     * Construct with an injected OpenGL context (dependency injection).
     * The context must be non-null and must outlive the ViewerApp.
     * ViewerApp does not own the context.
     */
    explicit ViewerApp(IOpenGLContext* context);

    ~ViewerApp();

    // Prevent copying (owns GL and GLFW resources)
    ViewerApp(const ViewerApp&) = delete;
    ViewerApp& operator=(const ViewerApp&) = delete;

    /*
     * Parse command-line arguments (--resolution, --debug-camera).
     * Must be called before initialize().
     */
    void parseArgs(int argc, char* argv[]);

    /*
     * Initialize GLFW, OpenGL context, camera, particles, shaders, and FBO.
     * Returns true on success, false if initialization fails.
     */
    bool initialize();

    /*
     * Run the main rendering loop. Blocks until the window is closed.
     */
    void run();

  private:
    // ============================================
    // Grouped State
    // ============================================
    IOpenGLContext* context_; // non-owning; managed by caller
    WindowConfig window_;
    RenderResources render_;
    SphereParams sphere_;
    RecordingState recording_;
    ShaderPaths paths_;
    MenuState menu_state_;

    // ============================================
    // Timing
    // ============================================
    GLfloat delta_time_;
    GLfloat last_frame_;

    // ============================================
    // Input State
    // ============================================
    GLboolean keys_[1024];

    // ============================================
    // Scene Objects
    // ============================================
    Camera* cam_;
    Particle* part_;
    SettingsIO* set_;
    glm::mat4 view_;
    glm::vec3 com_;

    // ============================================
    // Frame Playback State
    // ============================================
    GLint cur_frame_;

    // ============================================
    // Pixel Buffer (for recording)
    // ============================================
    unsigned char* pixels_;

    // ============================================
    // Initialization Methods
    // ============================================
    void initScreen();
    void initPaths();
    void initImGui();
    void setResolution(const std::string& resolution);
    void setSphereScale(GLfloat scale);
    void setupCallbacks();

    // ============================================
    // Rendering Pipeline
    // ============================================
    void setupGLStuff();
    void setupScreenFBO();
    GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil);
    void beforeDraw();
    void drawScene();
    void drawFBO();
    void updateDeltaTime();

    // ============================================
    // Frame Control
    // ============================================
    void seekFrame(int frames, bool forward);
    void processMinorKeys();
    void handleLoadFile();

    // ============================================
    // Input Handling
    // ============================================
    void keyCallback(int key, int scancode, int action, int mods);

    // ============================================
    // Resource Cleanup
    // ============================================
    void cleanup();
    void shutdownImGui();

    // ============================================
    // Static GLFW Callbacks (delegate to instance via user pointer)
    // ============================================
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
};

#endif // PARTICLE_VIEWER_VIEWER_APP_H
