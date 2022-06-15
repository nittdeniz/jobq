#include "config.hpp"
#include "queue.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <thread>

int main(int, char**){
    using namespace std::chrono_literals;
    std::ofstream log_stream(LOG_FILE);

    if( !log_stream ){
        std::cerr << "Could not open log_file file: "<< LOG_FILE << "\n";
        return EXIT_FAILURE;
    }

    std::cerr << "queue\n";
    JobQ::Queue queue(MAX_N_PROCESSORS, log_stream, QUEUE_FILE);

    while( queue.running() ){
        std::cerr << "loop\n";
        queue.process();
        std::this_thread::sleep_for(2s);
    }
}