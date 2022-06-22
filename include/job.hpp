#ifndef JOBQ_JOB_HPP
#define JOBQ_JOB_HPP

#include "time.hpp"

#include <ctime>
#include <string>
#include <vector>

namespace JobQ
{
    struct Job
    {
        Time_Point start_time;
        Time_Point end_time;
        unsigned int n_cores;
        Seconds max_runtime;
        std::string command;
        std::string cout;
        std::string cerr;
        std::string working_directory;
        std::vector<unsigned int> processor_ids;
    };
}

#endif //JOBQ_JOB_HPP
