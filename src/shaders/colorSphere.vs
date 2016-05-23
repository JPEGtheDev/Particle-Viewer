#version 330 core
out vec3 fColor;
out vec3 lightDir;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 pos;
uniform vec3 color;
uniform vec3 lightDirection = vec3(0.1, 0.1, 0.85);
uniform float radius = 100.0f;
uniform float scale = 7.5;
void main()
{
    gl_Position = projection * view * vec4(pos,1.0f);
	float dist = length(gl_Position);
	gl_PointSize = radius * (scale / dist);
	fColor = color;
	lightDir = lightDirection;
}
