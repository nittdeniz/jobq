cmake_minimum_required(VERSION 3.17)
project(jobq)

set(CMAKE_CXX_STANDARD 20)

include_directories(include)

add_executable(jobq_server source/jobq_server.cpp include/job.hpp source/job.cpp include/config.hpp include/file_lock.hpp source/file_lock.cpp include/time.hpp source/time.cpp include/system_call.hpp source/system_call.cpp include/log.hpp source/log.cpp source/queue.cpp include/queue.hpp include/slurp.hpp source/slurp.cpp)
add_executable(jobq_server_start source/jobq_server_start.cpp include/job.hpp source/job.cpp include/config.hpp include/file_lock.hpp source/file_lock.cpp include/time.hpp source/time.cpp include/system_call.hpp source/system_call.cpp)
add_executable(jobq_server_stop source/jobq_server_stop.cpp include/job.hpp source/job.cpp include/config.hpp include/file_lock.hpp source/file_lock.cpp include/time.hpp source/time.cpp include/system_call.hpp source/system_call.cpp)
add_executable(jobq_status source/jobq_status.cpp include/job.hpp source/job.cpp include/config.hpp include/file_lock.hpp source/file_lock.cpp include/time.hpp source/time.cpp include/system_call.hpp source/system_call.cpp include/slurp.hpp source/slurp.cpp)
add_executable(jobq_submit source/jobq_submit include/job.hpp source/job.cpp include/config.hpp include/file_lock.hpp source/file_lock.cpp include/time.hpp source/time.cpp include/system_call.hpp source/system_call.cpp)
add_executable(jobq_stop source/jobq_stop include/job.hpp source/job.cpp include/config.hpp include/file_lock.hpp source/file_lock.cpp include/time.hpp source/time.cpp include/system_call.hpp source/system_call.cpp)
