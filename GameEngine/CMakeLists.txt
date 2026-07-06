cmake_minimum_required(VERSION 3.20)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# ===============================================================
# GLFW (SINGLE BUILD, SHARED RUNTIME)
# ===============================================================
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)

add_subdirectory(external/glfw)

find_package(OpenGL REQUIRED)

# ===============================================================
# GLAD
# ===============================================================
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# ===============================================================
# IMGUI (NO GLFW LINK HERE)
# ===============================================================
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
    external/glfw/include
)

# IMPORTANT: no glfw here anymore
target_link_libraries(imgui PUBLIC glad)

# ===============================================================
# ENGINE DLL (NO GLFW LINKAGE)
# ===============================================================
file(GLOB_RECURSE SRC_ENGINE CONFIGURE_DEPENDS
    src/Engine/*.cpp
    src/Engine/ECS/*.cpp
    src/Engine/SimpleAPI/*.cpp
    src/Engine/Scripting/*.cpp 
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
        OpenGL::GL
        glad
        glfw
)

if(WIN32)
    target_link_libraries(GameEngineLib PUBLIC opengl32)
endif()

# ===============================================================
# EDITOR (ONLY PLACE GLFW IS LINKED)
# ===============================================================
file(GLOB_RECURSE SRC_EDITOR CONFIGURE_DEPENDS
    src/Editor/*.cpp
    src/Common/*.cpp
)

add_executable(Editor ${SRC_EDITOR})

target_include_directories(Editor PRIVATE
    include/Engine
    include/Editor
    external/nlohmann
    external/glfw/include
)

target_link_libraries(Editor PRIVATE
    GameEngineLib
    imgui
    glfw            # <-- ONLY HERE
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(Editor PRIVATE opengl32)
endif()

# ===============================================================
# WINDOWS SUBSYSTEM SETTINGS
# ===============================================================
if(MSVC)
    target_link_options(Editor PRIVATE
        $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>
        $<$<NOT:$<CONFIG:Debug>>:/ENTRY:mainCRTStartup>
    )
endif()

set_target_properties(Editor PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)