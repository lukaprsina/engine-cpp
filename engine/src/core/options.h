#pragma once

#include "docopt.h"

namespace engine
{
    class Options
    {
    public:
        Options() = default;
        ~Options() = default;

        void ParseOptions(std::string &usage, std::vector<std::string> &arguments);

    private:
        std::string m_Usage;
        std::map<std::string, docopt::value> m_ParseResult;
    };
}