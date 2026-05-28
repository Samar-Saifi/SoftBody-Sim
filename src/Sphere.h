#ifndef SOFT_BODY_SPHERE_H
#define SOFT_BODY_SPHERE_H

#include <vector>
#include <glad/glad.h>
#include "glm/glm.hpp"

struct Particle {
    glm::vec3 pos   = glm::vec3(0.0f);
    glm::vec3 vel   = glm::vec3(0.0f);
    glm::vec3 force = glm::vec3(0.0f);
    float mass      = 1.0f;
};

struct Spring {
    int a, b;
    float len, k, damping;
};

class Sphere {
public:
    Sphere(unsigned int prog, float radius = 1.0f, int stacks = 20, int sectors = 20);
    ~Sphere();

    void draw(bool isWireframe, GLuint wireframeLoc) const;
    void UpdateParticle(float dt);

    int  PickParticle(glm::vec3 rayOrigin, glm::vec3 rayDir, float threshold = 2.0f);
    void BeginDrag(int particleIdx, glm::vec3 worldTarget);
    void MoveDrag(glm::vec3 worldTarget);
    void EndDrag();

    inline glm::vec3 getParticlePos(int idx) const { return mParticles[idx].pos; }


private:
    void build();
    void AddSpring(int pointA, int pointB, float stiffness, float damping);
    float computeVolume() const;
    void  applyPressureForces(float subDt);

private:
    float radius;
    int stacks;
    int sectors;
    unsigned int progID;
    float groundHeight = 15;

    int mDragIdx = -1;
    glm::vec3 mDragTarget = {};
    float mDragK = 600.0f;

    float restVolume = 0.0f;
    float pressureK = 20000.0f;
    int mFrameCount = 0;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<Particle> mParticles;
    std::vector<Spring> mSprings;
};

#endif