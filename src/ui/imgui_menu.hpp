/*
 * imgui_menu.hpp
 *
 * ImGui-based menu system for Particle-Viewer.
 * Provides a main menu bar with File and View menus.
 *
 * The menu communicates user actions back to the caller via MenuActions.
 * Menu visibility and debug mode state are tracked in MenuState.
 */

#ifndef PARTICLE_VIEWER_IMGUI_MENU_H
#define PARTICLE_VIEWER_IMGUI_MENU_H

/*
 * Actions triggered by menu interactions, communicated back to ViewerApp.
 */
struct MenuActions
{
    bool load_file = false;
    bool quit = false;
    bool change_resolution = false;
    int target_width = 0;
    int target_height = 0;
};

/*
 * Persistent state for the menu system.
 */
struct MenuState
{
    bool visible = true;
    bool debug_mode = false;
};

/*
 * Renders the ImGui main menu bar.
 * Returns actions triggered by user interaction.
 * Call after ImGui::NewFrame() each frame.
 */
MenuActions renderMainMenu(MenuState& state);

#endif // PARTICLE_VIEWER_IMGUI_MENU_H
