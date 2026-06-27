#version 430 core

in vec3 fColor;
in vec3 fWorldNormal;
in vec3 fLightDir;

out vec4 fragColor;

void main()
{
    // Phong diffuse -- abs() gives double-sided shading so back-facing triangles
    // (which MC can produce due to winding inconsistency across cube configs)
    // are lit identically to front-facing ones rather than appearing as holes.
    float diffuse = abs(dot(normalize(fWorldNormal), normalize(fLightDir)));
    // Add ambient term to prevent completely dark surfaces
    float ambient = 0.15;
    fragColor = vec4(fColor * (ambient + diffuse), 1.0);
}
