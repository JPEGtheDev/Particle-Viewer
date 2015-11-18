R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 offset;

out vec3 fColor;
//in int gl_InstanceID;
uniform mat4 projection;
uniform mat4 view;
void main()
{

    gl_Position = projection * view * vec4(position + offset, 1.0f);

    fColor = vec3( gl_InstanceID%30 /30.0f, gl_InstanceID%900/900.0f, gl_InstanceID%27000/27000.0f);
}
)"