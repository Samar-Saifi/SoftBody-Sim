#include "Sphere.h"
#include <cmath>

Sphere::Sphere(unsigned int prog, float radius, int stacks, int sectors)
    : radius(radius), stacks(stacks), sectors(sectors), progID(prog)
{
    build();
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

    const float PI = 3.14159265359f;

    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = PI / 2 - i * PI / stacks;
        float xy = radius * cosf(stackAngle);
        float z  = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * 2 * PI / sectors;

            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }


    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {

            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),vertices.data(),GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(),GL_STATIC_DRAW);

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