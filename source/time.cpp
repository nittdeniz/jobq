#include "time.hpp"

#include <iomanip>
#include <sstream>

namespace JobQ{
    std::string str_time()
    {
        return str_time(now());
    }

    std::string str_time(long long int delta_t)
    {
        return str_time(now() + Seconds(delta_t));
    }

    std::string str_time(const Time_Point &tp)
    {
        auto converted = std::chrono::system_clock::to_time_t(tp);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&converted), "[%Y-%m-%d %X]");
        return ss.str();
    }
}


