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
    bool toggle_auto_com = false;     // toggled by the COM/Cache submenu checkbox
    bool scale_changed = false;       // user selected a new UI scale
    float new_scale = 1.0f;           // the newly selected scale value (only valid when scale_changed == true)
    bool toggle_debug_mode = false;   // toggles debug_mode in MenuState
    bool close_panel = false;         // close the controller panel and exit MenuMode
    bool stop_recording = false;      // stop an active recording
    bool render_mode_changed = false; // user selected a render mode from sub-panel
    int new_render_mode = 0;          // 0=Spheres, 1=SSM, 2=MarchingCubes (valid only when render_mode_changed)
    bool mc_params_changed = false;   // MC parameter (iso_value, influence_radius, grid_resolution) changed
};

// Named integer value for the MarchingCubes render mode, matching RenderMode::MarchingCubes
// in viewer_app.hpp. Used in MenuState::current_render_mode and MenuActions::new_render_mode.
static constexpr int kRenderModeMC = 2;

/*
 * Tracks which layer of the controller panel is active.
 * Main: the standard panel item list.
 * RenderMode: the Render Mode sub-panel (mode selector).
 */
enum class PanelLayer
{
    Main,
    RenderMode,
};

/*
 * Grid resolution for the marching cubes density field.
 * Higher values produce finer meshes at the cost of more VRAM and compute.
 */
enum class GridResolution : int
{
    Grid64 = 64,
    Grid128 = 128,
    Grid256 = 256,
};

/*
 * Named item counts for controller panel sub-panels.
 * Each enumerator's integer value equals the number of selectable items
 * rendered in that sub-panel configuration.
 */
enum class SubPanelItemCount : int
{
    kBaseRenderMode = 3,    // Spheres + MC + Back
    kMarchingCubesMode = 9, // Spheres + MC + Back + Grid64 + Grid128 + Grid256 + Iso + Radius + LiveFreeze
};

/*
 * Controls whether the marching cubes mesh updates every frame (Live)
 * or is frozen at its last computed state (Freeze).
 */
enum class LiveFreezeMode
{
    Live,
    Freeze,
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
    bool panel_back_pressed =
        false; // B-button back signal; read+reset by renderControllerPanel to navigate sub-panel back to Main
    bool is_recording = false; // mirrors recording_.is_active; set by ViewerApp each frame

    // Sub-panel state -- set by ViewerApp each frame before renderControllerPanel()
    int current_render_mode = 0;            // 0=Spheres, 1=SSM, 2=MarchingCubes
    bool compute_shaders_available = false; // true when OpenGL 4.3 compute shaders are available

    // Marching cubes parameters
    GridResolution grid_resolution = GridResolution::Grid128;
    float iso_value = 0.5f;
    float influence_radius = 2.0f;
    LiveFreezeMode live_freeze = LiveFreezeMode::Live;
    bool mc_refresh_requested = false;
    bool mc_vram_downgrade_notification = false;
    bool m_key_recording_notification = false;
    // True when the hardware can support 256^3 grid resolution.
    // Set once at startup by ViewerApp after resolveGridResolution().
    // When false, the 256^3 radio button is disabled in the MC sub-panel.
    bool mc_256_available = true;
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

inline const char* getMarchingCubesLabel(bool compute_available)
{
    return compute_available ? "Marching Cubes" : "Marching Cubes (unavailable)";
}

#endif // PARTICLE_VIEWER_IMGUI_MENU_H
