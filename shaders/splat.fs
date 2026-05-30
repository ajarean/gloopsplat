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
uniform float diffuse;
uniform float specular;
uniform float roughness;
uniform vec3 cameraPos;
uniform int type; // [rgba, envMap, fresnel]

out vec4 fragColor;

void main()
{
	// see Botsch et al
	float r = dot(fragPos, fragPos);
	if (r > 1.0) discard;

	vec3 n = normalize(vNormal);
	float diff = max(dot(n, normalize(lightDir)), 0.0);
	// float spec = vSurface * min(pow(diff, 1.0 / roughness), 0.3);
	float alpha = 1.0/roughness;
	float spec = pow(diff, alpha);

	vec3 c_color = vColor.rgb * (0.4 + diffuse*0.6*diff) + specular * vec3(spec);

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
	vec3 c;

	vec3 k = vec3(0.5, 0.35, 0.2);
	float thickness = rho * (1.0 / (1.0 + vDepth * 0.1));
	vec3 dp = exp(-k*thickness) * Ls;
	vec3 c_env = diffuse * dp + vec3(specular) * Ls;

	vec3 v = normalize(cameraPos - vPosition);
	vec3 h = normalize(normalize(lightDir) + v);

	float ndv = max(dot(n, v), 0.0);
	// shlick specular reflection: F = F0 + (1-F0)(1-max(n dot v, 0)^5)
	float F0 = 0.02; // (n1-n2)/(n1+n2))^2
	float F = F0 + (1.0 - F0) * pow(1.0 - ndv, 5.0);

	// fresnel equation (van der laan eq 13), approximated
	// C_out = c_refrac * (1-F) + c_reflec*F + k_s(n dot h)^alpha
	vec3 c_fres = diffuse*dp * (1.0 - F) + Ls * F + specular * vec3(spec);

	if (type == 0) c = c_color;
	else if (type == 1) c = c_env;
	else if (type == 2) c = c_fres;
	fragColor = vec4(rho * c, rho);
}