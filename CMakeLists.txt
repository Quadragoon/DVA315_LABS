cmake_minimum_required(VERSION 3.13)
project(DVA315_LABS C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pthread")

#add_executable(Lab1 Lab1/main.c Lab1/wrapper.c)
#target_link_libraries(Lab1 rt)

#add_executable(Lab2 Lab2/main.c)

#add_executable(Lab3_Server Lab3/Server/main.c Lab3/wrapper.c)
#add_executable(Lab3_Client Lab3/Client/main.c Lab3/wrapper.c)
#find_package(PkgConfig REQUIRED)
#pkg_check_modules(GTK REQUIRED gtk+-3.0)
#target_include_directories(Lab3_Server PUBLIC ${GTK_INCLUDE_DIRS})
#target_link_libraries(Lab3_Server ${GTK_LIBRARIES})
#target_link_libraries(Lab3_Server rt m)
#target_link_libraries(Lab3_Client rt)

# uses a specific working directory for file handling and as such should never be commented out
add_executable(Lab4 Lab4/main.c)
target_link_libraries(Lab4 rt)

# uses a specific working directory for file handling and as such should never be commented out
add_executable(Lab5 Lab5/main.c)