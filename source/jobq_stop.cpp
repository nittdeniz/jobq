#include "commands.hpp"
#include "config.hpp"
#include "file_lock.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>


int main(int argc, char** argv){
    if( argc != 2 ){
        std::cout << "jobq_stop usage:\n" << "jobq_stop <pid>\nYou can find the pid by calling jobq_status\n";
        return EXIT_SUCCESS;
    }
    unsigned int pid = std::strtoull(argv[1], nullptr, 10);
    using namespace std::chrono_literals;
    int i{0};
    while( JobQ::is_locked(COMMAND_LOCK_FILE) ){
        std::this_thread::sleep_for(50ms);
        if( i++ > 50 ){
            std::cerr << "Command lock seems to be deadlocked.\n";
            return EXIT_FAILURE;
        }
    }
    JobQ::lock_file(COMMAND_LOCK_FILE);
    auto cmd_out = std::ofstream(COMMAND_FILE);
    cmd_out << JobQ::CMD_STOP << " " << pid;
    cmd_out.close();
    JobQ::unlock_file(COMMAND_LOCK_FILE);
    std::this_thread::sleep_for(3000ms);
    std::cout << "Job " << pid << " stopped.\n";
}