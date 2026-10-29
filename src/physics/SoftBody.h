#ifndef PHYSICS_SOFTBODY_H
#define PHYSICS_SOFTBODY_H

#include <vector>
#include <memory>
#include <string>
#include <glm/glm.hpp>
#include "../renderer/VertexArray.h"
#include "../renderer/Buffer.h"
#include "../renderer/Shader.h"

struct Particle {
    glm::vec3 pos   = glm::vec3(0.0f);
    glm::vec3 vel   = glm::vec3(0.0f);
    glm::vec3 force = glm::vec3(0.0f);
    glm::vec3 normal = glm::vec3(0.0f);
    float mass      = 1.0f;
};

struct Spring {
    int a = 0;
    int b = 0;
    float len = 0.0f;
    float k = 500.0f;
    float damping = 40.0f;
    int springType = 1;
};

class SoftBody {
public:
    SoftBody() = default;
    virtual ~SoftBody() = default;

    virtual void Build() = 0;
    virtual void PostStep() {}

    void UpdatePhysics(float dt);
    void UpdateSpringParameters();

    int PickParticle(const glm::vec3& rayOrigin, const glm::vec3& rayDir, float threshold = 2.0f);
    void BeginDrag(int particleIdx, const glm::vec3& worldTarget);
    void MoveDrag(const glm::vec3& worldTarget);
    void EndDrag();

    void Draw(const std::shared_ptr<Shader>& shader, const std::string& colorUniform = "color", const glm::vec4& color = glm::vec4(1.0f, 0.5f, 0.5f, 1.0f)) const;

    float ComputeVolume() const;
    void CalculateNormals();
    void UpdateGPUData();

    glm::vec3 GetParticlePos(int idx) const { return m_Particles[idx].pos; }
    const std::vector<Particle>& GetParticles() const { return m_Particles; }
    const std::vector<Spring>& GetSprings() const { return m_Springs; }
    const std::shared_ptr<VertexArray>& GetVertexArray() const { return m_VertexArray; }

    float GetRestVolume() const { return restVolume; }
    int GetDragIndex() const { return mDragIdx; }

public:
    float DAMPING   = 40.0f;
    float DAMPING_2 = 80.0f;
    float DAMPING_3 = 50.0f;

    float k_1 = 500.0f;
    float k_2 = 800.0f;
    float k_3 = 300.0f;

    int substeps = 10;
    float groundHeight = 0.0f;

protected:
    void AddSpring(int a, int b, float k, float damping, int type);
    void ApplyPressureForces(float subDt);
    void InitBuffers();

protected:
    std::vector<Particle> m_Particles;
    std::vector<Spring> m_Springs;
    std::vector<unsigned int> m_Indices;
    std::vector<float> m_Vertices;

    std::shared_ptr<VertexArray> m_VertexArray;
    std::shared_ptr<VertexBuffer> m_VertexBuffer;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;

    float restVolume = 0.0f;
    float pressureK = 20000.0f;
    int mFrameCount = 0;

    int mDragIdx = -1;
    glm::vec3 mDragTarget = glm::vec3(0.0f);
    float mDragK = 600.0f;
};

#endif // PHYSICS_SOFTBODY_H
