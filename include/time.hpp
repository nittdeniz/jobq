#ifndef JOBQ_TIME_HPP
#define JOBQ_TIME_HPP

#include <chrono>
#include <string>

namespace JobQ{
    using Clock = std::chrono::system_clock;
    using Time_Point = std::chrono::time_point<Clock>;
    using Seconds = std::chrono::duration<long long int, std::ratio<1>>;
    auto now = std::chrono::system_clock::now;

    std::string str_time();
    std::string str_time(long long int delta_t);
    std::string str_time(Time_Point const& tp);

}

#endif //JOBQ_TIME_HPP
