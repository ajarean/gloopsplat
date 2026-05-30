#ifndef STATIC_GRID_H
#define STATIC_GRID_H

#include <glm/glm.hpp>
#include <vector>
#include "particle.h"
#include <algorithm>
#include <cmath>

// Static grid approach to avoid reallocating hash table
struct StaticGrid {
	float h; //smoothing radius == cell size

	int tableSize;
	std::vector<int> cellCount; // particles per cell
	std::vector<int> cellStart; // start index of cell in flat
	std::vector<int> flat; // particle indices grouped by cell
	std::vector<int> insertAt;

	glm::ivec3 min = glm::ivec3(0);
	glm::ivec3 max = glm::ivec3(0);
  glm::ivec3 dims = glm::ivec3(0);

	StaticGrid(float h) : h(h) {};

	glm::ivec3 cell_coords(glm::vec3 pos) const;
	int index(glm::ivec3 c) const;
	void computeBounds(glm::vec3 minBounds, glm::vec3 maxBounds);
	void build(const std::vector<Particle>& particles);
	std::vector<int> neighbors(glm::vec3 pos, const std::vector<Particle>& particles) const;
};

#endif