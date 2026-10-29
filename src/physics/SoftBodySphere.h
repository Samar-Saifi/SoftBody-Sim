#ifndef PHYSICS_SOFTBODYSPHERE_H
#define PHYSICS_SOFTBODYSPHERE_H

#include "SoftBody.h"

class SoftBodySphere : public SoftBody {
public:
    SoftBodySphere(float radius = 5.0f, int stacks = 15, int sectors = 15, float initialY = 15.0f);
    ~SoftBodySphere() override = default;

    void Build() override;
    void PostStep() override;

    void Rebuild(float newRadius, int newStacks, int newSectors);

    float GetRadius() const { return radius; }
    int GetStacks() const { return stacks; }
    int GetSectors() const { return sectors; }

private:
    float radius;
    int stacks;
    int sectors;
    float initialY;
};

#endif // PHYSICS_SOFTBODYSPHERE_H
