#version 330 core

in vec2 TexCoords;

out vec4 blurred_accum;

uniform sampler2D densityTexture;
uniform float blurAmount = 2.0;
uniform vec2 texelSize;
uniform vec2 blurDir = vec2(1.0, 0.0);

void main()
{
    int radius = int(blurAmount);
    vec4 total = vec4(0.0);
    int count = 0;
    for (int i = -radius; i <= radius; i++)
    {
        total += texture(densityTexture, TexCoords + float(i) * texelSize * blurDir);
        count++;
    }
    blurred_accum = count > 0 ? total / float(count) : vec4(0.0);
}
