#ifndef GRID_H
#define GRID_H

#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "particle.h"

// https://matthias-research.github.io/pages/publications/tetraederCollision.pdf
// teschner et al

struct Grid {
    float h; //smoothing radius == cell size
    const int P1 = 73856093; // 
    const int P2 = 19349663;
    const int P3 = 83492791;

    std::unordered_map<int, std::vector<int>> cells;

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
        return (c.x*P1)^(c.y*P2)^(c.z*P3); 
        //no need to mod n because unordered map does that already
    }
    
    void build(const std::vector<Particle>& particles) {
        cells.clear();
        for (int i = 0; i < (int)particles.size(); i++) {
            // macklin and muller 2013 algo 1: 
            // find neighbors based on predicted positions
            int key = hash(cell_coords(particles[i].predicted));
            cells[key].push_back(i);
        }
    }

    std::vector<int> neighbors(glm::vec3 pos, const std::vector<Particle>& particles) const {
        std::vector<int> result;
        glm::ivec3 center = cell_coords(pos);

        for (int dx = -1; dx <= 1; dx++){
            for (int dy = -1; dy <= 1; dy++){
                for(int dz = -1; dz <= 1; dz++){
                    auto i = cells.find(hash(center + glm::ivec3(dx,dy,dz)));
                    if (i == cells.end()) continue;
                    for(int idx : i->second){
                        float dist = glm::length(particles[idx].predicted - pos);
                        // first check all the neighboring cells, then make sure the particles are '
                        // actually h distance away
                        if(dist <= h) result.push_back(idx);
                    }
                }
            }
        }
        return result;
    }

};

#endif