#ifndef COLLIDER_H
#define COLLIDER_H

#include <vector>
#include "particle.h"

struct Collider {
	virtual void resolveCollision(Particle &p) const = 0;
};

struct SphereCollider : Collider {
	glm::vec3 center;
	float radius;
	SphereCollider(glm::vec3 center, float radius) : center(center), radius(radius) {}
	void resolveCollision(Particle &p) const override {
		glm::vec3 r = p.predicted - center;
		glm::vec3 n = glm::normalize(r);
		float d = glm::length(r);
		if (d < radius + p.radius) {
			p.predicted = center + n * (radius + p.radius);
		}
	}
};

#endif