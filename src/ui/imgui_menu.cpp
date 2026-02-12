/*
 * imgui_menu.cpp
 *
 * Implementation of the ImGui main menu bar for Particle-Viewer.
 */

#include "imgui_menu.hpp"

#include <string>

#include <GLFW/glfw3.h>

#include "imgui.h"

/*
 * Helper to get the maximum window size that fits on the primary monitor.
 * Returns {width, height} clamped to monitor work area.
 */
static void getMonitorConstraints(int& max_width, int& max_height)
{
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (!primary) {
        // Fallback if no monitor info available
        max_width = 3840;
        max_height = 2160;
        return;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    if (mode) {
        // Use monitor resolution as max
        max_width = mode->width;
        max_height = mode->height;
    } else {
        max_width = 3840;
        max_height = 2160;
    }
}

/*
 * Helper to clamp requested resolution to fit monitor.
 * Returns true if resolution was clamped, false if it fit.
 */
static bool clampResolutionToMonitor(int& width, int& height)
{
    int max_width = 0;
    int max_height = 0;
    getMonitorConstraints(max_width, max_height);

    bool clamped = false;
    if (width > max_width || height > max_height) {
        // Maintain aspect ratio while clamping
        float aspect = static_cast<float>(width) / static_cast<float>(height);
        if (width > max_width) {
            width = max_width;
            height = static_cast<int>(width / aspect);
            clamped = true;
        }
        if (height > max_height) {
            height = max_height;
            width = static_cast<int>(height * aspect);
            clamped = true;
        }
    }
    return clamped;
}

MenuActions renderMainMenu(MenuState& state)
{
    MenuActions actions;

    if (!state.visible) {
        return actions;
    }

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Load File...", "T")) {
                actions.load_file = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Esc")) {
                actions.quit = true;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Resolution")) {
                // Common resolutions
                struct Resolution
                {
                    const char* name;
                    int width;
                    int height;
                };

                Resolution resolutions[] = {{"720p (1280x720)", 1280, 720},
                                            {"1080p (1920x1080)", 1920, 1080},
                                            {"1440p (2560x1440)", 2560, 1440},
                                            {"4K (3840x2160)", 3840, 2160}};

                for (const auto& res : resolutions) {
                    int width = res.width;
                    int height = res.height;
                    bool clamped = clampResolutionToMonitor(width, height);

                    // Show original name with (clamped) indicator if needed
                    if (clamped) {
                        std::string label = std::string(res.name) + " (clamped to " + std::to_string(width) + "x" +
                                            std::to_string(height) + ")";
                        if (ImGui::MenuItem(label.c_str())) {
                            actions.change_resolution = true;
                            actions.target_width = width;
                            actions.target_height = height;
                        }
                    } else {
                        if (ImGui::MenuItem(res.name)) {
                            actions.change_resolution = true;
                            actions.target_width = width;
                            actions.target_height = height;
                        }
                    }
                }

                ImGui::EndMenu();
            }
            ImGui::Separator();
            ImGui::MenuItem("Debug Mode", "F3", &state.debug_mode);
            ImGui::MenuItem("Show Menu", "F1", &state.visible);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    return actions;
}
