#pragma once

#include <vector>

#include <glm/glm.hpp>

// clang-format off
#include "glad/glad.h"  // NOLINT(llvm-include-order) -- must precede project headers
// clang-format on

/// Loads and compiles a single compute shader from a GLSL source file.
/// Owns the GL program object; deletes it on destruction.
/// Non-copyable and non-movable (owns a GL resource).
class ComputeShader
{
  public:
    explicit ComputeShader(const char* path);
    ~ComputeShader()
    {
        if (program_)
            glDeleteProgram(program_);
    }

    ComputeShader(const ComputeShader&) = delete;
    ComputeShader& operator=(const ComputeShader&) = delete;
    ComputeShader(ComputeShader&&) = delete;
    ComputeShader& operator=(ComputeShader&&) = delete;

    void use() const
    {
        glUseProgram(program_);
    }
    GLuint program() const
    {
        return program_;
    }

  private:
    GLuint program_ = 0;
};

/// Marching Cubes renderer: manages GPU resources for density field + isosurface extraction.
///
/// Lifecycle:
///   1. Construct with a grid resolution (64, 128, or 256)
///   2. Call markDirty() when particle data changes and a mesh refresh is needed
///   3. Call render() each frame (returns early if !dirty_flag_)
///   4. Call resize() when grid resolution changes
class MCRenderer
{
  public:
    explicit MCRenderer(int grid_res);
    ~MCRenderer();

    // Non-copyable, non-movable (owns GL resources)
    MCRenderer(const MCRenderer&) = delete;
    MCRenderer& operator=(const MCRenderer&) = delete;
    MCRenderer(MCRenderer&&) = delete;
    MCRenderer& operator=(MCRenderer&&) = delete;

    /// Marks the mesh as needing recomputation on the next render() call.
    void markDirty();

    /// Reallocates GPU resources when the grid resolution changes.
    void resize(int new_grid_res);

    /// Runs the MC pipeline if dirty, then draws the mesh.
    ///
    /// When dirty_flag_ is set, dispatches density_field.comp then marching_cubes.comp
    /// to (re)generate the isosurface mesh in vertex_ssbo_. Then draws it with mesh_prog.
    ///
    /// @param particles            Particle positions (xyz) and category (w); used for
    ///                             marching_cubes.comp color-blending (binding 3).
    /// @param grid_origin          World-space origin of the density grid.
    /// @param voxel_size           World-space size of one voxel.
    /// @param influence_radius     Gaussian sigma and hard cutoff for particle contribution.
    /// @param iso_value            Density threshold for isosurface extraction.
    /// @param density_prog         GL program for density_field.comp.
    /// @param mc_prog              GL program for marching_cubes.comp.
    /// @param mesh_prog            GL program for mesh.vert / mesh.frag.
    /// @param projection           Camera projection matrix.
    /// @param view                 Camera view matrix.
    /// @param cell_starts_ssbo     SSBO containing SpatialGrid::cell_starts (binding 2 in density_field.comp).
    /// @param sorted_particles_ssbo SSBO containing SpatialGrid::sorted_particles (binding 3 in density_field.comp).
    /// @param cell_size            SpatialGrid cell size (== influence_radius used during build).
    /// @param cell_origin          SpatialGrid world-space origin.
    /// @param num_cells_x          SpatialGrid cell count along X.
    /// @param num_cells_y          SpatialGrid cell count along Y.
    /// @param num_cells_z          SpatialGrid cell count along Z.
    void render(const std::vector<glm::vec4>& particles, const glm::vec3& grid_origin, float voxel_size,
                float influence_radius, float iso_value, GLuint density_prog, GLuint mc_prog, GLuint mesh_prog,
                const glm::mat4& projection, const glm::mat4& view, GLuint cell_starts_ssbo,
                GLuint sorted_particles_ssbo, float cell_size, const glm::vec3& cell_origin, int num_cells_x,
                int num_cells_y, int num_cells_z);

    // GL resource accessors (for use by render() in Todo 10, and for testing)
    GLuint densityTexture() const
    {
        return density_tex_;
    }
    GLuint vertexSSBO() const
    {
        return vertex_ssbo_;
    }
    GLuint atomicCounter() const
    {
        return atomic_counter_;
    }
    GLuint tableUBO() const
    {
        return table_ubo_;
    }
    int gridRes() const
    {
        return grid_res_;
    }
    bool isDirty() const
    {
        return dirty_flag_;
    }

  private:
    void allocate(int grid_res);
    void deallocate();
    void uploadMCTables();

    int grid_res_ = 0;
    bool dirty_flag_ = true;
    GLuint vertex_count_ = 0; ///< Number of vertices written by the last MC dispatch.
    GLuint density_tex_ = 0;
    GLuint vertex_ssbo_ = 0;
    GLuint atomic_counter_ = 0;
    GLuint table_ubo_ = 0;
    GLuint vao_ = 0;

    // SSBO capacity: 2M triangles * 3 vertices * 9 floats/vertex = 54M floats = 216MB
    static constexpr int kMaxTriangles = 2'000'000;
    static constexpr int kMaxVertices = kMaxTriangles * 3;
};
