#ifndef JOBQ_LOG_HPP
#define JOBQ_LOG_HPP

#include <ostream>
#include <string>

namespace JobQ{
    enum class Message_Type{
        STATUS,
        WARNING,
        ERROR
    };
    bool log(std::ostream& out, std::string const& msg, Message_Type type = Message_Type::STATUS);
}

#endif //JOBQ_LOG_HPP
