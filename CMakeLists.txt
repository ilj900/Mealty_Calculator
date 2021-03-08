cmake_minimum_required(VERSION 3.13)

project(MealMe)

set(CMAKE_CXX_STANDARD 14)

include_directories(include
                    include/CLI11)

set(SOURCE src/main.cpp)

set(INCLUDE include/main.h)

add_executable(MealMe ${SOURCE} ${INCLUDE})