#version 330 core
out vec4 accum;

flat in float v_category_id;
flat in int v_instance_id;

uniform sampler2D u_prepass_depth;
uniform vec2 u_viewport_inv;
uniform float u_near;
uniform float u_far;

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
		if (linear_depth(gl_FragCoord.z) > front_linear + 30.0) {
			discard;
		}
	}

	vec2 coord = gl_PointCoord - vec2(0.5);
	float d = length(coord) * 2.0;
	// Gaussian falloff: non-zero at sprite corners (unlike polynomial max(0,1-d²)).
	// k=1.4 ≈ ln(2)/0.5 places the threshold=0.5 visual boundary at the same d=0.707
	// as the old polynomial, but corner density ≈ 0.06 prevents the black-square
	// artifact where adjacent sprites did not cover each other's corners.
	float contribution = exp(-1.4 * d * d);

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
