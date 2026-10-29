#ifndef RENDERER_VERTEXARRAY_H
#define RENDERER_VERTEXARRAY_H

#include <memory>
#include <vector>
#include "Buffer.h"

struct VertexAttribute {
    unsigned int count;
    unsigned int offset;
};

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void SetBuffer(
        std::shared_ptr<VertexBuffer> vb,
        std::shared_ptr<IndexBuffer> ib,
        unsigned int stride,
        const std::vector<VertexAttribute>& attributes
    );

    const std::shared_ptr<IndexBuffer>& GetIndexBuffer() const { return m_IndexBuffer; }
    const std::shared_ptr<VertexBuffer>& GetVertexBuffer() const { return m_VertexBuffer; }

private:
    unsigned int m_ID = 0;
    std::shared_ptr<VertexBuffer> m_VertexBuffer;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
};

#endif // RENDERER_VERTEXARRAY_H
