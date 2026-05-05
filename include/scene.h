#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include "particle.h"
#include "solver.h"

struct Scene {
    std::vector<Particle> particles;
    Solver solver;

    Scene(float h = 0.3f, float gravity = 9.8f, float rho0 = 100.0f, float epsilon = 100.0f) 
        : solver(h, gravity, rho0, epsilon) {}

    void addParticle(glm::vec3 position, float mass, float radius, glm::vec4 color) {
        particles.emplace_back(position, mass, radius, color);
    }

    void addParticle(const Particle &p) {
        particles.push_back(p);
    }

    void update(float dt) {
        solver.update(particles, dt);
    }
    void addBlock(glm::vec3 origin, int nx, int ny, int nz, float spacing,
                float mass, float radius, glm::vec4 color) {
        for (int x = 0; x < nx; x++)
        for (int y = 0; y < ny; y++)
        for (int z = 0; z < nz; z++) {
            glm::vec3 pos = origin + glm::vec3(x, y, z) * spacing;
            addParticle(pos, mass, radius, color);
        }
    }
};

#endif