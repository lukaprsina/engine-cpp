#include "core/options.h"

namespace engine
{
    void Options::ParseOptions(std::string &usage, std::vector<std::string> &arguments)
    {
        if (usage.size() == 0)
            return;

        m_Usage = usage;

        if (arguments.size() == 0)
            return;

        m_ParseResult = docopt::docopt(usage, arguments, false);
    }
}