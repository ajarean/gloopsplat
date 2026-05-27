#ifndef SPLATRENDERER_H
#define SPLATRENDERER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "particle.h"
#include "shader.h"

class SplatRenderer {
public:
  SplatRenderer();
  ~SplatRenderer();
  void draw(Shader &shader, std::vector<Particle>& particles, glm::mat4 &projection,
    glm::mat4 &modelView, glm::vec2 &focal, glm::vec2 &viewport);
private:
  unsigned int VAO, VBO, EBO, splatVBO;
  std::vector<unsigned int> indices;
  std::vector<SplatData> splatBuffer;
  void depthSort(const std::vector<Particle>& particles, const glm::mat4& modelView);
  void updateBuffers(const std::vector<Particle>& particles);
};

#endif