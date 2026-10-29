#ifndef SOFT_BODY_SHAPECONFIG_H
#define SOFT_BODY_SHAPECONFIG_H

#include <string>

enum class ShapeType {
    Sphere = 0,
    Cube = 1
};

struct ShapeConfig {
    ShapeType type = ShapeType::Sphere;
    std::string name = "Shape";
    virtual ~ShapeConfig() = default;
};

struct SphereConfig : public ShapeConfig {
    float radius = 5.0f;
    int stacks = 15;
    int sectors = 15;

    SphereConfig() {
        type = ShapeType::Sphere;
        name = "Sphere";
    }
};

struct CubeConfig : public ShapeConfig {
    float size = 8.0f;
    int resolution = 4;

    CubeConfig() {
        type = ShapeType::Cube;
        name = "Cube";
    }
};

#endif // SOFT_BODY_SHAPECONFIG_H