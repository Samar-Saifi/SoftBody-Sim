#ifndef SOFT_BODY_UI_H
#define SOFT_BODY_UI_H

#include <memory>
#include "ShapeConfig.h"
#include "physics/SoftBody.h"

struct GLFWwindow;

class UI {
public:
    void Initialize(GLFWwindow* window);
    void BeginFrame();
    void Draw(std::unique_ptr<SoftBody>& activeBody);
    void EndFrame();
    void Shutdown();

private:
    ShapeType m_SelectedShapeType = ShapeType::Sphere;
    SphereConfig m_SphereConfig;
    CubeConfig m_CubeConfig;
};

#endif // SOFT_BODY_UI_H
