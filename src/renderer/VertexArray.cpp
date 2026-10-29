#include "VertexArray.h"
#include <glad/glad.h>

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_ID);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_ID);
}

void VertexArray::Bind() const {
    glBindVertexArray(m_ID);
}

void VertexArray::Unbind() const {
    glBindVertexArray(0);
}

void VertexArray::SetBuffer(
    std::shared_ptr<VertexBuffer> vb,
    std::shared_ptr<IndexBuffer> ib,
    unsigned int stride,
    const std::vector<VertexAttribute>& attributes
) {
    Bind();
    vb->Bind();
    if (ib) {
        ib->Bind();
    }

    m_VertexBuffer = vb;
    m_IndexBuffer = ib;

    for (size_t i = 0; i < attributes.size(); ++i) {
        glEnableVertexAttribArray(static_cast<GLuint>(i));
        glVertexAttribPointer(
            static_cast<GLuint>(i),
            attributes[i].count,
            GL_FLOAT,
            GL_FALSE,
            static_cast<GLsizei>(stride),
            reinterpret_cast<const void*>(static_cast<uintptr_t>(attributes[i].offset))
        );
    }

    Unbind();
}
