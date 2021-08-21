#pragma once

#include "common/vulkan.h"

namespace engine
{
    const std::string ToString(VkResult format);
    const std::string ToString(VkFormat format);
    const std::string ToString(VkPresentModeKHR present_mode);
    const std::string ToString(VkResult result);
    const std::string ToString(VkPhysicalDeviceType type);
    const std::string ToString(VkSurfaceTransformFlagBitsKHR transform_flag);
    const std::string ToString(VkSurfaceFormatKHR surface_format);
    const std::string ToString(VkCompositeAlphaFlagBitsKHR composite_alpha);
    const std::string ToString(VkImageUsageFlagBits image_usage);
    const std::string ToString(VkExtent2D format);
    const std::string ToString(VkSampleCountFlagBits flags);
    const std::string ToString(VkImageTiling tiling);
    const std::string ToString(VkImageType type);
    const std::string ToString(VkBlendFactor blend);
    const std::string ToString(VkVertexInputRate rate);
    const std::string ToString(VkPrimitiveTopology topology);
    const std::string ToString(VkFrontFace face);
    const std::string ToString(VkPolygonMode mode);
    const std::string ToString(VkCompareOp operation);
    const std::string ToString(VkStencilOp operation);
    const std::string ToString(VkLogicOp operation);
    const std::string ToString(VkBlendOp operation);

    std::vector<std::string> Split(const std::string &input, char delim);

    template <typename T>
    inline const std::string ToString(uint32_t bitmask, const std::map<T, const char *> string_map)
    {
        std::stringstream result;
        bool append = false;
        for (const auto &s : string_map)
        {
            if (bitmask & s.first)
            {
                if (append)
                {
                    result << " / ";
                }
                result << s.second;
                append = true;
            }
        }
        return result.str();
    }
}