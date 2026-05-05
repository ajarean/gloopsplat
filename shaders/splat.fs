#version 330 core

in vec2 fragPos;
in vec4 vColor;
in vec3 vNormal;
in float vDepth;

uniform vec3 lightDir;
uniform float blur;

out vec4 fragColor;

void main()
{
  // TODO: make these configurable through imGui
  float specular = 0.4;
  float roughness = 0.3;

  // see Botsch et al
  float r = dot(fragPos, fragPos);
  if (r > 1.0) discard;

  float diff = max(dot(normalize(vNormal), normalize(lightDir)), 0.0);
  float spec = min(pow(diff, 1.0 / roughness), 0.3);
  // TODO: get thickness
  // cp(ri) = dp + sp ⊙ Ls(ri, np, ρp)
  vec3 c = vColor.rgb * (0.6 + 0.4*diff) + specular * vec3(spec);

  // vec3 depthColor = vColor.rgb * (1.0 / (1.0 + vDepth * 0.12));
  float depthOpacity = vColor.a * (1.0 / (1.0 + vDepth * 0.1));
  float rho = exp(-r * blur) * depthOpacity;
  // float rho = exp(-r * blur) * vColor.a;
  fragColor = vec4(rho * c, rho);
}