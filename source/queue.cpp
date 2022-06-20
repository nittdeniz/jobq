#include "commands.hpp"
#include "config.hpp"
#include "file_lock.hpp"
#include "log.hpp"
#include "queue.hpp"
#include "slurp.hpp"
#include "system_call.hpp"
#include "time.hpp"

#include <fmt/core.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <thread>

namespace JobQ{
    Queue::Queue(unsigned int n_cores, std::ostream &log_stream, const std::string &queue_file)
    : _n_max_cores(n_cores)
    , _log_stream(log_stream)
    , _queue_file(queue_file)
    , _running(true)
    {
        JobQ::log(_log_stream, "Starting JobQ server.");
        clean_files();
        for( unsigned int i = 0; i < _n_max_cores; ++i ){
            _cores_in_use[i] = false;
        }
    }

    void Queue::clean_files(){
        if( std::filesystem::exists(QUEUE_LOCK_FILE.c_str()) && !std::remove(QUEUE_LOCK_FILE.c_str()) ){
            JobQ::log(_log_stream, fmt::format("Could not remove queue lock file."));
        }
        if( std::filesystem::exists(COMMAND_LOCK_FILE.c_str()) && !std::remove(COMMAND_LOCK_FILE.c_str()) ){
            JobQ::log(_log_stream, fmt::format("Could not remove command lock file."));
        }
        if( !std::filesystem::exists(QUEUE_FILE) ){
            if( !std::ofstream(QUEUE_FILE) ){
                JobQ::log(_log_stream, fmt::format("Could not create file {}", QUEUE_FILE));
            }
        }
        if( !std::filesystem::exists(STATUS_FILE) ){
            if( !std::ofstream(STATUS_FILE) ){
                JobQ::log(_log_stream, fmt::format("Could not create file {}", STATUS_FILE));
            }
        }
        if( !std::filesystem::exists(COMMAND_FILE) ){
            if( !std::ofstream(COMMAND_FILE) ){
                JobQ::log(_log_stream, fmt::format("Could not create file {}", COMMAND_FILE));
            }
        }
    }

    bool Queue::running() const{
    	return _running;
    }

    bool Queue::is_process_running(PID pid){
        std::string result = system_call(fmt::format("ps -p {}", pid));
        std::stringstream buffer_stream(result);
        std::string buffer;
        while( std::getline(buffer_stream, buffer) ){
            std::string reg_str = "[ \t]*" + std::to_string(pid) + ".*";
            if( std::regex_match(buffer, std::regex(reg_str)) ){
                return true;
            }
        }
        return false;
    }

    void Queue::free_cores(Job const& job){
        for( auto const& i : job.processor_ids ){
            _cores_in_use[i] = false;
        }
    }

    void Queue::clear_processes()
    {
        using namespace std::chrono_literals;
        for( auto it = running_jobs.begin(); it != running_jobs.end(); ){
            auto pid = it->first;
            auto job = it->second;
            if( is_process_running(pid) && (now() > job.end_time ) ){
                system_call(fmt::format("kill -9 {}", pid));
                log(_log_stream, fmt::format("Job {} terminated (exceeded time limit).", pid));
            }
            std::this_thread::sleep_for(50ms);
            if( !is_process_running(pid) ){
                free_cores(job);
                it = running_jobs.erase(it);
                log(_log_stream, fmt::format("Job {} ended.", pid));
            }
            else{
                it++;
            }
        }
    }

    void Queue::write_status(std::ostream& out){
        out << "Status\t\tPID\tCores\tStart Time\t\tEnd Time\t\tCommand\n";
        for( auto const& [pid, job] : running_jobs ){
            out << "[running]\t" << pid << "\t" << job.processor_ids.size() << "\t" << str_time(job.start_time) << "\t" << str_time(job.end_time) << "\t" << job.command << "\n";
        }
        if( _priority_job.has_value() ){
            auto job = _queue[_priority_job.value().first];
            auto time = _priority_job.value().second;
            out << "[priority]\tn/a\t" << job.n_cores << "\t" <<str_time(time) << "\t" << str_time(job.end_time) << "\t" << job.command << "\n";
        }
        for( auto const& job : _queue ){
            out << "[queued]\tn/a\t" << job.n_cores << "\t" << "n/a" << "\t" << "n/a" << "\t" << job.command << "\n";
        }
    }

    std::vector<unsigned int> Queue::allocate_cores(unsigned int n){
        std::vector<unsigned int> cores;
        int i = 0;
        for( auto& [cid, in_use] : _cores_in_use ){
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

    void Queue::load_new_processes()
    {
        using namespace std::chrono_literals;
        int i{0};
        while( is_locked(QUEUE_LOCK_FILE) ){
            std::this_thread::sleep_for(50ms);
            if( ++i > 100 ){
                log(_log_stream, "Command file lock has not been released for 100 iterations. Deadlock?", Message_Type::ERROR);
                exit(EXIT_FAILURE);
            }
        }
        lock_file(QUEUE_LOCK_FILE);
        std::stringstream queue_stream;
        try{
            queue_stream << slurp(QUEUE_FILE);
            std::ignore = std::ofstream(QUEUE_FILE);
            unlock_file(QUEUE_LOCK_FILE);
        }
        catch( std::exception& e){
            log(_log_stream, std::string(e.what()), Message_Type::ERROR);
            unlock_file(QUEUE_LOCK_FILE);
            return;
        }
        std::string job_line;
        while( std::getline(queue_stream, job_line) ){
            try
            {
                std::stringstream parser(job_line);
                Job job;
                parser >> job.n_cores;
                long long int temp;
                parser >> temp;
                job.max_runtime = Seconds(temp);
                parser >> job.cout;
                parser >> job.cerr;
                std::string cmd(std::istreambuf_iterator<char>(parser.rdbuf()), {});
                job.command = cmd.substr(1);
                _queue.push_back(job);
                log(_log_stream, fmt::format("Loaded job `{}`", job_line));
            }
            catch( std::exception& e){
                log(_log_stream, fmt::format("Could not load job: `{}`", job_line));
            }
        }
    }

    unsigned int Queue::_n_free_cores() const{
        return std::count_if(_cores_in_use.cbegin(), _cores_in_use.cend(), [](std::pair<unsigned int, bool> const& pair){
            return !pair.second;
        });
    }

    void Queue::start_processes()
    {
        auto free_cores = _n_free_cores();
        for( auto it = _queue.begin(); it != _queue.end(); ){
            auto& job = *it;
            if( _priority_job.has_value() ){
                auto& next_job = _queue[_priority_job.value().first];
                if( next_job.n_cores <= free_cores ){
                    start_process(next_job);
                    free_cores -= next_job.n_cores;
                    it = _queue.erase(_queue.begin() + _priority_job.value().first);
                    _priority_job.reset();
                    continue;
                }else if( now() + job.max_runtime < _priority_job.value().second && job.n_cores <= free_cores ){
                    start_process(job);
                    free_cores -= job.n_cores;
                    it = _queue.erase(it);
                }else{
                    it = std::next(it);
                }
            }
            else{
                if( job.n_cores <= free_cores ){
                    start_process(job);
                    free_cores -= job.n_cores;
                    it = _queue.erase(it);
                }else{
                    _priority_job = std::make_pair(_queue.begin() - it, _latest_ending_time());
                    it = std::next(it);
                }
            }
        }
    }

    Time_Point JobQ::Queue::_latest_ending_time() const
    {
        Time_Point latest(now());
        for( auto const& [key, job] : running_jobs ){
            if( job.end_time > latest ){
                latest = job.end_time;
            }
        }
        return latest;
    }

    void Queue::start_process(Job &job)
    {
        job.start_time = now();
        job.end_time = now() + job.max_runtime;
        job.processor_ids = allocate_cores(job.n_cores);
        std::stringstream cmd;
        cmd << JOB_EXEC << " " << job.cout << " " << job.cerr << " " << SYSTEM_BUFFER << " 'taskset -ac ";
        for( std::size_t i = 0; i < job.processor_ids.size(); ++i ){
            if( i > 0 ){
                cmd << ",";
            }
            cmd << job.processor_ids[i];
        }
        cmd << " " << job.command << "'";

        std::string result = system_call(cmd.str());
        unsigned int pid = std::stoul(result);
        if( running_jobs.contains(pid) ){
            log(_log_stream, fmt::format("Starting job with same id `{}`. Stopping server.", pid), Message_Type::ERROR);
            exit(EXIT_FAILURE);
        }
        running_jobs[pid] = job;
        log(_log_stream, fmt::format("Started process `{}`", pid));
        log(_log_stream, fmt::format("Cores: `{}`", job.n_cores));
        log(_log_stream, fmt::format("Automatic Termination on: {}.", str_time(job.end_time)));
    }

    void Queue::check_status(){
        using namespace std::chrono_literals;
        int i{0};
        while( is_locked(COMMAND_LOCK_FILE) ){
            std::this_thread::sleep_for(50ms);
            if( ++i > 100 ){
                log(_log_stream, "Command file lock has not been released for 100 iterations. Deadlock?", Message_Type::ERROR);
                exit(EXIT_FAILURE);
            }
        }
        lock_file(COMMAND_LOCK_FILE);
        std::stringstream commands(slurp(COMMAND_FILE));
        std::ignore = std::ofstream(COMMAND_FILE, std::ofstream::trunc);
        unlock_file(COMMAND_LOCK_FILE);
        std::cerr << "`" << commands.str() << "`";
        if( commands.str() == CMD_SERVER_STOP ){
        	_running = false;
        }
        else if( commands.str() == CMD_SERVER_STATUS ){
            log(_log_stream, "Server status command issued.");
            auto status_out = std::ofstream(STATUS_FILE);
            if( !status_out ){
                std::cerr << "Can not open status file.\n";
                log(_log_stream, "Can not open status file.", Message_Type::ERROR);
            }else{
                write_status(std::cerr);
                write_status(status_out);
            }
        }else{
            std::string cmd;
            int pid;
            commands >> cmd;
            commands >> pid;
            if( cmd == CMD_STOP ){
                std::ignore = system_call(fmt::format("kill -15 {}", pid));
                log(_log_stream, fmt::format("Job {} stopped ({}).", pid, CMD_STOP));
            }
        }
    }

    void Queue::process()
    {
        std::cerr << "check status\n";
        check_status();
        std::cerr << "check clear processes\n";
        clear_processes();
        std::cerr << "check load new processes\n";
        load_new_processes();
        std::cerr << "check start processes\n";
        start_processes();
    }

}