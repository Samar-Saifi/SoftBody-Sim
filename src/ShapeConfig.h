//
// Created by mdsamar on 08/06/26.
//

#ifndef SOFT_BODY_SHAPECONFIG_H
#define SOFT_BODY_SHAPECONFIG_H

#include <string>

enum class ShapeType {
    Sphere,
};

struct ShapeConfig {
    ShapeType type;
    std::string name;
    virtual ~ShapeConfig() = default;
};

struct SphereConfig : ShapeConfig {
    float radius = 5.0f;
    int stacks = 15;
    int sectors = 15;

    SphereConfig() {
        type = ShapeType::Sphere;
        name = "Sphere";
    }
};

#endif