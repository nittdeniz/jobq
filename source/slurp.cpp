#include "slurp.hpp"

#include <fmt/core.h>

#include <exception>
#include <fstream>
#include <sstream>

namespace JobQ{
    std::string slurp(const std::string &file)
    {
        std::ifstream in(file);
        if( !in ){
            throw std::ios_base::failure(fmt::format("Could not open file {}.", file));
        }
        std::stringstream buffer;
        buffer << in.rdbuf();
        return buffer.str();
    }
}