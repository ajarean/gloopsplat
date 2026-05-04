#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include "particle.h"
#include "grid.h"

struct Solver {
    float h;
    float gravity;
    float rho0; // pbf muller et al eq1: C_i = rho_i/rho_0 - 1
    int iterations;
    Grid grid;
    
    Solver(float h = 1.0f, float gravity = 9.8f, float rho0 = 1000.0f, int iterations = 3)
        : h(h), gravity(gravity), rho0(rho0), iterations(iterations), grid(h) {}
    
    void update(std::vector<Particle>& particles, float dt){
        applyForcesAndPredict(particles,dt);
        // todo: pbf solver
        // grid.build will be called in here
        updateVelocities(particles,dt);
    }

private:
    //pbf muller et al algo1 lines 1-4
    void applyForcesAndPredict(std::vector<Particle>& particles, float dt){
        for(auto& p : particles) {
            p.velocity += dt + glm::vec3(0.0f, -gravity, 0.0f);
            p.predicted = p.position + dt*p.velocity;
        }
    }

    void updateVelocities(std::vector<Particle>& particles, float dt){
        // lines 21-23
        for(auto& p : particles) {
            p.velocity = (p.predicted - p.position) / dt;
            p.position = p.predicted;
        }
    }
};

#endif