#ifndef JOBQ_QUEUE_HPP
#define JOBQ_QUEUE_HPP

#include "job.hpp"

#include <map>
#include <vector>

namespace JobQ
{
    class Queue
    {
        using PID = unsigned int;
        int TERMINATE_SERVER = -1;
        int WRITE_STATUS = -2;
    private:
        std::map<PID, Job> running_jobs;
        std::vector<Job> _queue;
        std::string _queue_file;
        std::ostream& _log_stream;
        unsigned int _n_max_cores;

        std::optional<std::pair<std::size_t, Time_Point>> _priority_job;

        Time_Point _latest_ending_time() const;

        void start_process(Job& job);
        void check_status();

        bool is_running(PID pid);
        void free_cores(Job const& job);

        std::vector<unsigned int> allocate_cores(unsigned int n);
        std::map<unsigned int, bool> _cores_in_use;

        unsigned int _n_free_cores() const;

        void clear_processes();
        void load_new_processes();
        void start_processes();
    public:
        Queue(unsigned int n_cores, std::ostream& log_stream, std::string const& queue_file);
        void process();
    };
}
#endif //JOBQ_QUEUE_HPP
