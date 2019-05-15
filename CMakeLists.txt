cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(MiniBow)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(.)

add_executable(demo demo.cpp MiniBow.h)
target_link_libraries(demo ${OpenCV_LIBS})
file(COPY images DESTINATION ${CMAKE_BINARY_DIR}/)
file(COPY ORBvoc.minibow DESTINATION ${CMAKE_BINARY_DIR}/)

