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


file(GLOB_RECURSE SRC_EDITOR CONFIGURE_DEPENDS
    src/Editor/*.cpp
    src/Common/*.cpp
)

add_library(EditorLib STATIC ${SRC_EDITOR})

target_include_directories(EditorLib PUBLIC
    include/Engine
    include/Editor
    external/nlohmann
    external/glfw/include
)

target_link_libraries(EditorLib PUBLIC
    GameEngineLib
    imgui
    glfw            
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(EditorLib PUBLIC opengl32)
endif()


if(MSVC)
    target_link_options(EditorLib PUBLIC
        $<$<NOT:$<CONFIG:Debug>>:/SUBSYSTEM:WINDOWS>
        $<$<NOT:$<CONFIG:Debug>>:/ENTRY:mainCRTStartup> 
    )
endif()

add_custom_command(TARGET EditorLib POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include/Engine   $<TARGET_FILE_DIR:EditorLib>/EngineSDK/include/Engine
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/include/Executable   $<TARGET_FILE_DIR:EditorLib>/EngineSDK/include/Executable
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/external/glm  $<TARGET_FILE_DIR:EditorLib>/EngineSDK/external/glm
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/external/entt $<TARGET_FILE_DIR:EditorLib>/EngineSDK/external/entt
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/external/nlohmann $<TARGET_FILE_DIR:EditorLib>/EngineSDK/external/nlohmann
)

set_target_properties(EditorLib PROPERTIES
    WIN32_EXECUTABLE $<NOT:$<CONFIG:Debug>>)

file(GLOB_RECURSE SRC_EDITOR_EXECUTABLE CONFIGURE_DEPENDS
    src/Main/main.cpp
    src/Executable/*.cpp
)

add_executable(Editor ${SRC_EDITOR_EXECUTABLE})

target_link_libraries(Editor 
    PRIVATE 
    EditorLib
)

target_link_directories(Editor
    PUBLIC
    include/Executable 
)

option(ENABLE_TESTS "Build unit tests" ON) 

if(ENABLE_TESTS)

set(INSTALL_GTEST OFF)
set(BUILD_GMOCK OFF)

add_subdirectory(external/googletest)

enable_testing()

file(GLOB TEST_SRC CONFIGURE_DEPENDS
    tests/unit/*.cpp
)

add_executable(UnitTests ${TEST_SRC})

target_compile_definitions(UnitTests PRIVATE
    TEST_DATA_DIR="${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/tests/data"
)

target_compile_options(UnitTests PRIVATE /EHsc)

target_link_libraries(UnitTests
    PRIVATE
    EditorLib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(UnitTests)

endif()