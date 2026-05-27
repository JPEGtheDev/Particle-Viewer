#version 330 core
out vec4 accum;

flat in float v_category_id;

void main()
{
	vec2 coord = gl_PointCoord - vec2(0.5);
	float d = length(coord) * 2.0;
	float contribution = max(0.0, 1.0 - d * d);

	int cat = int(v_category_id);
	// Category color formula replicates sphereVertex.vs:38; designed for full category range.
	// Near-black for small category indices is expected and correct.
	vec3 catColor = vec3(
	    float(cat % 40) / 40.0,
	    float(cat % 1600) / 1600.0,
	    float(cat % 64000) / 64000.0
	);

	accum = vec4(catColor * contribution, contribution);
}
