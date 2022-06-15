#ifndef JOBQ_CONFIG_HPP
#define JOBQ_CONFIG_HPP

#include <string>

extern std::string const& LOG_FILE;
extern std::string const& QUEUE_FILE;
extern std::string const& SYSTEM_BUFFER;
extern std::string const& STATUS_FILE;
extern std::string const& QUEUE_LOCK_FILE;
extern std::string const& COMMAND_LOCK_FILE;
extern std::string const& JOB_EXEC;
extern std::string const& COMMAND_FILE;
extern unsigned int const MAX_N_PROCESSORS;

#endif //JOBQ_CONFIG_HPP
