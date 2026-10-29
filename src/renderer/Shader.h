#ifndef RENDERER_SHADER_H
#define RENDERER_SHADER_H

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
public:
    Shader(const std::string& vertPath, const std::string& fragPath);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    void SetMat4(const std::string& name, const glm::mat4& mat);
    void SetVec4(const std::string& name, const glm::vec4& vec);
    void SetVec3(const std::string& name, const glm::vec3& vec);
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);

    unsigned int GetID() const { return m_ID; }
    int GetUniformLocation(const std::string& name);

private:
    unsigned int m_ID = 0;
    std::unordered_map<std::string, int> m_UniformLocationCache;

    static std::string ReadStringFromFile(const std::string& path);
    static unsigned int CompileShader(unsigned int type, const std::string& src);
};

#endif // RENDERER_SHADER_H
