#include "log.hpp"
#include "time.hpp"

#include <iostream>

namespace JobQ{
    bool log(std::ostream& out, const std::string &msg, Message_Type type)
    {
        switch( type ){
            case Message_Type::STATUS:
                out << "###[" << str_time() << "]: " << msg << "\n";
                break;
            case Message_Type::WARNING:
                out << "@@@[" << str_time() << "]: " << msg << "\n";
                break;
            case Message_Type::ERROR:
                out << "!!![" << str_time() << "]: " << msg << "\n" << std::flush;
                break;
            default:
                std::cerr << "Server log received unknown message type.";
        }
        return out.good();
    }
}