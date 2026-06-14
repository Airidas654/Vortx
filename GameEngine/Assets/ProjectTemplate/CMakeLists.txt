cmake_minimum_required(VERSION 3.20)
project(Game)

set(CMAKE_CXX_STANDARD 17)
#set(ENGINE_PATH "{{ENGINE_PATH}}")

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
file(WRITE ${GENERATED_FILE} ${GENERATED_CONTENT})

file(GLOB_RECURSE SRC_GAME Assets/*.cpp)

add_library(Game SHARED
    ${SRC_GAME}
    ${GENERATED_FILE}
)

target_include_directories(Game PRIVATE
    "${ENGINE_PATH}/include/Engine"
    "${ENGINE_PATH}/include/Game"
    "${ENGINE_PATH}/external/glm"
    "${ENGINE_PATH}/external/entt"
)

target_link_libraries(Game PRIVATE
    "${ENGINE_PATH}/bin/GameEngineLib"
)