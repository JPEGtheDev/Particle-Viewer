R"(
#version 330 core
out vec4 color;
in vec3 fColor;
in vec3 lightDir;
void main()
{
   vec3 N;
   N.xy = gl_PointCoord* 2.0 - vec2(1.0); 
   float mag = dot(N.xy, N.xy);
   if(mag > 1.0) 
     discard; // kill pixels outside circle
   N.z = sqrt(1.0-mag);
   // calculate lighting
   float diffuse = max(0.0, dot(lightDir, N));
   color = vec4(fColor*1.25,1) * diffuse;
}
)"