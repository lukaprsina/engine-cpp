#include "common/error_common.h"

namespace engine
{
    VulkanException::VulkanException(const VkResult result, const std::string &message)
        : result{result}, std::runtime_error{message}
    {
        error_message = std::string(std::runtime_error::what());
    }

    const char *VulkanException::what() const noexcept
    {
        return error_message.c_str();
    }
}