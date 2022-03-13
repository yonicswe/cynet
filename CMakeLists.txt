cmake_minimum_required(VERSION 3.8.1)
project(cynet)

find_package (Threads)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11")

add_executable(server server.cpp)
add_executable(client client.cpp)

# target_link_libraries (server ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
# target_link_libraries (client ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})
