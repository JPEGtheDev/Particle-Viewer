#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "SpatialGrid.hpp"

// ---------------------------------------------------------------------------
// TDD RED phase -- SpatialGrid.hpp does not exist yet.
// This file is intentionally uncompilable until Todo 22 is implemented.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Test 1: Single particle maps to cell (0,0,0)
// ---------------------------------------------------------------------------
TEST(SpatialGridTest, SingleParticle_MapsToCell000)
{
    glm::vec4 p{0.5f, 0.5f, 0.5f, 1.0f};

    SpatialGrid grid;
    grid.build(&p, 1, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f), /*max_cells_per_axis=*/64);

    // Cell (0,0,0) has linear id 0; sentinel is at cell_starts[1]
    EXPECT_EQ(grid.cell_starts[1] - grid.cell_starts[0], 1u);
    EXPECT_FLOAT_EQ(grid.sorted_particles[0].x, 0.5f);
}

// ---------------------------------------------------------------------------
// Test 2: Particle at cell boundary maps to the expected cell
// ---------------------------------------------------------------------------
TEST(SpatialGridTest, Particle_AtCellBoundary_MapsToExpectedCell)
{
    // x=1.0 with origin=0, cell_size=1.0 -> floor(1.0/1.0)=1 -> cell_x=1
    // linear_cell_id = 1 + 0*10 + 0*100 = 1
    glm::vec4 p{1.0f, 0.5f, 0.5f, 1.0f};

    SpatialGrid grid;
    grid.build(&p, 1, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f), /*max_cells_per_axis=*/64);

    // Cell 0 must be empty
    EXPECT_EQ(grid.cell_starts[1] - grid.cell_starts[0], 0u);
    // Cell 1 must contain the particle
    EXPECT_EQ(grid.cell_starts[2] - grid.cell_starts[1], 1u);
}

// ---------------------------------------------------------------------------
// Test 3: cell_starts contains the correct particle ranges
// ---------------------------------------------------------------------------
TEST(SpatialGridTest, CellStarts_ContainsCorrectParticleRange)
{
    // p0, p1 -> cell (0,0,0) -> linear_id=0
    // p2     -> cell (1,0,0) -> linear_id=1
    glm::vec4 particles[3] = {
        {0.5f, 0.5f, 0.5f, 1.0f},
        {0.5f, 0.5f, 0.5f, 1.0f},
        {1.5f, 0.5f, 0.5f, 1.0f},
    };

    SpatialGrid grid;
    grid.build(particles, 3, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f), /*max_cells_per_axis=*/64);

    EXPECT_EQ(grid.cell_starts[1] - grid.cell_starts[0], 2u);
    EXPECT_EQ(grid.cell_starts[2] - grid.cell_starts[1], 1u);
    EXPECT_EQ(grid.sorted_particles.size(), 3u);
}

// ---------------------------------------------------------------------------
// Test 4: Empty particle list -- no crash, empty output
// ---------------------------------------------------------------------------
TEST(SpatialGridTest, EmptyParticleList_NoCrash)
{
    SpatialGrid grid;
    grid.build(nullptr, 0, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f), /*max_cells_per_axis=*/64);

    EXPECT_TRUE(grid.sorted_particles.empty());

    // Both "empty cell_starts" and "all-zero cell_starts" are valid contracts for
    // build(nullptr, 0, ...). The implementation may allocate the sentinel array
    // (num_cells+1 entries all zero) or return an empty vector -- both are correct.
    bool all_zero = true;
    for (auto v : grid.cell_starts) {
        if (v != 0u) {
            all_zero = false;
            break;
        }
    }
    EXPECT_TRUE(grid.cell_starts.empty() || all_zero);
}

// ---------------------------------------------------------------------------
// Test 5: All particles in the same cell -- correct count
// ---------------------------------------------------------------------------
TEST(SpatialGridTest, AllParticlesInSameCell_CorrectCount)
{
    // ir=5.0 -> cell_size=5.0 -> all particles at (0.5,0.5,0.5) map to cell (0,0,0)
    glm::vec4 particles[5] = {
        {0.5f, 0.5f, 0.5f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f},
        {0.5f, 0.5f, 0.5f, 1.0f}, {0.5f, 0.5f, 0.5f, 1.0f},
    };

    SpatialGrid grid;
    grid.build(particles, 5, /*influence_radius=*/5.0f, glm::vec3(0.0f), glm::vec3(10.0f), /*max_cells_per_axis=*/64);

    constexpr uint32_t cell0 = 0u;
    EXPECT_EQ(grid.cell_starts[cell0 + 1] - grid.cell_starts[cell0], 5u);
    EXPECT_EQ(grid.sorted_particles.size(), 5u);
}

// ---------------------------------------------------------------------------
// Test 6: Rebuild after influence radius change updates cell assignments
// ---------------------------------------------------------------------------
TEST(SpatialGridTest, RebuildAfterIrChange_UpdatesCellAssignments)
{
    glm::vec4 p{1.5f, 0.5f, 0.5f, 1.0f};

    SpatialGrid grid;

    // First build: ir=1.0 -> cell_size=1.0 -> cell_x = floor(1.5/1.0) = 1 -> linear_id=1
    grid.build(&p, 1, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f), /*max_cells_per_axis=*/64);

    EXPECT_EQ(grid.cell_starts[2] - grid.cell_starts[1], 1u) << "First build: particle should be in cell 1";

    // Second build: ir=2.0 -> cell_size=2.0 -> cell_x = floor(1.5/2.0) = 0 -> linear_id=0
    grid.build(&p, 1, /*influence_radius=*/2.0f, glm::vec3(0.0f), glm::vec3(10.0f), /*max_cells_per_axis=*/64);

    EXPECT_EQ(grid.cell_starts[1] - grid.cell_starts[0], 1u) << "Second build: particle should now be in cell 0";
}

// ---------------------------------------------------------------------------
// Test 7: num_cells per axis is capped at max_cells_per_axis
// ---------------------------------------------------------------------------
TEST(SpatialGridTest, NumCells_CappedAtMaxCellsPerAxis)
{
    // extent=100, ir=0.001 -> uncapped would need 100000 cells per axis -> must clamp to 64
    glm::vec4 particles[4] = {
        {10.0f, 10.0f, 10.0f, 1.0f},
        {20.0f, 20.0f, 20.0f, 1.0f},
        {50.0f, 50.0f, 50.0f, 1.0f},
        {90.0f, 90.0f, 90.0f, 1.0f},
    };

    SpatialGrid grid;
    grid.build(particles, 4, /*influence_radius=*/0.001f, glm::vec3(0.0f), glm::vec3(100.0f),
               /*max_cells_per_axis=*/64);

    EXPECT_LE(grid.num_cells_x, 64);
    EXPECT_LE(grid.num_cells_y, 64);
    EXPECT_LE(grid.num_cells_z, 64);
}

// ---------------------------------------------------------------------------
// Test 8: Particle outside grid bounds -- clamped to nearest valid cell
// ---------------------------------------------------------------------------
// Particles outside [origin, origin+extent) must not cause out-of-bounds
// access. The implementation should clamp cell coordinates to [0, num_cells-1].
TEST(SpatialGridTest, ParticleOutsideBounds_ClampsToNearestCell)
{
    // particle at x=-1.0 is outside origin=0.0 -- should clamp to cell_x=0
    glm::vec4 p{-1.0f, 0.5f, 0.5f, 1.0f};

    SpatialGrid grid;
    // This must not crash or invoke undefined behavior
    EXPECT_NO_FATAL_FAILURE(grid.build(&p, 1, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f),
                                       /*max_cells_per_axis=*/64));

    // Particle should have been placed somewhere (clamped to a valid cell)
    EXPECT_EQ(grid.sorted_particles.size(), 1u);
}
