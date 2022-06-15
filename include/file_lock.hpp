#ifndef JOBQ_FILE_LOCK_HPP
#define JOBQ_FILE_LOCK_HPP

#include <string>

namespace JobQ{
    bool lock_file(std::string const& file);
    bool is_locked(std::string const& file);
    bool unlock_file(std::string const& file);
}

#endif //JOBQ_FILE_LOCK_HPP
