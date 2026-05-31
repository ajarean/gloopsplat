#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 position;
layout (location = 2) in float radius;
layout (location = 3) in vec4 color;
layout (location = 4) in vec3 normal;
layout (location = 5) in float surface;

uniform mat4 projection;
uniform mat4 modelView;
uniform vec2 viewport;
uniform vec2 focal;

out vec4 vColor;
out vec3 vNormal;
out vec2 fragPos;
out float vDepth;
out float vSurface;
out vec3 vPosition;

void main()
{
	vColor = color;
	vNormal = normal;
	vPosition = position;
	vec4 camPos = modelView * vec4(position, 1.0);
	if (camPos.z > -0.1) return;
	vDepth = -camPos.z;
	vec4 clipPos = projection * camPos;

	mat3 J = mat3(
		focal.x / camPos.z, 0.0, -(focal.x * camPos.x) / (camPos.z * camPos.z),
		0.0, focal.y / camPos.z, -(focal.y * camPos.y) / (camPos.z * camPos.z),
		0.0, 0.0, 0.0
	);
	mat3 W = mat3(modelView);
	// assume radii are equal for fluid particles
	mat3 cov3d = mat3(radius*radius);
	mat3 cov2d = transpose(J) * W * cov3d * transpose(W) * J;

	// add low pass filter (2x2 identity)
	cov2d[0][0] += 1.0;
	cov2d[1][1] += 1.0;

	// follows from fluid assumption
	float d = sqrt(2.0 * (cov2d[0][0]));
	fragPos = aPos.xy;
	vec2 vCenter = clipPos.xy / clipPos.w;

	gl_Position = vec4(
		vCenter + aPos.xy * d / viewport,
		0.0, 1.0
	);
}