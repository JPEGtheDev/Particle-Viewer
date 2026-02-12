/*
 * windowConfig.hpp
 *
 * Save and load window configuration (resolution, fullscreen state).
 * Uses a simple INI-style format.
 */

#ifndef PARTICLE_VIEWER_WINDOW_CONFIG_H
#define PARTICLE_VIEWER_WINDOW_CONFIG_H

#include <fstream>
#include <iostream>
#include <string>

#ifdef _WIN32
    #include <direct.h>
#else
    #include <sys/stat.h>
    #include <sys/types.h>
#endif

/*
 * Load window configuration from file.
 * Returns true if successful, false if file doesn't exist or is invalid.
 */
inline bool loadWindowConfig(const std::string& filepath, int& width, int& height, bool& fullscreen)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false; // File doesn't exist, use defaults
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
        }
    }

    file.close();
    return found_width && found_height && found_fullscreen;
}

/*
 * Save window configuration to file.
 * Returns true if successful, false on error.
 */
inline bool saveWindowConfig(const std::string& filepath, int width, int height, bool fullscreen)
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

    file.close();
    return true;
}

/*
 * Get the config file path.
 * Uses XDG_CONFIG_HOME/particle-viewer/window.cfg on Linux/Mac (Flatpak-compatible),
 * falling back to $HOME/.config/particle-viewer/window.cfg,
 * or %APPDATA%\particle-viewer\window.cfg on Windows.
 */
inline std::string getConfigPath()
{
#ifdef _WIN32
    const char* appdata = std::getenv("APPDATA");
    if (appdata) {
        return std::string(appdata) + "\\particle-viewer\\window.cfg";
    }
    return "window.cfg"; // Fallback to current directory
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
    return "window.cfg"; // Fallback to current directory
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
