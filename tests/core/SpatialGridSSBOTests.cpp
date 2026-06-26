// tests/core/SpatialGridSSBOTests.cpp
// Tests for Todo 24: SpatialGrid SSBO wiring into MCRenderer density dispatch.
//
// NOTE: viewer_app.cpp is NOT linked in ParticleViewerTests; ViewerApp cannot be
// instantiated at runtime here. Structural tests use static_assert on public
// member types and method signatures. Behavioural tests exercise the pure-C++
// portions of the pipeline (SpatialGrid output correctness), validating what
// would be uploaded to GPU SSBOs.

// clang-format off
#include <glad/glad.h>  // NOLINT(llvm-include-order) -- must precede GL headers
// clang-format on

#include <type_traits>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include <glm/glm.hpp>

#include "MCRenderer.hpp"
#include "SpatialGrid.hpp"
#include "viewer_app.hpp"

// ============================================================================
// Structural: MCRenderer header must include SpatialGrid.hpp must be included
// from viewer_app.hpp -- compilation of this file verifies the include chain.
// ============================================================================
// (The four private members spatial_grid_, cell_starts_ssbo_, sorted_particles_ssbo_,
// last_spatial_grid_ir_ are verified implicitly: viewer_app.hpp must include
// SpatialGrid.hpp for the file to compile, and their presence is confirmed when
// viewer_app.cpp compiles successfully.)

// ============================================================================
// Structural: MCRenderer::render() must accept the extended SSBO signature
//
// Before Todo 24 the signature has 10 parameters; after it has 15.
// These tests verify the new parameter types exist at compile time.
// ============================================================================

// Helper: the *new* render() signature after Todo 24
//   render(particles, grid_origin, voxel_size, influence_radius, iso_value,
//          density_prog, mc_prog, mesh_prog, projection, view,
//          cell_starts_ssbo, sorted_particles_ssbo,
//          cell_size, cell_origin, num_cells_x, num_cells_y, num_cells_z)
//
// The test below calls render() with the *new* argument count via a
// function-pointer alias. If the signature doesn't match, this won't compile.

// Verify the extended render() is callable with 17 arguments.
// (std::is_invocable_v cannot access private methods; we check the method
// pointer directly via std::declval on the public interface.)

using RenderFnPtr17 = void (MCRenderer::*)(const std::vector<glm::vec4>&, const glm::vec3&, float, float, float, GLuint,
                                           GLuint, GLuint, const glm::mat4&, const glm::mat4&, GLuint, GLuint, float,
                                           glm::vec3, int, int, int);

// RED phase: this static_assert will fail before the signature is extended.
// After implementation it becomes GREEN.
static_assert(std::is_same_v<RenderFnPtr17, decltype(&MCRenderer::render)>,
              "MCRenderer::render() must accept 17 parameters after Todo 24 SSBO wiring");

// ============================================================================
// Behavioural: SpatialGrid output matches what rebuildSpatialGrid() would upload
//
// These tests exercise the pure-C++ SpatialGrid path that feeds GPU SSBOs.
// They do NOT call any GL functions, so they run headlessly.
// ============================================================================

// Test 1: After build(), cell_starts has (total_cells + 1) entries (sentinel).
TEST(SpatialGridSSBOWiring, Build_CellStartsHasSentinel)
{
    // Arrange
    glm::vec4 particles[2] = {
        {0.5f, 0.5f, 0.5f, 1.0f},
        {1.5f, 0.5f, 0.5f, 1.0f},
    };
    SpatialGrid grid;

    // Act
    grid.build(particles, 2, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f),
               /*max_cells_per_axis=*/64);

    // Assert: sentinel at cell_starts.back() must equal sorted_particles.size()
    ASSERT_FALSE(grid.cell_starts.empty());
    EXPECT_EQ(grid.cell_starts.back(), static_cast<uint32_t>(grid.sorted_particles.size()));
}

// Test 2: sorted_particles contains all input particles (no data loss).
TEST(SpatialGridSSBOWiring, Build_SortedParticlesCountMatchesInput)
{
    // Arrange
    constexpr int kCount = 5;
    glm::vec4 particles[kCount] = {
        {0.1f, 0.1f, 0.1f, 0.0f}, {2.1f, 0.1f, 0.1f, 1.0f}, {4.1f, 0.1f, 0.1f, 2.0f},
        {6.1f, 0.1f, 0.1f, 3.0f}, {8.1f, 0.1f, 0.1f, 4.0f},
    };
    SpatialGrid grid;

    // Act
    grid.build(particles, kCount, /*influence_radius=*/2.0f, glm::vec3(0.0f), glm::vec3(10.0f),
               /*max_cells_per_axis=*/64);

    // Assert: GPU upload will send exactly kCount vec4s
    EXPECT_EQ(static_cast<int>(grid.sorted_particles.size()), kCount);
}

// Test 3: cell metadata (cell_size, cell_origin, num_cells_*) is set after build().
// These are the values rebuildSpatialGrid() passes as uniforms to MCRenderer::render().
TEST(SpatialGridSSBOWiring, Build_CellMetadataIsPopulated)
{
    // Arrange
    glm::vec4 p{1.0f, 1.0f, 1.0f, 0.0f};
    const glm::vec3 origin(0.0f);
    const glm::vec3 extent(10.0f);
    const float ir = 2.0f;
    SpatialGrid grid;

    // Act
    grid.build(&p, 1, ir, origin, extent, /*max_cells_per_axis=*/64);

    // Assert: cell_size == influence_radius; cell_origin == origin
    EXPECT_FLOAT_EQ(grid.cell_size, ir);
    EXPECT_FLOAT_EQ(grid.cell_origin.x, origin.x);
    EXPECT_FLOAT_EQ(grid.cell_origin.y, origin.y);
    EXPECT_FLOAT_EQ(grid.cell_origin.z, origin.z);
    EXPECT_GE(grid.num_cells_x, 1);
    EXPECT_GE(grid.num_cells_y, 1);
    EXPECT_GE(grid.num_cells_z, 1);
}

// Test 4: cell_starts SSBO size that would be uploaded equals
//         (total_cells + 1) * sizeof(uint32_t).
TEST(SpatialGridSSBOWiring, Build_CellStartsUploadSizeIsCorrect)
{
    // Arrange
    glm::vec4 particles[3] = {
        {0.5f, 0.5f, 0.5f, 1.0f},
        {1.5f, 0.5f, 0.5f, 1.0f},
        {2.5f, 0.5f, 0.5f, 1.0f},
    };
    SpatialGrid grid;
    grid.build(particles, 3, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f),
               /*max_cells_per_axis=*/64);

    // Act: compute the upload size as rebuildSpatialGrid() would
    const std::size_t upload_bytes = grid.cell_starts.size() * sizeof(uint32_t);

    // Assert: at minimum 2 entries (1 cell + sentinel) * 4 bytes = 8 bytes
    EXPECT_GE(upload_bytes, static_cast<std::size_t>(8));
    // The sentinel is the last entry
    EXPECT_EQ(grid.cell_starts.back(), static_cast<uint32_t>(grid.sorted_particles.size()));
}

// Test 5: Rebuild with changed influence_radius updates cell_size and num_cells.
// This mirrors the condition: last_spatial_grid_ir_ != mc_scaled_ir_ triggers rebuild.
TEST(SpatialGridSSBOWiring, Rebuild_WithNewIr_UpdatesCellSizeAndNumCells)
{
    // Arrange
    glm::vec4 particles[2] = {
        {0.5f, 0.5f, 0.5f, 1.0f},
        {8.5f, 8.5f, 8.5f, 1.0f},
    };
    SpatialGrid grid;

    // Act: first build with ir=1.0
    grid.build(particles, 2, /*influence_radius=*/1.0f, glm::vec3(0.0f), glm::vec3(10.0f),
               /*max_cells_per_axis=*/64);
    const float cell_size_first = grid.cell_size;
    const int num_cells_x_first = grid.num_cells_x;

    // Act: rebuild with ir=5.0 (larger cells, fewer of them)
    grid.build(particles, 2, /*influence_radius=*/5.0f, glm::vec3(0.0f), glm::vec3(10.0f),
               /*max_cells_per_axis=*/64);
    const float cell_size_second = grid.cell_size;
    const int num_cells_x_second = grid.num_cells_x;

    // Assert: cell_size must change; larger ir -> fewer cells
    EXPECT_FLOAT_EQ(cell_size_first, 1.0f);
    EXPECT_FLOAT_EQ(cell_size_second, 5.0f);
    EXPECT_LT(num_cells_x_second, num_cells_x_first);
}
