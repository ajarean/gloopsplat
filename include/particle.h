#include <glm/glm.hpp>
#include <vector>

struct Particle {
  // PBD
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 predicted;
  float mass; // should be constant per particle for our use case

  // PBF
  float lambda;
  float density; // ρ

  // Gaussian
  // glm::vec3 scale;
  // glm::quat rotation;
  // The above are commented out because we are only implementing fluids currently
  // Paper assumes fluids are spherical and equal in scale
  // Thus use radius directly, replace this w the above if we want to use solids
  float radius;
  glm::vec3 color;
  float opacity;

  glm::mat3 covariance() const {
    float r2 = radius * radius;
    return glm::mat3(r2); // r2 * I
  }
};