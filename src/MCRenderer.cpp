#include "MCRenderer.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/gtc/type_ptr.hpp>

#include "marching_cubes_tables.hpp"

// ---------------------------------------------------------------------------
// ComputeShader
// ---------------------------------------------------------------------------

ComputeShader::ComputeShader(const char* path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ComputeShader: cannot open " << path << '\n';
        return;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    const std::string src = ss.str();
    const char* cstr = src.c_str();

    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(shader, 1, &cstr, nullptr);
    glCompileShader(shader);

    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "ComputeShader compile error (" << path << "):\n" << log << '\n';
        glDeleteShader(shader);
        return; // program_ stays 0; destructor will skip glDeleteProgram
    }

    program_ = glCreateProgram();
    glAttachShader(program_, shader);
    glLinkProgram(program_);

    glGetProgramiv(program_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048];
        glGetProgramInfoLog(program_, 2048, nullptr, log);
        std::cerr << "ComputeShader link error (" << path << "):\n" << log << '\n';
        glDeleteProgram(program_);
        program_ = 0; // zero out so program() == 0 signals failure to callers
    }

    glDeleteShader(shader);
}

MCRenderer::MCRenderer(int grid_res)
{
    allocate(grid_res);
    uploadMCTables();
}

MCRenderer::~MCRenderer()
{
    deallocate();
}

void MCRenderer::markDirty()
{
    dirty_flag_ = true;
}

void MCRenderer::resize(int new_grid_res)
{
    if (new_grid_res == grid_res_) {
        return;
    }
    deallocate();
    allocate(new_grid_res);
    // Re-upload MC tables -- they don't change with resolution, but table_ubo_ is a new object
    uploadMCTables();
    dirty_flag_ = true;
}

void MCRenderer::allocate(int grid_res)
{
    grid_res_ = grid_res;

    // --- Density field texture (GL_TEXTURE_3D, GL_R32F) ---
    glGenTextures(1, &density_tex_);
    glBindTexture(GL_TEXTURE_3D, density_tex_);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, grid_res, grid_res, grid_res, 0, GL_RED, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);

    // --- Vertex SSBO (triangle output from marching_cubes.comp) ---
    // Layout: 9 floats per vertex [px py pz nx ny nz cr cg cb]
    // Cap: kMaxVertices * 9 floats * 4 bytes = 216MB at 2M triangles
    glGenBuffers(1, &vertex_ssbo_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, vertex_ssbo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(kMaxVertices) * 9 * sizeof(GLfloat), nullptr,
                 GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // --- Vertex counter buffer (vertex count written by marching_cubes.comp) ---
    // Bound as GL_SHADER_STORAGE_BUFFER (binding 2) so the shader can use
    // atomicAdd(vertex_counter, 3u), which is available from GLSL 4.30.
    // (atomicCounterAdd on atomic_uint requires GLSL 4.60.)
    glGenBuffers(1, &atomic_counter_);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomic_counter_);
    GLuint zero = 0;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // --- MC lookup table UBO ---
    glGenBuffers(1, &table_ubo_);
    // (content uploaded in uploadMCTables)

    // --- VAO for VAO-less mesh draw (gl_VertexID indexing) ---
    glGenVertexArrays(1, &vao_);
}

void MCRenderer::deallocate()
{
    if (density_tex_) {
        glDeleteTextures(1, &density_tex_);
        density_tex_ = 0;
    }
    if (vertex_ssbo_) {
        glDeleteBuffers(1, &vertex_ssbo_);
        vertex_ssbo_ = 0;
    }
    if (atomic_counter_) {
        glDeleteBuffers(1, &atomic_counter_);
        atomic_counter_ = 0;
    }
    if (table_ubo_) {
        glDeleteBuffers(1, &table_ubo_);
        table_ubo_ = 0;
    }
    if (vao_) {
        glDeleteVertexArrays(1, &vao_);
        vao_ = 0;
    }
    grid_res_ = 0;
}

void MCRenderer::render(const std::vector<glm::vec4>& particles, const glm::vec3& grid_origin, float voxel_size,
                        float influence_radius, float iso_value, GLuint density_prog, GLuint mc_prog, GLuint mesh_prog,
                        const glm::mat4& projection, const glm::mat4& view)
{
    // Zero-particle guard: nothing to render
    if (particles.empty()) {
        vertex_count_ = 0;
        return;
    }

    if (dirty_flag_) {
        // --- Step 1: Upload particle data to a temporary SSBO ---
        // density_field.comp reads particles from binding 1.
        // marching_cubes.comp reads particles from binding 3.
        GLuint particle_ssbo = 0;
        glGenBuffers(1, &particle_ssbo);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, particle_ssbo);
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(particles.size()) * sizeof(glm::vec4),
                     particles.data(), GL_STREAM_DRAW);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // --- Step 2: Dispatch density_field.comp ---
        glUseProgram(density_prog);

        // density_field.comp binding 0: image3D density_field (write)
        glBindImageTexture(0, density_tex_, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32F);
        // density_field.comp binding 1: ParticleBuffer SSBO (read)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particle_ssbo);

        glUniform1i(glGetUniformLocation(density_prog, "grid_size"), grid_res_);
        glUniform3fv(glGetUniformLocation(density_prog, "grid_origin"), 1, glm::value_ptr(grid_origin));
        glUniform1f(glGetUniformLocation(density_prog, "voxel_size"), voxel_size);
        glUniform1f(glGetUniformLocation(density_prog, "influence_radius"), influence_radius);
        glUniform1i(glGetUniformLocation(density_prog, "particle_count"), static_cast<GLint>(particles.size()));

        const int groups = (grid_res_ + 7) / 8;
        glDispatchCompute(groups, groups, groups);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        // --- Step 3: Reset vertex counter to 0 before MC dispatch ---
        const GLuint zero = 0;
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomic_counter_);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &zero);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        // --- Step 4: Dispatch marching_cubes.comp ---
        glUseProgram(mc_prog);

        // marching_cubes.comp binding 0: density_field (read)
        glBindImageTexture(0, density_tex_, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32F);
        // marching_cubes.comp binding 1: VertexBuffer SSBO (write)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertex_ssbo_);
        // marching_cubes.comp binding 2: VertexCounterBuffer SSBO (uint vertex_counter)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, atomic_counter_);
        // marching_cubes.comp binding 3: ParticleBuffer SSBO (read, for color blending)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, particle_ssbo);
        // marching_cubes.comp binding 4: MCTables SSBO (read, std430)
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, table_ubo_);

        glUniform1i(glGetUniformLocation(mc_prog, "grid_size"), grid_res_);
        glUniform3fv(glGetUniformLocation(mc_prog, "grid_origin"), 1, glm::value_ptr(grid_origin));
        glUniform1f(glGetUniformLocation(mc_prog, "voxel_size"), voxel_size);
        glUniform1f(glGetUniformLocation(mc_prog, "influence_radius"), influence_radius);
        glUniform1i(glGetUniformLocation(mc_prog, "particle_count"), static_cast<GLint>(particles.size()));
        glUniform1f(glGetUniformLocation(mc_prog, "iso_value"), iso_value);
        glUniform1i(glGetUniformLocation(mc_prog, "max_vertices"), kMaxVertices);

        glDispatchCompute(groups, groups, groups);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

        // --- Step 5: Read vertex count from counter SSBO ---
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, atomic_counter_);
        GLuint count = 0;
        glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLuint), &count);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

        vertex_count_ = std::min(count, static_cast<GLuint>(kMaxVertices));

        glDeleteBuffers(1, &particle_ssbo);
        dirty_flag_ = false;

        if (vertex_count_ == 0) {
            // No surface generated (density too low for iso_value, or empty region)
            return;
        }
    }

    if (vertex_count_ == 0) {
        return;
    }

    // --- Step 6: Draw the mesh ---
    glUseProgram(mesh_prog);

    glUniformMatrix4fv(glGetUniformLocation(mesh_prog, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(mesh_prog, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // mesh.vert reads vertices via gl_VertexID from binding 1
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vertex_ssbo_);

    glEnable(GL_DEPTH_TEST);
    // No back-face culling: MC tri_table winding is inconsistent across cube
    // configurations; mesh.frag uses abs(dot(N,L)) to shade both sides correctly.

    // VAO-less draw: mesh.vert uses gl_VertexID to index into the SSBO
    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertex_count_));
    glBindVertexArray(0);

    glDisable(GL_DEPTH_TEST); // restore: MCRenderer enables depth test only during mesh draw
}

void MCRenderer::uploadMCTables()
{
    // std140 UBO layout requires the 2D tri_table to be flattened into a 1D array.
    // edge_table: 256 ints
    // tri_table:  256 * 16 = 4096 ints (flattened from constexpr int tri_table[256][16])
    static_assert(sizeof(mc_tables::tri_table) == 256 * 16 * sizeof(int),
                  "tri_table size mismatch -- check mc_tables namespace");

    // Build flat tri_table
    int flat_tri[256 * 16];
    for (int i = 0; i < 256; ++i) {
        for (int j = 0; j < 16; ++j) {
            flat_tri[i * 16 + j] = mc_tables::tri_table[i][j];
        }
    }

    // UBO layout (std140):
    //   int edge_table[256]      -- 256 * 4 bytes = 1024 bytes
    //   int tri_table[256 * 16]  -- 4096 * 4 bytes = 16384 bytes
    // Total: 17408 bytes
    const GLsizeiptr edge_size = 256 * sizeof(int);
    const GLsizeiptr tri_size = 256 * 16 * sizeof(int);

    // MCTables is an SSBO (std430, binding 4). std430 packs ints at 4-byte stride,
    // matching this CPU upload. std140 would require 16-byte stride (69,632 bytes
    // total), exceeding GL_MAX_UNIFORM_BLOCK_SIZE on NVIDIA and corrupting table reads.
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, table_ubo_);
    glBufferData(GL_SHADER_STORAGE_BUFFER, edge_size + tri_size, nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, edge_size, mc_tables::edge_table);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, edge_size, tri_size, flat_tri);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
