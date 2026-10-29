#include "SoftBodyCube.h"
#include <iostream>

SoftBodyCube::SoftBodyCube(float size, int resolution, float initialY)
    : m_Size(size), m_Resolution(resolution), m_InitialY(initialY) {
    if (m_Resolution < 2) m_Resolution = 2;
    Build();
}

void SoftBodyCube::Build() {
    m_Particles.clear();
    m_Springs.clear();
    m_Indices.clear();

    int res = m_Resolution;
    float step = m_Size / static_cast<float>(res - 1);

    auto pIndex = [res](int x, int y, int z) {
        return x + y * res + z * res * res;
    };

    for (int z = 0; z < res; ++z) {
        for (int y = 0; y < res; ++y) {
            for (int x = 0; x < res; ++x) {
                Particle p;
                p.pos = glm::vec3(
                    -m_Size * 0.5f + x * step,
                    m_InitialY + y * step,
                    -m_Size * 0.5f + z * step
                );
                m_Particles.push_back(p);
            }
        }
    }

    for (int z = 0; z < res; ++z) {
        for (int y = 0; y < res; ++y) {
            for (int x = 0; x < res; ++x) {
                int curr = pIndex(x, y, z);

                if (x + 1 < res) AddSpring(curr, pIndex(x + 1, y, z), k_2, DAMPING_2, 2);
                if (y + 1 < res) AddSpring(curr, pIndex(x, y + 1, z), k_2, DAMPING_2, 2);
                if (z + 1 < res) AddSpring(curr, pIndex(x, y, z + 1), k_2, DAMPING_2, 2);

                if (x + 1 < res && y + 1 < res) {
                    AddSpring(curr, pIndex(x + 1, y + 1, z), k_1, 6.0f, 1);
                    AddSpring(pIndex(x + 1, y, z), pIndex(x, y + 1, z), k_1, 6.0f, 1);
                }
                if (x + 1 < res && z + 1 < res) {
                    AddSpring(curr, pIndex(x + 1, y, z + 1), k_1, 6.0f, 1);
                    AddSpring(pIndex(x + 1, y, z), pIndex(x, y, z + 1), k_1, 6.0f, 1);
                }
                if (y + 1 < res && z + 1 < res) {
                    AddSpring(curr, pIndex(x, y + 1, z + 1), k_1, 6.0f, 1);
                    AddSpring(pIndex(x, y + 1, z), pIndex(x, y, z + 1), k_1, 6.0f, 1);
                }

                if (x + 1 < res && y + 1 < res && z + 1 < res) {
                    AddSpring(curr, pIndex(x + 1, y + 1, z + 1), k_1, DAMPING, 1);
                    AddSpring(pIndex(x + 1, y, z), pIndex(x, y + 1, z + 1), k_1, DAMPING, 1);
                    AddSpring(pIndex(x, y + 1, z), pIndex(x + 1, y, z + 1), k_1, DAMPING, 1);
                    AddSpring(pIndex(x, y, z + 1), pIndex(x + 1, y + 1, z), k_1, DAMPING, 1);
                }

                if (x + 2 < res) AddSpring(curr, pIndex(x + 2, y, z), k_3, 5.0f, 3);
                if (y + 2 < res) AddSpring(curr, pIndex(x, y + 2, z), k_3, 5.0f, 3);
                if (z + 2 < res) AddSpring(curr, pIndex(x, y, z + 2), k_3, 5.0f, 3);
            }
        }
    }

    auto addQuad = [this](int i0, int i1, int i2, int i3) {
        m_Indices.push_back(i0);
        m_Indices.push_back(i1);
        m_Indices.push_back(i2);

        m_Indices.push_back(i0);
        m_Indices.push_back(i2);
        m_Indices.push_back(i3);
    };

    for (int y = 0; y < res - 1; ++y) {
        for (int x = 0; x < res - 1; ++x) {
            int z = res - 1;
            addQuad(pIndex(x, y, z), pIndex(x + 1, y, z), pIndex(x + 1, y + 1, z), pIndex(x, y + 1, z));
        }
    }

    for (int y = 0; y < res - 1; ++y) {
        for (int x = 0; x < res - 1; ++x) {
            int z = 0;
            addQuad(pIndex(x + 1, y, z), pIndex(x, y, z), pIndex(x, y + 1, z), pIndex(x + 1, y + 1, z));
        }
    }

    for (int z = 0; z < res - 1; ++z) {
        for (int x = 0; x < res - 1; ++x) {
            int y = res - 1;
            addQuad(pIndex(x, y, z), pIndex(x, y, z + 1), pIndex(x + 1, y, z + 1), pIndex(x + 1, y, z));
        }
    }

    for (int z = 0; z < res - 1; ++z) {
        for (int x = 0; x < res - 1; ++x) {
            int y = 0;
            addQuad(pIndex(x, y, z), pIndex(x + 1, y, z), pIndex(x + 1, y, z + 1), pIndex(x, y, z + 1));
        }
    }

    for (int z = 0; z < res - 1; ++z) {
        for (int y = 0; y < res - 1; ++y) {
            int x = res - 1;
            addQuad(pIndex(x, y, z), pIndex(x, y + 1, z), pIndex(x, y + 1, z + 1), pIndex(x, y, z + 1));
        }
    }

    for (int z = 0; z < res - 1; ++z) {
        for (int y = 0; y < res - 1; ++y) {
            int x = 0;
            addQuad(pIndex(x, y, z), pIndex(x, y, z + 1), pIndex(x, y + 1, z + 1), pIndex(x, y + 1, z));
        }
    }

    CalculateNormals();
    InitBuffers();

    restVolume = ComputeVolume();
}

void SoftBodyCube::Rebuild(float newSize, int newResolution) {
    m_Size = newSize;
    m_Resolution = newResolution < 2 ? 2 : newResolution;

    mFrameCount = 0;
    mDragIdx = -1;

    Build();
}
