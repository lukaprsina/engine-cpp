#pragma once

#include "window/window.h"

namespace engine
{
    class GlfwWindow : public Window
    {
    public:
        GlfwWindow(Platform &platform,
                   uint32_t width = 1280,
                   uint32_t height = 720);

        virtual ~GlfwWindow();

        virtual VkSurfaceKHR CreateSurface(Instance &instance) override;
        virtual bool ShouldClose() override;
        virtual void Close() override;
    };
}