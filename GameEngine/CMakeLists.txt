cmake_minimum_required(VERSION 3.20)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    src/*.cpp
)

add_library(GameEngineLib ${SRC})

target_include_directories(GameEngineLib PUBLIC
    include
    external/glfw/include
    external/glad/include
    external/imgui
    external/imgui/backends
    external/glm
    external/miniaudio
    external/nlohmann
    external/stb_image
    external/stb_truetype
)

target_compile_definitions(GameEngineLib PUBLIC
    GLM_ENABLE_EXPERIMENTAL
)

add_subdirectory(external/glfw)

find_package(OpenGL REQUIRED)

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

file(GLOB IMGUI_SRC
    external/imgui/*.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imgui/misc/cpp/imgui_stdlib.cpp
)

add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    external/imgui
    external/imgui/backends
)

target_link_libraries(imgui PUBLIC glfw glad)

target_link_libraries(GameEngineLib
    PUBLIC
    glfw
    OpenGL::GL
    glad
    imgui
)

if (WIN32)
    target_link_libraries(GameEngineLib PUBLIC opengl32)
endif()

add_executable(GameEngine main.cpp)

target_link_libraries(GameEngine PRIVATE GameEngineLib)