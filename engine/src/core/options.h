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
        bool Contains(const std::string &argument) const;

        int32_t GetInt(const std::string &argument) const;

    private:
        std::string m_Usage;
        std::map<std::string, docopt::value> m_ParseResult;
    };
}