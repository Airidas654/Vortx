cmake_minimum_required(VERSION 3.20)
project(VortxTests CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(CTest)
include(GoogleTest)

# ---- GoogleTest (skip if your project already provides GTest::gtest_main) ----
if(NOT TARGET GTest::gtest_main)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# ---- Adjust these two for your tree ----------------------------------------
# Target that provides the engine (Scene, Physics, Renderer, ...).
set(VORTX_ENGINE_TARGET GameEngineLib CACHE STRING "Engine library target")
# Folder holding the editor .cpp/.h files (Editor.cpp, Logger.cpp, ...).
set(VORTX_EDITOR_DIR "" CACHE PATH "Editor source directory")
# -----------------------------------------------------------------------------

# ======================= Engine tests =======================
file(GLOB ENGINE_TEST_SOURCES CONFIGURE_DEPENDS engine/*.cpp)
add_executable(EngineTests ${ENGINE_TEST_SOURCES})
target_include_directories(EngineTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(EngineTests PRIVATE ${VORTX_ENGINE_TARGET} GTest::gtest_main)
gtest_discover_tests(EngineTests)

# ======================= Editor tests =======================
# Compiles every editor source except main.cpp into the test exe.
# Link whatever the editor itself links (glfw, glad, imgui, ...).
if(VORTX_EDITOR_DIR)
    file(GLOB EDITOR_SOURCES CONFIGURE_DEPENDS ${VORTX_EDITOR_DIR}/*.cpp)
    list(FILTER EDITOR_SOURCES EXCLUDE REGEX ".*/main\\.cpp$")
    file(GLOB EDITOR_TEST_SOURCES CONFIGURE_DEPENDS editor/*.cpp)

    add_executable(EditorTests ${EDITOR_TEST_SOURCES} ${EDITOR_SOURCES})
    target_include_directories(EditorTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${VORTX_EDITOR_DIR})
    target_link_libraries(EditorTests PRIVATE ${VORTX_ENGINE_TARGET} GTest::gtest_main
        # glfw glad imgui   <-- add your editor's dependencies here
    )
    gtest_discover_tests(EditorTests)
endif()
