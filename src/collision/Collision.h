#ifndef COLLISION_COLLISION_H
#define COLLISION_COLLISION_H

#include <vector>
#include "../physics/SoftBody.h"

class Collision {
public:
    static void ResolveGroundCollision(
        std::vector<Particle>& particles,
        float groundY = 0.0f,
        float restitution = 0.4f,
        float friction = 0.85f
    );
};

#endif // COLLISION_COLLISION_H
