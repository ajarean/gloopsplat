#version 330 core

in vec2 fragPos;
in vec4 vColor;
in vec3 vNormal;

uniform vec3 lightDir;

out vec4 fragColor;

void main()
{
  // TODO: make these configurable through imGui
  float specular = 1.0;
  float roughness = 0.05;

  // see Botsch et al
  float r = dot(fragPos, fragPos);
  if (r > 1.0) discard;

  float diff = max(dot(normalize(vNormal), normalize(lightDir)), 0.0);
  float spec = pow(diff, 1.0 / roughness);
  // TODO: get thickness
  // cp(ri) = dp + sp ⊙ Ls(ri, np, ρp)
  vec3 c = vColor.rgb * diff + specular * vec3(spec);

  float rho = exp(-r) * vColor.a;
  fragColor = vec4(rho * c, rho);
}