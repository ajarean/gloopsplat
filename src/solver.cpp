#define _USE_MATH_DEFINES

#include "solver.h"
#include <cmath>

static const bool useStaticGrid = true;

Solver::Solver(float h, float gravity, float rho0, float epsilon,
	int iterations, float scorr_k, float scorr_dq, int scorr_n, float xsph_c)
	: h(h), gravity(gravity), rho0(rho0), epsilon(epsilon), iterations(iterations),
	scorr_k(scorr_k), scorr_dq(scorr_dq), scorr_n(scorr_n), xsph_c(xsph_c), grid(h)
{
	computeKernels();
  computeBounds();
}

void Solver::computeBounds() {
	glm::vec3 padding = glm::vec3(h); // catch particles on edges
  grid.computeBounds(
		glm::vec3(-wall_x, floor_y, -wall_z) - padding, // min bounds
		glm::vec3(wall_x, 10.0f, wall_z) + padding // max bounds
	);
}

void Solver::update(std::vector<Particle>& particles, std::vector<Collider*>& colliders, float dt) {
	if (particles.size() == 0) return;

	applyForcesAndPredict(particles, dt);
  buildNeighborList(particles);
	for (int i = 0; i < iterations; i++) {
		calculateLambda(particles);
		updatePositions(particles);
		applyBoundaryConditions(particles);
		applyCollisions(particles, colliders);
	}
	updateVelocities(particles, dt);
  	detectSurfaces(particles);
	computeNormals(particles);
	applyViscosity(particles);
	applySurfaceTension(particles, dt);
	smoothNormals(particles);
}

void Solver::computeKernels() {
	float h6 = h*h*h*h*h*h;
	float h9 = h6*h*h*h;
	poly6_coeff = 315.0f/(64.0f*M_PI*h9);
	spiky_coeff = -45.0f/(M_PI*h6);
	scorr_wdq = poly6(scorr_dq*h, h, poly6_coeff);
	grid.h = h;
}

//pbf muller et al algo1 lines 1-4
void Solver::applyForcesAndPredict(std::vector<Particle>& particles, float dt) {
	for (auto& p : particles) {
		p.velocity += dt * gravity * g_dir;
		p.predicted = p.position + dt*p.velocity;
	}
}

void Solver::buildNeighborList(std::vector<Particle> &particles) {
  grid.build(particles);
	neighborList.resize(particles.size());

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < particles.size(); i++) {
		neighborList[i] = grid.neighbors(particles[i].predicted, particles);
	}
}

// pbf muller et al algo1 lines 21-23
void Solver::updateVelocities(std::vector<Particle>& particles, float dt) {
	for (auto& p : particles) {
		p.velocity = (p.predicted - p.position) / dt;
		p.position = p.predicted;
	}
}

void Solver::calculateLambda(std::vector<Particle>& particles) {
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < particles.size(); i++) {
		Particle& p_i = particles[i];
		const auto& neighbors = neighborList[i];

		float rho_i = 0.0f;
		// eq2
		for (int j : neighbors) {
			float r = glm::length(p_i.predicted - particles[j].predicted);
			// rho_i += poly6(r, h, poly6_coeff);
			rho_i += particles[j].mass * poly6(r, h, poly6_coeff);
		}
		

		// eq1
		float C_i = rho_i/rho0 - 1.0f;
		// float C_i = std::max(rho_i / rho0 - 1.0f, 0.0f);

		// eq8
		float denominator = 0.0f;
		glm::vec3 grad_i(0.0f);
		for (int j : neighbors) {
			if (i == j) continue;
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
void Solver::updatePositions(std::vector<Particle>& particles) {
	std::vector<glm::vec3> deltas(particles.size(), glm::vec3(0.0f));
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < particles.size(); i++) {
		Particle& p_i = particles[i];
		const auto& neighbors = neighborList[i];

		for (int j : neighbors) {
			if (i == j) continue; //skip self

			float dist = glm::length(p_i.predicted - particles[j].predicted);
			float w_ij = poly6(dist, h, poly6_coeff);
			float ratio = w_ij/scorr_wdq;
			float s_corr = -1*scorr_k*pow(ratio, scorr_n);
			glm::vec3 grad = spiky_grad(p_i.predicted - particles[j].predicted, h, spiky_coeff);
			deltas[i] += (p_i.lambda + particles[j].lambda + s_corr) * grad;
		}
		deltas[i] /= rho0;
	}

	for (int i = 0; i < particles.size(); i++) {
		particles[i].predicted += deltas[i];
	}
}

void Solver::detectSurfaces(std::vector<Particle>& particles) {
  for (int i = 0; i < particles.size(); i++) {
    // interpolate closer to 1 if less neighbors (more surface weight)
    particles[i].surface = glm::mix(particles[i].surface, neighborList[i].size() < surfaceThreshold ? 1.0f : 0.0f, 0.05f);
	}
}

// algo 1 line 14
void Solver::applyBoundaryConditions(std::vector<Particle>& particles) {
	// axis aligned box for now; can replace with something more complex if needed
	for (auto& p : particles) {
    if (p.predicted.y > 10.0f - p.radius) {
      p.predicted.y = 10.0f - p.radius;
    }
		if (p.predicted.y < floor_y + p.radius) {
			p.predicted.y = floor_y + p.radius;
		}
		if (p.predicted.x < -wall_x + p.radius) p.predicted.x = -wall_x + p.radius;
		if (p.predicted.x >  wall_x - p.radius) p.predicted.x =  wall_x - p.radius;
		if (p.predicted.z < -wall_z + p.radius) p.predicted.z = -wall_z + p.radius;
		if (p.predicted.z >  wall_z - p.radius) p.predicted.z =  wall_z - p.radius;
	}
}

void Solver::applyCollisions(std::vector<Particle>& particles, std::vector<Collider*>& colliders) {
	for (auto& p : particles) {
		for (auto& c : colliders) {
			c->resolveCollision(p);
		}
	}
}

void Solver::applyViscosity(std::vector<Particle>& particles) {
	if (xsph_c == 0) return;
	std::vector<glm::vec3> deltas(particles.size(), glm::vec3(0.0f));

	#pragma omp parallel for schedule(dynamic)
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

void Solver::computeNormals(std::vector<Particle>& particles) {
	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < particles.size(); i++) {
		glm::vec3 grad(0.0f);
		for (int j : neighborList[i]) {
			if (i == j) continue;
			grad += (particles[j].mass / particles[j].density) * spiky_grad(particles[i].position - particles[j].position, h, spiky_coeff);
		}
		particles[i].normal = h * grad;
	}
}

// Akinci et al 2013 eq 5: combined surface tension force
// F_st = K_ij * (F_cohesion + F_curvature)
void Solver::applySurfaceTension(std::vector<Particle>& particles, float dt) {
	if (gamma_st == 0) return;
	std::vector<glm::vec3> forces(particles.size(), glm::vec3(0.0f));

	#pragma omp parallel for schedule(dynamic)
	for (int i = 0; i < (int)particles.size(); i++) {
		Particle& p_i = particles[i];

		for (int j : neighborList[i]) {
			if (i == j) continue;
			Particle& p_j = particles[j];

			glm::vec3 x_ij = p_i.position - p_j.position;
			float dist = glm::length(x_ij);
			if (dist < 1e-6f) continue;

			// eq4
			float K_ij = 2.0f * rho0 / (p_i.density + p_j.density);

			// eq1
			float C = cohesion(dist, h);
			glm::vec3 F_cohesion = -gamma_st * p_i.mass * p_j.mass * C * (x_ij / dist);

			// eq3
			glm::vec3 F_curvature = -gamma_st * p_i.mass* (p_i.normal - p_j.normal);

			// eq 5
			forces[i] += K_ij * (F_cohesion + F_curvature);
		}
	}

	for (int i = 0; i < (int)particles.size(); i++)
		particles[i].velocity += (forces[i] / particles[i].mass) * dt;
}

void Solver::smoothNormals(std::vector<Particle>& particles) {
    std::vector<glm::vec3> smoothed_normals(particles.size(), glm::vec3(0.0f));
	
	// smooth densities
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < particles.size(); i++) {
        glm::vec3 smoothed(0.0f);
        for (int j : neighborList[i]) {
            if (i == j) continue;
            float dist = glm::length(particles[i].position - particles[j].position);
            float w_ij = poly6(dist, h, poly6_coeff);
            smoothed += w_ij * particles[j].normal; 
        }
        smoothed_normals[i] = smoothed;
    }

    // normalize 
    #pragma omp parallel for schedule(dynamic)
    for (int i = 0; i < particles.size(); i++) {
        float len = glm::length(smoothed_normals[i]);
        particles[i].normal = len > 1e-6f ? -smoothed_normals[i] / len : glm::vec3(0.0f, 1.0f, 0.0f);
    }
}