#include "splatRenderer.h"

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

  glBindVertexArray(VAO);
  for (auto& p : particles) {
    shader.setVec3("position", p.position);
    shader.setVec4("color", p.color);
    shader.setFloat("radius", p.radius);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);
}

SplatRenderer::~SplatRenderer() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}