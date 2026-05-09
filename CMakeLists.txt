cmake_minimum_required(VERSION 3.15)
project(Soft_Body)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)

find_package(OpenGL REQUIRED)

add_executable(Soft_Body main.cpp)

target_link_libraries(Soft_Body
        OpenGL::GL
        glfw
)

