cmake_minimum_required(VERSION 3.26)

project(chip8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} main.cpp chip8.cpp gui.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE chip8.h gui.h)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/roms DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Create imgui library
file(GLOB imgui_sources imgui/*.cpp)
add_library(ImGui STATIC ${imgui_sources})

target_include_directories(ImGui PUBLIC ./include/imgui)

# Add SDL to imgui
target_include_directories(ImGui PRIVATE ./include/SDL2)
target_link_libraries(ImGui PRIVATE ${PROJECT_SOURCE_DIR}/lib/libSDL2.dylib)

# Add SDL to project
target_include_directories(${PROJECT_NAME} PRIVATE ./include/SDL2)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/lib/libSDL2.dylib)

# Add IMgui to project
target_link_libraries(${PROJECT_NAME} PUBLIC ImGui)
