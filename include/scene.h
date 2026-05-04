#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "particle.h"
#include "solver.h"

struct Scene {
    std::vector<Particle> particles;
    Solver solver;

    Scene(float h = 1.0f, float rho0 = 1000.0f) : solver(h, rho0) {}

    void addParticle(glm::vec3 position, float mass, float radius, glm::vec3 color, float opacity) {
        particles.emplace_back(position, mass, radius, color, opacity);
    }

    void update(float dt) {
        solver.update(particles, dt);
    }
};

#endif