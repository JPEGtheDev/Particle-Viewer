#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D blurredDensity;
uniform float threshold = 0.5;

void main()
{
    vec4 blurred = texture(blurredDensity, TexCoords);
    float density = blurred.a;
    if (density <= threshold) {
        discard;
    }
    // Derive weighted-average color from accumulated RGBA (RGB = color×falloff, A = falloff)
    vec3 metaballColor = (blurred.a > 0.001) ? blurred.rgb / blurred.a : vec3(0.4, 0.7, 1.0);
    color = vec4(metaballColor, 1.0);
}
