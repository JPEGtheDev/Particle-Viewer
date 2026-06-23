#pragma once

#include <vector>

#include <glm/glm.hpp>

// 64-particle 4x4x4 grid fixture for Marching Cubes visual regression tests.
//
// Particles are on a regular 4x4x4 lattice at sim-space positions i,j,k
// with spacing 1.0. After SIM_TO_DISPLAY_SCALE (0.25), display positions span
// (-0.375, -0.375, -0.375) to (0.375, 0.375, 0.375) -- a 0.75-unit cube.
//
// All particles have category 0 (red) for deterministic color.

namespace grid4x4x4
{

inline constexpr int GRID_DIM = 4;
inline constexpr float SIM_TO_DISPLAY_SCALE = 0.25f;
inline constexpr float GRID_HALF_OFFSET = (GRID_DIM - 1) / 2.0f; // 1.5: centers grid around origin

inline std::vector<glm::vec4> makeParticles()
{
    // Particle grid: i,j,k in {-1.5, -0.5, 0.5, 1.5} (sim space)
    // Display space: multiply by SIM_TO_DISPLAY_SCALE = 0.25

    std::vector<glm::vec4> particles;
    particles.reserve(GRID_DIM * GRID_DIM * GRID_DIM);

    for (int i = 0; i < GRID_DIM; ++i) {
        for (int j = 0; j < GRID_DIM; ++j) {
            for (int k = 0; k < GRID_DIM; ++k) {
                float x = (static_cast<float>(i) - GRID_HALF_OFFSET) * SIM_TO_DISPLAY_SCALE;
                float y = (static_cast<float>(j) - GRID_HALF_OFFSET) * SIM_TO_DISPLAY_SCALE;
                float z = (static_cast<float>(k) - GRID_HALF_OFFSET) * SIM_TO_DISPLAY_SCALE;
                particles.emplace_back(x, y, z, 0.0f); // category 0 = red
            }
        }
    }

    return particles;
}

// Camera configuration for the MC 4x4x4 grid visual regression test.
//
// Calculation:
//   - Isosurface cloud diameter: ~1.75 display units
//     (0.375 particle extent + 0.5 influence radius = 0.875 per side)
//   - Desired coverage: ~45% of viewport width
//   - FOV: 45 degrees (tan(22.5 deg) = 0.4142)
//   - Ideal distance = 1.75 / (0.45 * 0.4142) = ~9.4 display units
//   - Camera positioned at ~9.4 units from origin along (0.5, 0.65, 0.57) normalized
struct CameraConfig
{
    glm::vec3 position = {4.7f, 6.1f, 5.4f}; // ~9.4 units from origin
    glm::vec3 target = {0.0f, 0.0f, 0.0f};   // looking at grid center
    glm::vec3 up = {0.0f, 1.0f, 0.0f};
    float fov_degrees = 45.0f;
    float near_clip = 0.1f;
    float far_clip = 100.0f;
};

inline constexpr CameraConfig CAMERA;

} // namespace grid4x4x4
