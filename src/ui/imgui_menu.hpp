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
    bool toggle_auto_com = false;   // toggled by the COM/Cache submenu checkbox
    bool scale_changed = false;     // user selected a new UI scale
    float new_scale = 1.0f;         // the newly selected scale value (only valid when scale_changed == true)
    bool toggle_debug_mode = false; // toggles debug_mode in MenuState
    bool close_panel = false;       // close the controller panel and exit MenuMode
    bool stop_recording = false;    // stop an active recording
};

/*
 * Tracks which layer of the controller panel is active.
 * Main: the standard panel item list.
 * RenderMode: the Render Mode sub-panel (mode selector + parameter sliders).
 */
enum class PanelLayer
{
    Main,
    RenderMode,
};

/*
 * Persistent state for the menu system.
 */
struct MenuState
{
    bool visible = false;
    bool debug_mode = false;
    bool auto_com_compute = false;             // reflects the current auto-COM toggle state
    CacheStatus cache_status;                  // populated by ViewerApp each frame
    float ui_scale = 0.0f;                     // current active UI scale (0.0 = not yet set)
    bool settings_open = false;                // Settings window visibility
    bool controller_panel_open = false;        // is the controller panel overlay visible?
    PanelLayer panel_layer = PanelLayer::Main; // which layer of the panel is active
    bool button_hints_visible = false;         // show button hint row in panel
    bool file_loading_enabled = true;          // enables file-load items in the controller panel
    int selected_panel_item = -1;              // currently highlighted item index; -1 = none
    int panel_item_count = 0;                  // total selectable items in panel this frame
    bool confirm_panel_item = false;           // A-button confirm pending; read+reset by panel
    bool panel_back_pressed = false;           // B-button back signal; used to navigate from sub-panel back to Main
    bool is_recording = false;                 // mirrors recording_.is_active; set by ViewerApp each frame
};

/*
 * Renders the ImGui main menu bar.
 * Returns actions triggered by user interaction.
 * Call after ImGui::NewFrame() each frame.
 */
MenuActions renderMainMenu(MenuState& state);

/*
 * Renders the controller panel overlay when state.controller_panel_open is true.
 * Resets state.button_hints_visible to false when the panel is closed.
 * When open, also updates state.panel_item_count and sets button_hints_visible = true.
 * Never mutates state.controller_panel_open directly — close actions are signalled
 * via MenuActions::close_panel and handled by the caller.
 * Returns actions triggered by panel item selection.
 * Call after renderMainMenu() each frame.
 */
MenuActions renderControllerPanel(MenuState& state);

#endif // PARTICLE_VIEWER_IMGUI_MENU_H
