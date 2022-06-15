#include "config.hpp"
#include "file_lock.hpp"

#include <chrono>
#include <fstream>
#include <thread>

int main(){
	using namespace std::chrono_literals;
	while( JobQ::lock_exists(COMMAND_LOCK_FILE) ){
		std::this_thread::sleep_for(50ms);
	}
	JobQ::lock_file(COMMAND_LOCK_FILE);
	std::ofstream out(COMMAND_FILE);
	out << "off";
	JobQ::unlock_file(COMMAND_LOCK_FILE);
}