/*
 * viewer_app.cpp
 *
 * Implementation of the ViewerApp class.
 * Contains all application logic previously spread across clutter.hpp and main.cpp.
 */

#include "viewer_app.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

// clang-format off
#include <SDL3/SDL.h>          // NOLINT(llvm-include-order)
// clang-format on

#include "MassParams.hpp"
#include "debugOverlay.hpp"
#include "file_dialog_helpers.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"
#include "osFile.hpp"
#include "uiScale.hpp"
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
static const std::array<QuadVertex, 6> QUAD_VERTICES = {{{-1.0f, 1.0f, 0.0f, 1.0f},
                                                         {-1.0f, -1.0f, 0.0f, 0.0f},
                                                         {1.0f, -1.0f, 1.0f, 0.0f},
                                                         {-1.0f, 1.0f, 0.0f, 1.0f},
                                                         {1.0f, -1.0f, 1.0f, 0.0f},
                                                         {1.0f, 1.0f, 1.0f, 1.0f}}};

// ============================================================================
// Construction / Destruction
// ============================================================================

ViewerApp::ViewerApp(IOpenGLContext* context)
    : context_(context), imgui_initialized_(false), delta_time_(0.0f), last_frame_(0.0f), cam_(nullptr), part_(nullptr),
      set_(nullptr), view_(), com_(), cur_frame_(0), pixels_(nullptr), com_executor_(nullptr), frame_executor_(nullptr),
      com_cache_(nullptr), frame_cache_(nullptr), com_file_provider_(nullptr), auto_com_compute_(false),
      com_file_present_(false)
{
    for (int i = 0; i < 1024; i++) {
        keys_[i] = false;
    }
    set_ = new SettingsIO();

    std::string folder = extractFolder(set_->posName);
    if (!folder.empty()) {
        loadViewerConfig(folder, auto_com_compute_);
        menu_state_.auto_com_compute = auto_com_compute_;
        rebuildCacheInfrastructure();
    }
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

    // Pre-load ui_scale before initImGui so the font can be sized correctly.
    // Only the scale value is captured here; the full window resize/fullscreen
    // restoration happens later in loadWindowSettings() after FBO setup.
    {
        std::string pre_path = getConfigPath();
        int tmp_w = 0, tmp_h = 0;
        bool tmp_fs = false;
        loadWindowConfig(pre_path, tmp_w, tmp_h, tmp_fs, &window_.ui_scale);
        window_.ui_scale = selectUiScale(context_->getContentScale(), window_.ui_scale);
    }

    initImGui();
    cam_->initGL();
    part_ = new Particle();
    setupGLStuff();
    setupScreenFBO();

    // Create in-app folder browsers (after ImGui is initialized).
    // Only assign file_dialog_ / recording_dialog_ if no mock was injected
    // before initialize() via setFileDialog() / setRecordingDialog().
    file_browser_ = std::make_unique<ImGuiFolderBrowser>();
    recording_browser_ = std::make_unique<ImGuiFolderBrowser>(ImGuiFolderBrowser::ValidationMode::kAnyDirectory);
    if (file_dialog_ == nullptr) {
        file_dialog_ = file_browser_.get();
    }
    if (recording_dialog_ == nullptr) {
        recording_dialog_ = recording_browser_.get();
    }

    // Load window settings AFTER FBO is set up (prevents crash during resize callback)
    loadWindowSettings();

    gamepad_.openFirstGamepad();

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
    paths_.font = paths_.exe + "/Viewer-Assets/fonts/Hack-Regular.ttf";
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

    // Register event handling (SDL3 events are polled in run())
    setupCallbacks();

    // NOTE: Window settings are loaded AFTER FBO setup in initialize()
    // to prevent crash when resize callback is triggered before FBO exists
}

void ViewerApp::setupCallbacks()
{
    // No callbacks to register: SDL3 events are processed in run() via SDL_PollEvent().
}

void ViewerApp::initImGui()
{
    SDL_Window* native_window = static_cast<SDL_Window*>(context_->getNativeWindowHandle());
    if (!native_window) {
        return;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr; // Disable imgui.ini file

    ImGui::StyleColorsDark();

    // Load Hack font sized for the current UI scale.
    // If the font file is missing, fall back to the built-in default.
    io.Fonts->Clear();
    ImFont* font = io.Fonts->AddFontFromFileTTF(paths_.font.c_str(), 16.0f * window_.ui_scale);
    if (font == nullptr) {
        SDL_Log("Failed to load font from: %s — falling back to default", paths_.font.c_str());
        io.Fonts->AddFontDefault();
    }

    SDL_GLContext gl_ctx = SDL_GL_GetCurrentContext();
    bool sdl3_init_ok = ImGui_ImplSDL3_InitForOpenGL(native_window, gl_ctx);
    bool gl3_init_ok = ImGui_ImplOpenGL3_Init("#version 410 core");
    if (!sdl3_init_ok || !gl3_init_ok) {
        if (gl3_init_ok) {
            ImGui_ImplOpenGL3_Shutdown();
        }
        if (sdl3_init_ok) {
            ImGui_ImplSDL3_Shutdown();
        }
        ImGui::DestroyContext();
        return;
    }
    imgui_initialized_ = true;
}

/*
 * applyUiScale — rebuild the font atlas and rescale the ImGui style.
 *
 * Must be called at the start of a frame (before ImGui::NewFrame).
 * Sequence:
 *   1. DestroyFontsTexture  — frees the existing GL texture (prevents leak)
 *   2. Fonts->Clear()       — drops font data
 *   3. AddFontFromFileTTF   — loads Hack at 16 * ui_scale
 *   4. Fonts->Build()       — rasterises the new atlas
 *   5. CreateFontsTexture   — uploads to GPU
 *   6. Reset ImGuiStyle and ScaleAllSizes — ScaleAllSizes is cumulative;
 *      resetting to default first prevents multiplied-scale artefacts
 */
void ViewerApp::applyUiScale()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplOpenGL3_DestroyFontsTexture();
    io.Fonts->Clear();

    ImFont* font = io.Fonts->AddFontFromFileTTF(paths_.font.c_str(), 16.0f * window_.ui_scale);
    if (font == nullptr) {
        SDL_Log("Failed to load font from: %s — falling back to default", paths_.font.c_str());
        io.Fonts->AddFontDefault();
    }

    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();

    ImGuiStyle& style = ImGui::GetStyle();
    style = ImGuiStyle{};
    ImGui::StyleColorsDark(&style);
    style.ScaleAllSizes(window_.ui_scale);

    scale_pending_ = false;
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
        // Process SDL3 events (replaces GLFW callbacks)
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (imgui_initialized_) {
                ImGui_ImplSDL3_ProcessEvent(&event);
            }
            gamepad_.handleEvent(event);
            if (event.type == SDL_EVENT_QUIT) {
                context_->setShouldClose(true);
            } else if (event.type == SDL_EVENT_KEY_DOWN || event.type == SDL_EVENT_KEY_UP) {
                handleKeyEvent(event.key.scancode, event.type == SDL_EVENT_KEY_DOWN,
                               static_cast<unsigned int>(event.key.mod));
            } else if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
                handleResize(event.window.data1, event.window.data2);
            }
        }

        gamepad_.poll();

        // Start ImGui frame (only if ImGui was initialized)
        if (imgui_initialized_) {
            // Apply any pending font-atlas rebuild before starting the new frame.
            if (scale_pending_) {
                applyUiScale();
            }
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
        }

        cam_->Move();
        processGamepadInput();

        beforeDraw();
        drawScene();
        if (render_mode_ == RenderMode::Spheres) {
            cam_->RenderSphere();
        }
        drawFBO();

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

        // Update menu state with current cache status — must happen before
        // renderMainMenu so the UI reflects values from the current frame.
        menu_state_.auto_com_compute = auto_com_compute_;
        menu_state_.ui_scale = window_.ui_scale;
        menu_state_.is_recording = recording_.is_active;
        menu_state_.current_render_mode = static_cast<int>(render_mode_);
        const std::size_t cached_frames = frame_cache_ ? frame_cache_->cachedCount() : 0;
        menu_state_.cache_status.frames_cached = static_cast<int>(cached_frames);
        menu_state_.cache_status.bytes_used = frame_cache_ ? cached_frames * frame_cache_->frameSizeBytes() : 0;

        if (imgui_initialized_) {
            if (menu_state_.debug_mode) {
                float fps = (delta_time_ > 0.0f) ? 1.0f / delta_time_ : 0.0f;
                renderCameraDebugOverlay(cam_, window_.width, window_.height, fps, PARTICLE_VIEWER_VERSION);
            }

            // Render ImGui menu and process actions
            MenuActions actions = renderMainMenu(menu_state_);
            {
                MenuActions panel_actions = renderControllerPanel(menu_state_);
                actions.load_file |= panel_actions.load_file;
                actions.select_recording_folder |= panel_actions.select_recording_folder;
                actions.quit |= panel_actions.quit;
                actions.toggle_fullscreen |= panel_actions.toggle_fullscreen;
                actions.toggle_auto_com |= panel_actions.toggle_auto_com;
                if (panel_actions.toggle_debug_mode) {
                    menu_state_.debug_mode = !menu_state_.debug_mode;
                }
                if (panel_actions.close_panel && current_mode_ == InputMode::MenuMode) {
                    toggleControllerPanel();
                }
                if (panel_actions.stop_recording) {
                    recording_.is_active = false;
                    recording_.folder = "";
                }
                if (panel_actions.render_mode_changed) {
                    switch (panel_actions.new_render_mode) {
                        case 0:
                            render_mode_ = RenderMode::Spheres;
                            break;
                        default:
                            break;
                    }
                }
                // Defensive sync: if panel was closed via a path that bypassed
                // toggleControllerPanel() (e.g. direct ImGui close), exit MenuMode.
                if (!menu_state_.controller_panel_open && current_mode_ == InputMode::MenuMode) {
                    current_mode_ = InputMode::ViewMode;
                    if (imgui_initialized_) {
                        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
                    }
                    nav_last_nav_time_ms_ = 0;
                    nav_had_initial_repeat_ = false;
                    menu_state_.confirm_panel_item = false;
                }
            }
            if (actions.load_file) {
                pauseIfPlaying();
                file_dialog_open_ = true;
            }
            if (actions.select_recording_folder) {
                openRecordingFolderDialog();
            }
            if (actions.change_resolution) {
                SDL_Window* native_window = static_cast<SDL_Window*>(context_->getNativeWindowHandle());
                if (native_window) {
                    SDL_SetWindowSize(native_window, actions.target_width, actions.target_height);
                    // Update windowed size tracking and save
                    window_.windowed_width = actions.target_width;
                    window_.windowed_height = actions.target_height;
                    saveWindowSettings();
                }
            }
            if (actions.toggle_fullscreen) {
                toggleFullscreen();
            }
            if (actions.toggle_auto_com) {
                auto_com_compute_ = !auto_com_compute_;
                menu_state_.auto_com_compute = auto_com_compute_;
                std::string folder = extractFolder(set_->posName);
                if (!folder.empty()) {
                    saveViewerConfig(folder, auto_com_compute_);
                }
                if (auto_com_compute_) {
                    createCOMInfrastructure();
                } else {
                    teardownCOMInfrastructure();
                }
            }
            if (actions.quit) {
                context_->setShouldClose(true);
            }
            if (actions.scale_changed) {
                window_.ui_scale = actions.new_scale;
                scale_pending_ = true;
                saveWindowSettings();
            }

            // File load dialog (per-frame; renders inside ImGui frame)
            if (file_dialog_open_ && file_dialog_ != nullptr) {
                const std::string result = file_dialog_->selectFolder("Select Simulation Folder");
                if (!result.empty()) {
                    handleLoadFromFolder(result);
                    file_dialog_open_ = false;
                } else if (!file_dialog_->isOpen()) {
                    file_dialog_open_ = false;
                }
            }

            // Recording folder dialog (per-frame; no particle validation needed)
            if (recording_dialog_open_ && recording_dialog_ != nullptr) {
                const std::string result = recording_dialog_->selectFolder("Select Recording Folder");
                if (!result.empty()) {
                    [[maybe_unused]] bool applied = applyRecordingFolderResult(result, recording_);
                    // result is non-empty here; the false (cancel) return cannot fire.
                    recording_dialog_open_ = false;
                } else if (!recording_dialog_->isOpen()) {
                    recording_dialog_open_ = false;
                }
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        context_->swapBuffers();

        if (set_->frames > 1) {
            bool loaded = false;
            if (frame_cache_) {
                auto frame_data = frame_cache_->getFrame(cur_frame_);
                if (frame_data) {
                    part_->stageTranslations(frame_data->data(), static_cast<long>(frame_data->size()));
                    loaded = true;
                }
                frame_cache_->prefetch(cur_frame_, PREFETCH_LOOKAHEAD_FRAMES, set_->frames);
            }
            if (!loaded) {
                set_->readPosVelFile(cur_frame_, part_, false);
            }
        }
        // COM prefetch — only when COM lock is active, auto-compute is enabled,
        // and no COMFile is present (COMFile takes precedence over auto-compute).
        if (cam_->isComLocked() && auto_com_compute_ && com_cache_ && !com_file_present_) {
            com_cache_->prefetchAsync(cur_frame_, PREFETCH_LOOKAHEAD_FRAMES, set_->frames);
        }
        if (menu_state_.debug_mode) {
            const std::size_t cached_frames_now = frame_cache_ ? frame_cache_->cachedCount() : 0;
            fprintf(stderr, "[Cache] frame=%ld frames_cached=%zu com_cached=%zu auto_com=%d locked=%d\n", cur_frame_,
                    cached_frames_now, com_cache_ ? com_cache_->cachedCount() : 0, static_cast<int>(auto_com_compute_),
                    static_cast<int>(cam_->isComLocked()));
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(QUAD_VERTICES), QUAD_VERTICES.data(), GL_STATIC_DRAW);
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
    // Try COMFile first (no regression when COMFile is present)
    glm::vec3 new_com{};
    bool com_set = com_file_provider_ && com_file_provider_->getCOM(cur_frame_, new_com);
    if (com_set) {
        com_ = new_com;
    }
    // Fallback: computed COM from cache if enabled and no COMFile hit
    bool com_from_cache = false;
    if (!com_set && auto_com_compute_ && cam_->isComLocked() && com_cache_) {
        auto maybe = com_cache_->getCOM(cur_frame_);
        if (maybe.has_value()) {
            com_ = *maybe;
            com_from_cache = true;
        }
        // On miss: keep previous com_ — camera does not snap
    }
    if (menu_state_.debug_mode) {
        fprintf(stderr, "[COM] frame=%ld file_hit=%d cache_hit=%d com=(%.3f,%.3f,%.3f) auto=%d locked=%d\n", cur_frame_,
                static_cast<int>(com_set), static_cast<int>(com_from_cache), static_cast<double>(com_.x),
                static_cast<double>(com_.y), static_cast<double>(com_.z), static_cast<int>(auto_com_compute_),
                static_cast<int>(cam_->isComLocked()));
    }

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

void ViewerApp::pauseIfPlaying()
{
    if (set_->isPlaying) {
        set_->togglePlay();
    }
}

void ViewerApp::processMinorKeys()
{
    if (keys_[SDL_SCANCODE_Q]) {
        seekFrame(3, false);
    }
    if (keys_[SDL_SCANCODE_E]) {
        seekFrame(3, true);
    }
}

void ViewerApp::handleLoadFromFolder(const std::string& folder)
{
    SettingsIO* new_set = set_->loadFromFolder(folder, part_, false);
    if (isNewFileSelected(new_set, set_)) {
        teardownCacheInfrastructure();
        delete set_;
        set_ = new_set;
        com_ = glm::vec3(0.0f);
        auto_com_compute_ = false;
        cur_frame_ = 0;
        std::string sim_folder = extractFolder(set_->posName);
        if (!sim_folder.empty()) {
            loadViewerConfig(sim_folder, auto_com_compute_);
            menu_state_.auto_com_compute = auto_com_compute_;
        }
        rebuildCacheInfrastructure();
    }

    // Persist the confirmed folder so the browser reopens there next time.
    // Guard against empty: an empty folder means no selection was made.
    if (!folder.empty()) {
        last_confirmed_folder_ = folder;
        if (file_browser_) {
            file_browser_->setLastConfirmedFolder(folder);
        }
        saveWindowSettings();
    }
}

void ViewerApp::openRecordingFolderDialog()
{
    if (recording_.is_active || recording_dialog_open_)
        return;
    // Guard: recording dialog conflicts with the render mode sub-panel; user must navigate back first.
    if (menu_state_.panel_layer == PanelLayer::RenderMode)
        return;
    recording_.error_count = 0;
    pauseIfPlaying();
    recording_dialog_open_ = true;
}

// ============================================================================
// Input Handling
// ============================================================================

void ViewerApp::handleKeyEvent(unsigned int scancode, bool is_pressed, unsigned int mods)
{
    // Alt+Enter toggles fullscreen (handle first, always works)
    if (scancode == SDL_SCANCODE_RETURN && is_pressed && (mods & SDL_KMOD_ALT)) {
        toggleFullscreen();
        return;
    }

    // Guard against out-of-bounds access
    if (scancode < 1024) {
        keys_[scancode] = is_pressed;
    }

    // In MenuMode: panel nav keys consume the event and return. Non-nav keys
    // (Space, F1, F3, etc.) fall through so global shortcuts remain active —
    // the panel is non-modal for keyboard. WantCaptureKeyboard is checked
    // AFTER this block so ImGui focus on the panel window cannot block nav.
    if (current_mode_ == InputMode::MenuMode) {
        if (is_pressed) {
            const int count = menu_state_.panel_item_count;
            if (scancode == SDL_SCANCODE_DOWN) {
                menu_state_.selected_panel_item = applyNavMove(menu_state_.selected_panel_item, count, 1);
                return;
            }
            if (scancode == SDL_SCANCODE_UP) {
                menu_state_.selected_panel_item = applyNavMove(menu_state_.selected_panel_item, count, -1);
                return;
            }
            if (scancode == SDL_SCANCODE_RETURN || scancode == SDL_SCANCODE_KP_ENTER) {
                menu_state_.confirm_panel_item = true;
                return;
            }
            if (scancode == SDL_SCANCODE_ESCAPE) {
                toggleControllerPanel();
                return;
            }
        }
        // Non-nav keys fall through to global shortcuts below
    }

    // If ImGui wants keyboard input (and we are not in MenuMode nav),
    // only process global toggle keys (F1/F3) and swallow everything else.
    if (imgui_initialized_ && ImGui::GetIO().WantCaptureKeyboard) {
        if (scancode == SDL_SCANCODE_F1 && is_pressed) {
            menu_state_.visible = !menu_state_.visible;
        }
        if (scancode == SDL_SCANCODE_F3 && is_pressed) {
            menu_state_.debug_mode = !menu_state_.debug_mode;
        }
        return;
    }

    // Camera movement — ViewMode only; panel is non-modal for global shortcuts
    // but camera input while the panel is open is disorienting.
    if (current_mode_ == InputMode::ViewMode && scancode < 1024) {
        cam_->KeyReader(static_cast<SDL_Scancode>(scancode), is_pressed);
    }

    if (scancode == SDL_SCANCODE_ESCAPE && is_pressed && current_mode_ == InputMode::ViewMode) {
        toggleControllerPanel();
    }
    if (scancode == SDL_SCANCODE_SPACE && is_pressed) {
        set_->togglePlay();
    }
    if (scancode == SDL_SCANCODE_T && is_pressed && current_mode_ == InputMode::ViewMode) {
        pauseIfPlaying();
        file_dialog_open_ = true;
    }
    if (scancode == SDL_SCANCODE_RIGHT && is_pressed && current_mode_ == InputMode::ViewMode) {
        seekFrame(1, true);
    }
    if (scancode == SDL_SCANCODE_LEFT && is_pressed && current_mode_ == InputMode::ViewMode) {
        seekFrame(1, false);
    }
    if (scancode == SDL_SCANCODE_F1 && is_pressed) {
        menu_state_.visible = !menu_state_.visible;
    }
    if (scancode == SDL_SCANCODE_F3 && is_pressed) {
        menu_state_.debug_mode = !menu_state_.debug_mode;
    }
    if (scancode == SDL_SCANCODE_R && is_pressed && current_mode_ == InputMode::ViewMode) {
        openRecordingFolderDialog();
        if (recording_.is_active) {
            recording_.folder = "";
            recording_.is_active = false;
        }
    }
    if (scancode == SDL_SCANCODE_M && is_pressed && current_mode_ == InputMode::ViewMode && !recording_.is_active) {
        render_mode_ = cycleRenderMode(render_mode_);
    }
}

// ============================================================================
// Gamepad Input
// ============================================================================

void ViewerApp::processGamepadInput()
{
    if (!gamepad_.isConnected()) {
        return;
    }
    // Block all gamepad input while a file dialog is open. The panel emits
    // close_panel before opening any dialog, so we are always in ViewMode here;
    // suspending Start/B during a dialog is intentional — the user must
    // dismiss the dialog before returning to controller navigation.
    if (file_dialog_open_ || recording_dialog_open_) {
        return;
    }

    if (current_mode_ == InputMode::ViewMode) {
        // Look sensitivity (degrees per frame at full deflection)
        constexpr float LOOK_SPEED = 3.0f;
        // Zoom increment per frame at full stick deflection
        constexpr float ZOOM_SPEED = 0.5f;
        // Trigger threshold before frame seeking activates (0..1)
        constexpr float TRIGGER_THRESHOLD = 0.3f;

        const float left_x = gamepad_.getLeftStickX();
        const float left_y = gamepad_.getLeftStickY();
        const float right_x = gamepad_.getRightStickX();
        const float right_y = gamepad_.getRightStickY();
        const float left_trigger = gamepad_.getLeftTrigger();
        const float right_trigger = gamepad_.getRightTrigger();

        // X (West) — speed boost while held (mirrors Shift key)
        cam_->setSpeedBoost(gamepad_.isButtonHeld(SDL_GAMEPAD_BUTTON_WEST));

        // ---- Movement / Orbit ----
        // When rotation is locked (orbit mode) the left stick orbits the sphere;
        // otherwise it provides free-camera movement.
        if (cam_->isRotLocked()) {
            // Orbit: replicate W/A/S/D rotLock behaviour with analog input
            cam_->applyGamepadOrbit(left_y, left_x);
            // Right stick Y zooms (adjusts sphere distance) when locked
            if (right_y != 0.0f) {
                cam_->adjustSphereDistance(right_y * ZOOM_SPEED);
            }
        } else {
            // Free camera movement
            cam_->applyGamepadMovement(left_y, left_x);
            // Right stick look
            cam_->applyGamepadLook(right_x * LOOK_SPEED, right_y * LOOK_SPEED);
        }

        // L3 / R3 adjust sphere distance when the sphere is visible
        if (cam_->isRenderingSphere()) {
            if (gamepad_.isButtonHeld(SDL_GAMEPAD_BUTTON_LEFT_STICK)) {
                cam_->adjustSphereDistance(-ZOOM_SPEED);
            }
            if (gamepad_.isButtonHeld(SDL_GAMEPAD_BUTTON_RIGHT_STICK)) {
                cam_->adjustSphereDistance(ZOOM_SPEED);
            }
        }

        // ---- Frame Playback ----
        // Triggers: fast-forward / rewind (continuous, mirrors Q/E keys)
        if (right_trigger > TRIGGER_THRESHOLD) {
            seekFrame(3, true);
        }
        if (left_trigger > TRIGGER_THRESHOLD) {
            seekFrame(3, false);
        }

        // Bumpers: single-frame advance / rewind (mirrors arrow keys)
        if (gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) {
            seekFrame(1, true);
        }
        if (gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)) {
            seekFrame(1, false);
        }

        // ---- Action Buttons ----
        // A (South) — toggle play/pause
        if (gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_SOUTH)) {
            set_->togglePlay();
        }

        // Back/Select — open file load dialog
        if (gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_BACK)) {
            pauseIfPlaying();
            file_dialog_open_ = true;
        }

        // B (East) — cycle point lock state (mirrors P key)
        if (gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_EAST)) {
            cam_->cycleRotateState();
        }

        // Y (North) -- toggle COM lock when rotation is locked; cycle render mode when free camera
        if (gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_NORTH)) {
            if (cam_->isRotLocked()) {
                cam_->toggleComLock();
            } else if (!recording_.is_active) {
                render_mode_ = cycleRenderMode(render_mode_);
            }
        }
    }

    // Start — toggle controller panel (any mode)
    if (gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_START)) {
        toggleControllerPanel();
    }
    // B (East) in MenuMode -- go back in sub-panel, or close panel from main layer
    if (current_mode_ == InputMode::MenuMode && gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_EAST)) {
        if (menu_state_.panel_layer == PanelLayer::RenderMode) {
            menu_state_.panel_back_pressed = true;
        } else {
            toggleControllerPanel();
        }
    }
    // MenuMode navigation — D-pad repeat + A-confirm
    if (current_mode_ == InputMode::MenuMode) {
        processMenuNavigation();
        if (gamepad_.isButtonJustPressed(SDL_GAMEPAD_BUTTON_SOUTH) && menu_state_.selected_panel_item >= 0) {
            menu_state_.confirm_panel_item = true;
        }
    }
}

void ViewerApp::toggleControllerPanel()
{
    if (current_mode_ == InputMode::ViewMode) {
        current_mode_ = InputMode::MenuMode;
        menu_state_.controller_panel_open = true;
        menu_state_.selected_panel_item = -1;
        menu_state_.confirm_panel_item = false;
        nav_last_nav_time_ms_ = 0;
        nav_had_initial_repeat_ = false;
        // Close any open dialogs/windows that must not coexist with the panel
        menu_state_.settings_open = false;
        file_dialog_open_ = false;
        recording_dialog_open_ = false;
        // Disable ImGui keyboard nav so arrow keys drive selected_panel_item,
        // not ImGui's own nav cursor
        if (imgui_initialized_) {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
        }
        if (set_->isPlaying) {
            set_->togglePlay();
        }
    } else {
        current_mode_ = InputMode::ViewMode;
        menu_state_.controller_panel_open = false;
        nav_last_nav_time_ms_ = 0;
        nav_had_initial_repeat_ = false;
        menu_state_.confirm_panel_item = false;
        if (imgui_initialized_) {
            ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        }
    }
}

void ViewerApp::processMenuNavigation()
{
    const Uint64 now = SDL_GetTicks();
    const int count = menu_state_.panel_item_count;
    if (count <= 0)
        return;

    const float stick_y = gamepad_.getLeftStickY();
    const bool down = gamepad_.isButtonHeld(SDL_GAMEPAD_BUTTON_DPAD_DOWN) || (stick_y > NAV_STICK_THRESHOLD);
    const bool up = gamepad_.isButtonHeld(SDL_GAMEPAD_BUTTON_DPAD_UP) || (stick_y < -NAV_STICK_THRESHOLD);

    if (down || up) {
        const bool first_press = (nav_last_nav_time_ms_ == 0);
        if (first_press) {
            nav_last_nav_time_ms_ = now;
            nav_had_initial_repeat_ = false;
            const int delta = down ? 1 : -1;
            menu_state_.selected_panel_item = applyNavMove(menu_state_.selected_panel_item, count, delta);
        } else {
            const Uint64 threshold = nav_had_initial_repeat_ ? NAV_REPEAT_DELAY_MS : NAV_INITIAL_DELAY_MS;
            if ((now - nav_last_nav_time_ms_) >= threshold) {
                nav_last_nav_time_ms_ = now;
                nav_had_initial_repeat_ = true;
                const int delta = down ? 1 : -1;
                menu_state_.selected_panel_item = applyNavMove(menu_state_.selected_panel_item, count, delta);
            }
        }
    } else {
        nav_last_nav_time_ms_ = 0;
        nav_had_initial_repeat_ = false;
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
    // Cache teardown: drain executors FIRST so no in-flight tasks reference
    // the caches or SettingsIO after they are deleted.
    teardownCacheInfrastructure();

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
        ImGui_ImplSDL3_Shutdown();
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

    // Note: We don't save settings on every resize event to avoid excessive I/O
    // during window dragging. Settings are saved when:
    // 1. User selects a resolution from the menu
    // 2. User toggles fullscreen (Alt+Enter)
    // 3. Application exits (future enhancement)
    // Users who manually resize can select "View → Resolution" to save their size.
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
    SDL_Window* native_window = static_cast<SDL_Window*>(context_->getNativeWindowHandle());
    if (!native_window) {
        return;
    }

    SDL_WindowFlags flags = SDL_GetWindowFlags(native_window);
    bool is_fullscreen = (flags & SDL_WINDOW_FULLSCREEN) != 0;

    if (is_fullscreen) {
        // Currently fullscreen, switch to windowed mode
        SDL_SetWindowFullscreen(native_window, false);
        SDL_SetWindowSize(native_window, window_.windowed_width, window_.windowed_height);
        window_.fullscreen = 0;
    } else {
        // Currently windowed, switch to fullscreen
        // Save current window size and position
        SDL_GetWindowSize(native_window, &window_.windowed_width, &window_.windowed_height);
        SDL_GetWindowPosition(native_window, &window_.windowed_x, &window_.windowed_y);

        SDL_SetWindowFullscreen(native_window, true);
        window_.fullscreen = 1;
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
    bool success = saveWindowConfig(config_path, window_.windowed_width, window_.windowed_height, fullscreen,
                                    window_.ui_scale, &last_confirmed_folder_);

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

    if (loadWindowConfig(config_path, width, height, fullscreen, &window_.ui_scale, &last_confirmed_folder_)) {
        // Merge the OS-detected content scale with the persisted preference.
        // selectUiScale returns the persisted value if it is a real preference
        // (>= 1.0), or falls back to the OS-detected scale (min 1.5).
        window_.ui_scale = selectUiScale(context_->getContentScale(), window_.ui_scale);

        // Restore last confirmed folder into the browser so it opens there.
        if (file_browser_ && !last_confirmed_folder_.empty()) {
            file_browser_->setLastConfirmedFolder(last_confirmed_folder_);
        }

        std::cout << "Loaded window config: " << width << "x" << height << " fullscreen=" << fullscreen << std::endl;

        // Apply loaded settings
        SDL_Window* native_window = static_cast<SDL_Window*>(context_->getNativeWindowHandle());
        if (native_window) {
            if (fullscreen) {
                // Store windowed size before going fullscreen
                window_.windowed_width = width;
                window_.windowed_height = height;

                SDL_SetWindowFullscreen(native_window, true);
                window_.fullscreen = 1;
            } else {
                // Set windowed size
                window_.windowed_width = width;
                window_.windowed_height = height;
                SDL_SetWindowSize(native_window, width, height);
                window_.fullscreen = 0;
            }
        }
    } else {
        std::cout << "No window config found, using defaults" << std::endl;
    }
}

// ============================================================================
// Helpers
// ============================================================================

/*
 * Returns the directory portion of a file path (everything before the last '/').
 * Returns an empty string when no '/' is found (no parent directory).
 */
std::string ViewerApp::extractFolder(const std::string& posName)
{
    auto slash = posName.rfind('/');
    return (slash != std::string::npos) ? posName.substr(0, slash) : "";
}

void ViewerApp::createCOMInfrastructure()
{
    assert(com_executor_ == nullptr && com_cache_ == nullptr &&
           "createCOMInfrastructure called with live COM objects; call teardownCOMInfrastructure first");
    MassParams mp = MassParams::fromSettingsIO(*set_);
    com_executor_ = new ThreadedExecutor();
    try {
        com_cache_ = new COMCache(*set_, mp, *com_executor_);
    } catch (...) {
        delete com_executor_;
        com_executor_ = nullptr;
        throw;
    }
}

void ViewerApp::teardownCOMInfrastructure()
{
    // Drain the executor first: its worker thread holds lambdas that reference
    // COMCache internals. Joining (via delete) before deleting the cache is safe.
    delete com_executor_;
    com_executor_ = nullptr;
    delete com_cache_;
    com_cache_ = nullptr;
}

void ViewerApp::rebuildCacheInfrastructure()
{
    frame_executor_ = new ThreadedExecutor();
    frame_cache_ = new FrameCache(*set_, FRAME_CACHE_CAPACITY_BYTES, *frame_executor_);
    com_file_provider_ = new COMFileProvider(*set_);
    com_file_present_ = set_->checkCOM();
    if (auto_com_compute_) {
        createCOMInfrastructure();
    }
}

void ViewerApp::teardownCacheInfrastructure()
{
    // Drain COM executor before deleting the COM cache.
    teardownCOMInfrastructure();
    // Drain frame executor before deleting the frame cache.
    delete frame_executor_;
    frame_executor_ = nullptr;
    delete frame_cache_;
    frame_cache_ = nullptr;
    delete com_file_provider_;
    com_file_provider_ = nullptr;
    com_file_present_ = false;
}
