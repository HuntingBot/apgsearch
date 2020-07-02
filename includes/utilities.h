#pragma once

#include <sstream>
#include <string>
#include <utility>

template<class... Ts>
std::string strConcat(const Ts&... ts)
{
    std::ostringstream oss;
    int dummy[] = {
        ((oss << ts), 0)...
    };
    (void)dummy;
    return std::move(oss).str();
}
