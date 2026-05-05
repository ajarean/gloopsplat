#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 modelView;
uniform vec2 viewport;
uniform vec2 focal;
uniform vec3 position;
uniform float radius;
uniform vec4 color;
uniform vec3 normal;

out vec4 vColor;
out vec3 vNormal;
out vec2 fragPos;

void main()
{
  vColor = color;
  vNormal = normal;
  vec4 camPos = modelView * vec4(position, 1.0);
  if (camPos.z > -0.1) return;
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