#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

/// Acceleration structure that bins particles into a uniform 3-D grid using
/// counting sort (no std::sort, no per-call heap allocations beyond vectors).
///
/// After build(), particles in cell c are located at:
///   sorted_particles[ cell_starts[c] .. cell_starts[c+1] )
struct SpatialGrid
{
    /// Reorders particles into cell-major order.
    ///
    /// @param particles        Pointer to particle data (may be nullptr when count == 0).
    /// @param count            Number of particles.
    /// @param influence_radius Desired cell size; must be > 0.
    /// @param origin           World-space minimum corner of the grid volume.
    /// @param extent           World-space size of the grid volume (per axis).
    /// @param max_cells_per_axis  Maximum number of cells along any single axis.
    void build(const glm::vec4* particles, int count, float influence_radius, glm::vec3 origin, glm::vec3 extent,
               int max_cells_per_axis)
    {
        // Guard: invalid inputs -> clear and return.
        if (influence_radius <= 0.0f || count <= 0) {
            sorted_particles.clear();
            cell_starts.clear();
            num_cells_x = 0;
            num_cells_y = 0;
            num_cells_z = 0;
            cell_size = 0.0f;
            cell_origin = glm::vec3(0.0f);
            return;
        }

        // Step 1: Compute grid dimensions.
        cell_size = influence_radius;
        cell_origin = origin;

        num_cells_x = std::min(static_cast<int>(std::ceil(extent.x / cell_size)), max_cells_per_axis);
        num_cells_y = std::min(static_cast<int>(std::ceil(extent.y / cell_size)), max_cells_per_axis);
        num_cells_z = std::min(static_cast<int>(std::ceil(extent.z / cell_size)), max_cells_per_axis);

        // Ensure at least 1 cell per axis so clamping is well-defined.
        if (num_cells_x < 1)
            num_cells_x = 1;
        if (num_cells_y < 1)
            num_cells_y = 1;
        if (num_cells_z < 1)
            num_cells_z = 1;

        const int total_cells = num_cells_x * num_cells_y * num_cells_z;

        // Step 2: Compute the linear cell index for each particle (clamped).
        std::vector<int> particle_cell(static_cast<std::size_t>(count));
        for (int i = 0; i < count; ++i) {
            int cx = static_cast<int>(std::floor((particles[i].x - origin.x) / cell_size));
            int cy = static_cast<int>(std::floor((particles[i].y - origin.y) / cell_size));
            int cz = static_cast<int>(std::floor((particles[i].z - origin.z) / cell_size));

            cx = std::clamp(cx, 0, num_cells_x - 1);
            cy = std::clamp(cy, 0, num_cells_y - 1);
            cz = std::clamp(cz, 0, num_cells_z - 1);

            particle_cell[static_cast<std::size_t>(i)] = cx + cy * num_cells_x + cz * num_cells_x * num_cells_y;
        }

        // Step 3: Counting sort.

        // Count particles per cell.
        std::vector<uint32_t> counts(static_cast<std::size_t>(total_cells), 0u);
        for (int i = 0; i < count; ++i) {
            counts[static_cast<std::size_t>(particle_cell[static_cast<std::size_t>(i)])]++;
        }

        // Exclusive prefix sum -> cell_starts (total_cells + 1 entries, sentinel at end).
        cell_starts.resize(static_cast<std::size_t>(total_cells) + 1u, 0u);
        cell_starts[0] = 0u;
        for (int c = 0; c < total_cells; ++c) {
            cell_starts[static_cast<std::size_t>(c) + 1u] =
                cell_starts[static_cast<std::size_t>(c)] + counts[static_cast<std::size_t>(c)];
        }

        // Scatter particles into sorted order using write_pos as moving heads.
        sorted_particles.resize(static_cast<std::size_t>(count));
        std::vector<uint32_t> write_pos(cell_starts.begin(),
                                        cell_starts.begin() + total_cells); // copy prefix sums
        for (int i = 0; i < count; ++i) {
            const std::size_t cell_idx = static_cast<std::size_t>(particle_cell[static_cast<std::size_t>(i)]);
            const uint32_t dest = write_pos[cell_idx]++;
            sorted_particles[dest] = particles[i];
        }
    }

    std::vector<glm::vec4> sorted_particles; ///< Particles reordered by cell.
    std::vector<uint32_t> cell_starts;       ///< cell_starts[c]..cell_starts[c+1] = range in cell c.
    int num_cells_x = 0;
    int num_cells_y = 0;
    int num_cells_z = 0;
    float cell_size = 0.0f;
    glm::vec3 cell_origin{};
};
