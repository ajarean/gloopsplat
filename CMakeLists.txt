# Andy Jarean, Tingxuan Wu
cmake_minimum_required(VERSION 3.30)
project(gloopsplat)
set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG 1.0.3
)
FetchContent_MakeAvailable(glm)

find_package(OpenGL REQUIRED)
find_package(OpenMP)

file(GLOB IMGUI_SOURCES src/imgui/*.cpp)

add_executable(gloopsplat
  src/main.cpp
  src/glad.c
  src/shader.cpp
  src/splatRenderer.cpp
  ${IMGUI_SOURCES}
)

target_include_directories(gloopsplat PUBLIC include include/imgui)

target_link_libraries(gloopsplat PRIVATE glfw glm::glm OpenGL::GL)
target_link_libraries(gloopsplat PRIVATE OpenMP::OpenMP_CXX)

add_custom_command(TARGET gloopsplat POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:gloopsplat>/shaders"
)