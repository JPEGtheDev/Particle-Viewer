R"(
#version 330 core
layout (location = 0) in vec3 offset;
out vec3 fColor;
out vec3 lightDir;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightDirection = vec3(0.1, 0.1, 0.85);
uniform float radius = 100.0f;
uniform float scale = 6.0;

void main()
{

    gl_Position = projection * view * vec4(offset, 1.0f);
    float dist = length(gl_Position);
	gl_PointSize = radius * (scale / dist); //radius * scale
	fColor = vec3( gl_InstanceID%40/40.0f, gl_InstanceID%1600/1600.0f, gl_InstanceID%64000/64000.0f);
    lightDir = lightDirection;
}
)"