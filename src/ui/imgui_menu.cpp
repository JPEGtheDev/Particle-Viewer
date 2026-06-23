/*
 * imgui_menu.cpp
 *
 * Implementation of the ImGui main menu bar for Particle-Viewer.
 */

#include "imgui_menu.hpp"

#include <cassert>
#include <cmath>
#include <string>

#include <SDL3/SDL.h>

#include "imgui.h"

/*
 * Named indices for selectable items in the controller panel, in registration order.
 *
 * Non-conditional items (FULLSCREEN through CLOSE): enumerator integer value equals the
 * zero-based visual index of its item() call. The invariant holds unconditionally for
 * these entries.
 *
 * Conditional items (VRAM_NOTIFICATION, M_KEY_RECORDING_NOTIFICATION): the enumerator
 * value represents the slot index when ALL preceding conditional items are also present.
 * When a preceding conditional item is absent, a later conditional item occupies an
 * earlier slot. The switch dispatch in renderControllerPanel() accounts for this by
 * checking the active flags before acting -- see comments in those cases.
 *
 * Must stay in sync with the item() calls in renderControllerPanel().
 */
enum class PanelItem : int
{
    FULLSCREEN = 0,
    AUTO_COM,
    DEBUG_MODE,
    QUIT,
    LOAD_FILE,
    RECORDING_FOLDER,
    RENDER_MODE,
    CLOSE,
    VRAM_NOTIFICATION,           // conditional: only present when mc_vram_downgrade_notification is true
    M_KEY_RECORDING_NOTIFICATION // conditional: only present when m_key_recording_notification is true
};

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
            if (ImGui::MenuItem("Select Recording Folder...", "R")) {
                actions.select_recording_folder = true;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Quit", nullptr)) {
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
        if (ImGui::BeginMenu("COM / Cache")) {
            bool auto_com = state.auto_com_compute;
            if (ImGui::Checkbox("Auto COM compute", &auto_com)) {
                actions.toggle_auto_com = true;
            }
            ImGui::Separator();
            ImGui::Text("Frames cached: %d", state.cache_status.frames_cached);
            ImGui::Text("Memory: %.1f MB", static_cast<float>(state.cache_status.bytes_used) / (1024.0f * 1024.0f));
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("UI Scale...")) {
                state.settings_open = true;
            }
            ImGui::EndMenu();
        }
        if (state.is_recording) {
            // Right-align the REC indicator in the remaining menu bar space
            const float text_w = ImGui::CalcTextSize("  \xe2\x97\x8f REC  ").x;
            const float avail_x = ImGui::GetContentRegionAvail().x;
            const float offset = avail_x - text_w - ImGui::GetStyle().ItemSpacing.x;
            if (offset > 0.0f) {
                ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
            }
            ImGui::TextColored(ImVec4(1.0f, 0.25f, 0.25f, 1.0f), "  \xe2\x97\x8f REC  ");
        }
        ImGui::EndMainMenuBar();
    }

    if (state.settings_open) {
        ImGui::SetNextWindowSize(ImVec2(300, 120), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Settings", &state.settings_open)) {
            // UI Scale combo
            static const float kScaleSteps[] = {1.0f, 1.25f, 1.5f, 1.75f, 2.0f, 2.5f, 3.0f};
            static const char* kScaleLabels[] = {"1x", "1.25x", "1.5x", "1.75x", "2x", "2.5x", "3x"};
            static_assert(std::size(kScaleSteps) == std::size(kScaleLabels),
                          "scale step/label arrays must be the same length");
            constexpr int kNumSteps = static_cast<int>(std::size(kScaleSteps));

            // Find the closest step to the current ui_scale
            int current_idx = 0;
            float min_dist = std::abs(state.ui_scale - kScaleSteps[0]);
            for (int i = 1; i < kNumSteps; ++i) {
                float dist = std::abs(state.ui_scale - kScaleSteps[i]);
                if (dist < min_dist) {
                    min_dist = dist;
                    current_idx = i;
                }
            }

            ImGui::Text("UI Scale:");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(100.0f);
            if (ImGui::BeginCombo("##ui_scale", kScaleLabels[current_idx])) {
                for (int i = 0; i < kNumSteps; ++i) {
                    bool is_selected = (i == current_idx);
                    if (ImGui::Selectable(kScaleLabels[i], is_selected)) {
                        actions.scale_changed = true;
                        actions.new_scale = kScaleSteps[i];
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::End();
    }

    return actions;
}

MenuActions renderControllerPanel(MenuState& state)
{
    if (!state.controller_panel_open) {
        state.button_hints_visible = false;
        return MenuActions{};
    }

    state.button_hints_visible = true;

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::Begin("Controller Panel", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    int item_count = 0;
    MenuActions actions;

    auto item = [&](const char* label, bool enabled, auto action) {
        bool highlighted = (state.selected_panel_item == item_count);
        if (!enabled) {
            ImGui::BeginDisabled();
        }
        if (highlighted) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered]);
        }
        bool clicked = ImGui::Button(label);
        if (highlighted) {
            ImGui::PopStyleColor();
        }
        if (!enabled) {
            ImGui::EndDisabled();
        }
        if (clicked) {
            action();
        }
        ++item_count;
    };

    if (state.panel_layer == PanelLayer::RenderMode) {
        ImGui::Text("D-pad: Navigate | A: Select | B: Back");
        ImGui::Separator();

        // Handle B-button FIRST -- navigate back without closing the panel
        if (state.panel_back_pressed) {
            state.panel_back_pressed = false;
            state.panel_layer = PanelLayer::Main;
            ImGui::End();
            return actions;
        }

        // Named actions -- single source of truth for both click and A-button confirm paths
        auto selectSpheres = [&] {
            state.panel_layer = PanelLayer::Main;
            actions.render_mode_changed = true;
            actions.new_render_mode = 0;
        };
        auto selectMarchingCubes = [&] {
            state.panel_layer = PanelLayer::Main;
            actions.render_mode_changed = true;
            actions.new_render_mode = kRenderModeMC;
        };
        auto goBack = [&] { state.panel_layer = PanelLayer::Main; };
        auto setGrid64 = [&] {
            state.grid_resolution = GridResolution::Grid64;
            actions.mc_params_changed = true;
        };
        auto setGrid128 = [&] {
            state.grid_resolution = GridResolution::Grid128;
            actions.mc_params_changed = true;
        };
        auto setGrid256 = [&] {
            state.grid_resolution = GridResolution::Grid256;
            actions.mc_params_changed = true;
        };
        auto incrementIso = [&] {
            state.iso_value += 0.01f;
            if (state.iso_value > 2.0f) {
                state.iso_value = 0.0f;
            }
            actions.mc_params_changed = true;
        };
        auto incrementRadius = [&] {
            state.influence_radius += 0.1f;
            if (state.influence_radius > 10.0f) {
                state.influence_radius = 0.1f;
            }
            actions.mc_params_changed = true;
        };
        auto toggleLiveFreeze = [&] {
            state.live_freeze =
                (state.live_freeze == LiveFreezeMode::Live) ? LiveFreezeMode::Freeze : LiveFreezeMode::Live;
            actions.mc_params_changed = true;
        };
        auto refreshMesh = [&] { state.mc_refresh_requested = true; };

        item("Spheres", true, selectSpheres);
        if (state.current_render_mode == 0) {
            ImGui::SameLine();
            ImGui::Text("[active]");
        }

        item("Marching Cubes", state.compute_shaders_available, selectMarchingCubes);
        if (!state.compute_shaders_available) {
            ImGui::SetItemTooltip("Requires OpenGL 4.3 compute shaders");
        }

        item("Back", true, goBack);

        // When MC is the active render mode, show parameter controls below the mode items.
        // Each item() call adds a navigable row; item indices 3-8 cover the MC parameters;
        // index 9 (Refresh Mesh) shown in Freeze mode only.
        if (state.current_render_mode == kRenderModeMC) {
            ImGui::Separator();
            ImGui::Text("Grid Resolution:");
            item("Grid: 64", true, setGrid64);
            if (state.grid_resolution == GridResolution::Grid64) {
                ImGui::SameLine();
                ImGui::Text("[active]");
            }
            item("Grid: 128", true, setGrid128);
            if (state.grid_resolution == GridResolution::Grid128) {
                ImGui::SameLine();
                ImGui::Text("[active]");
            }
            item("Grid: 256", state.mc_256_available, setGrid256);
            if (!state.mc_256_available) {
                ImGui::SetItemTooltip("Requires more VRAM than available on this hardware");
            }
            if (state.grid_resolution == GridResolution::Grid256) {
                ImGui::SameLine();
                ImGui::Text("[active]");
            }

            // Iso-value: displayed as a button showing current value; D-pad confirm increments by step.
            // Left/right navigation not supported in this panel; use as a single selectable row.
            char iso_label[32];
            SDL_snprintf(iso_label, sizeof(iso_label), "Iso: %.2f", state.iso_value);
            item(iso_label, true, incrementIso);

            char ir_label[32];
            SDL_snprintf(ir_label, sizeof(ir_label), "Radius: %.1f", state.influence_radius);
            item(ir_label, true, incrementRadius);

            // Live/Freeze toggle (index 8): always shown when MC is active
            const char* lf_label = (state.live_freeze == LiveFreezeMode::Live) ? "Live" : "Freeze";
            item(lf_label, true, toggleLiveFreeze);

            // Refresh Mesh button (index 9): only shown in Freeze mode
            if (state.live_freeze == LiveFreezeMode::Freeze) {
                item("Refresh Mesh", true, refreshMesh);
            }
        }

        state.panel_item_count = item_count;

        if (state.confirm_panel_item) {
            state.confirm_panel_item = false;
            if (state.selected_panel_item >= 0 && state.selected_panel_item < state.panel_item_count) {
                switch (state.selected_panel_item) {
                    case 0:
                        selectSpheres();
                        break;
                    case 1:
                        if (state.compute_shaders_available) {
                            selectMarchingCubes();
                        }
                        break;
                    case 2:
                        goBack();
                        break;
                    case 3:
                        if (state.current_render_mode == kRenderModeMC) {
                            setGrid64();
                        }
                        break;
                    case 4:
                        if (state.current_render_mode == kRenderModeMC) {
                            setGrid128();
                        }
                        break;
                    case 5:
                        if (state.current_render_mode == kRenderModeMC && state.mc_256_available) {
                            setGrid256();
                        }
                        break;
                    case 6:
                        if (state.current_render_mode == kRenderModeMC) {
                            incrementIso();
                        }
                        break;
                    case 7:
                        if (state.current_render_mode == kRenderModeMC) {
                            incrementRadius();
                        }
                        break;
                    case 8:
                        if (state.current_render_mode == kRenderModeMC) {
                            toggleLiveFreeze();
                        }
                        break;
                    case 9:
                        if (state.current_render_mode == kRenderModeMC && state.live_freeze == LiveFreezeMode::Freeze) {
                            refreshMesh();
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    } else {
        // Main panel
        ImGui::Text("D-pad: Navigate | A: Select | B: Close");
        ImGui::Separator();

        item("Fullscreen", true, [&] { actions.toggle_fullscreen = true; });
        item("Auto-COM", true, [&] { actions.toggle_auto_com = true; });
        item("Debug Mode", true, [&] { actions.toggle_debug_mode = true; });
        item("Quit", true, [&] { actions.quit = true; });
        item("Load File", state.file_loading_enabled, [&] {
            actions.load_file = true;
            actions.close_panel = true;
        });
        const char* rec_label = state.is_recording ? "Stop Recording" : "Recording Folder";
        const bool rec_enabled = state.is_recording || state.file_loading_enabled;
        item(rec_label, rec_enabled, [&] {
            if (state.is_recording) {
                actions.stop_recording = true;
                actions.close_panel = true;
            } else {
                actions.select_recording_folder = true;
                actions.close_panel = true;
            }
        });

        item("Render Mode", !state.is_recording, [&] {
            state.panel_layer = PanelLayer::RenderMode;
            state.selected_panel_item = -1;
        });

        item("Close", true, [&] { actions.close_panel = true; });

        if (state.mc_vram_downgrade_notification) {
            item("VRAM: 256^3 unavailable, using 128^3", true, [&] { state.mc_vram_downgrade_notification = false; });
        }

        if (state.m_key_recording_notification) {
            item("M: mode locked (recording)", true, [&] { state.m_key_recording_notification = false; });
        }

        state.panel_item_count = item_count;
        if (state.confirm_panel_item) {
            state.confirm_panel_item = false;
            if (state.selected_panel_item >= 0 && state.selected_panel_item < state.panel_item_count) {
                switch (static_cast<PanelItem>(state.selected_panel_item)) {
                    case PanelItem::FULLSCREEN:
                        actions.toggle_fullscreen = true;
                        break;
                    case PanelItem::AUTO_COM:
                        actions.toggle_auto_com = true;
                        break;
                    case PanelItem::DEBUG_MODE:
                        actions.toggle_debug_mode = true;
                        break;
                    case PanelItem::QUIT:
                        actions.quit = true;
                        break;
                    case PanelItem::LOAD_FILE:
                        if (state.file_loading_enabled) {
                            actions.load_file = true;
                            actions.close_panel = true;
                        }
                        break;
                    case PanelItem::RECORDING_FOLDER:
                        if (state.is_recording) {
                            actions.stop_recording = true;
                            actions.close_panel = true;
                        } else if (state.file_loading_enabled) {
                            actions.select_recording_folder = true;
                            actions.close_panel = true;
                        }
                        break;
                    case PanelItem::RENDER_MODE:
                        if (!state.is_recording) {
                            state.panel_layer = PanelLayer::RenderMode;
                            state.selected_panel_item = -1;
                        }
                        break;
                    case PanelItem::CLOSE:
                        actions.close_panel = true;
                        break;
                    case PanelItem::VRAM_NOTIFICATION:
                        // This slot is VRAM when mc_vram_downgrade_notification is true,
                        // or M_KEY when only m_key_recording_notification is true.
                        if (state.mc_vram_downgrade_notification) {
                            state.mc_vram_downgrade_notification = false;
                        } else if (state.m_key_recording_notification) {
                            state.m_key_recording_notification = false;
                        }
                        break;
                    case PanelItem::M_KEY_RECORDING_NOTIFICATION:
                        // This slot is only reached when both VRAM and M_KEY notifications are active.
                        // The inner guard is defensive (matches the pattern in VRAM_NOTIFICATION above).
                        if (state.m_key_recording_notification) {
                            state.m_key_recording_notification = false;
                        }
                        break;
                    default:
                        assert(false && "selected_panel_item in-range but no PanelItem case -- enum out of sync");
                        break;
                }
            }
        }
    }

    ImGui::End();
    return actions;
}
