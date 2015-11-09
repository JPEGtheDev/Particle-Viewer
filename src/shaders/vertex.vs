#version 330 core
layout (location = 0) in vec2 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 offset;

out vec3 fColor;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * vec4(position.x + offset.x, position.y + offset.y, 0.0f + offset.z, 1.0f); 
    fColor = color;
}