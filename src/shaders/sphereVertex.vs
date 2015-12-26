R"(
#version 330 core
layout (location = 0) in vec4 offset;
out vec3 fColor;
out vec3 lightDir;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 lightDirection = vec3(0.1, 0.1, 0.85);
uniform float radius = 100.0f;
uniform float scale = 5.0;
uniform float transScale = 0.25;

void main()
{
	int colVal = int(offset.w);
    gl_Position = projection * view * vec4(offset.xyz * transScale,1.0f);
    float dist = length(gl_Position);
	gl_PointSize = radius * (scale / dist); //radius * scale
	if(colVal == 0)
	{
		fColor = vec3(1,0,0);
	}
	else if(colVal == 1)
	{
		fColor = vec3(0,1,0);
	}
	else if(colVal == 2)
	{
		fColor = vec3(0,0,1);
	}
	else if(colVal == 3)
	{
		fColor = vec3(1,1,0);
	}
	else if (colVal == 500)
	{
		fColor = vec3( gl_InstanceID%40/40.0f, gl_InstanceID%1600/1600.0f, gl_InstanceID%64000/64000.0f);
	}
	
    lightDir = lightDirection;
}
)"