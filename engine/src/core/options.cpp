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

    bool Options::Contains(const std::string &argument) const
    {
        if (m_ParseResult.count(argument) != 0)
        {
            if (const auto &result = m_ParseResult.at(argument))
            {
                if (result.isBool())
                    return result.asBool();

                return true;
            }
        }

        return false;
    }

    int32_t Options::GetInt(const std::string &argument) const
    {
        if (Contains(argument))
        {
            auto result = m_ParseResult.at(argument);
            if (result.isString())
                return std::stoi(result.asString());

            else if (result.isLong())
                return static_cast<int32_t>(result.asLong());

            throw std::runtime_error("Argument option is not int type");
        }

        throw std::runtime_error("Couldn't find argument option");
    }

    const std::string Options::GetString(const std::string &argument) const
    {
        if (Contains(argument))
        {
            auto result = m_ParseResult.at(argument);
            if (result.isString())
                return result.asString();

            throw std::runtime_error("Argument option is not int type");
        }

        throw std::runtime_error("Couldn't find argument option");
    }
}