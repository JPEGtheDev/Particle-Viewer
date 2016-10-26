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
uniform float angleRot = 20;
uniform vec3 rotPoint = vec3(0,0,0);

vec4 quat_from_axis_angle(vec3 axis, float angle)
{
  vec4 qr;
  float half_angle = (angle * 0.5) * 3.14159 / 180.0;
  qr.x = axis.x * sin(half_angle);
  qr.y = axis.y * sin(half_angle);
  qr.z = axis.z * sin(half_angle);
  qr.w = cos(half_angle);
  return qr;
}

vec4 quat_conj(vec4 q)
{
  return vec4(-q.x, -q.y, -q.z, q.w);
}

vec4 quat_mult(vec4 q1, vec4 q2)
{
  vec4 qr;
  qr.x = (q1.w * q2.x) + (q1.x * q2.w) + (q1.y * q2.z) - (q1.z * q2.y);
  qr.y = (q1.w * q2.y) - (q1.x * q2.z) + (q1.y * q2.w) + (q1.z * q2.x);
  qr.z = (q1.w * q2.z) + (q1.x * q2.y) - (q1.y * q2.x) + (q1.z * q2.w);
  qr.w = (q1.w * q2.w) - (q1.x * q2.x) - (q1.y * q2.y) - (q1.z * q2.z);
  return qr;
}

vec3 rotate_vertex_position(vec3 position, vec3 axis, float angle)
{
  vec4 qr = quat_from_axis_angle(axis, angle);
  vec4 qr_conj = quat_conj(qr);
  vec4 q_pos = vec4(position.x, position.y, position.z, 0);

  vec4 q_tmp = quat_mult(qr, q_pos);
  qr = quat_mult(q_tmp, qr_conj);

  return vec3(qr.x, qr.y, qr.z);
}

void main()
{	
	float yaw = 2;
	float pitch = .9;
	vec3 P = rotate_vertex_position(vec3((offset.xyz  - rotPoint.xyz)* transScale).xyz, vec3(0,1,0), angleRot);
	
	int colVal = int(offset.w);
	gl_Position = (projection * view) * vec4(P,1.0f);
	float dist = length(gl_Position);
	gl_PointSize = radius * (scale / dist); //radius * scale
	if(colVal == 0)
	{
		fColor = vec3(1.0,0,0); //core1
	}
	else if(colVal == 1)
	{
		fColor = vec3(0.2,0.6,1.0); //sil1
	}
	else if(colVal == 2)
	{
		fColor = vec3(1.0,0,1.0); //core2
	}
	else if(colVal == 3)
	{
		fColor = vec3(0.89,0.59,0);
	}
	else if (colVal == 500)
	{
		fColor = vec3( gl_InstanceID%40/40.0f, gl_InstanceID%1600/1600.0f, gl_InstanceID%64000/64000.0f);
	}

    lightDir = lightDirection;
}
