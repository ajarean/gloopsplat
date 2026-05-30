#ifndef GRID_H
#define GRID_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_set>
#include "particle.h"
#include <chrono>
#include <iostream>

// https://matthias-research.github.io/pages/publications/tetraederCollision.pdf
// teschner et al

struct Grid {
	float h; //smoothing radius == cell size
	const int P1 = 73856093; // 
	const int P2 = 19349663;
	const int P3 = 83492791;

	// std::unordered_map<int, std::vector<int>> cells;
	int tableSize;
	int lastN = -1;
	std::vector<int> cellCount; // particles per cell
	std::vector<int> cellStart; // start index of cell in flat
	std::vector<int> flat; // particle indices grouped by cell
	std::vector<int> insertAt;

	Grid(float h){
		this->h = h;
	}
	
	glm::ivec3 cell_coords(glm::vec3 pos) const {
		return glm::ivec3(
			(int)std::floor(pos.x / h),
			(int)std::floor(pos.y / h),
			(int)std::floor(pos.z / h)
		);
	}

	int hash(glm::ivec3 c) const {
		return std::abs((c.x*P1)^(c.y*P2)^(c.z*P3)) % tableSize; 
	}
	
	void build(const std::vector<Particle>& particles) {
		int n = (int)particles.size();
		// tableSize = n; // table size = particle count, good tradeoff (Teschner)
		// tableSize = nextPrime(10*n) ;

		// cellCount.assign(tableSize, 0);
		// cellStart.resize(tableSize);
		// flat.resize(n);

		if (n != lastN) {
			tableSize = 3 * n;
			cellCount.resize(tableSize);
			cellStart.resize(tableSize);
			insertAt.resize(tableSize);
			flat.resize(n);
			lastN = n;
		}

		std::fill(cellCount.begin(), cellCount.end(), 0);

		// pass 1: count particles per cell
		// macklin and muller 2013 algo 1: 
		// find neighbors based on predicted positions
		for (int i = 0; i < n; i++)
			cellCount[hash(cell_coords(particles[i].predicted))]++;

		// prefix sum to get start indices
		cellStart[0] = 0;
		for (int i = 1; i < tableSize; i++)
			cellStart[i] = cellStart[i-1] + cellCount[i-1];

		// pass 2: fill flat array
		insertAt = cellStart; // copy to track insertion point
		for (int i = 0; i < n; i++) {
			int h = hash(cell_coords(particles[i].predicted));
			flat[insertAt[h]++] = i;
		}
	}

	std::vector<int> neighbors(glm::vec3 pos, const std::vector<Particle>& particles) const {
		std::vector<int> result;
		glm::ivec3 center = cell_coords(pos);
		int visitedKeys[27]; // 27 neighbors
		int visitedCount = 0;

		for (int dx = -1; dx <= 1; dx++){
			for (int dy = -1; dy <= 1; dy++){
				for(int dz = -1; dz <= 1; dz++){
					int key = hash(center + glm::ivec3(dx, dy, dz));
					bool seen = false;
					for (int v = 0; v < visitedCount; v++) {
						if (visitedKeys[v] == key)
							seen = true; break;
					}
					if (seen) continue;
					visitedKeys[visitedCount] = key;
					visitedCount++;
					for (int k = cellStart[key]; k < cellStart[key] + cellCount[key]; k++) {
						int idx = flat[k];
						float dist = glm::length(particles[idx].predicted - pos);
						if (dist <= h) result.push_back(idx);
						// check neighboring cells and make sure the particles are actually h distance away
					}
				}
			}
		}
		return result;
	}
};

#endif