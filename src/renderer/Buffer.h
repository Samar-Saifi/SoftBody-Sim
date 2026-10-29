#ifndef RENDERER_BUFFER_H
#define RENDERER_BUFFER_H

class VertexBuffer {
public:
    VertexBuffer(const void* data, unsigned int size, bool dynamic = false);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;

    void SetSubData(const void* data, unsigned int size, unsigned int offset = 0);

private:
    unsigned int m_ID = 0;
};

class IndexBuffer {
public:
    IndexBuffer(const unsigned int* indices, unsigned int count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    unsigned int GetCount() const { return m_Count; }

private:
    unsigned int m_ID = 0;
    unsigned int m_Count = 0;
};

#endif // RENDERER_BUFFER_H
