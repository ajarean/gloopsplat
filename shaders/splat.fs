#version 330 core

in vec2 fragPos;
in vec4 vColor;

out vec4 fragColor;

void main()
{
  // see Botsch et al
  float r = dot(fragPos, fragPos);
  if (r > 1.0) discard;
  float rho = exp(-r) * vColor.a;
  fragColor = vec4(rho * vColor.rgb, rho);
}