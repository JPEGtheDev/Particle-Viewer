#pragma once
#include <cmath>
#include <vector>

#include <glm/glm.hpp>

namespace mc_color
{

/// Returns the category color for a given category code and buffer index.
///
/// Category 500 uses a rainbow-by-buffer-index formula so that neighboring
/// particles in the buffer get slightly different hues, making distinct
/// structures visible even when they share the same category.
inline glm::vec3 categoryColor(int category, std::size_t buffer_index)
{
    switch (category) {
        case 0:
            return glm::vec3(1.0f, 0.0f, 0.0f); // red
        case 1:
            return glm::vec3(0.2f, 0.6f, 1.0f); // blue
        case 2:
            return glm::vec3(1.0f, 0.0f, 1.0f); // magenta
        case 3:
            return glm::vec3(0.89f, 0.59f, 0.0f); // orange
        case 500:
            return glm::vec3(static_cast<float>(buffer_index % 40) / 40.0f,
                             static_cast<float>(buffer_index % 1600) / 1600.0f,
                             static_cast<float>(buffer_index % 64000) / 64000.0f);
        default:
            return glm::vec3(0.5f, 0.5f, 0.5f); // grey
    }
}

/// Blends the colors of nearby particles using Gaussian-weighted averaging.
///
/// Each particle contributes its category color weighted by a Gaussian falloff
/// based on the distance from voxel_center.  Particles beyond influence_radius
/// are excluded entirely (weight clamped to zero) to avoid long-range color
/// contamination.
///
/// @param particles       Particle buffer; xyz = position, w = category code.
/// @param voxel_center    The point at which to evaluate the blended color.
/// @param influence_radius Hard cutoff and Gaussian sigma parameter (in world units).
/// @return Weighted-average color, or vec3(0) when no particle contributes.
inline glm::vec3 blendColors(const std::vector<glm::vec4>& particles, glm::vec3 voxel_center, float influence_radius)
{
    if (influence_radius <= 0.0f) {
        return glm::vec3(0.0f);
    }

    glm::vec3 color_sum(0.0f);
    float weight_sum = 0.0f;

    for (std::size_t i = 0; i < particles.size(); ++i) {
        glm::vec3 pos(particles[i].x, particles[i].y, particles[i].z);
        float dist = glm::length(pos - voxel_center);

        if (dist > influence_radius) {
            continue; // hard cutoff
        }

        float ratio = dist / influence_radius;
        float weight = std::exp(-0.5f * ratio * ratio);

        int category = static_cast<int>(particles[i].w);
        glm::vec3 cat_color = categoryColor(category, i);

        color_sum += weight * cat_color;
        weight_sum += weight;
    }

    if (weight_sum == 0.0f) {
        return glm::vec3(0.0f, 0.0f, 0.0f);
    }

    return color_sum / weight_sum;
}

} // namespace mc_color
