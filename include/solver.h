#ifndef SOLVER_H
#define SOLVER_H

#define _USE_MATH_DEFINES

#include <vector>
#include "particle.h"
#include "grid.h"
#include "kernel.h"

struct Solver {
    float h;
    float gravity;
    float rho0; // pbf muller et al eq1: C_i = rho_i/rho_0 - 1
    float epsilon;
    int iterations;
    Grid grid;

    float poly6_coeff; // 315/(64pi*h^9)
    float spiky_coeff; // -45/(pi*h^6)
    
    // eq13/14
    float scorr_k;
    float scorr_dq;
    float scorr_wdq;
    int scorr_n;

    float xsph_c; // XSPH viscosity coefficient, eq. 17, tune in [0,1]

    Solver(float h = 1.0f, float gravity = 9.8f, float rho0 = 100.0f, float epsilon = 100.0f, 
        int iterations = 3, float scorr_k = 0.0002f, float scorr_dq = 0.1f, int scorr_n = 4, float xsph_c = 0.005f)
        : h(h), gravity(gravity), rho0(rho0), epsilon(epsilon), iterations(iterations), 
        scorr_k(scorr_k), scorr_dq(scorr_dq), scorr_n(scorr_n), grid(h), xsph_c(xsph_c) {
        float h6 = h*h*h*h*h*h;
        float h9 = h6*h*h*h;
        poly6_coeff = 315.0f/(64.0f*M_PI*h9);
        spiky_coeff = -45.0f/(M_PI*h6);

        scorr_wdq = poly6(scorr_dq*h, h, poly6_coeff);
    }

    void update(std::vector<Particle>& particles, float dt){
        applyForcesAndPredict(particles,dt);
        grid.build(particles);
        std::vector<std::vector<int>> neighborList(particles.size());
        for(int i = 0; i < (int)particles.size(); i++){
            neighborList[i] = grid.neighbors(particles[i].predicted, particles);
        }
        for(int i=0; i<iterations; i++){
            calculateLambda(particles, neighborList);
            updatePositions(particles, neighborList);
            applyBoundaryConditions(particles);
        }
        computeNormals(particles, neighborList);
        updateVelocities(particles,dt);
        applyViscosity(particles, neighborList);
    }

private:
    //pbf muller et al algo1 lines 1-4
    void applyForcesAndPredict(std::vector<Particle>& particles, float dt){
        for(auto& p : particles) {
            p.velocity += dt * glm::vec3(0.0f, -gravity, 0.0f);
            p.predicted = p.position + dt*p.velocity;
        }
    }

    // pbf muller et al algo1 lines 21-23
    void updateVelocities(std::vector<Particle>& particles, float dt){
        for(auto& p : particles) {
            p.velocity = (p.predicted - p.position) / dt;
            p.position = p.predicted;
        }
    }

    void calculateLambda(std::vector<Particle>& particles, const std::vector<std::vector<int>>& neighborList){
        for(int i = 0; i<particles.size(); i++){
            Particle& p_i = particles[i];
            auto neighbors = neighborList[i];

            float rho_i = 0.0f;
            // eq2
            for(int j : neighbors) {
                float r = glm::length(p_i.predicted - particles[j].predicted);
                rho_i += poly6(r,h,poly6_coeff);
            }

            // eq1
            float C_i = rho_i/rho0 - 1.0f;

            // eq8
            float denominator = 0.0f;
            glm::vec3 grad_i(0.0f); 
            for(int j : neighbors) {
                if (i==j) continue;
                glm::vec3 grad = spiky_grad(p_i.predicted - particles[j].predicted, h, spiky_coeff)/rho0;
                grad_i += grad; //k = i case: if the particle moves, calculate how it changes for every neighbor and accumulate before squaring
                denominator += glm::dot(grad, grad); //k = j case: if the particle's neighbor moves, accumulate it directly
                // grad should be negative in k=j case but doesn't matter because we square
            }
            denominator += glm::dot(grad_i, grad_i);
            p_i.lambda = -C_i / (denominator + epsilon);
        }
    }

    // eq12; algo 1 lines 13, 17
    // delta p_i = 1/rho0 * Sigma_j((lambda_i + lambda_j) * gradW(p_i - p_j, h))
    void updatePositions(std::vector<Particle>& particles, const std::vector<std::vector<int>>& neighborList){
        // algo 1 line 13
        std::vector<glm::vec3> deltas(particles.size(), glm::vec3(0.0f));
        for(int i = 0; i<particles.size(); i++){
            Particle& p_i = particles[i];
            auto neighbors = neighborList[i];

            for(int j : neighbors){
                if (i==j) continue; //skip self

                float dist = glm::length(p_i.predicted - particles[j].predicted);
                float w_ij = poly6(dist,h,poly6_coeff);
                float ratio = w_ij/scorr_wdq;
                float s_corr = -1*scorr_k*pow(ratio,scorr_n);
                glm::vec3 grad = spiky_grad(p_i.predicted - particles[j].predicted, h, spiky_coeff);
                deltas[i] += (p_i.lambda + particles[j].lambda + s_corr) * grad;
            }
            deltas[i] /= rho0;
        }

        // algo 1 line 17
        for(int i = 0; i<particles.size(); i++){
            particles[i].predicted += deltas[i];
        }
    }


    // algo 1 line 14
    void applyBoundaryConditions(std::vector<Particle>& particles) {
        // axis aligned box for now; can replace with something more complex if needed
        const float floor_y  =  0.0f;
        const float wall_x   =  1.5f;
        const float wall_z   =  1.5f;

        for (auto& p : particles) {
            if (p.predicted.y < floor_y + p.radius) {
                p.predicted.y = floor_y + p.radius;
            }
            if (p.predicted.x < -wall_x + p.radius) p.predicted.x = -wall_x + p.radius;
            if (p.predicted.x >  wall_x - p.radius) p.predicted.x =  wall_x - p.radius;
            if (p.predicted.z < -wall_z + p.radius) p.predicted.z = -wall_z + p.radius;
            if (p.predicted.z >  wall_z - p.radius) p.predicted.z =  wall_z - p.radius;
        }
    }
    
    void applyViscosity(std::vector<Particle>& particles, const std::vector<std::vector<int>>& neighborList) {
        std::vector<glm::vec3> deltas(particles.size(), glm::vec3(0.0f));

        for (int i = 0; i < (int)particles.size(); i++) {
            Particle& p_i = particles[i];
            for (int j : neighborList[i]) {
                if (i == j) continue;
                float r = glm::length(p_i.position - particles[j].position);
                glm::vec3 v_ij = particles[j].velocity - p_i.velocity;
                deltas[i] += v_ij * poly6(r, h, poly6_coeff);
            }
            deltas[i] *= xsph_c;
        }

        for (int i = 0; i < (int)particles.size(); i++)
            particles[i].velocity += deltas[i];
    }

    // eq 8
    void computeNormals(std::vector<Particle>& particles, const std::vector<std::vector<int>>& neighborList){
        float epsilon = 1e-6;
        for(int i = 0; i < particles.size(); i++){
            Particle& p_i = particles[i];
            glm::vec3 grad(0.0f);
            for(int j : neighborList[i]){
                if(i==j) continue;
                grad += spiky_grad(p_i.position - particles[j].position, h, spiky_coeff);
            }
            float len = glm::length(grad);
            p_i.normal = len > epsilon ? -grad / len : glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
};

#endif