#include "config.hpp"
#include "file_lock.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

void help(){
    std::cout << "jobq_submit usage:\n";
    std::cout << "jobq <n_cores> <std_out> <std_err> <timelimit in seconds> <cmd>\n";
    std::cout << "Example: jobq 2 60 'out/std.txt' 'out/err.txt' ping example.com\n";
    std::cout << "Will start the ping command on 2 cores and will automatically terminate after 60 seconds.\n";
    std::cout << "All output of the programm will be redirected to out/std.txt and out/err.txt respectively.\n";
}

int main(int argc, char** argv){
    using namespace std::chrono_literals;
    if( argc == 1 ){
        help();
        return EXIT_SUCCESS;
    }
    if( argc < 6 ){
        std::cout << "jobq_submit invalid number of arguments.\n";
        help();
        return EXIT_FAILURE;
    }
    try{
        unsigned int n_cores = std::atoi(argv[1]);
        unsigned int timelimit = std::atoi(argv[2]);
        std::string out(argv[3]);
        std::string err(argv[4]);
        std::string command(argv[5]);
        for( int i = 6; i < argc; ++i ){
            command += " ";
            command += std::string(argv[i]);
        }
        int i{0};
        while( JobQ::is_locked(QUEUE_LOCK_FILE) ){
            std::this_thread::sleep_for(50ms);
            if( i++ > 50 ){
                std::cerr << "Queue lock file seems to be deadlocked.\n";
                return EXIT_FAILURE;
            }
        }
        JobQ::lock_file(QUEUE_LOCK_FILE);
        std::ofstream queue_out(QUEUE_FILE, std::ofstream::app);
        queue_out << n_cores << " " << timelimit << " " << out << " " << err << " " << command << "\n";
        JobQ::unlock_file(QUEUE_LOCK_FILE);
        std::cout << "Job submitted. Type jobq_status to see status.\n";
    }
    catch( std::exception& e ){
        std::cerr << "Could not submit job.\n" << e.what() << "\n";
    }
}