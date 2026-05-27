#ifndef KERNEL_H
#define KERNEL_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>

// since h should stay constant(?), just precompute the coefficient and pass it in
// SPH muller et al eq 20
float poly6(float r, float h, float coefficient){
    if (r>h) return 0.0;
    float h2 = h*h;
    float r2 = r*r;
    float h9 = h2*h2*h2*h2*h; //explicit multiplication is better than using pow i think
    // float coefficient = 315.0f/(64.0f * M_PI * h9)
    return coefficient * ((h2-r2)*(h2-r2)*(h2-r2));
}

// SPH muller et al eq 21:
// 15/(pi*h^6) * (h-r)^3
// so gradient w.r.t. r would be:
// -45/(pi*h^6) * (h-r)^2
glm::vec3 spiky_grad(glm::vec3 r_vec, float h, float coefficient) {
    float r = glm::length(r_vec);
    // if (r > h || r < 1e-6f) return glm::vec3(0.0f);
    // float coefficient = -45.0f / (M_PI * h*h*h*h*h*h); //h^6
    if (r > h) return glm::vec3(0.0f);
    
    // if overlapping, apply a small offset 
    if (r < 1e-6f) {
        r_vec = glm::vec3(1e-5f, 0.0f, 0.0f); 
        r = glm::length(r_vec);
    }
    
    float diff = h - r;
    return coefficient * (diff*diff) * (r_vec / r);
}

#endif