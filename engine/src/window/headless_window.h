#pragma once

#include "window/window.h"

namespace engine
{
    class HeadlessWindow : public Window
    {
    public:
        HeadlessWindow(Platform &platform,
                       WindowSettings &settings);

        ~HeadlessWindow();

        VkSurfaceKHR CreateSurface(Instance &instance, PhysicalDevice &) override;
        bool ShouldClose() const override;
        void Close() override;
        void *GetNativeWindow() override { return nullptr; }

    private:
        bool m_Closed{false};
    };
}