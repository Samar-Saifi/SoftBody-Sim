#ifndef SOFT_BODY_SPHERE_H
#define SOFT_BODY_SPHERE_H

#include <vector>
#include <glad/glad.h>

class Sphere {
public:
    Sphere(unsigned int prog, float radius = 1.0f, int stacks = 20, int sectors = 20);
    ~Sphere();

    void draw(bool isWireframe, GLuint wireframeLoc) const;

private:
    void build();

private:
    float radius;
    int stacks;
    int sectors;
    unsigned int progID;
    float groundHeight = 15;

    GLuint VAO = 0;
    GLuint VBO = 0;
    GLuint EBO = 0;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
};

#endif