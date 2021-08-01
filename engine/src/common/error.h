#pragma once

#include "common/vulkan.h"
#include <stdexcept>

#if defined(__GNUC__) || defined(__GNUG__)
// GCC ENABLE/DISABLE WARNING DEFINITION
#define ENG_DISABLE_WARNINGS()                                          \
    _Pragma("GCC diagnostic push")                                      \
        _Pragma("GCC diagnostic ignored \"-Wall\"")                     \
            _Pragma("GCC diagnostic ignored \"-Wextra\"")               \
                _Pragma("GCC diagnostic ignored \"-Wunused-variable\"") \
                    _Pragma("GCC diagnostic ignored \"-Wtautological-compare\"")

#define ENG_ENABLE_WARNINGS() \
    _Pragma("GCC diagnostic pop")
#elif defined(_MSC_VER)
// MSVC ENABLE/DISABLE WARNING DEFINITION
#define ENG_DISABLE_WARNINGS() \
    __pragma(warning(push, 0))

#define ENG_ENABLE_WARNINGS() \
    __pragma(warning(pop))
#endif

namespace engine
{
    class VulkanException : public std::runtime_error
    {
    public:
        VulkanException(VkResult result, const std::string &message = "Vulkan error");

        const char *what() const noexcept override;
        VkResult result;

    private:
        std::string error_message{};
    };
}

#define VK_CHECK(x)                                                             \
    do                                                                          \
    {                                                                           \
        VkResult err = x;                                                       \
        if (err)                                                                \
        {                                                                       \
            ENG_CORE_ERROR("Detected Vulkan error: {}", engine::ToString(err)); \
            abort();                                                            \
        }                                                                       \
    } while (0)

#define ASSERT_VK_HANDLE(handle)                 \
    do                                           \
    {                                            \
        if ((handle) == VK_NULL_HANDLE)          \
        {                                        \
            ENG_CORE_CRITICAL("Handle is NULL"); \
            abort();                             \
        }                                        \
    } while (0)
