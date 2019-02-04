cmake_minimum_required(VERSION 3.13)
project(DVA315_LABS C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lrt")

add_executable(Lab1 Lab1/main.c Lab1/wrapper.c)
add_executable(Lab2 Lab2/main.c)

target_link_libraries(Lab1 rt)