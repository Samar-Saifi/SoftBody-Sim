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
Sphere* globalSphere = nullptr;
glm::vec3 lightPosition(15.0f, 50.0f, 10.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

bool rmbHeld = false;
glm::vec3 dragPlanePoint = {};
glm::vec3 dragPlaneNormal = {};
bool isDragging = false;

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

void Draw(GLuint colorLoc) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glUniform4f(colorLoc, 0.2f, 0.6f, 0.2f, 1.0f );
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
        exit(-1);
    }
    return shader;
}

glm::vec3 getMouseRay(double mx, double my, glm::mat4 view, glm::mat4 proj)
{
    float nx = (2.0f * mx) / winWidth  - 1.0f;
    float ny = 1.0f - (2.0f * my) / winHeight;

    glm::vec4 eyeRay = glm::inverse(proj) * glm::vec4(nx, ny, -1.0f, 1.0f);
    eyeRay = glm::vec4(eyeRay.x, eyeRay.y, -1.0f, 0.0f);

    return glm::normalize(glm::vec3(glm::inverse(view) * eyeRay));
}

glm::vec3 rayPlaneIntersect(glm::vec3 origin, glm::vec3 dir, glm::vec3 planePoint, glm::vec3 planeNormal)
{
    float denom = glm::dot(planeNormal, dir);
    if (std::abs(denom) < 1e-6f) return planePoint;
    float t = glm::dot(planePoint - origin, planeNormal) / denom;
    return origin + dir * t;
}

void OnMouseClicked(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rmbHeld = (action == GLFW_PRESS);
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && globalSphere)
    {
        if (action == GLFW_PRESS)
        {
            lmbHeld = true;
            glm::mat4 view = camera.getViewMatrix();
            glm::mat4 proj = camera.getProjectionMatrix((float)winWidth / winHeight);

            glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);
            glm::vec3 ray    = getMouseRay(lastX, lastY, view, proj);

            int idx = globalSphere->PickParticle(camPos, ray);
            if (idx >= 0)
            {
                isDragging = true;
                dragPlaneNormal = glm::normalize(camPos - globalSphere->getParticlePos(idx));
                dragPlanePoint  = globalSphere->getParticlePos(idx);
                globalSphere->BeginDrag(idx, dragPlanePoint);
            }
        }
        else
        {
            isDragging = false;
            lmbHeld = false;
            globalSphere->EndDrag();
        }
    }

}

void OnCursorMove(GLFWwindow* window, double x, double y) {
    float newX = x - lastX;
    float newY = y - lastY;
    lastX = x; lastY = y;

    if (rmbHeld) {
        camera.RotateCamera(newX, newY);
    }

    if (lmbHeld && globalSphere && isDragging)
    {
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix((float)winWidth / winHeight);
        glm::vec3 camPos = glm::vec3(glm::inverse(view)[3]);
        glm::vec3 ray    = getMouseRay(x, y, view, proj);

        glm::vec3 target = rayPlaneIntersect(camPos, ray, dragPlanePoint, dragPlaneNormal);
        globalSphere->MoveDrag(target);
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
    glfwSetMouseButtonCallback(window, OnMouseClicked);
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
    globalSphere = &sphere;

    float groundVertices[] = {
        -50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,
        50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,
        50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f,
        -50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f
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

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glClearColor(0, 0.59f, 1, 1);

    while (!glfwWindowShouldClose(window)) {

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(progID);

        glUniform3fv(glGetUniformLocation(progID, "lightPos"), 1, glm::value_ptr(lightPosition));
        glUniform3fv(glGetUniformLocation(progID, "lightColor"), 1, glm::value_ptr(lightColor));

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        deltaTime = std::min(deltaTime, 0.016f);

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        glViewport(0, 0, w, h);
        winWidth  = w; winHeight = h;

        glm::mat4 view = camera.getViewMatrix();
        float aspect = (h > 0)? (float)w / (float)h : 1.0f;

        glm::mat4 projection = camera.getProjectionMatrix(aspect);

        glUniformMatrix4fv(glGetUniformLocation(progID, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(progID, "projection"), 1, GL_FALSE, &projection[0][0]);

        glm::mat4 groundModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(progID, "model"), 1, GL_FALSE, glm::value_ptr(groundModel));
        glBindVertexArray(gVAO);
        GLuint colorUniformLoc = glGetUniformLocation(progID, "color");

        Draw(colorUniformLoc);

        sphere.UpdateParticle(deltaTime);

        sphere.draw(colorUniformLoc);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}