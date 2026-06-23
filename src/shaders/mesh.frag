#version 430 core

in vec3 fColor;
in vec3 fWorldNormal;
in vec3 fLightDir;

out vec4 fragColor;

void main()
{
    // Phong diffuse using world-space quantities -- view-angle-invariant as camera orbits
    float diffuse = max(0.0, dot(normalize(fWorldNormal), normalize(fLightDir)));
    // Add ambient term to prevent completely dark surfaces
    float ambient = 0.15;
    fragColor = vec4(fColor * (ambient + diffuse), 1.0);
}
