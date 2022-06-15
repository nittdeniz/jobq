#include "config.hpp"

std::string const& LOG_FILE = "/etc/jobq/log.txt";
std::string const& QUEUE_FILE = "/etc/jobq/.queue";
std::string const& SYSTEM_BUFFER = "/etc/jobq/.system_buffer";
std::string const& STATUS_FILE = "/etc/jobq/.status";
std::string const& QUEUE_LOCK_FILE = "/etc/jobq/.queue_lock";
std::string const& COMMAND_LOCK_FILE = "/etc/jobq/.cmd_lock";
std::string const& JOB_EXEC = "/etc/jobq/.jobexec";
std::string const& COMMAND_FILE = "/etc/jobq/.commands";
unsigned int const MAX_N_PROCESSORS = 20;
