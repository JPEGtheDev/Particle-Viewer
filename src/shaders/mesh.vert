#version 430 core

// The SSBO written by marching_cubes.comp -- 9 floats per vertex
// [px py pz nx ny nz cr cg cb]
layout(std430, binding = 1) readonly buffer VertexBuffer {
    float vertices[];
};

// Standard MVP uniforms
uniform mat4 projection;
uniform mat4 view;
// No model matrix -- mesh is already in world space

// Outputs to fragment shader
out vec3 fColor;
out vec3 fWorldNormal;  // world-space, no view transform
out vec3 fLightDir;

// Same light direction as sphere shaders (sphereVertex.vs line 7)
uniform vec3 lightDirection = vec3(0.1, 0.1, 0.85);

void main()
{
    uint base = uint(gl_VertexID) * 9u;

    vec3 pos    = vec3(vertices[base + 0u], vertices[base + 1u], vertices[base + 2u]);
    vec3 normal = vec3(vertices[base + 3u], vertices[base + 4u], vertices[base + 5u]);
    vec3 color  = vec3(vertices[base + 6u], vertices[base + 7u], vertices[base + 8u]);

    gl_Position  = projection * view * vec4(pos, 1.0);

    fWorldNormal = normal;  // world-space, passed through AS-IS (DO NOT multiply by view)
    fColor       = color;
    fLightDir    = lightDirection;
}
