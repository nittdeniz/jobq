#ifndef JOBQ_JOB_HPP
#define JOBQ_JOB_HPP

#include <ctime>
#include <string>
#include <vector>

struct Job
{
    std::time_t start_time;
    unsigned int n_cores;
    unsigned int max_time;
    std::string command;
    std::string cout;
    std::string cerr;
    std::vector<unsigned short> processor_ids;
};


#endif //JOBQ_JOB_HPP
