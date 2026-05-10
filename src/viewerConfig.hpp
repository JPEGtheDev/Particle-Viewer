/*
 * viewerConfig.hpp
 *
 * Save and load viewer configuration (per-simulation settings).
 * Config file is stored in the simulation folder as viewer.cfg.
 * Uses a simple INI-style key=value format.
 */

#ifndef PARTICLE_VIEWER_VIEWER_CONFIG_H
#define PARTICLE_VIEWER_VIEWER_CONFIG_H

#include <fstream>
#include <string>

/*
 * Load viewer.cfg from the given folder path.
 * Returns true if file exists and auto_com_compute was found.
 * Returns false (uses defaults) if file doesn't exist or is invalid.
 * Unknown keys are silently ignored.
 */
inline bool loadViewerConfig(const std::string& folder, bool& auto_com_compute)
{
    std::string filepath = folder + "/viewer.cfg";
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false; // File doesn't exist, use defaults
    }

    std::string line;
    bool found_auto_com_compute = false;

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

        if (key == "auto_com_compute") {
            auto_com_compute = (value == "1" || value == "true" || value == "True");
            found_auto_com_compute = true;
        }
        // Unknown keys are silently ignored
    }

    file.close();
    return found_auto_com_compute;
}

/*
 * Save viewer.cfg to the given folder path.
 * Returns true on success.
 * Returns false silently if folder is not writable (no stderr output).
 */
inline bool saveViewerConfig(const std::string& folder, bool auto_com_compute)
{
    std::string filepath = folder + "/viewer.cfg";
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false; // Silently return false — simulation folders may be read-only
    }

    file << "# Particle-Viewer Simulation Configuration\n";
    file << "# Auto-generated - modify with care\n\n";
    file << "auto_com_compute=" << (auto_com_compute ? "1" : "0") << "\n";

    file.close();
    return true;
}

#endif // PARTICLE_VIEWER_VIEWER_CONFIG_H
