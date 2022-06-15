#include "commands.hpp"
#include "config.hpp"
#include "file_lock.hpp"

#include <fmt/core.h>

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

int main(){
	using namespace std::chrono_literals;
	std::cerr << "server stop\n";
	while( JobQ::is_locked(COMMAND_LOCK_FILE) ){
		std::this_thread::sleep_for(50ms);
		std::cerr << "locked\n";
	}
	JobQ::lock_file(COMMAND_LOCK_FILE);
	std::ofstream out(COMMAND_FILE);
	if( !out ){
	    std::cerr << fmt::format("Can not open file {}\n", COMMAND_FILE);
	}
	out << JobQ::CMD_SERVER_STOP;
	JobQ::unlock_file(COMMAND_LOCK_FILE);
}