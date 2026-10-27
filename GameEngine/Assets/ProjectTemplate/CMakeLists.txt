cmake_minimum_required(VERSION 3.20)
project(Game)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ENGINE_PATH     = engine source root  (for headers)
# ENGINE_BIN_PATH = engine bin output   (for GameEngineLib.lib import stub)
# Both are passed in by the editor via -DENGINE_PATH=... -DENGINE_BIN_PATH=... 

# Written by the editor on project load/compile. VS picks it up automatically.
include("${CMAKE_SOURCE_DIR}/EnginePaths.cmake" OPTIONAL)

if(NOT ENGINE_PATH OR NOT ENGINE_BIN_PATH)
    message(FATAL_ERROR
        "ENGINE_PATH / ENGINE_BIN_PATH not set. Open this project once in the "
        "Vortx editor so it generates EnginePaths.cmake, or pass them with -D.")
endif()

# ===============================================================
# LOCATE ENGINE IMPORT LIB (.lib is the DLL import stub on Windows)
# ===============================================================
find_library(GAME_ENGINE_LIB
    NAMES GameEngineLib
    PATHS "${ENGINE_BIN_PATH}"
    NO_DEFAULT_PATH
)

if(NOT GAME_ENGINE_LIB)
    message(FATAL_ERROR
        "Could not find GameEngineLib.lib under ${ENGINE_BIN_PATH}. "
        "Make sure the engine has been built before compiling the Game project.")
endif()

file(GLOB_RECURSE GAME_SCRIPT_HEADERS Assets/*.h)

set(GENERATED_FILE "${CMAKE_BINARY_DIR}/generated/RegisterScripts.cpp") 
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/generated")

set(GENERATED_CONTENT "// AUTO-GENERATED — do not edit\n")
foreach(HEADER ${GAME_SCRIPT_HEADERS})
    file(READ ${HEADER} CONTENT)
    if(CONTENT MATCHES "ScriptBase<")
        file(RELATIVE_PATH REL ${CMAKE_SOURCE_DIR} ${HEADER})
        string(APPEND GENERATED_CONTENT "#include \"${REL}\"\n")
    endif()
endforeach()
string(APPEND GENERATED_CONTENT "
void RegisterScripts() {}
")
file(WRITE ${GENERATED_FILE} ${GENERATED_CONTENT})

file(GLOB_RECURSE SRC_GAME CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Assets/*.cpp")

add_library(Game SHARED
    ${SRC_GAME}
    ${GENERATED_FILE}
)

target_compile_definitions(Game 
	PRIVATE GLM_ENABLE_EXPERIMENTAL
)

#message(STATUS "Linking against: ${GAME_ENGINE_LIB}")

# Only include/Game is the public API the user's scripts need.
# include/Engine is internal — users should never include engine internals directly.
target_include_directories(Game PRIVATE
    "${ENGINE_PATH}/include/Game"
    "${ENGINE_PATH}/include/Engine"
    "${ENGINE_PATH}/external/glm"
    "${ENGINE_PATH}/external/entt"
    "${CMAKE_SOURCE_DIR}/Assets"
    "${CMAKE_SOURCE_DIR}"
)

# Link against the DLL import stub — NOT a static lib copy.
# This ensures Game.dll shares the same GameEngineLib.dll instance as Editor.exe.
target_link_libraries(Game PRIVATE
    "${GAME_ENGINE_LIB}"
)