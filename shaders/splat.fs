#version 330 core

in vec2 fragPos;
in vec4 vColor;
in vec3 vNormal;
in float vDepth;
in float vSurface;
in vec3 vPosition;

uniform samplerCube cubeMap;
uniform vec3 lightDir;
uniform float blur;
uniform float specular;
uniform float roughness;
uniform vec3 cameraPos;
uniform float envWeight; // [0, 1]

out vec4 fragColor;

void main()
{
  // see Botsch et al
  float r = dot(fragPos, fragPos);
  if (r > 1.0) discard;

  float diff = max(dot(normalize(vNormal), normalize(lightDir)), 0.0);
  // float spec = vSurface * min(pow(diff, 1.0 / roughness), 0.3);
  float spec = max(pow(diff, 1.0 / roughness), 0.5);

  // TODO: get thickness
  // cp(ri) = dp + sp ⊙ Ls(ri, np, ρp) (Gaussian Splashing eq. 4)
  // below code from https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.2.cubemaps_environment_mapping/6.2.cubemaps.fs
  vec3 I = normalize(vPosition - cameraPos);
  vec3 R = reflect(I, normalize(vNormal));

  vec3 Ls = texture(cubeMap, R).rgb;
  // vec3 c = vColor.rgb * (0.6 + 0.4*diff) + specular * vec3(spec);

  // vec3 depthColor = vColor.rgb * (1.0 / (1.0 + vDepth * 0.12));
  // float depthOpacity = vColor.a * (1.0 / (1.0 + vDepth * 0.1));
  float rho = exp(-r * blur) * vColor.a;

  vec3 k = vec3(0.5, 0.35, 0.2);
  float thickness = rho * (1.0 / (1.0 + vDepth * 0.1));
  vec3 dp = exp(-k*thickness) * Ls;
  vec3 c_color = vColor.rgb * (0.6 + 0.4*diff) + specular * vec3(spec);
  vec3 c_env = dp + vec3(specular) * Ls;
  vec3 c = (1 - envWeight)*c_color + envWeight * c_env;
  // vec3 c = Ls;
  fragColor = vec4(rho * c, rho);
}