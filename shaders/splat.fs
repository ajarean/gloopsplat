#version 330 core
#define PI 3.14159265359

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
uniform int specType; // [phong, ggx]
uniform float colorStrength; // [0,1]

out vec4 fragColor;

void main()
{
	// see Botsch et al
	float r = dot(fragPos, fragPos);
	if (r > 1.0) discard;

	vec3 n = normalize(vNormal); // normal
	vec3 v = normalize(cameraPos - vPosition); // vector to cam
	vec3 l = normalize(lightDir);
	vec3 h = normalize(normalize(lightDir) + v); // halfway

	float ndv = max(dot(n, v), 0.0);
	float ndh = max(dot(n, h), 0.0);

	float diff = max(dot(n, normalize(lightDir)), 0.0);
	// shlick specular reflection: F = F0 + (1-F0)(1-max(n dot v, 0)^5)
	float F0 = 0.02; // (n1-n2)/(n1+n2))^2
	float F = F0 + (1.0 - F0) * pow(1.0 - ndv, 5.0);

	float spec;
	if (specType == 0) {
		// blinn phong
		float shininess = pow(1000, 1.0 - roughness);
		spec = pow(max(dot(n, h), 0.0), shininess) * diff;
	}
	else {
		// walter et al., GGX impl.
		// tan^2(theta)v = (1-ndv^2)/ndv^2
		float a2 = pow(roughness, 4.0); // a = roughness^2
		float denom = ndh * ndh * (a2 - 1.0) + 1.0; // = a2*ndh² + 1 - ndh²
		float D = a2 / (PI * denom * denom);
		float G1_v = 2.0 * ndv / (ndv + sqrt(a2 + (1.0 - a2) * ndv * ndv));
		float G1_l = 2.0 * diff / (diff + sqrt(a2 + (1.0 - a2) * diff * diff));
		float G = G1_v*G1_l;
		spec = (D * G * F) / max(4.0 * ndv * diff, 0.001) * diff;
	}

	vec3 c_color = vColor.rgb * min((0.4 + diffuse*0.6*diff), 1.0) + specular * vec3(spec);

	// cp(ri) = dp + sp ⊙ Ls(ri, np, ρp) (Gaussian Splashing eq. 4)
	// below code from https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.2.cubemaps_environment_mapping/6.2.cubemaps.fs
	vec3 I = normalize(vPosition - cameraPos);
	vec3 R = reflect(I, normalize(n));

	vec3 Ls = vec3(0.0);
	if (type > 0) { // avoid fetching textures on RGBA case
		Ls = texture(cubeMap, R).rgb;
	}

	float rho = exp(-r * blur) * vColor.a;
	vec3 c;

	vec3 k = vec3(0.3, 0.3, 0.3); // absorption
	// TODO: get thickness from thickness pass (eventually)
	float thickness = rho * (1.0 / (1.0 + vDepth * 0.1)); // approximation based on depth
	vec3 tint = mix(vec3(1.0), vColor.rgb, colorStrength);
	vec3 dp = exp(-k*thickness) * Ls * tint;
	vec3 c_env = diffuse * dp + vec3(specular) * Ls;

	// fresnel equation (van der laan eq 13), approximated
	// C_out = c_refrac * (1-F) + c_reflec*F + k_s(n dot h)^alpha
	vec3 c_fres = diffuse*dp * (1.0 - F) + Ls * F + specular * vec3(spec);

	if (type == 0) c = c_color;
	else if (type == 1) c = c_env;
	else if (type == 2) c = c_fres;
	fragColor = vec4(rho * c, rho);
}