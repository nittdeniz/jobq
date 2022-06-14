#include "job.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>


using P_ID = unsigned int;
using namespace std::chrono_literals;

unsigned int TERMINATE_SERVER = 666;
std::vector<Job> job_queue;
std::map<P_ID, Job> running_jobs;
std::map<unsigned int, bool> cores_in_use;
std::optional<std::pair<std::size_t, std::time_t>> job_pair;

std::ofstream log_file;

std::string LOG_FILE, QUEUE_FILE, buffer, JOB_EXEC, OUTPUT_BUFFER, STATUS_FILE;
unsigned int MAX_N_PROCESSORS;

std::string str_time(){
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %X]");
    return ss.str();
}

std::string str_time(long long int delta_t){
    auto now = std::chrono::system_clock::now();
    std::cerr << "\nstr_time: {" << delta_t << "}\n";
    auto in_time_t = std::chrono::system_clock::to_time_t(now + std::chrono::duration<long long int,std::ratio<1>>(delta_t));
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "[%Y-%m-%d %X]");
    return ss.str();
}

time_t now(){
    auto now = std::chrono::system_clock::now();
    return std::chrono::system_clock::to_time_t(now);
}

void send_sigterm(P_ID pid){
//    std::cerr << "kill -9 " << pid << "\n";
    std::stringstream cmd;
    cmd << "kill -9 " << pid;
    std::system(cmd.str().c_str());
}

bool is_running(P_ID pid){
    std::stringstream cmd;
    cmd << "ps -p " << pid << " > " << OUTPUT_BUFFER << " 2>&1";
    std::system(cmd.str().c_str());
    std::this_thread::sleep_for(50ms);
    std::ifstream in_buffer(OUTPUT_BUFFER);
    std::string buffer;
    std::size_t i{0};
    while( std::getline(in_buffer, buffer) ){
        std::string reg_str = "[ \t]*" + std::to_string(pid) + ".*";
        if( std::regex_match(buffer, std::regex(reg_str)) ){
            return true;
        }
    }
    return false;
}

unsigned int n_free_cores(){
    return std::count_if(cores_in_use.begin(), cores_in_use.end(), [](std::pair<unsigned int, bool> pair){
        return !pair.second;
    });
}

void free_cores(Job const& j){
    for( auto const& i : j.processor_ids ){
        cores_in_use[i] = false;
    }
}

std::vector<unsigned int> allocate_cores(unsigned int n){
    std::vector<unsigned int> cores;
    int i = 0;
    for( auto& [cid, in_use] : cores_in_use ){
        if( !in_use ){
            cores.push_back(cid);
            ++i;
            in_use = true;
        }
        if( i == n ){
            break;
        }
    }
    return cores;
}


void start(Job& job){
    job.start_time = now();

    auto core_ids = allocate_cores(job.n_cores);
    std::stringstream cmd;
    cmd << JOB_EXEC << " " << job.cout << " " << job.cerr << " " << OUTPUT_BUFFER << " 'taskset -c ";
    for( std::size_t i = 0; i < core_ids.size(); ++i ){
        if( i > 0 ){
            cmd << ", ";
        }
        cmd << core_ids[i];
    }
    cmd << " " << job.command << "'";
    std::system(cmd.str().c_str());
    std::this_thread::sleep_for(50ms);
    std::ifstream pid_in(OUTPUT_BUFFER);
    if( pid_in ){
        P_ID pid;
        pid_in >> pid;
        if( running_jobs.contains(pid) ){
            std::cerr << "Starting job with same id `" << pid << "`. Terminating\n";
            exit(EXIT_FAILURE);
        }
        running_jobs[pid] = job;
        log_file << str_time() << ": started process[" << pid << "]: `" << job.command << "`.\n\tCores: " << job.n_cores << ". \n\tAutomatic termination on: " << str_time(job.max_time) << "\n" << std::flush;
    }else{
        std::cerr << "Could not open buffer file. Terminating.\n";
        exit(EXIT_FAILURE);
    }
}


void clear_processes(){
    for( auto& [pid, job] : running_jobs ){
        if( is_running(pid) && (now() - job.start_time > job.max_time) ){
            send_sigterm(pid);
            log_file << str_time() << ": Job " << pid << " terminated.\n" << std::flush;
        }
        std::this_thread::sleep_for(50ms);
        if( !is_running(pid) ){
            free_cores(job);
            running_jobs.erase(pid);
            log_file << str_time() << ": Job " << pid << " ended.\n"  << std::flush;
        }
        if( running_jobs.empty() ){
            break;
        }
    }
}

void load_new_processes(){
    std::string job_line;
    std::fstream job_file(QUEUE_FILE, std::ifstream::in);
    if( !job_file ){
        std::cerr << "Could not open job_file: " << QUEUE_FILE << "\n";
        exit(EXIT_FAILURE);
    }
    while( std::getline(job_file, job_line) ){
        try
        {
            std::stringstream parser(job_line);
            Job job;
            parser >> job.n_cores;
            if( job.n_cores == TERMINATE_SERVER ){
                exit(0);
            }
            parser >> job.max_time;
            parser >> job.cout;
            parser >> job.cerr;
            std::string cmd(std::istreambuf_iterator<char>(parser.rdbuf()), {});
            job.command = cmd.substr(1);
            job_queue.push_back(job);
            log_file << str_time() << ": loaded job `" << job_line << "`\n" << std::flush;
        }
        catch( std::exception& e){
            log_file << str_time() << ": Could not load job: `" << job_line << "`\n" << std::flush;
        }
    }
    job_file.close();
    job_file.open(QUEUE_FILE, std::ostream::out);
}

unsigned int longest_remaining_time(){
    unsigned int longest = 0;
    for( auto const& [key, job] : running_jobs ){
        auto temp = job.start_time + job.max_time;
        if( temp > longest ){
            longest = temp;
        }
    }
    return longest;
}

void start_new_processes(){
    auto free_cores = n_free_cores();
    for( std::size_t i = 0; i < job_queue.size(); ){
        auto& job = job_queue[i];
        if( job_pair.has_value() ){
            auto& next_job = job_queue[job_pair.value().first];
            if( next_job.n_cores <= free_cores ){
                start(next_job);
                free_cores -= next_job.n_cores;
                job_queue.erase(job_queue.begin() + job_pair.value().first);
                job_pair.reset();
                continue;
            }else{
                if( now() + job.max_time < job_pair.value().second ){
                    start(job);
                    free_cores -= job.n_cores;
                    job_queue.erase(job_queue.begin() + i);
                    continue;
                }
            }
        }
        else{
            if( job.n_cores <= free_cores ){
                start(job);
                free_cores -= job.n_cores;
                job_queue.erase(job_queue.begin() + i);
                continue;
            }else{
                job_pair = std::make_pair(i, now() + longest_remaining_time());
            }
        }
    }
}

void write_status(){
    std::ofstream status_out(STATUS_FILE);
    std::cerr << "Status\tPID\tStart Time\tEnd Time\tCommand\n";
    status_out << "Status\tPID\tStart Time\tEnd Time\tCommand\n";
    for( auto const& [pid, job] : running_jobs ){
        std::cerr  << "[running]" << pid << "\t" << str_time(job.start_time) << "\t" << str_time(job.start_time+job.max_time - now()) << "\t" << job.command << "\n";
        status_out << "[running]" << pid << "\t" << str_time(job.start_time) << "\t" << str_time(job.start_time+job.max_time - now()) << "\t" << job.command << "\n";
    }
    if( job_pair.has_value() ){
        auto job = job_queue[job_pair.value().first];
        auto time = job_pair.value().second;
        std::cerr << "[priority]\tn/a\t" << str_time(time - now()) << "\t" << str_time(job.start_time + job.max_time - now()) << "\t" << job.command << "\n";
        status_out << "[priority]\tn/a\t" << str_time(time - now()) << "\t" << str_time(job.start_time + job.max_time - now()) << "\t" << job.command << "\n";
    }
    for( auto const& job : job_queue ){
        std::cerr << "[queued]\tn/a\t" << "n/a" << "\t" << "n/a" << "\t" << job.command << "\n";
        status_out << "[queued]\tn/a\t" << "n/a" << "\t" << "n/a" << "\t" << job.command << "\n";
    }
}

int main(int argc, char** argv){
    std::string const CONFIG_FILE = argc > 1 ? std::string(argv[1]) : "/etc/jobq/config.txt";

    std::ifstream config_in(CONFIG_FILE);
    if( !config_in ){
        std::cerr << "Could not open config file.\n";
        return EXIT_FAILURE;
    }
    std::getline(config_in, LOG_FILE);
    std::getline(config_in, QUEUE_FILE);
    std::getline(config_in, JOB_EXEC);
    std::getline(config_in, OUTPUT_BUFFER);
    std::getline(config_in, STATUS_FILE);
    std::getline(config_in, buffer);
    MAX_N_PROCESSORS = std::stoi(buffer) == 0 ? 1 : std::stoi(buffer);

    for( unsigned int i = 0; i < MAX_N_PROCESSORS; ++i ){
        cores_in_use[i] = false;
    }

    log_file.open(LOG_FILE);
    if( !log_file ){
        std::cerr << "Could not open log_file file: "<< LOG_FILE << "\n";
        return EXIT_FAILURE;
    }

    log_file << str_time() << ": Starting JobQ server.\n Configuration file: " << CONFIG_FILE << "\n" << std::flush;

    while( true ){
        clear_processes();
        load_new_processes();
        start_new_processes();
        write_status();
        std::this_thread::sleep_for(2s);
    }
}
