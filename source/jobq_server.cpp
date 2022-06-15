#include "config.hpp"
#include "log.hpp"
#include "queue.hpp"
#include "system_call.hpp"

#include <format>
#include <fstream>
#include <iostream>
#include <map>

int main(int, char**){
    std::ofstream log_stream(LOG_FILE);

    if( !log_stream ){
        std::cerr << "Could not open log_file file: "<< LOG_FILE << "\n";
        return EXIT_FAILURE;
    }

    JobQ::Queue queue(MAX_N_PROCESSORS, log_stream, QUEUE_FILE);

    while( true ){
        check_status();
        clear_processes();
        load_new_processes();
        start_new_processes();
        std::ofstream status_out(STATUS_FILE);
        if( status_out ){
            write_status(status_out);
        }else{
            log_file << str_time() << ": Can not write to status file: " << STATUS_FILE << "\n" << std::flush;
        }
//        write_status(std::cerr);
        std::this_thread::sleep_for(2s);
    }
}