#include "file_lock.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

namespace JobQ{
    bool is_locked(const std::string &file)
    {
        return std::filesystem::exists(file);
    }

    bool lock_file(const std::string &file)
    {
        using namespace std::chrono_literals;
        while( is_locked(file) ){
            std::this_thread::sleep_for(50ms);
        }
        std::ofstream lock_file(file);
        return std::filesystem::exists(file);
    }

    bool unlock_file(const std::string &file)
    {
        return std::remove(file.c_str()) == 0;
    }
}