#include "static_grid.h"

glm::ivec3 StaticGrid::cell_coords(glm::vec3 pos) const {
	return glm::ivec3(
		(int)std::floor(pos.x / h),
		(int)std::floor(pos.y / h),
		(int)std::floor(pos.z / h)
	);
}

int StaticGrid::index(glm::ivec3 c) const {
	glm::ivec3 pos = c - min; // convert to grid space
	return (pos.z * dims.x + dims.y) + (pos.y * dims.x) + pos.x;
}

void StaticGrid::computeBounds(glm::vec3 minBounds, glm::vec3 maxBounds) {
	min = cell_coords(minBounds);
	max = cell_coords(maxBounds);
	dims = max - min + glm::ivec3(1);
	tableSize = dims.x * dims.y * dims.z;
	cellCount.resize(tableSize);
	cellStart.resize(tableSize);
	insertAt.reserve(tableSize);
}

void StaticGrid::build(const std::vector<Particle>& particles) {
	int n = (int)particles.size();
	if (n == 0) return;

	flat.resize(n);
	std::fill(cellCount.begin(), cellCount.end(), 0);

	// count particles per cell
	for (int i = 0; i < n; i++)
		cellCount[index(cell_coords(particles[i].predicted))]++;

	// prefix sum to get start indices
	cellStart[0] = 0;
	for (int i = 1; i < tableSize; i++)
		cellStart[i] = cellStart[i-1] + cellCount[i-1];

	// fill flat array
	insertAt = cellStart;
	for (int i = 0; i < n; i++) {
		int idx = index(cell_coords(particles[i].predicted));
		flat[insertAt[idx]++] = i;
	}
}

std::vector<int> StaticGrid::neighbors(glm::vec3 pos, const std::vector<Particle>& particles) const {
	std::vector<int> result;
	if (tableSize == 0) return result;
	glm::ivec3 center = cell_coords(pos);

	for (int dx = -1; dx <= 1; dx++){
		for (int dy = -1; dy <= 1; dy++){
			for(int dz = -1; dz <= 1; dz++){
				glm::ivec3 cell = center + glm::ivec3(dx, dy, dz);

				if (cell.x < min.x || cell.y < min.y || cell.z < min.z ||
					cell.x > max.x || cell.y > max.y || cell.z > max.z)
					continue;

				int key = index(cell);
				// check neighboring cells and make sure the particles are actually h distance away
				for (int k = cellStart[key]; k < cellStart[key] + cellCount[key]; k++) {
					int idx = flat[k];
					float dist = glm::length(particles[idx].predicted - pos);
					if (dist <= h)
						result.push_back(idx);
				}
			}
		}
	}
	return result;
}
