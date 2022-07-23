cmake_minimum_required(VERSION 3.17)
project(jobq C)

include_directories(include)

add_executable(server server.c)
add_executable(client client.c)