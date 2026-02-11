/*
 * imgui_menu.cpp
 *
 * Implementation of the ImGui main menu bar for Particle-Viewer.
 */

#include "imgui_menu.hpp"

#include "imgui.h"

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
            ImGui::MenuItem("Debug Mode", "F3", &state.debug_mode);
            ImGui::MenuItem("Show Menu", "F1", &state.visible);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    return actions;
}
