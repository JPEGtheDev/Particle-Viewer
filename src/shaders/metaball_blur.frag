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
    // Gaussian weights: sigma = radius/2 → kernel decays to ~14% at the edge.
    // Two separable 1-D Gaussian passes (H then V) compose to a 2-D circular
    // Gaussian, giving round blobs instead of the squircles produced by a box blur.
    float sigma = float(radius) / 2.0;
    float twoSigmaSq = 2.0 * sigma * sigma;
    vec4 total = vec4(0.0);
    float totalWeight = 0.0;
    for (int i = -radius; i <= radius; i++)
    {
        float w = exp(-float(i * i) / twoSigmaSq);
        total += w * texture(densityTexture, TexCoords + float(i) * texelSize * blurDir);
        totalWeight += w;
    }
    blurred_accum = totalWeight > 0.0 ? total / totalWeight : vec4(0.0);
}
