#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H

#include <memory>
#include <glm/glm.hpp>
#include "VertexArray.h"

class Renderer {
public:
    static void Init();
    static void SetViewport(int x, int y, int width, int height);
    static void Clear(const glm::vec4& color = glm::vec4(0.0f, 0.59f, 1.0f, 1.0f));
    static void ClearDepth();

    static void DrawIndexed(const std::shared_ptr<VertexArray>& va, unsigned int count = 0);
};

#endif // RENDERER_RENDERER_H
