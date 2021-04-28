#pragma once

#include "common/vulkan_common.h"

namespace engine
{
    class Platform;
    class Instance
    {
    };

    class Window
    {
    public:
        Window(Platform &platform, uint32_t width, uint32_t height);
        virtual ~Window() = default;

        virtual void ProcessEvents() {}
        virtual VkSurfaceKHR CreateSurface(Instance &instance) = 0;
        virtual bool ShouldClose() const = 0;
        virtual void Close() = 0;

    protected:
        Platform &m_Platform;

    private:
        uint32_t m_Width;
        uint32_t m_Height;
    };
}