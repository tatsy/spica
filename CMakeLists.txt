cmake_minimum_required(VERSION 2.8.4)
project(Rainy)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")

set(SOURCE_FILES src/main.cc)
list(APPEND SOURCE_FILES src/Random.h src/Random_detail.h)
list(APPEND SOURCE_FILES src/Vector3.h src/Vector3.cc)
list(APPEND SOURCE_FILES src/Sphere.h src/Sphere.cc)
list(APPEND SOURCE_FILES src/Ray.h src/Ray.cc)
list(APPEND SOURCE_FILES src/Scene.h src/Scene.cc)
list(APPEND SOURCE_FILES src/Renderer.h src/Renderer.cc)
list(APPEND SOURCE_FILES src/material.h)
list(APPEND SOURCE_FILES src/rainy.h src/common.h)

add_executable(Rainy ${SOURCE_FILES})

include_directories(${CMAKE_CURRENT_LIST_DIR})
