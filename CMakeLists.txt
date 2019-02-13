cmake_minimum_required(VERSION 3.13)
project(DVA315_LABS C)

set(CMAKE_C_STANDARD 99)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

add_executable(Lab1 Lab1/main.c Lab1/wrapper.c)
add_executable(Lab2 Lab2/main.c)
add_executable(Lab3_SERVER Lab3_SERVER/main.c)
add_executable(Lab3_CLIENT Lab3_CLIENT/main.c)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread -lrt")

target_link_libraries(Lab1 rt)
target_include_directories(Lab3_SERVER PUBLIC ${GTK_INCLUDE_DIRS})
target_link_libraries(Lab3_SERVER ${GTK_LIBRARIES})