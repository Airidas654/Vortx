cmake_minimum_required(VERSION 3.20)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# ---------------------------------------------------------------
# External dependencies
# ---------------------------------------------------------------
add_subdirectory(external/glfw)
find_package(OpenGL REQUIRED)

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# imgui is Editor-only — not linked into the Engine DLL
file(GLOB IMGUI_SRC
    external/imgui-docking/*.cpp
    external/imgui-docking/backends/imgui_impl_glfw.cpp
    external/imgui-docking/backends/imgui_impl_opengl3.cpp
    external/imgui-docking/misc/cpp/imgui_stdlib.cpp
)
add_library(imgui STATIC ${IMGUI_SRC})
target_include_directories(imgui PUBLIC
    external/imgui-docking
    external/imgui-docking/backends
)
target_link_libraries(imgui PUBLIC glfw glad)

# ---------------------------------------------------------------
# Engine DLL (no imgui)
# ---------------------------------------------------------------
file(GLOB_RECURSE SRC_ENGINE CONFIGURE_DEPENDS
    src/Engine/*.cpp
    src/Engine/ECS/*.cpp
    src/Engine/SimpleAPI/*.cpp
)

add_library(GameEngineLib SHARED ${SRC_ENGINE})

target_compile_definitions(GameEngineLib
    PRIVATE ENGINE_BUILD_DLL
    PUBLIC  GLM_ENABLE_EXPERIMENTAL
)

target_include_directories(GameEngineLib PUBLIC
    include/Engine
    external/glfw/include
    external/glad/include
    external/glm
    external/miniaudio
    external/nlohmann
    external/stb_image
    external/stb_truetype
    external/entt
)

target_link_libraries(GameEngineLib
    PUBLIC
        glfw
        OpenGL::GL
        glad
)

if(WIN32)
    target_link_libraries(GameEngineLib PUBLIC opengl32)
endif()

if(NOT MSVC)
    target_compile_options(GameEngineLib PRIVATE -fvisibility=hidden)
endif()

# ---------------------------------------------------------------
# Editor executable (Engine DLL + imgui)
# ---------------------------------------------------------------
file(GLOB_RECURSE SRC_EDITOR CONFIGURE_DEPENDS
    src/Editor/*.cpp
    src/Common/*.cpp
)

add_executable(Editor ${SRC_EDITOR})

target_include_directories(Editor PRIVATE
    external/imgui-docking
    external/imgui-docking/backends
    include/Common
    include/Engine
    include/Editor
)

target_link_libraries(Editor PRIVATE
    GameEngineLib
    imgui
)

if(MSVC)
    target_link_options(Editor PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>
        $<$<NOT:$<CONFIG:Debug>>:/ENTRY:mainCRTStartup>
    )
endif()

set_target_properties(Editor PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>
)