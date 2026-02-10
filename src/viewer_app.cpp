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
#include "osFile.hpp"
#include "tinyFileDialogs/tinyfiledialogs.h"

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

ViewerApp::ViewerApp()
    : window_(nullptr), screen_width_(0), screen_height_(0), screen_fullscreen_(0), debug_camera_(false),
      delta_time_(0.0f), last_frame_(0.0f), quad_vao_(0), quad_vbo_(0), framebuffer_(0), rbo_(0),
      texture_colorbuffer_(0), circle_vao_(0), circle_vbo_(0), cam_(nullptr), part_(nullptr), set_(nullptr), view_(),
      com_(), sphere_scale_(0.0f), sphere_base_radius_(250.0f), sphere_radius_(0.0f), is_recording_(false),
      image_error_(0), image_error_max_(5), sphere_vertex_shader_path_("/Viewer-Assets/shaders/sphereVertex.vs"),
      sphere_fragment_shader_path_("/Viewer-Assets/shaders/sphereFragment.frag"),
      screen_vertex_shader_path_("/Viewer-Assets/shaders/screenshader.vs"),
      screen_fragment_shader_path_("/Viewer-Assets/shaders/screenshader.frag"), cur_frame_(0), pixels_(nullptr)
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
            debug_camera_ = true;
        }
    }
    setResolution(resolution);
}

// ============================================================================
// Initialization
// ============================================================================

bool ViewerApp::initialize()
{
    initPaths();
    initScreen("Particle-Viewer");
    if (!window_) {
        return false;
    }
    cam_->initGL();
    part_ = new Particle();
    setupGLStuff();
    setupScreenFBO();
    return true;
}

void ViewerApp::initPaths()
{
    exe_path_ = ExePath();
    sphere_vertex_shader_path_ = exe_path_ + sphere_vertex_shader_path_;
    sphere_fragment_shader_path_ = exe_path_ + sphere_fragment_shader_path_;
    screen_vertex_shader_path_ = exe_path_ + screen_vertex_shader_path_;
    screen_fragment_shader_path_ = exe_path_ + screen_fragment_shader_path_;
}

void ViewerApp::initScreen(const char* title)
{
    pixels_ = new unsigned char[screen_width_ * screen_height_ * 3];
    cam_ = new Camera(screen_width_, screen_height_);
    glfwSetErrorCallback(errorCallbackStatic);
    if (!glfwInit()) {
        return;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    window_ = glfwCreateWindow(screen_width_, screen_height_, title, NULL, NULL);
    if (!window_) {
        glfwTerminate();
        return;
    }

    // Store 'this' pointer for GLFW callbacks
    glfwSetWindowUserPointer(window_, this);
    glfwSetKeyCallback(window_, keyCallbackStatic);
    glfwMakeContextCurrent(window_);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD: unable to load OpenGL function pointers." << std::endl;
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
        return;
    }

    glfwSwapInterval(1);
    glViewport(0, 0, screen_width_, screen_height_);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    stbi_flip_vertically_on_write(true);
}

void ViewerApp::setResolution(const std::string& resolution)
{
    // Sphere scale is computed as: scale = (screen_height / 720.0) ^ 0.6
    // This approximates the hand-tuned values: 720→1.0, 1080→1.25, 2160→1.75
    // TODO: make rendering resolution-independent on the shader side
    GLfloat scale;
    if (resolution == "4k") {
        screen_width_ = 3840;
        screen_height_ = 2160;
        scale = 1.75;
    } else if (resolution == "1080" || resolution == "1080p" || resolution == "HD") {
        screen_width_ = 1920;
        screen_height_ = 1080;
        scale = 1.25;
    } else {
        screen_width_ = 1280;
        screen_height_ = 720;
        scale = 1.0;
    }
    std::cout << "Setting resolution to:" << screen_width_ << "x" << screen_height_ << std::endl;
    setSphereScale(scale);
}

void ViewerApp::setSphereScale(GLfloat scale)
{
    sphere_scale_ = scale;
    sphere_radius_ = sphere_base_radius_ * sphere_scale_;
}

// ============================================================================
// Main Loop
// ============================================================================

void ViewerApp::run()
{
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        cam_->Move();

        beforeDraw();
        drawScene();
        cam_->RenderSphere();
        drawFBO();

        if (debug_camera_) {
            renderCameraDebugOverlay(cam_, screen_width_, screen_height_);
        }

        glfwSwapBuffers(window_);

        if (set_->frames > 1) {
            set_->readPosVelFile(cur_frame_, part_, false);
        }
        if (set_->isPlaying) {
            cur_frame_++;
        }
        if (cur_frame_ > set_->frames) {
            cur_frame_ = set_->frames;
        }
        processMinorKeys();
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
    sphere_shader_ = Shader(sphere_vertex_shader_path_.c_str(), sphere_fragment_shader_path_.c_str());
    screen_shader_ = Shader(screen_vertex_shader_path_.c_str(), screen_fragment_shader_path_.c_str());

    glGenVertexArrays(1, &circle_vao_);
    glGenBuffers(1, &circle_vbo_);
    glBindVertexArray(circle_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, circle_vbo_);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    part_->setUpInstanceArray();
    glBindVertexArray(0);
}

void ViewerApp::setupScreenFBO()
{
    glGenVertexArrays(1, &quad_vao_);
    glGenBuffers(1, &quad_vbo_);
    glBindVertexArray(quad_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)(2 * sizeof(GLfloat)));
    glBindVertexArray(0);

    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    texture_colorbuffer_ = generateAttachmentTexture(false, false);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_colorbuffer_, 0);
    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screen_width_, screen_height_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo_);
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
        glTexImage2D(GL_TEXTURE_2D, 0, attachment_type, screen_width_, screen_height_, 0, attachment_type,
                     GL_UNSIGNED_BYTE, NULL);
    } else {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, screen_width_, screen_height_, 0, GL_DEPTH_STENCIL,
                     GL_UNSIGNED_INT_24_8, NULL);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture_id;
}

void ViewerApp::updateDeltaTime()
{
    GLfloat current_frame = glfwGetTime();
    delta_time_ = current_frame - last_frame_;
    last_frame_ = current_frame;
}

void ViewerApp::beforeDraw()
{
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);
    cam_->update(delta_time_);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    updateDeltaTime();
    view_ = cam_->setupCam();
}

void ViewerApp::drawScene()
{
    set_->getCOM(cur_frame_, com_);
    cam_->setSphereCenter(com_);
    sphere_shader_.Use();
    part_->pushVBO();
    glBindVertexArray(circle_vao_);
    glBindBuffer(GL_ARRAY_BUFFER, part_->instanceVBO);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glUniformMatrix4fv(glGetUniformLocation(sphere_shader_.Program, "view"), 1, GL_FALSE, glm::value_ptr(view_));
    glUniformMatrix4fv(glGetUniformLocation(sphere_shader_.Program, "projection"), 1, GL_FALSE,
                       glm::value_ptr(cam_->getProjection()));
    glUniform1f(glGetUniformLocation(sphere_shader_.Program, "radius"), sphere_radius_);
    glUniform1f(glGetUniformLocation(sphere_shader_.Program, "scale"), sphere_scale_);
    glDrawArraysInstanced(GL_POINTS, 0, 1, part_->n);
    glBindVertexArray(0);

    if (set_->isPlaying && is_recording_) {
        glReadPixels(0, 0, (int)screen_width_, (int)screen_height_, GL_RGB, GL_UNSIGNED_BYTE, pixels_);
        if (!stbi_write_tga(std::string(record_folder_ + "/" + std::to_string(cur_frame_) + ".tga").c_str(),
                            (int)screen_width_, (int)screen_height_, 3, pixels_)) {
            if (image_error_ < image_error_max_) {
                image_error_++;
                std::cout << "Unable to save image: Error " << image_error_ << std::endl;
            } else {
                std::cout << "Max Image Error Count Reached! Ending Recording!" << std::endl;
                is_recording_ = false;
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
    screen_shader_.Use();
    glBindVertexArray(quad_vao_);
    glBindTexture(GL_TEXTURE_2D, texture_colorbuffer_);
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

// ============================================================================
// Input Handling
// ============================================================================

void ViewerApp::keyCallback(int key, int scancode, int action, int mods)
{
    // Guard against out-of-bounds access (GLFW_KEY_UNKNOWN is -1)
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            keys_[key] = true;
        } else if (action == GLFW_RELEASE) {
            keys_[key] = false;
        }
    }

    cam_->KeyReader(window_, key, scancode, action, mods);

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        set_->togglePlay();
    }
    if (key == GLFW_KEY_T && action == GLFW_PRESS) {
        SettingsIO* new_set = set_->loadFile(part_, false);
        if (new_set && new_set != set_) {
            delete set_;
            set_ = new_set;
        }
        cur_frame_ = 0;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        seekFrame(1, true);
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        seekFrame(1, false);
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (!is_recording_) {
            image_error_ = 0;
            std::string dialog = "Select Folder";
            const char* fol = tinyfd_selectFolderDialog(dialog.c_str(), "");

            std::string folder;
            if (fol != NULL) {
                folder = std::string(fol);
            } else {
                folder = "";
            }

            if (folder != "") {
                record_folder_ = folder;
                is_recording_ = true;
                return;
            }
            std::cout << "Folder not selected" << std::endl;
            is_recording_ = false;
        } else {
            record_folder_ = "";
            is_recording_ = false;
        }
    }
}

// ============================================================================
// Resource Cleanup
// ============================================================================

void ViewerApp::cleanup()
{
    delete part_;
    part_ = nullptr;
    delete[] pixels_;
    pixels_ = nullptr;

    // Delete all GL resources
    if (rbo_ != 0) {
        glDeleteRenderbuffers(1, &rbo_);
        rbo_ = 0;
    }
    if (texture_colorbuffer_ != 0) {
        glDeleteTextures(1, &texture_colorbuffer_);
        texture_colorbuffer_ = 0;
    }
    if (framebuffer_ != 0) {
        glDeleteFramebuffers(1, &framebuffer_);
        framebuffer_ = 0;
    }
    if (quad_vbo_ != 0) {
        glDeleteBuffers(1, &quad_vbo_);
        quad_vbo_ = 0;
    }
    if (quad_vao_ != 0) {
        glDeleteVertexArrays(1, &quad_vao_);
        quad_vao_ = 0;
    }
    if (circle_vbo_ != 0) {
        glDeleteBuffers(1, &circle_vbo_);
        circle_vbo_ = 0;
    }
    if (circle_vao_ != 0) {
        glDeleteVertexArrays(1, &circle_vao_);
        circle_vao_ = 0;
    }

    delete set_;
    set_ = nullptr;
    delete cam_;
    cam_ = nullptr;
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
        glfwTerminate();
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

void ViewerApp::errorCallbackStatic(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
