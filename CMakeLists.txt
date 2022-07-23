cmake_minimum_required(VERSION 3.17)
project(jobq C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

include_directories(include)

add_executable(jobq_server source/jobq_server.c include/util.h source/util.c include/config.h include/parse_config.h source/parse_config.c)
add_executable(jobq source/jobq.c include/util.h source/util.c include/config.h include/parse_config.h source/parse_config.c)