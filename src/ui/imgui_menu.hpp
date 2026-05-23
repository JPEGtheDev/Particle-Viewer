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

#include <cstddef>

/*
 * Live status of the frame and COM caches, populated by ViewerApp each frame.
 */
struct CacheStatus
{
    int frames_cached = 0;
    std::size_t bytes_used = 0;
};

/*
 * Actions triggered by menu interactions, communicated back to ViewerApp.
 */
struct MenuActions
{
    bool load_file = false;
    bool select_recording_folder = false;
    bool quit = false;
    bool change_resolution = false;
    bool toggle_fullscreen = false;
    int target_width = 0;
    int target_height = 0;
    bool toggle_auto_com = false; // toggled by the COM/Cache submenu checkbox
    bool scale_changed = false;   // user selected a new UI scale
    float new_scale = 1.0f;       // the newly selected scale value (only valid when scale_changed == true)
};

/*
 * Persistent state for the menu system.
 */
struct MenuState
{
    bool visible = true;
    bool debug_mode = false;
    bool auto_com_compute = false; // reflects the current auto-COM toggle state
    CacheStatus cache_status;      // populated by ViewerApp each frame
    float ui_scale = 0.0f;         // current active UI scale (0.0 = not yet set)
    bool settings_open = false;    // Settings window visibility
};

/*
 * Renders the ImGui main menu bar.
 * Returns actions triggered by user interaction.
 * Call after ImGui::NewFrame() each frame.
 */
MenuActions renderMainMenu(MenuState& state);

#endif // PARTICLE_VIEWER_IMGUI_MENU_H
