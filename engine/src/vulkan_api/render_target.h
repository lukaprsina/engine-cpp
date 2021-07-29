#pragma once

#include "vulkan_api/core/image.h"
#include "vulkan_api/core/image_view.h"

namespace engine
{
    class Device;
    class RenderTarget
    {
    public:
        RenderTarget(std::vector<core::Image> &&images);
        ~RenderTarget();

        using CreateFunc = std::function<std::unique_ptr<RenderTarget>(core::Image &&)>;

        static const CreateFunc s_DefaultCreateFunction;

    private:
        Device &m_Device;
        VkExtent2D m_Extent;
        std::vector<core::Image> m_Images;
        std::vector<core::ImageView> m_ImageViews;
    };
}
