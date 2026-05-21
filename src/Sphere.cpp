#include "Sphere.h"
#include <cmath>

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
    if (distance < 1e-9f) return;
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

            AddSpring(k1, k1+1, 200, 2);
            AddSpring(k1, k2, 200, 2);

            AddSpring(k1, k2+1, 120, 1);
            AddSpring(k1+1, k2, 120, 1);

            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (j<sectors - i) {
                AddSpring(k1, k1+2, 80, 0.6);
                if (i+2 <= stacks) {
                    AddSpring(k1, k1 + (2 * (sectors + 1)), 200, 2);
                }
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    for (size_t i = 0; i < mParticles.size() / 2; ++i) {
        size_t oppositeIdx = (i + (mParticles.size() / 2)) % mParticles.size();
        AddSpring(i, oppositeIdx, 150, 1.5);
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
}

void Sphere::draw(bool isWireframe, GLuint wireframeLoc) const
{
    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_LINE : GL_FILL);
    if (!isWireframe) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 2.0f);
    } else {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }
    glUniform1i(wireframeLoc, isWireframe);

    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()),GL_UNSIGNED_INT,0);

    if (!isWireframe)
        glDisable(GL_POLYGON_OFFSET_FILL);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(0);
}

void Sphere::UpdateParticle(float dt)
{
    const int substeps = 20;
    float subDt = dt / substeps;

    for (int step = 0; step < substeps; ++step)
    {
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

    vertices.clear();
    for (auto& p : mParticles) {
        vertices.push_back(p.pos.x);
        vertices.push_back(p.pos.y);
        vertices.push_back(p.pos.z);
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    vertices.size() * sizeof(float), vertices.data());

}