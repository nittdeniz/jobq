cmake_minimum_required(VERSION 3.17)
project(jobq)

set(CMAKE_CXX_STANDARD 20)

include_directories(.)

add_executable(jobq main.cpp job.hpp job.cpp)
