cmake_minimum_required(VERSION 3.17)
cmake_policy(VERSION 3.10...3.30)
project(MineCraftPlusPlus CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Configure Local GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/glfw)

find_package(OpenGL REQUIRED)

# 2. Collect all engine source files PLUS GLAD's source file
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(APPEND SOURCES vendor/glad/src/glad.c)

add_executable(${PROJECT_NAME} ${SOURCES})

# 3. Link libraries and point to local include directories
target_link_libraries(${PROJECT_NAME} PRIVATE OpenGL::GL glfw)
target_include_directories(${PROJECT_NAME} PRIVATE 
    src/
    vendor/glm
    vendor/glad/include
)