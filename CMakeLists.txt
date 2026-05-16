cmake_minimum_required(VERSION 3.15)
project(Soft_Body)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)

add_library(glad external/glad/src/glad.c)

file(COPY shaders DESTINATION ${CMAKE_SOURCE_DIR})

target_include_directories(glad PUBLIC
        external/glad/include
)

FetchContent_MakeAvailable(glfw)

FetchContent_Declare(glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.1
)

FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)

add_executable(Soft_Body
        src/main.cpp
        src/Camera.cpp
        src/Camera.h
        src/Sphere.cpp
        src/Sphere.h
)

target_link_libraries(Soft_Body
        OpenGL::GL
        glad
        glm
        glfw
)


