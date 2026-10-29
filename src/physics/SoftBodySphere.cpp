#include "SoftBodySphere.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

SoftBodySphere::SoftBodySphere(float radius, int stacks, int sectors, float initialY)
    : radius(radius), stacks(stacks), sectors(sectors), initialY(initialY) {
    Build();
}

void SoftBodySphere::Build() {
    m_Particles.clear();
    m_Springs.clear();
    m_Indices.clear();

    const float PI = glm::pi<float>();

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2.0f - i * PI / stacks;
        float xz = radius * std::cos(stackAngle);
        float y  = radius * std::sin(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2.0f * PI / sectors;

            float x = xz * std::cos(sectorAngle);
            float z = xz * std::sin(sectorAngle);

            float yOffset = y + initialY;

            Particle p;
            p.pos = { x, yOffset, z };
            m_Particles.push_back(p);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            AddSpring(k1, k1 + 1, k_2, DAMPING_2, 2);
            AddSpring(k1, k2, k_2, 8.0f, 2);

            m_Indices.push_back(k1);
            m_Indices.push_back(k2);
            m_Indices.push_back(k2 + 1);

            m_Indices.push_back(k1);
            m_Indices.push_back(k2 + 1);
            m_Indices.push_back(k1 + 1);

            if (i > 0 && i < stacks - 1) {
                // Diagonal/Shear Cross-Springs -> Category 1
                AddSpring(k1, k2 + 1, k_1, 6.0f, 1);
                AddSpring(k1 + 1, k2, k_1, 6.0f, 1);

                if (j < sectors - 1) {
                    AddSpring(k1, k1 + 2, k_3, 5.0f, 3);
                }

                if (i < stacks - 2) {
                    AddSpring(k1, k1 + (2 * (sectors + 1)), k_3, 7.0f, 3);
                }
            }
        }
    }

    for (int i = 0; i <= stacks; ++i) {
        int rowStart = i * (sectors + 1);
        int first = rowStart;
        int last = rowStart + sectors;
        AddSpring(first, last, k_2, 8.0f, 2);
    }

    {
        int topPole = 0;
        for (int j = 1; j <= sectors; ++j) {
            AddSpring(topPole, j, k_3, 19.0f, 3);
        }
    }

    {
        int bottomPole = stacks * (sectors + 1);
        for (int j = 1; j <= sectors; ++j) {
            AddSpring(bottomPole, bottomPole + j, k_3, 10.0f, 3);
        }
    }

    for (size_t i = 0; i < m_Particles.size() / 2; ++i) {
        size_t oppositeIdx = (i + (m_Particles.size() / 2)) % m_Particles.size();
        AddSpring(static_cast<int>(i), static_cast<int>(oppositeIdx), k_1, DAMPING, 1);
    }

    CalculateNormals();
    InitBuffers();

    restVolume = ComputeVolume();
}

void SoftBodySphere::PostStep() {
    for (int i = 0; i <= stacks; ++i) {
        int firstColumnIdx = i * (sectors + 1);
        int lastColumnIdx  = firstColumnIdx + sectors;

        glm::vec3 sharedPos = (m_Particles[firstColumnIdx].pos + m_Particles[lastColumnIdx].pos) * 0.5f;
        m_Particles[firstColumnIdx].pos = sharedPos;
        m_Particles[lastColumnIdx].pos  = sharedPos;

        glm::vec3 sharedVel = (m_Particles[firstColumnIdx].vel + m_Particles[lastColumnIdx].vel) * 0.5f;
        m_Particles[firstColumnIdx].vel = sharedVel;
        m_Particles[lastColumnIdx].vel  = sharedVel;
    }
}

void SoftBodySphere::Rebuild(float newRadius, int newStacks, int newSectors) {
    this->radius = newRadius;
    this->stacks = newStacks;
    this->sectors = newSectors;

    mFrameCount = 0;
    mDragIdx = -1;

    Build();
}
