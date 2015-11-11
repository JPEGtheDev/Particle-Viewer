#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 offset;

out vec3 fColor;
in int gl_InstanceID;
uniform mat4 projection;
uniform mat4 view;
void main()
{

    gl_Position = projection * view * vec4(position + offset, 1.0f);

    fColor = vec3( gl_InstanceID%58 / 58.0f, gl_InstanceID%3364/3364.0f, gl_InstanceID%195112/195112.0f);
}