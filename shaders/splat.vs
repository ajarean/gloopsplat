#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 projection;
uniform mat4 modelView;
uniform vec3 position;
uniform mat3 cov3d;

out vec3 FragPos;

void main()
{
  FragPos = position;
  vec4 camPos = modelView * vec4(position, 1.0);

  float z2 = camPos.z*camPos.z;
  mat3 J = mat3(
    1/camPos.z, 0, -camPos.x/z2,
    0, 1/camPos.z, -camPos.y/z2,
    0, 0, 0
  );
  mat3 W = mat3(modelView);
  mat2 cov2d = mat2(J*W*cov3d*transpose(W)*transpose(J));

  gl_Position = projection * (camPos + vec4(aPos * 0.1, 0.0));
}