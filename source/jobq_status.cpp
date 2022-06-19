#include "commands.hpp"
#include "config.hpp"
#include "file_lock.hpp"
#include "slurp.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

int main(){
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
    cmd_out << JobQ::CMD_SERVER_STATUS;
    cmd_out.close();
    JobQ::unlock_file(COMMAND_LOCK_FILE);
    std::this_thread::sleep_for(3000ms);
    std::cout << JobQ::slurp(STATUS_FILE);
}