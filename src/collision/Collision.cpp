#include "Collision.h"

void Collision::ResolveGroundCollision(
    std::vector<Particle>& particles,
    float groundY,
    float restitution,
    float friction
) {
    for (auto& p : particles) {
        if (p.pos.y < groundY) {
            p.pos.y = groundY;
            if (p.vel.y < 0.0f) {
                p.vel.y *= -restitution;
                p.vel.x *= friction;
                p.vel.z *= friction;
            }
        }
    }
}
