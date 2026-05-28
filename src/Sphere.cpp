#include "Sphere.h"
#include <cmath>
#include <iostream>

float DAMPING = 40;
float DAMPING_2 = 80;
float DAMPING_3 = 50;

float k_1 = 500;
float k_2 = 800;
float k_3 = 300;

Sphere::Sphere(unsigned int prog, float radius, int stacks, int sectors)
    : radius(radius), stacks(stacks), sectors(sectors), progID(prog)
{
    build();
}

void Sphere::AddSpring(int a, int b, float k, float damping)
{
    if (a == b) return;
    for (auto& spring : mSprings)
    {if ((spring.a == a && spring.b == b) || (spring.a == b && spring.b == a)) return;}
    float distance = glm::length((mParticles[a].pos - mParticles[b].pos));
    mSprings.push_back({ a, b, distance, k, damping });
}

Sphere::~Sphere()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Sphere::build()
{
    vertices.clear();
    indices.clear();
    mParticles.clear();
    mSprings.clear();

    const float PI = 3.14159265359f;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * PI / stacks;
        float xz = radius * cosf(stackAngle);
        float y  = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * PI / sectors;

            float x = xz * cosf(sectorAngle);
            float z = xz * sinf(sectorAngle);

            float yOffset = y + 15.0f;

            Particle p;
            p.pos = { x, yOffset, z };
            mParticles.push_back(p);
            vertices.push_back(x);
            vertices.push_back(yOffset);
            vertices.push_back(z);
        }
    }

    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {

            AddSpring(k1, k1+1, k_2, DAMPING_2);
            AddSpring(k1, k2, k_2, 8);

            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);

            indices.push_back(k1);

            indices.push_back(k2 + 1);
            indices.push_back(k1 + 1);

            if (i > 0 && i < stacks - 1)
            {
                AddSpring(k1, k2 + 1, k_1, 6);
                AddSpring(k1 + 1, k2, k_1, 6);

                if (j < sectors - 1)
                {
                    AddSpring(k1, k1 + 2, k_3, 5);
                }

                if (i < stacks - 2)
                {
                    AddSpring(k1, k1 + (2 * (sectors + 1)), k_3,7);
                }
            }
        }
    }

    for (int i = 0; i<= stacks; ++i) {
        int rowStart = i* (sectors + 1);
        int first = rowStart;
        int last = rowStart + sectors;

        AddSpring(first, last, k_2, 8);
    }

    {
        int topPole = 0;
        for (int j = 1; j <= sectors; ++j){
            AddSpring(topPole, j, k_3, 19);
        }
    }

    {
        int bottomPole = stacks * (sectors + 1);
        for (int j = 1; j <= sectors; ++j){
            AddSpring(bottomPole, bottomPole + j, k_3, 10);
        }
    }

    for (size_t i = 0; i < mParticles.size() / 2; ++i) {
        size_t oppositeIdx = (i + (mParticles.size() / 2)) % mParticles.size();
        AddSpring(i, oppositeIdx, k_1, 1.5);
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),vertices.data(),GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(),GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    restVolume = computeVolume();
}

void Sphere::draw(GLuint colorUniformLoc) const
{
    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUniform4f(colorUniformLoc, 1, 0.5, 0.5, 1);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()),GL_UNSIGNED_INT,0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);
}

float Sphere::computeVolume() const
{
    float vol = 0.0f;
    for (size_t i = 0; i < indices.size(); i += 3)
    {
        const glm::vec3& a = mParticles[indices[i    ]].pos;
        const glm::vec3& b = mParticles[indices[i + 1]].pos;
        const glm::vec3& c = mParticles[indices[i + 2]].pos;
        vol += glm::dot(a, glm::cross(b, c));
    }
    return std::abs(vol) / 6.0f;
}

void Sphere::applyPressureForces(float subDt)
{
    float ramp = std::min(mFrameCount / 60.0f, 1.0f);
    float currentVol = computeVolume();
    float pressure = pressureK * ramp * (restVolume - currentVol) / restVolume;

    for (size_t i = 0; i < indices.size(); i += 3)
    {
        int ia = indices[i], ib = indices[i + 1], ic = indices[i + 2];

        glm::vec3& pa = mParticles[ia].pos;
        glm::vec3& pb = mParticles[ib].pos;
        glm::vec3& pc = mParticles[ic].pos;

        glm::vec3 force = pressure * glm::cross(pc - pa, pb - pa) / 3.0f;

        mParticles[ia].vel += (force / mParticles[ia].mass) * subDt;
        mParticles[ib].vel += (force / mParticles[ib].mass) * subDt;
        mParticles[ic].vel += (force / mParticles[ic].mass) * subDt;
    }
}

void Sphere::UpdateParticle(float dt)
{
    const int substeps = 20;
    float subDt = dt / substeps;
    ++mFrameCount;

    for (int step = 0; step < substeps; ++step)
    {
        applyPressureForces(subDt);

        if (mDragIdx >= 0)
        {
            Particle& dp = mParticles[mDragIdx];
            glm::vec3 pull = mDragK * (mDragTarget - dp.pos);
            dp.vel += (pull / dp.mass) * subDt;
            dp.vel *= 0.80f;
        }

        for (auto& sp : mSprings)
        {
            glm::vec3 p1 = mParticles[sp.a].pos;
            glm::vec3 p2 = mParticles[sp.b].pos;

            float distance = glm::length(p1 - p2);
            if (distance < 1e-4f) continue;

            glm::vec3 dir = (p1 - p2) / distance;

            float springMag  = sp.k * (distance - sp.len);
            glm::vec3 relVel = mParticles[sp.a].vel - mParticles[sp.b].vel;
            float dampMag    = sp.damping * glm::dot(relVel, dir);

            glm::vec3 force  = dir * (springMag + dampMag);

            mParticles[sp.a].vel -= (force / mParticles[sp.a].mass) * subDt;
            mParticles[sp.b].vel += (force / mParticles[sp.b].mass) * subDt;
        }

        for (auto& p : mParticles)
        {
            p.vel += glm::vec3(0.0f, -9.8f, 0.0f) * subDt;
            p.vel *= (1.0f - 0.15f * subDt);
            p.pos += p.vel * subDt;

            if (p.pos.y < 0.0f) {
                p.pos.y = 0.0f;
                if (p.vel.y < 0.0f) {
                    p.vel.y *= -0.4f;
                    p.vel.x *= 0.85f;
                    p.vel.z *= 0.85f;
                }
            }
        }
    }

    for (int i = 0; i <= stacks; ++i) {
        int firstColumnIdx = i * (sectors + 1);          // where j = 0
        int lastColumnIdx  = firstColumnIdx + sectors;   // where j = sectors

        glm::vec3 sharedPos = (mParticles[firstColumnIdx].pos + mParticles[lastColumnIdx].pos) * 0.5f;

        mParticles[firstColumnIdx].pos = sharedPos;
        mParticles[lastColumnIdx].pos  = sharedPos;
        glm::vec3 sharedVel = (mParticles[firstColumnIdx].vel + mParticles[lastColumnIdx].vel) * 0.5f;
        mParticles[firstColumnIdx].vel = sharedVel;
        mParticles[lastColumnIdx].vel  = sharedVel;
    }

    vertices.clear();
    for (auto& p : mParticles) {
        vertices.push_back(p.pos.x);
        vertices.push_back(p.pos.y);
        vertices.push_back(p.pos.z);
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
}

int Sphere::PickParticle(glm::vec3 origin, glm::vec3 dir, float threshold)
{
    int   best = -1;
    float bestDist = threshold;

    for (int i = 0; i < (int)mParticles.size(); ++i){

        glm::vec3 toP = mParticles[i].pos - origin;
        float     t   = glm::dot(toP, dir);
        if (t < 0.0f) continue;

        float dist = glm::length(mParticles[i].pos - (origin + dir * t));
        if (dist < bestDist) { bestDist = dist; best = i; }
    }
    std::cout << "Picked: " << best << " minDist: " << bestDist << "\n";
    return best;
}

void Sphere::BeginDrag(int idx, glm::vec3 target)
{
    mDragIdx    = idx;
    mDragTarget = target;
}

void Sphere::MoveDrag(glm::vec3 target) { mDragTarget = target; }

void Sphere::EndDrag() { mDragIdx = -1; }