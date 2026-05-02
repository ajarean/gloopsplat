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
  glm::vec3 color;
  float opacity;

  glm::mat3 covariance() const {
    float r2 = radius * radius;
    return glm::mat3(r2); // r2 * I
  }
};