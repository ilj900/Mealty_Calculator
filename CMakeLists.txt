cmake_minimum_required(VERSION 3.13)

project(MealMe)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

include_directories(include)

set(SOURCE src/main.cpp)

set(INCLUDE include/main.h)

add_executable(MealMe ${SOURCE} ${INCLUDE})
