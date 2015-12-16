R"(
#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 offset;

out vec3 fColor;
out vec3 lightDir;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightDirection = vec3(0.1, 0.1, 0.85);
uniform float radius = 100.0f;
uniform float scale = 7.25f;

void main()
{

    gl_Position = projection * view * vec4(offset, 1.0f);
    float dist = length(gl_Position);
	gl_PointSize = 100.0f * (7.25f / dist); //radius * scale
    fColor = vec3( gl_InstanceID%100 /100.0f, gl_InstanceID%10000/10000.0f, gl_InstanceID%1000000/1000000.0f);
    lightDir = lightDirection;
}
)"