#version 330 core
out vec4 accum;

flat in float v_category_id;
flat in int v_instance_id;

uniform sampler2D u_prepass_depth;
uniform vec2 u_viewport_inv;
uniform float u_near;
uniform float u_far;
uniform float u_depth_cull_range;

float linear_depth(float d)
{
	float z = d * 2.0 - 1.0;
	return 2.0 * u_near * u_far / (u_far + u_near - z * (u_far - u_near));
}

void main()
{
	// Depth cull: discard fragments more than 30 linear-depth units behind the
	// front surface. Additive offset (not multiplicative) so a dense cloud viewed
	// up close is not aggressively culled — any particle within 30 world-units of
	// the front contributes to the density field regardless of camera distance.
	// Skip culling entirely when no particle wrote to the prepass (prepass_d == 1.0).
	float prepass_d = texture(u_prepass_depth, gl_FragCoord.xy * u_viewport_inv).r;
	if (prepass_d < (1.0 - 1e-5)) {
		float front_linear = linear_depth(prepass_d);
		if (linear_depth(gl_FragCoord.z) > front_linear + u_depth_cull_range) {
			discard;
		}
	}

	vec2 coord = gl_PointCoord - vec2(0.5);
	float d = length(coord) * 2.0;
	// Scaled polynomial falloff: max(0, 1-(d/S)²) with S=1.2.
	// S > 1.0 means the zero crossing moves beyond d=1.0 (sprite edge midpoints)
	// into the ring d∈[1.0,1.2], giving non-zero density in that ring for
	// merging. Corners at d≈1.414 are still exactly zero (1.2 < 1.414) so the
	// GL_POINTS square never shows as a visible rectangle.
	// Visual boundary (threshold=0.5) sits at d = 1.2*sqrt(0.5) ≈ 0.849.
	const float S = 1.2;
	float contribution = max(0.0, 1.0 - (d / S) * (d / S));

	int cat = int(v_category_id);
	// Category colors match sphereVertex.vs hardcoded colors for cats 0-3.
	vec3 catColor;
	if (cat == 0) {
	    catColor = vec3(1.0, 0.0, 0.0);
	} else if (cat == 1) {
	    catColor = vec3(0.2, 0.6, 1.0);
	} else if (cat == 2) {
	    catColor = vec3(1.0, 0.0, 1.0);
	} else if (cat == 3) {
	    catColor = vec3(0.89, 0.59, 0.0);
	} else if (cat == 500) { // 500 = per-instance debug/rainbow colour mode (matches sphereVertex.vs)
	    catColor = vec3(float(v_instance_id % 40) / 40.0,
	                    float(v_instance_id % 1600) / 1600.0,
	                    float(v_instance_id % 64000) / 64000.0);
	} else {
	    catColor = vec3(float(cat % 40) / 40.0,
	                    float(cat % 1600) / 1600.0,
	                    float(cat % 64000) / 64000.0);
	}

	accum = vec4(catColor * contribution, contribution);
}
