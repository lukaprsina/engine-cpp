#include "common/base.h"

namespace engine
{
    std::string ToSnakeCase(const std::string &text)
    {
        std::stringstream result;
        for (const auto ch : text)
        {
            if (std::isalpha(ch))
            {
                if (std::isspace(ch))
                    result << "_";

                else
                {
                    if (std::isupper(ch))
                        result << "_";

                    result << static_cast<char>(std::tolower(ch));
                }
            }
            else
                result << ch;
        }

        return result.str();
    }
}