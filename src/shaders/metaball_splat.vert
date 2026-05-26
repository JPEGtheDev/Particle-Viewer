#version 330 core
layout (location = 0) in vec4 offset;
uniform mat4 projection;
uniform mat4 view;
uniform float blobRadius = 100.0;
uniform float scale = 5.0;
uniform float transScale = 0.25;
uniform float viewportHeight = 720.0;
const float REFERENCE_HEIGHT = 720.0;

void main()
{
	gl_Position = projection * view * vec4(offset.xyz * transScale, 1.0);
	float dist = max(length(gl_Position), 0.0001);
	gl_PointSize = blobRadius * (scale / dist) * (viewportHeight / REFERENCE_HEIGHT);
}
