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

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "camera.h"
#include "splatRenderer.h"
#include "shader.h"
#include "particle.h"
#include "scene.h"

using namespace glm;

// screen size
int width = 1920;
int height = 1080;
glm::vec2 viewport = glm::vec2(width, height);

// camera
Camera camera(vec3(0.0f, 1.0f, 5.0f));
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// config
bool isPaused = false;
bool shouldReload = false;

void framebuffer_size_callback(GLFWwindow* window, int _width, int _height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

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

  Shader shader("./shaders/splat.vs", "./shaders/splat.fs");
  SplatRenderer renderer;

  Scene scene(0.4f, 9.8f, 68.0f, 100.0f);
  scene.addSphereCollider(glm::vec3(0.0f, 2.0f, 0.0f), 1.0f);
  Block block;
  scene.addBlock(block);

  float specular  = 0.5f;
  float roughness = 0.1f;
  float blur = 0.8f;

  while (!glfwWindowShouldClose(window)) {
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    deltaTime = glm::min(deltaTime, 0.016f); //cap to 60fps
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
        ImGui::SliderInt("Nx", &block.nx, 1, 20);
        ImGui::SliderInt("Ny", &block.ny, 1, 20);
        ImGui::SliderInt("Nz", &block.nz, 1, 20);
        ImGui::SliderFloat("Spacing", &block.spacing, 0.05f, 0.5f, "%.3f");
        ImGui::SliderFloat("Radius",  &block.radius,  0.05f, 0.5f, "%.3f");
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Shading")) {
        ImGui::ColorPicker3("Color", &block.color[0]);
        ImGui::SliderFloat("Opacity", &block.color[3], 0.01f, 1.0f);
        ImGui::SliderFloat("Specular",  &specular,  0.0f, 1.0f, "%.3f");
        ImGui::SliderFloat("Roughness", &roughness, 0.01f, 1.0f, "%.3f");
        ImGui::SliderFloat("Blur", &blur, 0.0f, 1.0f, "%.3f");
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Solver")) {
        bool reloadKernels = false;
        reloadKernels |= ImGui::SliderFloat("h", &scene.solver.h, 0.1f, 1.0f, "%.3f");
        ImGui::SliderFloat("Gravity", &scene.solver.gravity, 0.0f, 20.0f, "%.3f");
        ImGui::SliderFloat("rho0", &scene.solver.rho0, 1.0f, 200.0f, "%.3f");
        ImGui::SliderFloat("Epsilon", &scene.solver.epsilon, 1.0f, 1000.0f, "%.3f");
        ImGui::SliderInt("Iterations", &scene.solver.iterations, 1, 10);

        ImGui::Text("TODO:s_corr");
        // TODO: andy add stuff for this + xsph stuff cuz idk what the params do


        ImGui::SliderInt("Surface threshold", &scene.solver.surfaceThreshold, 1, 100);

        if (reloadKernels) { scene.solver.computeKernels(); }
        ImGui::EndTabItem();
      }

      if (ImGui::BeginTabItem("Colliders")) {
        ImGui::SliderFloat("Floor Y", &scene.solver.floor_y, -2.0f, 2.0f, "%.3f");
        ImGui::SliderFloat("Wall X", &scene.solver.wall_x, 0.5f, 5.0f, "%.3f");
        ImGui::SliderFloat("Wall Z", &scene.solver.wall_z, 0.5f, 5.0f, "%.3f");
        ImGui::EndTabItem();
      }

      ImGui::EndTabBar();
    }

    if (ImGui::Button("Reload [R]") || shouldReload) {
      scene.particles.clear();
      scene.clearColliders();
      scene.addSphereCollider(glm::vec3(0.0f, 2.0f, 0.0f), 1.0f);
      scene.addBlock(block);
      shouldReload = false;
    }
    ImGui::SameLine();
    ImGui::Checkbox("Paused [space]", &isPaused);

    ImGui::End();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if(!isPaused){
      scene.update(deltaTime);
    }

    mat4 projection = perspective(radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
    mat4 modelView = camera.GetViewMatrix(); // model is identity
    float tanHalfFovy = tan(radians(camera.Zoom) * 0.5f);
    vec2 focal = vec2((0.5f * (float)width) / tanHalfFovy, (0.5f * (float)height) / tanHalfFovy);

    shader.use();
    shader.setVec3("lightDir", glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)));
    shader.setFloat("blur", blur);
    shader.setFloat("specular",  specular);
    shader.setFloat("roughness", roughness);
    renderer.draw(shader, scene.particles, projection, modelView, focal, viewport);

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
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  if (action != GLFW_PRESS) return;
  if (key == GLFW_KEY_SPACE)
    isPaused = !isPaused;
  if (key == GLFW_KEY_R)
    shouldReload = true;
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
