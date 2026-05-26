#version 330 core
out float density;

void main()
{
	vec2 coord = gl_PointCoord - vec2(0.5);
	float d = length(coord) * 2.0;
	float contribution = max(0.0, 1.0 - d * d);
	density = contribution;
}
