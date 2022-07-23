cmake_minimum_required(VERSION 3.17)
project(jobq C)

include_directories(include)

add_executable(jobq_server source/jobq_server.c)
add_executable(jobq_submit source/jobq_submit.c)