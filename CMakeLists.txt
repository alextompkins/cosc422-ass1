cmake_minimum_required(VERSION 3.9)
project(cosc422_ass1)
set(CMAKE_CXX_STANDARD 11)

# Dependencies
find_package(GLUT REQUIRED)
include_directories(${GLUT_INCLUDE_DIRS})
link_directories(${GLUT_LIBRARY_DIRS})
add_definitions(${GLUT_DEFINITIONS})
if(NOT GLUT_FOUND)
    message(ERROR " GLUT not found!")
endif(NOT GLUT_FOUND)

find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

find_package(GLEW REQUIRED)
include_directories(${GLEW_INCLUDE_DIRS})
link_libraries(${GLEW_LIBRARIES})
if (NOT GLEW_FOUND)
    message(ERROR " GLEW not found!")
endif(NOT GLEW_FOUND)

add_executable(bezier Bezier.cpp)
add_executable(terrain Terrain.cpp)

# Add custom target to copy shaders into binary directory on every build
add_custom_target(copy_shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_BINARY_DIR}/shaders
        COMMENT "Copying shaders" VERBATIM)
add_dependencies(bezier copy_shaders)
add_dependencies(terrain copy_shaders)

# Link all dependencies
target_link_libraries(bezier ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY})
target_link_libraries(terrain ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY})

# Copy resources into binary directory
file(COPY textures DESTINATION ${CMAKE_BINARY_DIR})
file(COPY geom DESTINATION ${CMAKE_BINARY_DIR})
