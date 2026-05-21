#include <filesystem>
#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <sstream>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "Camera.h"
#include "Sphere.h"

Camera camera;
bool   lmbHeld  = false;
double lastX    = 0.0;
double lastY    = 0.0;
int    winWidth  = 1280;
int    winHeight = 720;

static std::string readFile(const char* path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cout << "ERROR: can't open shader file: " << path << std::endl;
        return "";
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

void Draw(bool isWireframe, GLuint wireframeLoc) {
    glPolygonMode(GL_FRONT_AND_BACK, isWireframe ? GL_LINE : GL_FILL);

    if (!isWireframe) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 2.0f);
    } else {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    glUniform1i(wireframeLoc, isWireframe);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    if (!isWireframe)
        glDisable(GL_POLYGON_OFFSET_FILL);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}


static unsigned int compileShader(GLenum type, const char* src) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);

    int success;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cout << "Shader error:\n" << log << std::endl;
    }
    return shader;
}

void OnLmbClicked(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        lmbHeld = (action == GLFW_PRESS);
    }
}

void OnCursorMove(GLFWwindow* window, double x, double y) {
    float newX = x - lastX;
    float newY = y - lastY;
    lastX = x; lastY = y;

    if (lmbHeld) {
        camera.RotateCamera(newX, newY);
    }
}

void OnMouseScroll(GLFWwindow* window, double xOffset, double yOffset) {
    camera.ZoomCamera((float)yOffset);
}

int main() {
    if (!glfwInit())
        return -1;

    std::cout << "CWD: " << std::filesystem::current_path() << std::endl;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "3D Ground Test", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, OnLmbClicked);
    glfwSetCursorPosCallback(window, OnCursorMove);
    glfwSetScrollCallback(window, OnMouseScroll);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to init GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    std::string vertSrc = readFile("shaders/vert.shader");
    std::string fragSrc = readFile("shaders/frag.shader");

    unsigned int vert = compileShader(GL_VERTEX_SHADER, vertSrc.c_str());
    unsigned int frag = compileShader(GL_FRAGMENT_SHADER, fragSrc.c_str());

    unsigned int progID = glCreateProgram();
    glAttachShader(progID, vert);
    glAttachShader(progID, frag);
    glLinkProgram(progID);
    Sphere sphere( progID, 5, 15,15);

    float groundVertices[] = {
        -50.0f, 0.0f, -50.0f,
         50.0f, 0.0f, -50.0f,
         50.0f, 0.0f,  50.0f,
        -50.0f, 0.0f,  50.0f
    };

    float lastFrame = glfwGetTime();
    float deltaTime = 0.0f;

    unsigned int groundIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int gVAO, gVBO, gEBO;
    glGenVertexArrays(1, &gVAO);
    glGenBuffers(1, &gVBO);
    glGenBuffers(1, &gEBO);

    glBindVertexArray(gVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(groundVertices), groundVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(groundIndices), groundIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(progID);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        deltaTime = std::min(deltaTime, 0.016f);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);

        glm::mat4 view = camera.getViewMatrix();
        float aspect = (h > 0)? (float)w / (float)h : 1.0f;

        glm::mat4 projection = camera.getProjectionMatrix(aspect);

        glUniformMatrix4fv(glGetUniformLocation(progID, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(progID, "projection"), 1, GL_FALSE, &projection[0][0]);

        glm::mat4 groundModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(progID, "model"), 1, GL_FALSE, glm::value_ptr(groundModel));
        glUniform3f(glGetUniformLocation(progID, "color"), 0.2f, 0.6f, 0.2f);
        glBindVertexArray(gVAO);
        GLuint wireframeUniformLoc = glGetUniformLocation(progID, "wireframe");

        Draw(false, wireframeUniformLoc);

        sphere.UpdateParticle(deltaTime);

        sphere.draw(false, wireframeUniformLoc);
        sphere.draw(true, wireframeUniformLoc);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}