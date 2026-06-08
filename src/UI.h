#ifndef SOFT_BODY_UI_H
#define SOFT_BODY_UI_H

#include "ShapeConfig.h"

class Sphere;
struct GLFWwindow;

class UI
{
public:
    void Initialize(GLFWwindow* window);
    void BeginFrame();
    void Draw(Sphere*& globalSphere);
    void EndFrame();
    void Shutdown();

private:
    ShapeType mCurrentSelectedShape = ShapeType::Sphere;
    SphereConfig mSphereParams;
};

#endif
