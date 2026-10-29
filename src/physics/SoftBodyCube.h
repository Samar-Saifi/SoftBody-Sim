#ifndef PHYSICS_SOFTBODYCUBE_H
#define PHYSICS_SOFTBODYCUBE_H

#include "SoftBody.h"

class SoftBodyCube : public SoftBody {
public:
    SoftBodyCube(float size = 8.0f, int resolution = 4, float initialY = 15.0f);
    ~SoftBodyCube() override = default;

    void Build() override;
    void Rebuild(float newSize, int newResolution);

    float GetSize() const { return m_Size; }
    int GetResolution() const { return m_Resolution; }

private:
    float m_Size;
    int m_Resolution;
    float m_InitialY;
};

#endif // PHYSICS_SOFTBODYCUBE_H
