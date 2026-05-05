#include "splatRenderer.h"
#include <algorithm>

SplatRenderer::SplatRenderer() {
  float vertices[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f,
  };
  unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glBindVertexArray(0);
}

void SplatRenderer::draw(Shader &shader, std::vector<Particle> particles, glm::mat4 &projection,
    glm::mat4 &modelView, glm::vec2 &focal, glm::vec2 &viewport) {
  shader.use();
  // uniform per gaussian
  shader.setMat4("projection", projection);
  shader.setMat4("modelView", modelView);
  shader.setVec2("focal", focal);
  shader.setVec2("viewport", viewport);
  shader.setVec3("lightDir", glm::normalize(glm::vec3(1.0f, 1.0f, 1.0f)));

  glBindVertexArray(VAO);
  depthSort(particles, modelView);
  for (int i : indices) {
    Particle p = particles[i];
    shader.setVec3("position", p.position);
    shader.setVec4("color", p.color);
    shader.setVec3("normal", p.normal);
    shader.setFloat("radius", p.radius);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);
}

void SplatRenderer::depthSort(const std::vector<Particle>& particles, const glm::mat4& modelView) {
  indices.resize(particles.size());
  for (int i = 0; i < particles.size(); i++) {
    indices[i] = i;
    // float z = (modelView * glm::vec4(particles[i].position, 1.0f)).z;
    // std::cout << "particle " << i << " z: " << z << std::endl;
  }

  std::sort(indices.begin(), indices.end(), [&](int a, int b) {
    float z_a = (modelView*glm::vec4(particles[a].position, 1.0f)).z;
    float z_b = (modelView*glm::vec4(particles[b].position, 1.0f)).z;
    return z_a < z_b; // -z is more in front in opengl
  });
}

SplatRenderer::~SplatRenderer() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}