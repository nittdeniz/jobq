cmake_minimum_required(VERSION 3.14)
project(jobq C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

include_directories(include)

add_executable(jobq_server source/jobq_server.c include/util.h source/util.c include/config.h include/parse_config.h source/parse_config.c include/messages.h include/job.h include/server.h source/server.c include/queue.h source/queue.c source/job.c include/manager.h source/manager.c include/colours.h)
add_executable(jobq source/jobq.c include/util.h source/util.c include/config.h include/parse_config.h source/parse_config.c include/messages.h include/job.h include/colours.h)

add_executable(test test.c)

target_link_libraries(jobq_server m pthread)
target_link_libraries(jobq m)