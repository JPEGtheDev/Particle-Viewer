#version 330 core
in vec2 TexCoords;
out float blurred_density;
uniform sampler2D densityTexture;
uniform float blur_amount = 2.0;
uniform vec2 texel_size;
void main()
{
    int radius = int(blur_amount);
    float total = 0.0;
    int count = 0;
    for (int x = -radius; x <= radius; x++)
    {
        for (int y = -radius; y <= radius; y++)
        {
            total += texture(densityTexture, TexCoords + vec2(x, y) * texel_size).r;
            count++;
        }
    }
    blurred_density = total / float(count);
}
