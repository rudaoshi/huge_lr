cmake_minimum_required(VERSION 2.8.4)
project(huge_lr)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory (vowpalwabbit)
add_subdirectory (sample)