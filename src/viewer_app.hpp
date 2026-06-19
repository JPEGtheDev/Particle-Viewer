/*
 * viewer_app.hpp
 *
 * Main application class for Particle-Viewer.
 * Encapsulates all application state that was previously global in clutter.hpp.
 * Owns the main loop, window, rendering pipeline, and scene objects.
 *
 * Supports dependency injection: accepts an IOpenGLContext* for testability.
 * Production code typically uses SDL3Context; tests use MockOpenGLContext.
 *
 * Architecture: Input → Data Loading → Rendering
 */

#ifndef PARTICLE_VIEWER_VIEWER_APP_H
#define PARTICLE_VIEWER_VIEWER_APP_H

#include <memory>
#include <string>

// clang-format off
// GLAD must come before other OpenGL-related headers
#include <glad/glad.h>       // NOLINT(llvm-include-order)
#include <SDL3/SDL.h>        // NOLINT(llvm-include-order) — for Uint64
// clang-format on

#include "COMCache.hpp"
#include "COMFileProvider.hpp"
#include "FrameCache.hpp"
#include "IFileDialog.hpp"
#include "ThreadedExecutor.hpp"
#include "camera.hpp"
#include "constants.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "graphics/IOpenGLContext.hpp"
#include "input/gamepad_input.hpp"
#include "input/input_mode.hpp"
#include "particle.hpp"
#include "recording_state.hpp"
#include "settingsIO.hpp"
#include "shader.hpp"
#include "ui/imgui_file_dialog.hpp"
#include "ui/imgui_menu.hpp"
#include "viewerConfig.hpp"

/*
 * Render mode controls which particle rendering algorithm is used.
 * The app always starts in Spheres mode.
 * MarchingCubes is a placeholder for Story 2 (always greyed out for now).
 */
enum class RenderMode
{
    Spheres,
    ScreenSpaceMetaballs,
    MarchingCubes,
};

/*
 * Window configuration.
 */
struct WindowConfig
{
    GLint width = 0;
    GLint height = 0;
    GLint fullscreen = 0;
    bool debug_camera = false;
    // Store windowed mode size and position for restoring from fullscreen
    GLint windowed_width = 0;
    GLint windowed_height = 0;
    GLint windowed_x = 0;
    GLint windowed_y = 0;
    // UI scale factor (0.0f = sentinel meaning "no saved preference")
    float ui_scale = 0.0f;
    // Screen-Space Metaballs parameters (persisted to window.cfg)
    float ssm_threshold = 0.5f;   // valid range [0.0, 1.0]
    float ssm_blob_radius = 2.0f; // valid range [0.1, 10.0]
    float ssm_blur_amount = 3.0f; // valid range [0.0, 20.0]
};

/*
 * GL object handles for the Screen-Space Metaballs rendering pipeline.
 * density_fbo accumulates per-particle falloff contributions into a float texture.
 * blurred_fbo holds the optional box-blurred density field.
 * float_fbo_supported is false if GL_RGBA32F framebuffers are unavailable at runtime;
 * when false, SSM mode is greyed out with tooltip "Mode not supported".
 */
struct SSMResources
{
    GLuint density_fbo = 0;
    GLuint density_texture = 0;
    GLuint intermediate_fbo = 0;
    GLuint intermediate_texture = 0;
    GLuint blurred_fbo = 0;
    GLuint blurred_texture = 0;
    bool float_fbo_supported = false;
    Shader splat_shader;
    Shader blur_shader;
    Shader composite_shader;
};

/*
 * GL object handles for the framebuffer-based rendering pipeline.
 * Includes the offscreen FBO, fullscreen quad, particle circle VAO/VBO,
 * and SSM resources for the Screen-Space Metaballs pass.
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
    SSMResources ssm;
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
 * Paths to shader assets on disk.
 */
struct ShaderPaths
{
    std::string exe;
    std::string sphere_vertex = "/Viewer-Assets/shaders/sphereVertex.vs";
    std::string sphere_fragment = "/Viewer-Assets/shaders/sphereFragment.frag";
    std::string screen_vertex = "/Viewer-Assets/shaders/screenshader.vs";
    std::string screen_fragment = "/Viewer-Assets/shaders/screenshader.frag";
    std::string font;
    std::string ssm_splat_vertex = "/Viewer-Assets/shaders/metaball_splat.vert";
    std::string ssm_splat_fragment = "/Viewer-Assets/shaders/metaball_splat.frag";
    std::string ssm_blur_vertex = "/Viewer-Assets/shaders/metaball_blur.vert";
    std::string ssm_blur_fragment = "/Viewer-Assets/shaders/metaball_blur.frag";
    std::string ssm_composite_fragment = "/Viewer-Assets/shaders/metaball_composite.frag";
};

/*
 * ViewerApp owns all application state and manages the main loop.
 *
 * Replaces the global state previously in clutter.hpp with proper encapsulation.
 * Requires a non-null IOpenGLContext* passed to the constructor for testability.
 * Production code typically uses SDL3Context; tests use MockOpenGLContext.
 * ViewerApp does not create or own the OpenGL context itself.
 *
 * SDL3 events are polled directly in run() via SDL_PollEvent().
 *
 * Usage (production):
 *   SDL3Context context(1280, 720, "Particle-Viewer");
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

    // Prevent copying (owns GL and SDL3 resources)
    ViewerApp(const ViewerApp&) = delete;
    ViewerApp& operator=(const ViewerApp&) = delete;

    /*
     * Parse command-line arguments (--resolution, --debug-camera).
     * Must be called before initialize().
     */
    void parseArgs(int argc, char* argv[]);

    /*
     * Initialize SDL3 context, OpenGL, camera, particles, shaders, and FBO.
     * Returns true on success, false if initialization fails.
     */
    bool initialize();

    /*
     * Run the main rendering loop. Blocks until the window is closed.
     */
    void run();

    // ============================================
    // Test Injection
    // ============================================
    // Replace production dialog with a mock. Non-owning; caller retains the mock.
    void setFileDialog(IFileDialog* d)
    {
        file_dialog_ = d;
    }
    void setRecordingDialog(IFileDialog* d)
    {
        recording_dialog_ = d;
    }

    // Nav timer delays (milliseconds) — public so tests can assert on them
    static constexpr Uint64 NAV_INITIAL_DELAY_MS = 300; // delay before first D-pad repeat
    static constexpr Uint64 NAV_REPEAT_DELAY_MS = 150;  // interval between subsequent repeats
    static_assert(NAV_REPEAT_DELAY_MS < NAV_INITIAL_DELAY_MS, "Repeat delay must be shorter than initial delay");
    // Left-stick Y threshold for panel navigation (normalized 0..1)
    static constexpr float NAV_STICK_THRESHOLD = 0.5f;
    static_assert(NAV_STICK_THRESHOLD > 0.0f && NAV_STICK_THRESHOLD < 1.0f, "Stick threshold must be in (0, 1)");

    // Cycles through available (non-greyed) render modes: Spheres → SSM → Spheres.
    // MarchingCubes is always skipped (always greyed — placeholder for Story 2).
    static constexpr RenderMode cycleRenderMode(RenderMode current)
    {
        switch (current) {
            case RenderMode::Spheres:
                return RenderMode::ScreenSpaceMetaballs;
            case RenderMode::ScreenSpaceMetaballs:
                return RenderMode::Spheres;
            case RenderMode::MarchingCubes:
                return RenderMode::Spheres;
        }
        return RenderMode::Spheres;
    }

    // visible for testing — pure clamping helper used by processMenuNavigation()
    static int applyNavMove(int selected, int count, int delta)
    {
        if (count <= 0)
            return selected;
        if (selected < 0)
            return (delta != 0) ? 0 : selected;
        const int next = selected + delta;
        if (next < 0)
            return 0;
        if (next >= count)
            return count - 1;
        return next;
    }

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
    RenderMode render_mode_ = RenderMode::Spheres;
    bool imgui_initialized_;

    // ============================================
    // Timing
    // ============================================
    GLfloat delta_time_;
    GLfloat last_frame_;

    // ============================================
    // Input State
    // ============================================
    GLboolean keys_[1024];
    GamepadInput gamepad_;
    InputMode current_mode_ = InputMode::ViewMode; // ViewMode or MenuMode
    Uint64 nav_last_nav_time_ms_ = 0;              // SDL tick timestamp of last D-pad navigation event
    bool nav_had_initial_repeat_ = false; // true after the first auto-repeat fires (switches to NAV_REPEAT_DELAY_MS)

    // ============================================
    // Scene Objects
    // ============================================
    Camera* cam_;
    Particle* part_;
    SettingsIO* set_;
    glm::mat4 view_;
    glm::vec3 com_;

    // ============================================
    // COM / Frame Cache Infrastructure
    // ============================================
    ThreadedExecutor* com_executor_;
    ThreadedExecutor* frame_executor_;
    COMCache* com_cache_;
    FrameCache* frame_cache_;
    COMFileProvider* com_file_provider_;

    // COM auto-compute toggle (persisted in viewer.cfg per simulation folder)
    bool auto_com_compute_;

    // Cached result of checkCOM(). Updated by rebuildCacheInfrastructure(),
    // cleared by teardownCacheInfrastructure(). Avoids per-frame disk stat.
    bool com_file_present_;

    // ============================================
    // Frame Playback State
    // ============================================
    GLint cur_frame_;

    // ============================================
    // Pixel Buffer (for recording)
    // ============================================
    unsigned char* pixels_;

    // ============================================
    // Dialog State
    // ============================================
    // Owned production dialog instances (null until initialize())
    std::unique_ptr<ImGuiFolderBrowser> file_browser_;
    std::unique_ptr<ImGuiFolderBrowser> recording_browser_;

    // Active dialog pointers (non-owning; point to owned instances OR test mocks)
    IFileDialog* file_dialog_ = nullptr;
    IFileDialog* recording_dialog_ = nullptr;

    bool file_dialog_open_ = false;
    bool recording_dialog_open_ = false;

    // Last-confirmed simulation folder (persisted in window.cfg)
    std::string last_confirmed_folder_;

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
    // Window Management
    // ============================================
    void handleResize(int width, int height);
    void resizeFBO(int width, int height);
    void toggleFullscreen();
    void saveWindowSettings();
    void loadWindowSettings();

    // ============================================
    // UI Scale
    // ============================================
    bool scale_pending_ = false; // true when a font-atlas rebuild is scheduled
    void applyUiScale();         // full atlas rebuild: Destroy→Clear→Add→Build→Create

    // ============================================
    // Rendering Pipeline
    // ============================================
    void setupGLStuff();
    void setupScreenFBO();
    void initSSMResources();
    void drawSSMScene();
    GLuint generateAttachmentTexture(GLboolean depth, GLboolean stencil);
    void beforeDraw();
    void drawScene();
    void drawFBO();
    void updateDeltaTime();

    // ============================================
    // Frame Control
    // ============================================
    void seekFrame(int frames, bool forward);
    void pauseIfPlaying();
    void processMinorKeys();
    void handleLoadFromFolder(const std::string& folder);
    void openRecordingFolderDialog();

    // ============================================
    // Input Handling
    // ============================================
    void handleKeyEvent(unsigned int scancode, bool is_pressed, unsigned int mods);
    void processGamepadInput();
    void toggleControllerPanel(); // toggle MenuMode ↔ ViewMode and panel visibility
    void processMenuNavigation(); // D-pad repeat timer and A-confirm for panel navigation

    // ============================================
    // Resource Cleanup
    // ============================================
    void cleanup();
    void shutdownImGui();

    // ============================================
    // Helpers
    // ============================================
    static std::string extractFolder(const std::string& posName);

    void rebuildCacheInfrastructure();
    void teardownCacheInfrastructure();
    void createCOMInfrastructure();
    void teardownCOMInfrastructure();

    static constexpr std::size_t FRAME_CACHE_CAPACITY_BYTES = 256ULL * 1024 * 1024;
    static constexpr long PREFETCH_LOOKAHEAD_FRAMES = 64;
};

#endif // PARTICLE_VIEWER_VIEWER_APP_H
