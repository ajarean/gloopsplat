// Andy Jarean, Tingxuan Wu

/* ATTRIBUTIONS:
 * OpenGL boilerplate based on tutorials by Joey de Vries (https://learnopengl.com and https://github.com/JoeyDeVries/LearnOpenGL/)
 * License: CC BY-NC 4.0 (https://creativecommons.org/licenses/by-nc/4.0/)
 * Attributed to: Joey de Vries (Twitter: @JoeyDeVriez)
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <filesystem>
#include <chrono>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "camera.h"
#include "renderer.h"
#include "shader.h"
#include "particle.h"
#include "scene.h"
#include "skybox.h"

using namespace glm;

// screen size
int width = 1920;
int height = 1080;
glm::vec2 viewport = glm::vec2(width, height);

// camera
Camera camera(vec3(0.0f, 12.0f, 30.0f), glm::vec3(0,1,0), -90.0f, -15.0f);
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// config
bool isPaused = false;
bool shouldReload = false;
bool prerender = false;
bool shouldClear = false;
bool shouldAddBlock = false;
float shouldAddBlockCountdown = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadCubemap(int cubemapIndex);

std::vector<std::string> cubemaps = {
	"skybox", "space", "mountain"
};

int main() {
	if (!glfwInit()) return -1;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "gloopsplat", NULL, NULL);
	if (window == NULL) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = nullptr;
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);

	int cubemapIndex = 0;
	unsigned int cubemapTexture = loadCubemap(cubemapIndex);

	Shader shader("./shaders/splat.vs", "./shaders/splat.fs");
	SplatRenderer renderer;

	Scene scene;
	bool sphereEnabled = false;
	glm::vec3 sphereCenter(0.0f, 1.0f, 0.0f);
	float sphereRadius = 1.0f;

	Block block;
	scene.addBlock(block);

	Skybox skybox;

	float specular  = 0.8f;
	float roughness = 0.01f;
	float diffuse = 0.8f;
	vec3 lightDir = glm::normalize(glm::vec3(-1.0f, 1.0f, -1.0f));
	float blur = 0.8f;
	int shaderType = 0;
	float colorStrength = 0.5f;
	bool showSkybox = true;

	//recording
	int recordFps = 60;
	int recordTime = 10;
	char filename[20] = "output";

	while (!glfwWindowShouldClose(window)) {
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		deltaTime = glm::min(deltaTime, 1.0f/recordFps); //cap fps to restrict dt
		lastFrame = currentFrame;
		processInput(window);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(width - 2.0f, 2.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
		ImGui::Begin("Stats", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration);
		ImGui::Text("FPS: %.1f", io.Framerate);
		ImGui::End();

		ImGui::SetNextWindowPos(ImVec2(2.0f, 50.0f), ImGuiCond_Once, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Config", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

		if (ImGui::BeginTabBar("Config")) {
			if (ImGui::BeginTabItem("Scene")) {
				ImGui::SeparatorText("Particle Count");
				ImGui::SliderInt("Nx", &block.nx, 1, 30);
				ImGui::SliderInt("Ny", &block.ny, 1, 30);
				ImGui::SliderInt("Nz", &block.nz, 1, 30);
				ImGui::SliderFloat("Spacing", &block.spacing, 0.05f, 0.5f, "%.3f");
				ImGui::SliderFloat("Radius",  &block.radius,  0.05f, 0.5f, "%.3f");
				ImGui::SeparatorText("Origin");
				ImGui::SliderFloat("x", &block.origin.x, -2.0f, 2.0f, "%.3f");
				ImGui::SliderFloat("y", &block.origin.y, 0.0f, 8.0f, "%.3f");
				ImGui::SliderFloat("z", &block.origin.z, -2.0f, 2.0f, "%.3f");
				ImGui::Button("Add Particles [F]");
				if (ImGui::IsItemActive()) {
					shouldAddBlock = true;
				}
				ImGui::SameLine();
				if (ImGui::Button("Clear [C]")) {
					shouldClear = true;
				}
				ImGui::Text("Particle Count: %d", scene.particles.size());
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Shading")) {
				ImGui::Checkbox("Skybox", &showSkybox);

				const char* curr = cubemaps[cubemapIndex].c_str();
				if (ImGui::BeginCombo("Cubemap", curr)) {
					for (int i = 0; i < cubemaps.size(); i++) {
						if (ImGui::Selectable(cubemaps[i].c_str(), cubemapIndex == i)) {
							cubemapIndex = i;
							glDeleteTextures(1, &cubemapTexture);
							cubemapTexture = loadCubemap(cubemapIndex);
						}
						if (cubemapIndex == i) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGui::Text("Shading Type");
				if (ImGui::RadioButton("RGBA", shaderType == 0)) shaderType = 0;
				ImGui::SameLine();
				if (ImGui::RadioButton("Env Map", shaderType == 1)) shaderType = 1;
				ImGui::SameLine();
				if (ImGui::RadioButton("Fresnel", shaderType == 2)) shaderType = 2;
				ImGui::SeparatorText("RGBA (requires reset)");
				ImGui::ColorPicker3("Color", &block.color[0]);
				ImGui::SliderFloat("Opacity", &block.color[3], 0.01f, 1.0f);
				if (shaderType != 0) {
					ImGui::SliderFloat("Color Strength", &colorStrength, 0.0f, 1.0f);
				}
				ImGui::SeparatorText("Lighting");
				if (ImGui::SliderFloat3("Light Direction", &lightDir[0], -1.0, 1.0f, "%.01f")) {
					if (glm::length(lightDir) > 1e-6f)
						lightDir = glm::normalize(lightDir);
				}
				ImGui::SliderFloat("Specular",  &specular,  0.0f, 2.0f, "%.3f");
				ImGui::SliderFloat("Roughness", &roughness, 0.001f, 1.0f, "%.3f");
				ImGui::SliderFloat("Diffuse", &diffuse, 0.01, 2.0f, "%.3f");
				ImGui::SliderFloat("Blur", &blur, 0.0f, 1.0f, "%.3f");
				ImGui::SliderInt("Surface Threshold", &scene.solver.surfaceThreshold, 1, 100);

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Solver")) {
				bool reloadKernels = false;
				reloadKernels |= ImGui::SliderFloat("h", &scene.solver.h, 0.1f, 1.0f, "%.3f");
				ImGui::SliderFloat("rho0", &scene.solver.rho0, 1.0f, 200.0f, "%.3f");
				ImGui::SliderFloat("Epsilon", &scene.solver.epsilon, 1.0f, 1000.0f, "%.3f");
				ImGui::SliderInt("Iterations", &scene.solver.iterations, 1, 10);
				ImGui::SeparatorText("s_corr");
				ImGui::SliderFloat("k", &scene.solver.scorr_k, 0.0f, 0.01f, "%.5f");
				reloadKernels |= ImGui::SliderFloat("dq", &scene.solver.scorr_dq, 0.01f, 0.5f, "%.3f");
				ImGui::SliderInt("n", &scene.solver.scorr_n, 1, 8);

				ImGui::SeparatorText("XSPH");
				ImGui::SliderFloat("c", &scene.solver.xsph_c, 0.0f, 0.05f, "%.4f");

				ImGui::SeparatorText("Surface Tension");
				ImGui::SliderFloat("Gamma", &scene.solver.gamma_st, 0.0f, 1.0f, "%.4f");

				ImGui::Text("Gravity");
				float g_angle = 0.0f;
				ImGui::SliderFloat("Force", &scene.solver.gravity, 0.0f, 20.0f, "%.3f");
				if (ImGui::SliderFloat("Rotation", &g_angle, 0.0f, 359.0f, "%.1f")) {
					float rad = glm::radians(g_angle);
					scene.solver.g_dir = glm::vec3(0.0f, -cos(rad), sin(rad));
				}

				if (reloadKernels) {
					scene.solver.computeKernels();
					scene.solver.computeBounds();
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Colliders")) {
				bool boundsChanged = false;
				ImGui::Text("Boundaries");
				boundsChanged |= ImGui::SliderFloat("Floor Y", &scene.solver.floor_y, -5.0f, 5.0f, "%.3f");
				boundsChanged |= ImGui::SliderFloat("Wall X", &scene.solver.wall_x, 0.5f, 10.0f, "%.3f");
				boundsChanged |= ImGui::SliderFloat("Wall Z", &scene.solver.wall_z, 0.5f, 10.0f, "%.3f");
				if (boundsChanged) {
					scene.solver.computeBounds();
				}
				ImGui::SeparatorText("Sphere Colliders");
				bool sphereChanged = false;
				sphereChanged |= ImGui::Checkbox("Sphere", &sphereEnabled);
				sphereChanged |= ImGui::SliderFloat("Sphere X", &sphereCenter.x, -5.0f, 5.0f, "%.3f");
				sphereChanged |= ImGui::SliderFloat("Sphere Y", &sphereCenter.y, -5.0f, 5.0f, "%.3f");
				sphereChanged |= ImGui::SliderFloat("Radius", &sphereRadius, 0.5f, 3.0f, "%.3f");
				if (sphereChanged) {
					scene.clearColliders();
					if (sphereEnabled) {
						scene.addSphereCollider(sphereCenter, sphereRadius);
					}
				}
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Record")) {
				ImGui::InputInt("FPS (affects dt)", &recordFps);
				ImGui::InputInt("Time (seconds)", &recordTime);
				ImGui::InputText("Filename", filename, 20);
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
				if (ImGui::Button("Record [P]")) {
					prerender = true;
				}
				ImGui::PopStyleColor(1);
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		if (ImGui::Button("Reload [R]") || shouldReload) {
			scene.particles.clear();
			scene.clearColliders();
			if (sphereEnabled) {
				scene.addSphereCollider(sphereCenter, sphereRadius);
			}
			scene.addBlock(block);
			shouldReload = false;
		}
		ImGui::SameLine();
		ImGui::Checkbox("Paused [space]", &isPaused);
		ImGui::End();

		auto renderFrame = [&](std::vector<Particle>& particles, int w, int h) {
			glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

			mat4 projection = perspective(radians(camera.Zoom), (float)w / h, 0.1f, 100.0f);
			mat4 modelView = camera.GetViewMatrix();
			float tanHalfFovy = tan(radians(camera.Zoom) * 0.5f);
			vec2 focal = vec2((0.5f * w) / tanHalfFovy, (0.5f * h) / tanHalfFovy);

			if (showSkybox) {
				mat4 view = glm::mat4(glm::mat3(modelView));
				skybox.draw(view, projection, cubemapTexture);
			}

			shader.use();
			shader.setInt("cubeMap", 0);
			shader.setVec3("cameraPos", camera.Position);
			shader.setMat4("projection", projection);
			shader.setMat4("modelView", modelView);
			shader.setVec2("focal", focal);
			shader.setVec2("viewport", vec2(w, h));
			shader.setVec3("lightDir", lightDir);
			shader.setFloat("blur", blur);
			shader.setFloat("specular", specular);
			shader.setFloat("roughness", roughness);
			shader.setFloat("diffuse", diffuse);
			shader.setInt("type", shaderType);
			shader.setFloat("colorStrength", colorStrength);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
			renderer.draw(particles, modelView);
		};

		// prerender START
		if(prerender){
			prerender=false;
			std::cout << "Prerendering start!\n";
			if (isPaused) isPaused = false;

			const int OUT_W = 1280;
			const int OUT_H = 720;

			// https://learnopengl.com/Advanced-OpenGL/Framebuffers
			GLuint fbo, colorTex, rbo;
			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			glGenTextures(1, &colorTex);
			glBindTexture(GL_TEXTURE_2D, colorTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, OUT_W, OUT_H, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, OUT_W, OUT_H);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			const int FPS = recordFps;
			const int DURATION = recordTime;
			const int TOTAL_FRAMES = FPS * DURATION;
			const float FRAME_DT = 1.0f / FPS;

			std::string ffmpegCmd =
				"ffmpeg -y -f rawvideo -pixel_format rgb24 "
				"-video_size 1280x720 -framerate " + std::to_string(FPS) +
				" -i pipe:0 -c:v libx264 -pix_fmt yuv420p output/" + std::string(filename) + ".mp4";

			// https://stackoverflow.com/questions/34511312/how-to-encode-a-video-from-several-images-generated-in-a-c-program-without-wri
			FILE* ffmpeg = _popen(ffmpegCmd.c_str(), "wb");
			if (!ffmpeg) {
					std::cout << "Failed to open ffmpeg pipe\n";
					return -1;
			}

			std::vector<std::vector<Particle>> frames(TOTAL_FRAMES);
			std::cout << "Presimulating frames...\n";

			auto simStart = std::chrono::high_resolution_clock::now();

			for (int f = 0; f < TOTAL_FRAMES; f++) {
				scene.update(FRAME_DT);
				frames[f] = scene.particles;
				if (f % 5 == 0) glfwPollEvents();
				if (f % FPS == 0) std::cout << "  frame " << f << "/" << TOTAL_FRAMES << "\n";
			}

			auto simEnd = std::chrono::high_resolution_clock::now();
			float simMs = std::chrono::duration<float>(simEnd - simStart).count();

			std::vector<unsigned char> pixels(OUT_W * OUT_H * 3);

			std::cout << "Simulation completed in " << simMs << "s.\n\nRendering frames...\n";
			glViewport(0, 0, OUT_W, OUT_H);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			auto renderStart = std::chrono::high_resolution_clock::now();

			for (int f = 0; f < TOTAL_FRAMES; f++) {
				renderFrame(frames[f], OUT_W, OUT_H);

				// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glReadPixels.xhtml
				// Flip vertically, OpenGL and ffmpeg expect different origins
				glReadPixels(0, 0, OUT_W, OUT_H, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
				for (int row = 0; row < OUT_H / 2; row++) {
					int opposite = OUT_H - 1 - row;
					std::swap_ranges(
						pixels.begin() + row * OUT_W * 3,
						pixels.begin() + row * OUT_W * 3 + OUT_W * 3,
						pixels.begin() + opposite * OUT_W * 3
					);
				}
				fwrite(pixels.data(), 1, pixels.size(), ffmpeg);
				if (f % 5 == 0) glfwPollEvents();
				if (f % FPS == 0) std::cout << "  rendered " << f << "/" << TOTAL_FRAMES << "\n";
			}
			auto renderEnd = std::chrono::high_resolution_clock::now();
			float renderMs = std::chrono::duration<float>(renderEnd - renderStart).count();

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, width, height);
			_pclose(ffmpeg);
			std::cout << "Done! output.mp4 written.\n";

			std::cout << "Simulation done in " << simMs << "s\n";
			std::cout << "Rendering done in " << renderMs << "s\n";
			std::cout << "Total: " << (simMs + renderMs) << "s\n";
			// cleanup
			glDeleteFramebuffers(1, &fbo);
			glDeleteTextures(1, &colorTex);
			glDeleteRenderbuffers(1, &rbo);
		}
		// prerender END

		if (!isPaused) {
			scene.update(deltaTime);
		}
		renderFrame(scene.particles, width, height);

		if (shouldClear) {
			scene.particles.clear();
			shouldClear = false;
		}

		shouldAddBlockCountdown -= deltaTime;
		if (shouldAddBlock && shouldAddBlockCountdown <= 0.0f) {
			scene.addBlock(block);
			shouldAddBlockCountdown = 0.05f;
		}
		shouldAddBlock = false;

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float _deltaTime = deltaTime;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		_deltaTime *= 2;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, _deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, _deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, _deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, _deltaTime);
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, _deltaTime);
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, _deltaTime);
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
		shouldAddBlock = true;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action != GLFW_PRESS) return;
	if (key == GLFW_KEY_SPACE)
		isPaused = !isPaused;
	if (key == GLFW_KEY_R)
		shouldReload = true;
	if(key == GLFW_KEY_P)
		prerender = true;
	if (key == GLFW_KEY_C)
		shouldClear = true;
}

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height) {
	glViewport(0, 0, _width, _height);
	width = _width;
	height = _height;
	viewport = glm::vec2(_width, _height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse) {
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		camera.ProcessMouseMovement(xoffset, yoffset);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// specific tutorial(s) referenced: https://learnopengl.com/Advanced-OpenGL/Cubemaps
// https://learnopengl.com/code_viewer_gh.php?code=src/4.advanced_opengl/6.1.cubemaps_skybox/cubemaps_skybox.cpp

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front)
// -Z (back)
// -------------------------------------------------------
// unsigned int loadCubemap(std::vector<std::string> faces)
unsigned int loadCubemap(int cubemapIndex)
{
	std::string folder = "textures/" + cubemaps[cubemapIndex];

	std::vector<std::string> faces = {
		(std::filesystem::path(folder + "/right.jpg")).string(),
		(std::filesystem::path(folder + "/left.jpg")).string(),
		(std::filesystem::path(folder + "/top.jpg")).string(),
		(std::filesystem::path(folder + "/bottom.jpg")).string(),
		(std::filesystem::path(folder + "/front.jpg")).string(),
		(std::filesystem::path(folder + "/back.jpg")).string(),
	};

	unsigned int textureID;

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrComponents;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);

		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format,
										width, height, 0, format, GL_UNSIGNED_BYTE, data);

			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;

			stbi_image_free(data);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}