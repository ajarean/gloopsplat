#ifndef SOLVER_H
#define SOLVER_H

#include <vector>
#include "particle.h"
#include "grid.h"
#include "static_grid.h"
#include "kernel.h"
#include "collider.h"

#define USE_STATIC_GRID 1

class Solver {
public:
	float h;
	float gravity;
	glm::vec3 g_dir = glm::vec3(0,-1.0f,0);
	float rho0; // pbf muller et al eq1: C_i = rho_i/rho_0 - 1
	float epsilon;
	int iterations;
	#if USE_STATIC_GRID
		StaticGrid grid;
	#else
		Grid grid;
	#endif
	int surfaceThreshold = 20; // max number of neighbors to count as surface

	float floor_y = 0.0f;
	float wall_x = 2.0f;
	float wall_z = 2.0f;

	float poly6_coeff; // 315/(64pi*h^9)
	float spiky_coeff; // -45/(pi*h^6)

	// eq13/14
	float scorr_k;
	float scorr_dq;
	float scorr_wdq;
	int scorr_n;

	float xsph_c; // XSPH viscosity coefficient, eq. 17, tune in [0,1]

	float gamma_st=0.1f;  // surface tension coefficient, Akinci eq. 1 -- paper uses 1.0 for water
	float st_mass;   // particle mass used in surface tension (eq. 1 uses m_i * m_j)

	std::vector<std::vector<int>> neighborList;

	Solver(float h = 0.4f, float gravity = 9.8f, float rho0 = 60.0f, float epsilon = 100.0f,
		int iterations = 3, float scorr_k = 0.0002f, float scorr_dq = 0.1f, int scorr_n = 4, float xsph_c = 0.005f);

	void update(std::vector<Particle>& particles, std::vector<Collider*>& colliders, float dt);
	void computeKernels();
	void computeBounds();

private:
	void applyForcesAndPredict(std::vector<Particle>& particles, float dt);
	void buildNeighborList(std::vector<Particle> &particles);
	void updateVelocities(std::vector<Particle>& particles, float dt);
	void calculateLambda(std::vector<Particle>& particles);
	void updatePositions(std::vector<Particle>& particles);
	void applyBoundaryConditions(std::vector<Particle>& particles);
	void applyCollisions(std::vector<Particle>& particles, std::vector<Collider*>& colliders);
	void applyViscosity(std::vector<Particle>& particles);
	void computeNormals(std::vector<Particle>& particles);
	void applySurfaceTension(std::vector<Particle>& particles, float dt);
	void smoothNormals(std::vector<Particle>& particles);
	void computeDensities(std::vector<Particle>& particles);
};

#endif
