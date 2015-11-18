R"(
#version 330 core
out vec4 color;
in vec3 fColor;
void main()
{
    color = vec4(fColor, 1.0f);
}
)"