#include <iostream>
#include <memory>
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

#include "Camera.h"
#include "UI.h"
#include "renderer/Renderer.h"
#include "renderer/Shader.h"
#include "renderer/VertexArray.h"
#include "renderer/Buffer.h"
#include "physics/SoftBody.h"
#include "physics/SoftBodySphere.h"
#include "physics/SoftBodyCube.h"

Camera camera;
bool lmbHeld = false;
double lastX = 0.0;
double lastY = 0.0;
int winWidth = 1280;
int winHeight = 720;

std::unique_ptr<SoftBody> globalSoftBody = nullptr;
glm::vec3 lightPosition(15.0f, 50.0f, 10.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
const unsigned int SHADOW_WIDTH = 512;
const unsigned int SHADOW_HEIGHT = 512;

bool rmbHeld = false;
glm::vec3 dragPlanePoint = {};
glm::vec3 dragPlaneNormal = {};
bool isDragging = false;

glm::vec3 getMouseRay(double mx, double my, glm::mat4 view, glm::mat4 proj) {
    float nx = (2.0f * static_cast<float>(mx)) / static_cast<float>(winWidth) - 1.0f;
    float ny = 1.0f - (2.0f * static_cast<float>(my)) / static_cast<float>(winHeight);

    glm::vec4 eyeRay = glm::inverse(proj) * glm::vec4(nx, ny, -1.0f, 1.0f);
    eyeRay = glm::vec4(eyeRay.x, eyeRay.y, -1.0f, 0.0f);

    return glm::normalize(glm::vec3(glm::inverse(view) * eyeRay));
}

glm::vec3 rayPlaneIntersect(glm::vec3 origin, glm::vec3 dir, glm::vec3 planePoint, glm::vec3 planeNormal) {
    float denom = glm::dot(planeNormal, dir);
    if (std::abs(denom) < 1e-6f) return planePoint;
    float t = glm::dot(planePoint - origin, planeNormal) / denom;
    return origin + dir * t;
}

void OnMouseClicked(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rmbHeld = (action == GLFW_PRESS);
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && globalSoftBody) {
        if (action == GLFW_PRESS) {
            lmbHeld = true;
            glm::mat4 view = camera.getViewMatrix();
            glm::mat4 proj = camera.getProjectionMatrix(static_cast<float>(winWidth) / static_cast<float>(winHeight));

            glm::vec3 camPos = camera.getPosition();
            glm::vec3 ray = getMouseRay(lastX, lastY, view, proj);

            int idx = globalSoftBody->PickParticle(camPos, ray);
            if (idx >= 0) {
                isDragging = true;
                dragPlaneNormal = glm::normalize(camPos - globalSoftBody->GetParticlePos(idx));
                dragPlanePoint = globalSoftBody->GetParticlePos(idx);
                globalSoftBody->BeginDrag(idx, dragPlanePoint);
            }
        } else {
            isDragging = false;
            lmbHeld = false;
            globalSoftBody->EndDrag();
        }
    }
}

void OnCursorMove(GLFWwindow* window, double x, double y) {
    float newX = static_cast<float>(x - lastX);
    float newY = static_cast<float>(y - lastY);
    lastX = x;
    lastY = y;

    if (rmbHeld) {
        camera.RotateCamera(newX, newY);
    }

    if (lmbHeld && globalSoftBody && isDragging) {
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 proj = camera.getProjectionMatrix(static_cast<float>(winWidth) / static_cast<float>(winHeight));
        glm::vec3 camPos = camera.getPosition();
        glm::vec3 ray = getMouseRay(x, y, view, proj);

        glm::vec3 target = rayPlaneIntersect(camPos, ray, dragPlanePoint, dragPlaneNormal);
        globalSoftBody->MoveDrag(target);
    }
}

void OnMouseScroll(GLFWwindow* window, double xOffset, double yOffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    camera.ZoomCamera(static_cast<float>(yOffset));
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(winWidth, winHeight, "Soft Body Simulation", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, OnMouseClicked);
    glfwSetCursorPosCallback(window, OnCursorMove);
    glfwSetScrollCallback(window, OnMouseScroll);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    Renderer::Init();

    UI ui;
    ui.Initialize(window);

    Shader mainShader("shaders/vert.shader", "shaders/frag.shader");
    Shader depthShader("shaders/depthVert.shader", "shaders/depthFrag.shader");

    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR: Shadow Framebuffer is not complete!\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    globalSoftBody = std::make_unique<SoftBodySphere>(5.0f, 15, 15);

    float groundVertices[] = {
        -50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,
         50.0f, 0.0f, -50.0f,  0.0f, 1.0f, 0.0f,
         50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f,
        -50.0f, 0.0f,  50.0f,  0.0f, 1.0f, 0.0f
    };
    unsigned int groundIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    auto groundVAO = std::make_shared<VertexArray>();
    auto groundVBO = std::make_shared<VertexBuffer>(groundVertices, sizeof(groundVertices));
    auto groundEBO = std::make_shared<IndexBuffer>(groundIndices, 6);
    std::vector<VertexAttribute> groundAttribs = {
        { 3, 0 },
        { 3, 3 * sizeof(float) }
    };
    groundVAO->SetBuffer(groundVBO, groundEBO, 6 * sizeof(float), groundAttribs);

    float lastFrame = static_cast<float>(glfwGetTime());
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        ui.BeginFrame();
        ui.Draw(globalSoftBody);

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        deltaTime = std::min(deltaTime, 0.016f);

        if (globalSoftBody) {
            globalSoftBody->UpdatePhysics(deltaTime);
        }

        int w, h;
        glfwGetFramebufferSize(window, &w, &h);
        winWidth = w;
        winHeight = h;

        glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, 1.0f, 80.0f);
        glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightMat = lightProjection * lightView;
        Renderer::SetViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
        Renderer::ClearDepth();

        depthShader.Bind();
        depthShader.SetMat4("lightMat", lightMat);

        glm::mat4 groundModel = glm::mat4(1.0f);
        depthShader.SetMat4("model", groundModel);
        Renderer::DrawIndexed(groundVAO);

        if (globalSoftBody) {
            glm::mat4 bodyModel = glm::mat4(1.0f);
            depthShader.SetMat4("model", bodyModel);
            Renderer::DrawIndexed(globalSoftBody->GetVertexArray());
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        Renderer::SetViewport(0, 0, winWidth, winHeight);
        Renderer::Clear(glm::vec4(0.0f, 0.59f, 1.0f, 1.0f));
        mainShader.Bind();

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        mainShader.SetInt("shadowMap", 1);

        mainShader.SetMat4("lightMat", lightMat);
        mainShader.SetVec3("viewPos", camera.getPosition());
        mainShader.SetVec3("lightPos", lightPosition);
        mainShader.SetVec3("lightColor", lightColor);

        glm::mat4 view = camera.getViewMatrix();
        float aspect = (h > 0) ? (static_cast<float>(w) / static_cast<float>(h)) : 1.0f;
        glm::mat4 projection = camera.getProjectionMatrix(aspect);

        mainShader.SetMat4("view", view);
        mainShader.SetMat4("projection", projection);

        mainShader.SetMat4("model", groundModel);
        mainShader.SetVec4("color", glm::vec4(0.2f, 0.6f, 0.2f, 1.0f));
        Renderer::DrawIndexed(groundVAO);

        if (globalSoftBody) {
            glm::mat4 bodyModel = glm::mat4(1.0f);
            mainShader.SetMat4("model", bodyModel);
            mainShader.SetVec4("color", glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
            Renderer::DrawIndexed(globalSoftBody->GetVertexArray());
        }

        ui.EndFrame();
        glfwSwapBuffers(window);
    }

    ui.Shutdown();
    glfwTerminate();
    return 0;
}