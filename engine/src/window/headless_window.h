#pragma once

#include "window/window.h"

namespace engine
{
    class HeadlessWindow : public Window
    {
    public:
        HeadlessWindow(Platform &platform,
                       uint32_t width = 1280,
                       uint32_t height = 720);

        virtual ~HeadlessWindow();

        virtual VkSurfaceKHR CreateSurface(Instance &instance) override;
        virtual bool ShouldClose() const override;
        virtual void Close() override;
    };
}