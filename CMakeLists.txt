cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 17)

project(Zertz)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "-O3 -march=native -Wall")

set(SOURCES
    "src/main.cpp"
    "src/gui.cpp"
    "src/zertz.cpp"
)

set(HEADERS
    "src/controller.h"
    "src/game.h"
    "src/structures.h"
    "src/zertz.h"
    "src/gui.h"
)


# target_include_directories(
#     ${PROJECT_NAME} PUBLIC
#     external/imgui
#     external/imgui-node-editor
#     external/include
#     src/
#     src/gui
# )


# target_link_libraries(${PROJECT_NAME} pthread GL GLEW SDL2 pulse)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
add_executable(Zertz ${SOURCES} ${HEADERS})
target_link_libraries(Zertz sfml-graphics sfml-window sfml-system)
