cmake_minimum_required(VERSION 3.15)
project(Lumina3D VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)

# Use manually installed libraries if external directory exists
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")
if(EXISTS "${EXTERNAL_DIR}")
    message(STATUS "Using manually installed libraries from ${EXTERNAL_DIR}")
    
    # GLFW
    set(GLFW_INCLUDE_DIR "${EXTERNAL_DIR}/include")
    set(GLFW_LIBRARY "${EXTERNAL_DIR}/lib/glfw3dll.lib")
    
    # GLM
    set(GLM_INCLUDE_DIR "${EXTERNAL_DIR}/include")
    
    # Include directories
    include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${OPENGL_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
    )
else()
    # Try to find packages normally
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
    
    # Include directories
    include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${OPENGL_INCLUDE_DIR}
    )
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/Rasterizer.cpp
    src/Transform.cpp
    src/Renderer.cpp
)

# Header files
set(HEADERS
    include/Engine.h
    include/Rasterizer.h
    include/Transform.h
    include/Shaders.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link libraries
if(EXISTS "${EXTERNAL_DIR}")
    target_link_libraries(${PROJECT_NAME}
        OpenGL::GL
        ${GLFW_LIBRARY}
    )
    
    # Copy GLFW DLL to output directory
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${EXTERNAL_DIR}/bin/glfw3.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    target_link_libraries(${PROJECT_NAME}
        OpenGL::GL
        glfw
    )
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy assets to build directory
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
