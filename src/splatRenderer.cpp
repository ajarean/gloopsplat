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
  glGenBuffers(1, &splatVBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, splatVBO);
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

  // position (vec3)
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SplatData), (void*)offsetof(SplatData, position));
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1);

  // radius (float)
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SplatData), (void*)offsetof(SplatData, radius));
  glEnableVertexAttribArray(2);
  glVertexAttribDivisor(2, 1);

  // color (vec4)
  glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(SplatData), (void*)offsetof(SplatData, color));
  glEnableVertexAttribArray(3);
  glVertexAttribDivisor(3, 1);

  // normal (vec3)
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(SplatData), (void*)offsetof(SplatData, normal));
  glEnableVertexAttribArray(4);
  glVertexAttribDivisor(4, 1);

  // speed (float)
  glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(SplatData), (void*)offsetof(SplatData, speed));
  glEnableVertexAttribArray(5);
  glVertexAttribDivisor(5, 1);
  glBindVertexArray(0);
}

void SplatRenderer::draw(Shader &shader, std::vector<Particle> particles, glm::mat4 &projection,
    glm::mat4 &modelView, glm::vec2 &focal, glm::vec2 &viewport) {
  depthSort(particles, modelView);
  updateBuffers(particles);

  shader.setMat4("projection", projection);
  shader.setMat4("modelView", modelView);
  shader.setVec2("focal", focal);
  shader.setVec2("viewport", viewport);

  glBindVertexArray(VAO);
  glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, particles.size());
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

void SplatRenderer::updateBuffers(const std::vector<Particle> particles) {
  splatBuffer.resize(particles.size());
  for (int i = 0; i < splatBuffer.size(); i++) {
    const Particle& p = particles[indices[i]];
    splatBuffer[i].position = p.position;
    splatBuffer[i].radius = p.radius;
    splatBuffer[i].color = p.color;
    splatBuffer[i].normal = p.normal;
    splatBuffer[i].speed = glm::length(p.velocity);
  }
  glBindBuffer(GL_ARRAY_BUFFER, splatVBO);
  glBufferData(GL_ARRAY_BUFFER, splatBuffer.size() * sizeof(SplatData), splatBuffer.data(), GL_DYNAMIC_DRAW);
}

SplatRenderer::~SplatRenderer() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}