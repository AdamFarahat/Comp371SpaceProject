
cmake_minimum_required(VERSION 3.10)
project(proj-windows)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)
link_directories(lib)

add_executable(proj-windows src/main.cpp src/skybox.cpp)

target_link_libraries(proj-windows
    glfw3
    glew32s
    opengl32
)
# Copy texture files to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets/skybox DESTINATION ${CMAKE_BINARY_DIR})
