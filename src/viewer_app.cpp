/*
 * viewer_app.cpp
 *
 * Implementation of the ViewerApp class.
 * Contains all application logic previously spread across clutter.hpp and main.cpp.
 */

#include "viewer_app.hpp"

#include <cstdlib>
#include <iostream>
#include <string>

#include "debugOverlay.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "osFile.hpp"
#include "tinyFileDialogs/tinyfiledialogs.h"
#include "windowConfig.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

/*
 * Fullscreen quad vertex — positions (xy) + texture coords (uv).
 */
struct QuadVertex
{
    GLfloat x, y;
    GLfloat u, v;
};

// Fullscreen quad for FBO blit pass (two triangles covering NDC [-1,1])
static const QuadVertex QUAD_VERTICES[] = {{-1.0f, 1.0f, 0.0f, 1.0f}, {-1.0f, -1.0f, 0.0f, 0.0f},
                                           {1.0f, -1.0f, 1.0f, 0.0f}, {-1.0f, 1.0f, 0.0f, 1.0f},
                                           {1.0f, -1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}};

// ============================================================================
// Construction / Destruction
// ============================================================================

ViewerApp::ViewerApp(IOpenGLContext* context)
    : context_(context), imgui_initialized_(false), delta_time_(0.0f), last_frame_(0.0f), cam_(nullptr), part_(nullptr),
      set_(nullptr), view_(), com_(), cur_frame_(0), pixels_(nullptr)
{
    for (int i = 0; i < 1024; i++) {
        keys_[i] = false;
    }
    set_ = new SettingsIO();
}

ViewerApp::~ViewerApp()
{
    cleanup();
}

// ============================================================================
// Command-Line Argument Parsing
// ============================================================================

void ViewerApp::parseArgs(int argc, char* argv[])
{
    std::string resolution;
    for (int i = 1; i < argc; i++) {
        std::string arg = std::string(argv[i]);
        if (arg == "--resolution" || arg == "--res") {
            if (i + 1 < argc) {
                resolution = argv[++i];
            }
        } else if (arg == "--debug-camera" || arg == "-d") {
            window_.debug_camera = true;
        }
    }
    setResolution(resolution);
}

// ============================================================================
// Initialization
// ============================================================================

bool ViewerApp::initialize()
{
    if (!context_) {
        std::cerr << "ViewerApp::initialize() called with null context" << std::endl;
        return false;
    }

    initPaths();
    initScreen();
    initImGui();
    cam_->initGL();
    part_ = new Particle();
    setupGLStuff();
    setupScreenFBO();
    menu_state_.debug_mode = window_.debug_camera;
    return true;
}

void ViewerApp::initPaths()
{
    paths_.exe = ExePath();
    paths_.sphere_vertex = paths_.exe + paths_.sphere_vertex;
    paths_.sphere_fragment = paths_.exe + paths_.sphere_fragment;
    paths_.screen_vertex = paths_.exe + paths_.screen_vertex;
    paths_.screen_fragment = paths_.exe + paths_.screen_fragment;
}

void ViewerApp::initScreen()
{
    // Ensure context is current before any GL calls
    context_->makeCurrent();

    // Derive actual dimensions from the context.
    // Use live framebuffer size if available (handles HiDPI), but fall back
    // to the requested window size if the framebuffer reports 0×0 (can happen
    // on Wayland before the window surface is committed).
    auto fb_size = context_->getFramebufferSize();
    window_.width = fb_size.first;
    window_.height = fb_size.second;

    if (window_.width <= 0 || window_.height <= 0) {
        std::cerr << "Warning: framebuffer size is " << window_.width << "x" << window_.height
                  << ", falling back to default 1280x720" << std::endl;
        window_.width = 1280;
        window_.height = 720;
    }

    std::cout << "Framebuffer resolution: " << window_.width << "x" << window_.height << std::endl;

    // Store initial size as windowed mode size
    window_.windowed_width = window_.width;
    window_.windowed_height = window_.height;

    pixels_ = new unsigned char[window_.width * window_.height * 3];
    cam_ = new Camera(window_.width, window_.height);

    // Set up GL state that ViewerApp owns.
    context_->setSwapInterval(1);
    glViewport(0, 0, window_.width, window_.height);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    stbi_flip_vertically_on_write(true);

    // Register GLFW-specific callbacks if a native window is available
    setupCallbacks();

    // Load saved window settings (resolution, fullscreen)
    loadWindowSettings();
}

void ViewerApp::setupCallbacks()
{
    GLFWwindow* native_window = static_cast<GLFWwindow*>(context_->getNativeWindowHandle());
    if (native_window) {
        glfwSetWindowUserPointer(native_window, this);
        glfwSetKeyCallback(native_window, keyCallbackStatic);
        glfwSetFramebufferSizeCallback(native_window, framebufferSizeCallbackStatic);
    }
}

void ViewerApp::initImGui()
{
    GLFWwindow* native_window = static_cast<GLFWwindow*>(context_->getNativeWindowHandle());
    if (!native_window) {
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // Disable imgui.ini file

    ImGui::StyleColorsDark();

    // install_callbacks=true chains to existing GLFW callbacks
    bool glfw_init_ok = ImGui_ImplGlfw_InitForOpenGL(native_window, true);
    bool gl3_init_ok = ImGui_ImplOpenGL3_Init("#version 410 core");
    if (!glfw_init_ok || !gl3_init_ok) {
        if (gl3_init_ok) {
            ImGui_ImplOpenGL3_Shutdown();
        }
        if (glfw_init_ok) {
            ImGui_ImplGlfw_Shutdown();
        }
        ImGui::DestroyContext();
        return;
    }
    imgui_initialized_ = true;
}

void ViewerApp::setResolution(const std::string& resolution)
{
    // Resolution-independent scaling is handled automatically via the
    // viewportHeight shader uniform. The sphere scale is a user-configurable
    // visual size multiplier independent of resolution.
    (void)resolution; // currently unused, kept for API compatibility
    setSphereScale(1.0f);
}

void ViewerApp::setSphereScale(GLfloat scale)
{
    sphere_.scale = scale;
    sphere_.radius = sphere_.base_radius * sphere_.scale;
}

// ============================================================================
// Main Loop
// ============================================================================

void ViewerApp::run()
{
    while (!context_->shouldClose()) {
        context_->pollEvents();

        // Start ImGui frame (only if ImGui was initialized)
        if (imgui_initialized_) {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }

        cam_->Move();

        beforeDraw();
        drawScene();
        cam_->RenderSphere();
        drawFBO();

        if (imgui_initialized_) {
            if (menu_state_.debug_mode) {
                float fps = (delta_time_ > 0.0f) ? 1.0f / delta_time_ : 0.0f;
                renderCameraDebugOverlay(cam_, window_.width, window_.height, fps, PARTICLE_VIEWER_VERSION);
            }

            // Render ImGui menu and process actions
            MenuActions actions = renderMainMenu(menu_state_);
            if (actions.load_file) {
                handleLoadFile();
            }
            if (actions.change_resolution) {
                GLFWwindow* native_window = static_cast<GLFWwindow*>(context_->getNativeWindowHandle());
                if (native_window) {
                    glfwSetWindowSize(native_window, actions.target_width, actions.target_height);
                    // Update windowed size tracking and save
                    window_.windowed_width = actions.target_width;
                    window_.windowed_height = actions.target_height;
                    saveWindowSettings();
                }
            }
            if (actions.quit) {
                context_->setShouldClose(true);
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        context_->swapBuffers();

        if (set_->frames > 1) {
            set_->readPosVelFile(cur_frame_, part_, false);
        }
        if (set_->isPlaying) {
            cur_frame_++;
        }
        if (cur_frame_ > set_->frames) {
            cur_frame_ = set_->frames;
        }
        if (!imgui_initialized_ || !ImGui::GetIO().WantCaptureKeyboard) {
            processMinorKeys();
        }
        if (cur_frame_ < 0) {
            cur_frame_ = 0;
        }
    }
}

// ============================================================================
// Rendering Pipeline
// ============================================================================

void ViewerApp::setupGLStuff()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);
    render_.sphere_shader = Shader(paths_.sphere_vertex.c_str(), paths_.sphere_fragment.c_str());
    render_.screen_shader = Shader(paths_.screen_vertex.c_str(), paths_.screen_fragment.c_str());

    glGenVertexArrays(1, &render_.circle_vao);
    glGenBuffers(1, &render_.circle_vbo);
    glBindVertexArray(render_.circle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, render_.circle_vbo);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    part_->setUpInstanceArray();
    glBindVertexArray(0);
}

void ViewerApp::setupScreenFBO()
{
    glGenVertexArrays(1, &render_.quad_vao);
    glGenBuffers(1, &render_.quad_vbo);
    glBindVertexArray(render_.quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, render_.quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);

    glGenFramebuffers(1, &render_.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, render_.framebuffer);
    render_.texture_colorbuffer = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_.texture_colorbuffer, 0);
    glGenRenderbuffers(1, &render_.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, render_.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, window_.width, window_.height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_.rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint ViewerApp::generateAttachmentTexture(GLboolean depth, GLboolean stencil)
{
    GLenum attachment_type;
    if (!depth && !stencil) {
        attachment_type = GL_RGB;
    } else if (depth && !stencil) {
        attachment_type = GL_DEPTH_COMPONENT;
    } else if (!depth && stencil) {
        attachment_type = GL_STENCIL_INDEX;
    }

    GLuint texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    if (!depth && !stencil) {
        glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, window_.width, window_.height, 0, attachment_type,
                     GL_UNSIGNED_BYTE, NULL);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, window_.width, window_.height, 0, GL_DEPTH_STENCIL,
                     GL_UNSIGNED_INT_24_8, NULL);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture_id;
}

void ViewerApp::updateDeltaTime()
{
    GLfloat current_frame = context_->getTime();
    delta_time_ = current_frame - last_frame_;
    last_frame_ = current_frame;
}

void ViewerApp::beforeDraw()
{
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, render_.framebuffer);
    cam_->update(delta_time_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateDeltaTime();
    view_ = cam_->setupCam();
}

void ViewerApp::drawScene()
{
    set_->getCOM(cur_frame_, com_);
    cam_->setSphereCenter(com_);
    render_.sphere_shader.Use();
    part_->pushVBO();
    glBindVertexArray(render_.circle_vao);
    glBindBuffer(GL_ARRAY_BUFFER, part_->instanceVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUniformMatrix4fv(glGetUniformLocation(render_.sphere_shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view_));
    glUniformMatrix4fv(glGetUniformLocation(render_.sphere_shader.Program, "projection"), 1, GL_FALSE,
                       glm::value_ptr(cam_->getProjection()));
    glUniform1f(glGetUniformLocation(render_.sphere_shader.Program, "radius"), sphere_.radius);
    glUniform1f(glGetUniformLocation(render_.sphere_shader.Program, "scale"), sphere_.scale);
    GLint viewport[4] = {0, 0, 0, 0};
    glGetIntegerv(GL_VIEWPORT, viewport);
    glUniform1f(glGetUniformLocation(render_.sphere_shader.Program, "viewportHeight"),
                static_cast<GLfloat>(viewport[3]));
    glDrawArraysInstanced(GL_POINTS, 0, 1, part_->n);
    glBindVertexArray(0);

    if (set_->isPlaying && recording_.is_active) {
        glReadPixels(0, 0, (int)window_.width, (int)window_.height, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
        if (!stbi_write_tga(std::string(recording_.folder + "/" + std::to_string(cur_frame_) + ".tga").c_str(),
                            (int)window_.width, (int)window_.height, 3, pixels_)) {
            if (recording_.error_count < recording_.error_max) {
                recording_.error_count++;
                std::cout << "Unable to save image: Error " << recording_.error_count << std::endl;
            } else {
                std::cout << "Max Image Error Count Reached! Ending Recording!" << std::endl;
                recording_.is_active = false;
            }
        }
    }
}

void ViewerApp::drawFBO()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    render_.screen_shader.Use();
    glBindVertexArray(render_.quad_vao);
    glBindTexture(GL_TEXTURE_2D, render_.texture_colorbuffer);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ============================================================================
// Frame Control
// ============================================================================

void ViewerApp::seekFrame(int frames, bool forward)
{
    if (forward) {
        cur_frame_ += frames;
    } else {
        cur_frame_ -= frames;
    }
}

void ViewerApp::processMinorKeys()
{
    if (keys_[GLFW_KEY_Q]) {
        seekFrame(3, false);
    }
    if (keys_[GLFW_KEY_E]) {
        seekFrame(3, true);
    }
}

void ViewerApp::handleLoadFile()
{
    SettingsIO* new_set = set_->loadFile(part_, false);
    if (new_set && new_set != set_) {
        delete set_;
        set_ = new_set;
    }
    cur_frame_ = 0;
}

// ============================================================================
// Input Handling
// ============================================================================

void ViewerApp::keyCallback(int key, int scancode, int action, int mods)
{
    // Alt+Enter toggles fullscreen (handle first, always works)
    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS && (mods & GLFW_MOD_ALT)) {
        toggleFullscreen();
        return;
    }

    // Guard against out-of-bounds access (GLFW_KEY_UNKNOWN is -1)
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys_[key] = true;
        } else if (action == GLFW_RELEASE) {
            keys_[key] = false;
        }
    }

    // If ImGui wants keyboard input, only process menu toggle keys (F1/F3)
    if (imgui_initialized_ && ImGui::GetIO().WantCaptureKeyboard) {
        if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
            menu_state_.visible = !menu_state_.visible;
        }
        if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
            menu_state_.debug_mode = !menu_state_.debug_mode;
        }
        return;
    }

    // Only forward to camera if key is in valid range (GLFW_KEY_UNKNOWN is -1).
    if (key >= 0 && key < 1024) {
        GLFWwindow* native_window = static_cast<GLFWwindow*>(context_->getNativeWindowHandle());
        cam_->KeyReader(native_window, key, scancode, action, mods);
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        context_->setShouldClose(true);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        set_->togglePlay();
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        handleLoadFile();
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        seekFrame(1, true);
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        seekFrame(1, false);
    }
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        menu_state_.visible = !menu_state_.visible;
    }
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
        menu_state_.debug_mode = !menu_state_.debug_mode;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (!recording_.is_active) {
            recording_.error_count = 0;
            std::string dialog = "Select Folder";
            const char* fol = tinyfd_selectFolderDialog(dialog.c_str(), "");

            std::string folder;
            if (fol != NULL) {
                folder = std::string(fol);
            } else {
                folder = "";
            }

            if (folder != "") {
                recording_.folder = folder;
                recording_.is_active = true;
                return;
            }
            std::cout << "Folder not selected" << std::endl;
            recording_.is_active = false;
        } else {
            recording_.folder = "";
            recording_.is_active = false;
        }
    }
}

// ============================================================================
// Resource Cleanup
// ============================================================================

void ViewerApp::cleanup()
{
    shutdownImGui();

    delete part_;
    part_ = nullptr;
    delete[] pixels_;
    pixels_ = nullptr;

    // Delete all GL resources
    if (render_.rbo != 0) {
        glDeleteRenderbuffers(1, &render_.rbo);
        render_.rbo = 0;
    }
    if (render_.texture_colorbuffer != 0) {
        glDeleteTextures(1, &render_.texture_colorbuffer);
        render_.texture_colorbuffer = 0;
    }
    if (render_.framebuffer != 0) {
        glDeleteFramebuffers(1, &render_.framebuffer);
        render_.framebuffer = 0;
    }
    if (render_.quad_vbo != 0) {
        glDeleteBuffers(1, &render_.quad_vbo);
        render_.quad_vbo = 0;
    }
    if (render_.quad_vao != 0) {
        glDeleteVertexArrays(1, &render_.quad_vao);
        render_.quad_vao = 0;
    }
    if (render_.circle_vbo != 0) {
        glDeleteBuffers(1, &render_.circle_vbo);
        render_.circle_vbo = 0;
    }
    if (render_.circle_vao != 0) {
        glDeleteVertexArrays(1, &render_.circle_vao);
        render_.circle_vao = 0;
    }

    delete set_;
    set_ = nullptr;
    delete cam_;
    cam_ = nullptr;
    // Context cleanup is handled by the context's owner (caller), not ViewerApp.
}

// ============================================================================
// ImGui Cleanup
// ============================================================================

void ViewerApp::shutdownImGui()
{
    if (imgui_initialized_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        imgui_initialized_ = false;
    }
}

// ============================================================================
// Window Management
// ============================================================================

void ViewerApp::handleResize(int width, int height)
{
    if (width <= 0 || height <= 0) {
        return; // Invalid dimensions, ignore
    }

    window_.width = width;
    window_.height = height;

    // Update viewport
    glViewport(0, 0, width, height);

    // Update camera projection matrix
    if (cam_) {
        cam_->updateProjection(width, height);
    }

    // Resize framebuffer objects
    resizeFBO(width, height);

    // Reallocate pixel buffer for recording
    delete[] pixels_;
    pixels_ = new unsigned char[width * height * 3];
}

void ViewerApp::resizeFBO(int width, int height)
{
    // Delete old FBO attachments
    if (render_.texture_colorbuffer != 0) {
        glDeleteTextures(1, &render_.texture_colorbuffer);
        render_.texture_colorbuffer = 0;
    }
    if (render_.rbo != 0) {
        glDeleteRenderbuffers(1, &render_.rbo);
        render_.rbo = 0;
    }

    // Recreate texture attachment with new size
    glBindFramebuffer(GL_FRAMEBUFFER, render_.framebuffer);
    render_.texture_colorbuffer = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_.texture_colorbuffer, 0);

    // Recreate renderbuffer with new size
    glGenRenderbuffers(1, &render_.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, render_.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_.rbo);

    // Verify framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer incomplete after resize!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ViewerApp::toggleFullscreen()
{
    GLFWwindow* native_window = static_cast<GLFWwindow*>(context_->getNativeWindowHandle());
    if (!native_window) {
        return;
    }

    GLFWmonitor* monitor = glfwGetWindowMonitor(native_window);

    if (monitor) {
        // Currently fullscreen, switch to windowed mode
        glfwSetWindowMonitor(native_window, nullptr, 100, 100, window_.windowed_width, window_.windowed_height, GLFW_DONT_CARE);
        window_.fullscreen = 0;
    } else {
        // Currently windowed, switch to fullscreen
        // Save current window size
        glfwGetWindowSize(native_window, &window_.windowed_width, &window_.windowed_height);

        // Get primary monitor and its video mode
        GLFWmonitor* primary = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(primary);

        if (primary && mode) {
            glfwSetWindowMonitor(native_window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
            window_.fullscreen = 1;
        }
    }

    // Save updated settings
    saveWindowSettings();
}

void ViewerApp::saveWindowSettings()
{
    ensureConfigDir();
    std::string config_path = getConfigPath();

    // Save windowed size (not fullscreen size)
    bool fullscreen = (window_.fullscreen != 0);
    bool success = saveWindowConfig(config_path, window_.windowed_width, window_.windowed_height, fullscreen);

    if (!success) {
        std::cerr << "Warning: Failed to save window configuration" << std::endl;
    }
}

void ViewerApp::loadWindowSettings()
{
    std::string config_path = getConfigPath();
    int width = 0;
    int height = 0;
    bool fullscreen = false;

    if (loadWindowConfig(config_path, width, height, fullscreen)) {
        std::cout << "Loaded window config: " << width << "x" << height << " fullscreen=" << fullscreen << std::endl;

        // Apply loaded settings
        GLFWwindow* native_window = static_cast<GLFWwindow*>(context_->getNativeWindowHandle());
        if (native_window) {
            if (fullscreen) {
                // Store windowed size before going fullscreen
                window_.windowed_width = width;
                window_.windowed_height = height;

                GLFWmonitor* primary = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(primary);
                if (primary && mode) {
                    glfwSetWindowMonitor(native_window, primary, 0, 0, mode->width, mode->height, mode->refreshRate);
                    window_.fullscreen = 1;
                }
            } else {
                // Set windowed size
                window_.windowed_width = width;
                window_.windowed_height = height;
                glfwSetWindowSize(native_window, width, height);
                window_.fullscreen = 0;
            }
        }
    } else {
        std::cout << "No window config found, using defaults" << std::endl;
    }
}

// ============================================================================
// Static GLFW Callbacks
// ============================================================================

void ViewerApp::keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ViewerApp* app = static_cast<ViewerApp*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->keyCallback(key, scancode, action, mods);
    }
}

void ViewerApp::framebufferSizeCallbackStatic(GLFWwindow* window, int width, int height)
{
    ViewerApp* app = static_cast<ViewerApp*>(glfwGetWindowUserPointer(window));
    if (app) {
        app->handleResize(width, height);
    }
}
