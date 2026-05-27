#ifndef PARTICLE_H
#define PARTICLE_H
#include <glm/glm.hpp>
#include <vector>

struct Particle {
  // PBD
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 predicted;
  float mass; // assume constant per fluid particle

  // PBF
  // stored here bc neighbors need to read λ_j
  // area + distance constraints can be calced in solver
  float lambda; // λ_i, density constraint (see eq.7 + PBF paper)

  // Gaussian
  // glm::vec3 scale;
  // glm::quat rotation;
  // The above are commented out because we are only implementing fluids currently
  // Paper assumes fluids are spherical and equal in scale
  // Thus use radius directly, replace this w the above if we want to use solids
  float radius;
  glm::vec4 color;
  glm::vec3 normal;
  float surface = 0.0f; // weighted surface-ness of a particle

  Particle(glm::vec3 position, float mass, float radius, glm::vec4 color)
    : position(position), velocity(0.0f), predicted(0.0f), mass(mass),
      lambda(0.0f), radius(radius), color(color), normal(0.0f, 1.0f, 0.0f) {}
};

struct SplatData {
  glm::vec3 position;
  float radius;
  glm::vec4 color;
  glm::vec3 normal;
  float surface;
};

#endif