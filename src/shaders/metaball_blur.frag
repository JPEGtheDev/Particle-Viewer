#version 330 core

in vec2 TexCoords;

out vec4 blurred_accum;

uniform sampler2D densityTexture;
uniform float blurAmount = 2.0;
uniform vec2 texelSize;

void main()
{
    int radius = int(blurAmount);
    vec4 total = vec4(0.0);
    int count = 0;
    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            total += texture(densityTexture, TexCoords + vec2(x, y) * texelSize);
            count++;
        }
    }
    if (count == 0)
    {
        blurred_accum = vec4(0.0);
        return;
    }
    blurred_accum = total / float(count);
}
