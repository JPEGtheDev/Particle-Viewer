#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D blurredDensity;
uniform float threshold = 0.5;
uniform vec3 metaball_color = vec3(0.4, 0.7, 1.0);

void main()
{
    float density = texture(blurredDensity, TexCoords).r;
    if (density <= threshold) {
        discard;
    }
    // Smooth the surface edge: remap density near threshold for soft falloff
    float edge = smoothstep(threshold, threshold + 0.1, density);
    color = vec4(metaball_color * edge, edge);
}
