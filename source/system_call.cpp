#include "config.hpp"
#include "file_lock.hpp"
#include "slurp.hpp"
#include "system_call.hpp"

#include <chrono>
#include <fstream>
#include <sstream>
#include <thread>
#include <tuple>

namespace JobQ{
    std::string system_call(const std::string &cmd)
    {
        using namespace std::chrono_literals;
        auto const redirected_command = cmd + std::string(" > ") + SYSTEM_BUFFER;
        while( is_locked(SYSTEM_BUFFER) ){
            std::this_thread::sleep_for(50ms);
        }
        std::ignore = std::system(redirected_command.c_str());
        std::this_thread::sleep_for(50ms);
        return slurp(SYSTEM_BUFFER);
    }
}