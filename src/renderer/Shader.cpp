#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const std::string& vertPath, const std::string& fragPath) {
    std::string vertSource = ReadStringFromFile(vertPath);
    std::string fragSource = ReadStringFromFile(fragPath);

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertSource);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragSource);

    m_ID = glCreateProgram();
    glAttachShader(m_ID, vs);
    glAttachShader(m_ID, fs);
    glLinkProgram(m_ID);

    int success;
    glGetProgramiv(m_ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_ID, 512, nullptr, infoLog);
        std::cerr << "ERROR: Shader program linking failed (" << vertPath << ", " << fragPath << "):\n" << infoLog << "\n";
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader() {
    if (m_ID != 0) {
        glDeleteProgram(m_ID);
    }
}

void Shader::Bind() const {
    glUseProgram(m_ID);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

int Shader::GetUniformLocation(const std::string& name) {
    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end()) {
        return it->second;
    }
    int location = glGetUniformLocation(m_ID, name.c_str());
    m_UniformLocationCache[name] = location;
    return location;
}

void Shader::SetMat4(const std::string& name, const glm::mat4& mat) {
    int loc = GetUniformLocation(name);
    if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& vec) {
    int loc = GetUniformLocation(name);
    if (loc != -1) glUniform4f(loc, vec.x, vec.y, vec.z, vec.w);
}

void Shader::SetVec3(const std::string& name, const glm::vec3& vec) {
    int loc = GetUniformLocation(name);
    if (loc != -1) glUniform3f(loc, vec.x, vec.y, vec.z);
}

void Shader::SetInt(const std::string& name, int value) {
    int loc = GetUniformLocation(name);
    if (loc != -1) glUniform1i(loc, value);
}

void Shader::SetFloat(const std::string& name, float value) {
    int loc = GetUniformLocation(name);
    if (loc != -1) glUniform1f(loc, value);
}

std::string Shader::ReadStringFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "ERROR: Cannot open shader file: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& src) {
    unsigned int shader = glCreateShader(type);
    const char* c_src = src.c_str();
    glShaderSource(shader, 1, &c_src, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR: Shader compilation failed (" << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "):\n"
                  << infoLog << "\n";
    }
    return shader;
}
