/*
 * windowConfig.hpp
 *
 * Save and load window configuration (resolution, fullscreen state).
 * Uses a simple INI-style format.
 */

#ifndef PARTICLE_VIEWER_WINDOW_CONFIG_H
#define PARTICLE_VIEWER_WINDOW_CONFIG_H

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

#include "osFile.hpp"

#ifdef _WIN32
    #include <direct.h>
#else
    #include <sys/stat.h>
    #include <sys/types.h>
#endif

/*
 * Load window configuration from file.
 * Returns true if successful, false if file doesn't exist or is invalid.
 *
 * ui_scale (optional): if non-null and the key is present and parseable,
 * the pointed-to float is updated with the stored value. If the key is
 * absent or unparseable, the caller-supplied default is preserved unchanged.
 *
 * last_confirmed_folder (optional): if non-null and the key is present,
 * the pointed-to string is updated with the stored value. If the key is
 * absent, the caller-supplied default is preserved unchanged.
 *
 * mc_grid_resolution (optional): if non-null, updated from the key when
 * present; otherwise the default 128 is written to the pointed-to int.
 *
 * mc_iso_value (optional): if non-null, updated from the key when present;
 * otherwise the default 0.5f is written to the pointed-to float.
 *
 * mc_influence_radius (optional): if non-null, updated from the key when
 * present; otherwise the default 2.0f is written to the pointed-to float.
 *
 * mc_live_freeze (optional): if non-null, updated from the key when present;
 * otherwise the default 0 is written to the pointed-to int.
 *
 */
inline bool loadWindowConfig(const std::string& filepath, int& width, int& height, bool& fullscreen,
                             float* ui_scale = nullptr, std::string* last_confirmed_folder = nullptr,
                             int* mc_grid_resolution = nullptr, float* mc_iso_value = nullptr,
                             float* mc_influence_radius = nullptr, int* mc_live_freeze = nullptr)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false; // File doesn't exist, use defaults
    }

    // Apply MC defaults before parsing; if keys are absent the defaults remain.
    if (mc_grid_resolution != nullptr) {
        *mc_grid_resolution = 128;
    }
    if (mc_iso_value != nullptr) {
        *mc_iso_value = 0.1f;
    }
    if (mc_influence_radius != nullptr) {
        *mc_influence_radius = 2.0f;
    }
    if (mc_live_freeze != nullptr) {
        *mc_live_freeze = 0;
    }

    std::string line;
    bool found_width = false;
    bool found_height = false;
    bool found_fullscreen = false;

    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Parse key=value
        size_t eq_pos = line.find('=');
        if (eq_pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);

        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if (key == "width") {
            width = std::stoi(value);
            found_width = true;
        } else if (key == "height") {
            height = std::stoi(value);
            found_height = true;
        } else if (key == "fullscreen") {
            fullscreen = (value == "1" || value == "true" || value == "True");
            found_fullscreen = true;
        } else if (key == "ui_scale" && ui_scale != nullptr) {
            try {
                float parsed = std::stof(value);
                *ui_scale = parsed;
            } catch (...) {
                // Unparseable: leave caller's default unchanged
            }
        } else if (key == "last_confirmed_folder" && last_confirmed_folder != nullptr) {
            *last_confirmed_folder = value;
        } else if (key == "mc_grid_resolution" && mc_grid_resolution != nullptr) {
            try {
                *mc_grid_resolution = std::stoi(value);
            } catch (...) {
                // Unparseable: leave default unchanged
            }
        } else if (key == "mc_iso_value" && mc_iso_value != nullptr) {
            try {
                *mc_iso_value = std::stof(value);
            } catch (...) {
                // Unparseable: leave default unchanged
            }
        } else if (key == "mc_influence_radius" && mc_influence_radius != nullptr) {
            try {
                *mc_influence_radius = std::stof(value);
            } catch (...) {
                // Unparseable: leave default unchanged
            }
        } else if (key == "mc_live_freeze" && mc_live_freeze != nullptr) {
            try {
                *mc_live_freeze = std::stoi(value);
            } catch (...) {
                // Unparseable: leave default unchanged
            }
        }
    }

    file.close();
    return found_width && found_height && found_fullscreen;
}

/*
 * Save window configuration to file.
 * Returns true if successful, false on error.
 *
 * ui_scale: pass the current UI scale to persist it. The default value of
 * 0.0f is treated as a sentinel meaning "not set" — when 0.0f, the
 * ui_scale key is omitted from the file entirely, preserving any value
 * that a future loadWindowConfig call might have written there previously.
 *
 * last_confirmed_folder (optional): if non-null and non-empty, written to
 * the file under the key last_confirmed_folder. If null or empty, the key
 * is omitted entirely.
 *
 * mc_grid_resolution: marching cubes grid resolution (default 128).
 * mc_iso_value: marching cubes iso surface threshold (default 0.5f).
 * mc_influence_radius: marching cubes particle influence radius (default 2.0f).
 * mc_live_freeze: marching cubes live/freeze state (0=Live, 1=Freeze; default 0).
 *
 */
inline bool saveWindowConfig(const std::string& filepath, int width, int height, bool fullscreen, float ui_scale = 0.0f,
                             const std::string* last_confirmed_folder = nullptr, int mc_grid_resolution = 128,
                             float mc_iso_value = 0.5f, float mc_influence_radius = 2.0f, int mc_live_freeze = 0)
{
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to save window config to: " << filepath << std::endl;
        return false;
    }

    file << "# Particle-Viewer Window Configuration\n";
    file << "# Auto-generated - modify with care\n\n";
    file << "width=" << width << "\n";
    file << "height=" << height << "\n";
    file << "fullscreen=" << (fullscreen ? "1" : "0") << "\n";
    if (ui_scale != 0.0f) {
        file << "ui_scale=" << ui_scale << "\n";
    }
    if (last_confirmed_folder != nullptr && !last_confirmed_folder->empty()) {
        std::string sanitized = *last_confirmed_folder;
        sanitized.erase(
            std::remove_if(sanitized.begin(), sanitized.end(), [](char c) { return c == '\n' || c == '\r'; }),
            sanitized.end());
        if (!sanitized.empty()) {
            file << "last_confirmed_folder=" << sanitized << "\n";
        }
    }
    file << "mc_grid_resolution=" << mc_grid_resolution << "\n";
    file << "mc_iso_value=" << mc_iso_value << "\n";
    file << "mc_influence_radius=" << mc_influence_radius << "\n";
    file << "mc_live_freeze=" << mc_live_freeze << "\n";

    file.close();
    return true;
}

/*
 * Get the config file path.
 * Priority order (best practice for user config):
 * 1. XDG_CONFIG_HOME/particle-viewer/window.cfg (Flatpak, sandboxed apps)
 * 2. $HOME/.config/particle-viewer/window.cfg (Linux/Mac standard)
 * 3. %APPDATA%\particle-viewer\window.cfg (Windows standard)
 * 4. <exe_dir>/window.cfg (portable fallback for all platforms)
 *
 * Unlike shaders (read-only assets bundled with exe), config files are:
 * - User-writable preferences modified at runtime
 * - Should persist across application updates
 * - Per-user on multi-user systems
 * - May not have write access to exe directory (Program Files, /usr/bin, Flatpak)
 *
 * Exe-relative is used only as last resort for truly portable installs.
 */
inline std::string getConfigPath()
{
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        return std::string(appdata) + "\\particle-viewer\\window.cfg";
    }
    // Fallback to exe directory (portable install)
    return ExePath() + "\\window.cfg";
#else
    // Try XDG_CONFIG_HOME first (Flatpak and other sandboxed apps set this)
    const char* xdg_config = std::getenv("XDG_CONFIG_HOME");
    if (xdg_config) {
        return std::string(xdg_config) + "/particle-viewer/window.cfg";
    }

    // Fall back to $HOME/.config
    const char* home = std::getenv("HOME");
    if (home) {
        return std::string(home) + "/.config/particle-viewer/window.cfg";
    }
    // Fallback to exe directory (portable install)
    return ExePath() + "/window.cfg";
#endif
}

/*
 * Ensure the config directory exists.
 * Creates XDG_CONFIG_HOME/particle-viewer or ~/.config/particle-viewer on Linux/Mac,
 * or %APPDATA%\particle-viewer on Windows.
 */
inline void ensureConfigDir()
{
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        std::string dir = std::string(appdata) + "\\particle-viewer";
        _mkdir(dir.c_str());
    }
#else
    // Try XDG_CONFIG_HOME first (Flatpak and other sandboxed apps set this)
    const char* xdg_config = std::getenv("XDG_CONFIG_HOME");
    if (xdg_config) {
        std::string config_dir = std::string(xdg_config) + "/particle-viewer";
        mkdir(config_dir.c_str(), 0755);
    } else {
        // Fall back to $HOME/.config
        const char* home = std::getenv("HOME");
        if (home) {
            std::string config_base = std::string(home) + "/.config";
            std::string config_dir = config_base + "/particle-viewer";
            // Create both .config and .config/particle-viewer
            mkdir(config_base.c_str(), 0755);
            mkdir(config_dir.c_str(), 0755);
        }
    }
#endif
}

#endif // PARTICLE_VIEWER_WINDOW_CONFIG_H
