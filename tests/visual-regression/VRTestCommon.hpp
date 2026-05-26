#pragma once

#include <cstdio>
#include <string>
#include <vector>

// Shared constants and helpers for visual-regression tests.
namespace VRTestConfig
{
static const uint32_t RENDER_WIDTH = 1280;
static const uint32_t RENDER_HEIGHT = 720;
static const float PARTICLE_TOLERANCE = 2.0f / 255.0f;
static const float MAX_DIFF_RATIO = 0.0001f;
static const std::string BASELINES_DIR = "baselines";
} // namespace VRTestConfig

inline std::string getShaderPath(const std::string& shaderName)
{
    std::vector<std::string> possiblePaths = {"Viewer-Assets/shaders/" + shaderName,
                                              "../Viewer-Assets/shaders/" + shaderName,
                                              "../../src/shaders/" + shaderName, "../../../src/shaders/" + shaderName};

    for (const auto& path : possiblePaths) {
        FILE* file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            return path;
        }
    }

    return possiblePaths[0];
}

inline std::string getBaselinePath(const std::string& baselineName)
{
    std::vector<std::string> possiblePaths = {VRTestConfig::BASELINES_DIR + "/" + baselineName,
                                              "../../tests/visual-regression/baselines/" + baselineName,
                                              "../tests/visual-regression/baselines/" + baselineName,
                                              "../../../tests/visual-regression/baselines/" + baselineName};

    for (const auto& path : possiblePaths) {
        FILE* file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);
            return path;
        }
    }

    return possiblePaths[0];
}
