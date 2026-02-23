/*
 * imgui_menu.cpp
 *
 * Implementation of the ImGui main menu bar for Particle-Viewer.
 */

#include "imgui_menu.hpp"

#include <string>

#include <SDL3/SDL.h>

#include "imgui.h"

/*
 * Helper to get the maximum window size that fits on the primary display.
 * Returns {width, height} clamped to display work area.
 */
static void getMonitorConstraints(int& max_width, int& max_height)
{
    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    if (display == 0) {
        // Fallback to 720p if no display info available
        max_width = 1280;
        max_height = 720;
        return;
    }

    SDL_Rect bounds;
    if (SDL_GetDisplayBounds(display, &bounds)) {
        max_width = bounds.w;
        max_height = bounds.h;
    } else {
        max_width = 1280;
        max_height = 720;
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

/*
 * Common aspect ratios for display resolutions.
 */
enum class AspectRatio
{
    AR_16_9,  // Widescreen (1.778:1)
    AR_16_10, // Wide (1.6:1)
    AR_21_9,  // Ultrawide (2.333:1)
    AR_4_3    // Standard (1.333:1)
};

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
                // Common resolutions with aspect ratios
                struct Resolution
                {
                    const char* name;
                    int width;
                    int height;
                    AspectRatio aspect_ratio;
                };

                Resolution resolutions[] = {// 16:9 Widescreen (most common)
                                            {"720p (1280x720)", 1280, 720, AspectRatio::AR_16_9},
                                            {"1080p (1920x1080)", 1920, 1080, AspectRatio::AR_16_9},
                                            {"1440p (2560x1440)", 2560, 1440, AspectRatio::AR_16_9},
                                            {"4K (3840x2160)", 3840, 2160, AspectRatio::AR_16_9},
                                            // 4:3 Standard
                                            {"XGA (1024x768)", 1024, 768, AspectRatio::AR_4_3},
                                            {"SXGA (1280x1024)", 1280, 1024, AspectRatio::AR_4_3},
                                            // 21:9 Ultrawide
                                            {"Ultrawide 1080p (2560x1080)", 2560, 1080, AspectRatio::AR_21_9},
                                            {"Ultrawide 1440p (3440x1440)", 3440, 1440, AspectRatio::AR_21_9}};

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
            if (ImGui::MenuItem("Toggle Fullscreen", "Alt+Enter")) {
                actions.toggle_fullscreen = true;
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
