#include "SoftBody.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include "../collision/Collision.h"
#include "../renderer/Renderer.h"

void SoftBody::AddSpring(int a, int b, float k, float damping, int type) {
    if (a == b) return;
    for (const auto& spring : m_Springs) {
        if ((spring.a == a && spring.b == b) || (spring.a == b && spring.b == a)) return;
    }
    float distance = glm::length(m_Particles[a].pos - m_Particles[b].pos);
    m_Springs.push_back({ a, b, distance, k, damping, type });
}

void SoftBody::InitBuffers() {
    m_Vertices.clear();
    m_Vertices.reserve(m_Particles.size() * 6);
    for (const auto& p : m_Particles) {
        m_Vertices.push_back(p.pos.x);
        m_Vertices.push_back(p.pos.y);
        m_Vertices.push_back(p.pos.z);
        m_Vertices.push_back(p.normal.x);
        m_Vertices.push_back(p.normal.y);
        m_Vertices.push_back(p.normal.z);
    }

    m_VertexArray = std::make_shared<VertexArray>();
    m_VertexBuffer = std::make_shared<VertexBuffer>(m_Vertices.data(), static_cast<unsigned int>(m_Vertices.size() * sizeof(float)), true);
    m_IndexBuffer = std::make_shared<IndexBuffer>(m_Indices.data(), static_cast<unsigned int>(m_Indices.size()));

    std::vector<VertexAttribute> attributes = {
        { 3, 0 },
        { 3, 3 * sizeof(float) }
    };

    m_VertexArray->SetBuffer(m_VertexBuffer, m_IndexBuffer, 6 * sizeof(float), attributes);
}

void SoftBody::UpdateGPUData() {
    m_Vertices.clear();
    for (const auto& p : m_Particles) {
        m_Vertices.push_back(p.pos.x);
        m_Vertices.push_back(p.pos.y);
        m_Vertices.push_back(p.pos.z);
        m_Vertices.push_back(p.normal.x);
        m_Vertices.push_back(p.normal.y);
        m_Vertices.push_back(p.normal.z);
    }
    if (m_VertexBuffer) {
        m_VertexBuffer->SetSubData(m_Vertices.data(), static_cast<unsigned int>(m_Vertices.size() * sizeof(float)));
    }
}

void SoftBody::CalculateNormals() {
    for (auto& p : m_Particles) {
        p.normal = glm::vec3(0.0f);
    }

    for (size_t i = 0; i < m_Indices.size(); i += 3) {
        int a = m_Indices[i];
        int b = m_Indices[i + 1];
        int c = m_Indices[i + 2];

        glm::vec3 edge1 = m_Particles[b].pos - m_Particles[a].pos;
        glm::vec3 edge2 = m_Particles[c].pos - m_Particles[a].pos;

        glm::vec3 normal = glm::cross(edge2, edge1);
        m_Particles[a].normal += normal;
        m_Particles[b].normal += normal;
        m_Particles[c].normal += normal;
    }

    for (auto& p : m_Particles) {
        if (glm::length(p.normal) > 1e-4f) {
            p.normal = glm::normalize(p.normal);
        }
    }
}

float SoftBody::ComputeVolume() const {
    float vol = 0.0f;
    for (size_t i = 0; i < m_Indices.size(); i += 3) {
        const glm::vec3& a = m_Particles[m_Indices[i]].pos;
        const glm::vec3& b = m_Particles[m_Indices[i + 1]].pos;
        const glm::vec3& c = m_Particles[m_Indices[i + 2]].pos;
        vol += glm::dot(a, glm::cross(b, c));
    }
    return std::abs(vol) / 6.0f;
}

void SoftBody::ApplyPressureForces(float subDt) {
    if (restVolume <= 1e-4f) return;

    float ramp = std::min(mFrameCount / 60.0f, 1.0f);
    float currentVol = ComputeVolume();
    float pressure = pressureK * ramp * (restVolume - currentVol) / restVolume;

    for (size_t i = 0; i < m_Indices.size(); i += 3) {
        int ia = m_Indices[i];
        int ib = m_Indices[i + 1];
        int ic = m_Indices[i + 2];

        glm::vec3& pa = m_Particles[ia].pos;
        glm::vec3& pb = m_Particles[ib].pos;
        glm::vec3& pc = m_Particles[ic].pos;

        glm::vec3 force = pressure * glm::cross(pc - pa, pb - pa) / 3.0f;

        m_Particles[ia].vel += (force / m_Particles[ia].mass) * subDt;
        m_Particles[ib].vel += (force / m_Particles[ib].mass) * subDt;
        m_Particles[ic].vel += (force / m_Particles[ic].mass) * subDt;
    }
}

void SoftBody::UpdatePhysics(float dt) {
    if (substeps <= 0) substeps = 1;
    float subDt = dt / static_cast<float>(substeps);
    ++mFrameCount;

    for (int step = 0; step < substeps; ++step) {
        ApplyPressureForces(subDt);

        if (mDragIdx >= 0 && mDragIdx < static_cast<int>(m_Particles.size())) {
            Particle& dp = m_Particles[mDragIdx];
            glm::vec3 pull = mDragK * (mDragTarget - dp.pos);
            dp.vel += (pull / dp.mass) * subDt;
            dp.vel *= 0.80f;
        }

        for (auto& sp : m_Springs) {
            glm::vec3 p1 = m_Particles[sp.a].pos;
            glm::vec3 p2 = m_Particles[sp.b].pos;

            float distance = glm::length(p1 - p2);
            if (distance < 1e-4f) continue;

            glm::vec3 dir = (p1 - p2) / distance;
            float springMag  = sp.k * (distance - sp.len);
            glm::vec3 relVel = m_Particles[sp.a].vel - m_Particles[sp.b].vel;
            float dampMag    = sp.damping * glm::dot(relVel, dir);

            glm::vec3 force  = dir * (springMag + dampMag);

            m_Particles[sp.a].vel -= (force / m_Particles[sp.a].mass) * subDt;
            m_Particles[sp.b].vel += (force / m_Particles[sp.b].mass) * subDt;
        }

        for (auto& p : m_Particles) {
            p.vel += glm::vec3(0.0f, -9.8f, 0.0f) * subDt;
            p.vel *= (1.0f - 0.15f * subDt);
            p.pos += p.vel * subDt;
        }

        Collision::ResolveGroundCollision(m_Particles, groundHeight);
    }

    PostStep();
    CalculateNormals();
    UpdateGPUData();
}

void SoftBody::UpdateSpringParameters() {
    for (auto& spring : m_Springs) {
        switch (spring.springType) {
            case 1:
                spring.k = this->k_1;
                spring.damping = this->DAMPING;
                break;
            case 2:
                spring.k = this->k_2;
                spring.damping = this->DAMPING_2;
                break;
            case 3:
                spring.k = this->k_3;
                spring.damping = this->DAMPING_3;
                break;
            default:
                break;
        }
    }
}

int SoftBody::PickParticle(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float threshold) {
    int best = -1;
    float bestDist = threshold;

    for (int i = 0; i < static_cast<int>(m_Particles.size()); ++i) {
        glm::vec3 toP = m_Particles[i].pos - rayOrigin;
        float t = glm::dot(toP, rayDir);
        if (t < 0.0f) continue;

        float dist = glm::length(m_Particles[i].pos - (rayOrigin + rayDir * t));
        if (dist < bestDist) {
            bestDist = dist;
            best = i;
        }
    }
    if (best >= 0) {
        std::cout << "Picked particle index: " << best << " (dist: " << bestDist << ")\n";
    }
    return best;
}

void SoftBody::BeginDrag(int particleIdx, const glm::vec3& worldTarget) {
    mDragIdx = particleIdx;
    mDragTarget = worldTarget;
}

void SoftBody::MoveDrag(const glm::vec3& worldTarget) {
    mDragTarget = worldTarget;
}

void SoftBody::EndDrag() {
    mDragIdx = -1;
}

void SoftBody::Draw(const std::shared_ptr<Shader>& shader, const std::string& colorUniform, const glm::vec4& color) const {
    if (!m_VertexArray) return;
    if (shader && !colorUniform.empty()) {
        shader->SetVec4(colorUniform, color);
    }
    Renderer::DrawIndexed(m_VertexArray);
}
