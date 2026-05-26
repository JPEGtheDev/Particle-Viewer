/*
 * Color blending math for Screen-Space Metaballs.
 * Pure functions — no GL context required, fully unit testable.
 *
 * Replicates the GLSL category 500 color formula from sphereVertex.vs:
 *   fColor = vec3(gl_InstanceID%40/40.0f,
 *                 gl_InstanceID%1600/1600.0f,
 *                 gl_InstanceID%64000/64000.0f);
 */
#ifndef PARTICLE_VIEWER_SSM_COLOR_MATH_HPP
#define PARTICLE_VIEWER_SSM_COLOR_MATH_HPP

#include <utility>
#include <vector>

#include "glm/glm.hpp"

/// Returns the RGB color for a category 500 particle, replicating the GLSL
/// shader formula. Integer modulo is applied before float division, matching
/// GLSL operator precedence exactly.
///
/// @param particleId  The particle instance ID (gl_InstanceID equivalent).
/// @return            RGB color in [0, 1].
inline glm::vec3 category500Color(int particleId)
{
    return glm::vec3(static_cast<float>(particleId % 40) / 40.0f, static_cast<float>(particleId % 1600) / 1600.0f,
                     static_cast<float>(particleId % 64000) / 64000.0f);
}

/// Computes a weighted-average blend of the given colors.
///
/// @param weighted_colors  Pairs of (color, weight). Weights need not sum to 1.
/// @return                 Weighted-average RGB color. Returns black (0,0,0) if
///                         all weights are zero or the input is empty.
inline glm::vec3 blendColors(const std::vector<std::pair<glm::vec3, float>>& weighted_colors)
{
    float totalWeight = 0.0f;
    glm::vec3 blended(0.0f);

    for (const auto& [color, weight] : weighted_colors) {
        blended += color * weight;
        totalWeight += weight;
    }

    if (totalWeight == 0.0f) {
        return glm::vec3(0.0f);
    }

    return blended / totalWeight;
}

#endif // PARTICLE_VIEWER_SSM_COLOR_MATH_HPP
