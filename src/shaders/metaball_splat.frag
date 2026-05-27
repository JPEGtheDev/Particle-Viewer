#version 330 core
out vec4 accum;

flat in float v_category_id;
flat in int v_instance_id;

void main()
{
	vec2 coord = gl_PointCoord - vec2(0.5);
	float d = length(coord) * 2.0;
	float contribution = max(0.0, 1.0 - d * d);

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
	} else if (cat == 500) {
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
