#ifndef SCENE_H
#define SCENE_H

#include "solver.h"

struct Block {
    glm::vec3 origin = glm::vec3(-0.75f, 2.0f, -0.75f);
    int nx = 7;
    int ny = 7;
    int nz = 7;
    float spacing = 0.3f;
    float mass = 1.0f;
    float radius = 0.15f;
    glm::vec4 color = glm::vec4(0.0f, 0.5f, 1.0f, 0.6f);
};

struct Scene {
    std::vector<Particle> particles;
    std::vector<Collider*> colliders;
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
        solver.update(particles, colliders, dt);
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
    void addBlock(Block block) {
        addBlock(block.origin, block.nx, block.ny, block.nz, block.spacing, block.mass, block.radius, block.color);
    }

    void addSphereCollider(glm::vec3 center, float radius) {
        colliders.push_back(new SphereCollider(center, radius));
    }

    ~Scene() {
        for (auto* c : colliders) delete c;
    }
};

#endif