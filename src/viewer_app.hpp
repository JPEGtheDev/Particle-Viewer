/*
 * viewer_app.hpp
 *
 * Main application class for Particle-Viewer.
 * Encapsulates all application state that was previously global in clutter.hpp.
 * Owns the main loop, window, rendering pipeline, and scene objects.
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
#include "particle.hpp"
#include "settingsIO.hpp"
#include "shader.hpp"

/*
 * ViewerApp owns all application state and manages the main loop.
 *
 * Replaces the global state previously in clutter.hpp with proper encapsulation.
 * GLFW callbacks use the window user pointer to delegate to instance methods.
 *
 * Usage:
 *   ViewerApp app;
 *   app.parseArgs(argc, argv);
 *   if (app.initialize()) {
 *       app.run();
 *   }
 */
class ViewerApp
{
  public:
    ViewerApp();
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
    // Window State
    // ============================================
    GLFWwindow* window_;
    GLint screen_width_;
    GLint screen_height_;
    GLint screen_fullscreen_;
    bool debug_camera_;

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
    // Rendering Objects
    // ============================================
    GLuint quad_vao_;
    GLuint quad_vbo_;
    GLuint framebuffer_;
    GLuint rbo_;
    GLuint texture_colorbuffer_;
    GLuint circle_vao_;
    GLuint circle_vbo_;
    Shader sphere_shader_;
    Shader screen_shader_;

    // ============================================
    // Scene Objects
    // ============================================
    Camera* cam_;
    Particle* part_;
    SettingsIO* set_;
    glm::mat4 view_;
    glm::vec3 com_;

    // ============================================
    // Sphere Rendering Parameters
    // ============================================
    GLfloat sphere_scale_;
    GLfloat sphere_base_radius_;
    GLfloat sphere_radius_;

    // ============================================
    // Recording State
    // ============================================
    bool is_recording_;
    std::string record_folder_;
    int image_error_;
    int image_error_max_;

    // ============================================
    // Asset Paths
    // ============================================
    std::string exe_path_;
    std::string sphere_vertex_shader_path_;
    std::string sphere_fragment_shader_path_;
    std::string screen_vertex_shader_path_;
    std::string screen_fragment_shader_path_;

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
    void initScreen(const char* title);
    void initPaths();
    void setResolution(const std::string& resolution);
    void setSphereScale(GLfloat scale);

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

    // ============================================
    // Input Handling
    // ============================================
    void keyCallback(int key, int scancode, int action, int mods);

    // ============================================
    // Resource Cleanup
    // ============================================
    void cleanup();

    // ============================================
    // Static GLFW Callbacks (delegate to instance via user pointer)
    // ============================================
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void errorCallbackStatic(int error, const char* description);
};

#endif // PARTICLE_VIEWER_VIEWER_APP_H
